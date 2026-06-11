package org.cagnulen.qdomyoszwift

import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.usb.UsbConstants
import android.hardware.usb.UsbDevice
import android.hardware.usb.UsbDeviceConnection
import android.hardware.usb.UsbEndpoint
import android.hardware.usb.UsbInterface
import android.hardware.usb.UsbManager
import android.os.Build
import androidx.core.content.ContextCompat
import com.nettarion.hyperborea.core.AppLogger
import com.nettarion.hyperborea.core.model.DeviceCommand
import com.nettarion.hyperborea.core.model.ExerciseData
import com.nettarion.hyperborea.hardware.fitpro.session.DeviceDatabase
import com.nettarion.hyperborea.hardware.fitpro.session.ExerciseDataAccumulator
import com.nettarion.hyperborea.hardware.fitpro.session.FitProSession
import com.nettarion.hyperborea.hardware.fitpro.session.SessionState
import com.nettarion.hyperborea.hardware.fitpro.transport.UsbHidTransport
import com.nettarion.hyperborea.hardware.fitpro.v1.V1Session
import com.nettarion.hyperborea.hardware.fitpro.v2.V2Session
import java.util.concurrent.ConcurrentLinkedQueue
import kotlin.math.roundToInt
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume

/**
 * Drop-in replacement for the gRPC-backed GrpcTreadmillService. Exposes the exact same static JNI
 * surface the C++ NordicTrack devices call, but talks to the console **directly over USB-HID**
 * using the FitPro protocol (vendored from Hyperborea, MIT) instead of the iFit app's localhost
 * gRPC server.
 *
 * Setter semantics are kept identical to GrpcTreadmillService: adjustSpeed/Incline/Resistance take
 * a *delta*, which we fold onto the last cached reading to produce the absolute target FitPro
 * commands expect.
 */
object FitProDeviceService {

    private const val TAG = "FitProDeviceService"

    private const val FITPRO_VENDOR_ID = 0x213C // 8508
    private const val PRODUCT_ID_V1 = 2
    private const val PRODUCT_ID_V2 = 3
    private const val PRODUCT_ID_V2_FTDI = 4
    private val PRODUCT_IDS = setOf(PRODUCT_ID_V1, PRODUCT_ID_V2, PRODUCT_ID_V2_FTDI)

    private const val ACTION_USB_PERMISSION = "org.cagnulen.qdomyoszwift.FITPRO_USB_PERMISSION"

    // QZ workout states, mirroring the gRPC WorkoutState the C++ layer expects.
    private const val STATE_IDLE = 1
    private const val STATE_RUNNING = 3
    private const val STATE_PAUSED = 4
    private const val STATE_RESULTS = 5

    private val logger: AppLogger = object : AppLogger {
        override fun d(tag: String, message: String) { QLog.d(tag, message) }
        override fun i(tag: String, message: String) { QLog.i(tag, message) }
        override fun w(tag: String, message: String) { QLog.w(tag, message) }
        override fun e(tag: String, message: String, throwable: Throwable?) {
            if (throwable != null) QLog.e(tag, message, throwable) else QLog.e(tag, message)
        }
    }

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    @Volatile private var staticContext: Context? = null
    @Volatile private var session: FitProSession? = null
    @Volatile private var protocolIsV1 = true
    @Volatile private var connected = false
    @Volatile private var latest: ExerciseData = ExerciseData.ZERO
    private var collectJob: Job? = null

    // Workout-state machine, fed from the polled WORKOUT_MODE field.
    @Volatile private var currentWorkoutState = STATE_IDLE
    private val stateChangeQueue = ConcurrentLinkedQueue<IntArray>()

    // ---------------------------------------------------------------------------------------------
    // Lifecycle (JNI surface)
    // ---------------------------------------------------------------------------------------------

    @JvmStatic
    fun setContext(context: Context) {
        staticContext = context.applicationContext
    }

    @JvmStatic
    fun initialize() = initialize("")

    /** [host] is ignored — kept only for signature parity with GrpcTreadmillService. */
    @JvmStatic
    fun initialize(host: String) {
        if (connected || session != null) {
            QLog.i(TAG, "initialize() ignored — session already active")
            return
        }
        scope.launch { connect() }
    }

    private suspend fun connect() {
        val ctx = staticContext
        if (ctx == null) {
            QLog.e(TAG, "Context not set. Call setContext() first.")
            return
        }
        try {
            val opened = openUsb(ctx)
            if (opened == null) {
                QLog.e(TAG, "FitPro USB device not found / permission denied")
                return
            }
            val (transport, productId) = opened
            val info = DeviceDatabase.fromProductId(productId) ?: DeviceDatabase.fallback()
            val accumulator = ExerciseDataAccumulator()
            protocolIsV1 = productId == PRODUCT_ID_V1
            val newSession: FitProSession = if (protocolIsV1) {
                V1Session(transport, logger, scope, info, accumulator)
            } else {
                V2Session(transport, logger, scope, info, accumulator)
            }

            newSession.start()
            val st = newSession.sessionState.value
            if (st !is SessionState.Streaming) {
                val msg = (st as? SessionState.Error)?.message ?: "handshake did not complete"
                QLog.e(TAG, "FitPro session failed to start: $msg")
                return
            }

            session = newSession
            connected = true
            QLog.i(TAG, "FitPro session streaming (protocol=${if (protocolIsV1) "V1" else "V2"})")

            collectJob = scope.launch {
                newSession.exerciseData.collect { data ->
                    if (data != null) {
                        latest = data
                        updateWorkoutState(data.workoutMode)
                    }
                }
            }
        } catch (e: Exception) {
            QLog.e(TAG, "FitPro connect failed", e)
            connected = false
        }
    }

    @JvmStatic
    fun shutdown() {
        val s = session
        session = null
        connected = false
        collectJob?.cancel()
        collectJob = null
        scope.launch {
            try { s?.stop() } catch (e: Exception) { QLog.w(TAG, "stop error: ${e.message}") }
        }
        stateChangeQueue.clear()
        currentWorkoutState = STATE_IDLE
        latest = ExerciseData.ZERO
    }

    // The FitPro session polls continuously once started; these are no-ops kept for API parity.
    @JvmStatic fun startMetricsUpdates() { QLog.i(TAG, "startMetricsUpdates (no-op: session polls)") }
    @JvmStatic fun stopMetricsUpdates() { QLog.i(TAG, "stopMetricsUpdates (no-op)") }

    // ---------------------------------------------------------------------------------------------
    // Metric getters (JNI surface)
    // ---------------------------------------------------------------------------------------------

    @JvmStatic fun getCurrentSpeed(): Double = (latest.speed ?: 0f).toDouble()
    @JvmStatic fun getCurrentIncline(): Double = (latest.incline ?: 0f).toDouble()
    @JvmStatic fun getCurrentWatts(): Double = (latest.power ?: 0).toDouble()
    @JvmStatic fun getCurrentCadence(): Double = (latest.cadence ?: 0).toDouble()
    @JvmStatic fun getCurrentRpm(): Double = (latest.cadence ?: 0).toDouble()
    @JvmStatic fun getCurrentResistance(): Double = (latest.resistance ?: 0).toDouble()
    @JvmStatic fun getCurrentHeartRate(): Double = (latest.heartRate ?: 0).toDouble()
    @JvmStatic fun getCurrentFanSpeed(): Int = 0
    @JvmStatic fun getCurrentStrokesCount(): Double = (latest.strokeCount ?: 0).toDouble()
    @JvmStatic fun getCurrentStrokesLength(): Double = 0.0
    @JvmStatic fun getCurrentPaceSeconds(): Int = latest.splitTime ?: 0
    @JvmStatic fun getCurrentLast500mPaceSeconds(): Int = latest.avgSplitTime ?: 0

    // ---------------------------------------------------------------------------------------------
    // Setters (JNI surface) — delta semantics, mapped to absolute FitPro commands
    // ---------------------------------------------------------------------------------------------

    @JvmStatic
    fun adjustSpeed(delta: Double) {
        val target = ((latest.speed ?: 0f) + delta).toFloat().coerceAtLeast(0f)
        send(DeviceCommand.SetTargetSpeed(target))
    }

    @JvmStatic
    fun adjustIncline(delta: Double) {
        val target = ((latest.incline ?: 0f) + delta).toFloat()
        send(DeviceCommand.SetIncline(target))
    }

    @JvmStatic
    fun adjustResistance(delta: Double) {
        val target = ((latest.resistance ?: 0) + delta).roundToInt().coerceAtLeast(0)
        send(DeviceCommand.SetResistance(target))
    }

    @JvmStatic
    fun setWatts(watts: Double) {
        if (watts <= 0) send(DeviceCommand.SetErgMode(false))
        else send(DeviceCommand.SetTargetPower(watts.roundToInt()))
    }

    @JvmStatic
    fun disableConstantWatts() {
        send(DeviceCommand.SetErgMode(false))
    }

    @JvmStatic
    fun setFanSpeed(level: Int) {
        send(DeviceCommand.SetFanSpeed(level))
    }

    // ---------------------------------------------------------------------------------------------
    // Workout control (JNI surface)
    // ---------------------------------------------------------------------------------------------

    @JvmStatic fun startWorkout() { send(DeviceCommand.ResumeWorkout) }
    @JvmStatic fun resumeWorkout() { send(DeviceCommand.ResumeWorkout) }
    @JvmStatic fun pauseWorkout() { send(DeviceCommand.PauseWorkout) }
    @JvmStatic fun stopWorkout() { send(DeviceCommand.PauseWorkout) }

    @JvmStatic fun startWorkoutStateMonitoring() { /* handled by the exerciseData collector */ }
    @JvmStatic fun stopWorkoutStateMonitoring() { /* no-op */ }

    @JvmStatic fun getWorkoutState(): Int = currentWorkoutState

    /** Returns the next pending {from,to} transition, or null when the queue is empty. */
    @JvmStatic fun getNextWorkoutStateChange(): IntArray? = stateChangeQueue.poll()

    // ---------------------------------------------------------------------------------------------
    // Internals
    // ---------------------------------------------------------------------------------------------

    private fun send(command: DeviceCommand) {
        val s = session ?: return
        scope.launch {
            try { s.writeFeature(command) }
            catch (e: Exception) { QLog.w(TAG, "writeFeature($command) failed: ${e.message}") }
        }
    }

    private fun updateWorkoutState(rawMode: Int?) {
        val mapped = toQzState(rawMode)
        if (mapped != currentWorkoutState) {
            val from = currentWorkoutState
            currentWorkoutState = mapped
            stateChangeQueue.add(intArrayOf(from, mapped))
            QLog.i(TAG, "Workout state changed: $from -> $mapped (raw=$rawMode)")
        }
    }

    /** Maps the raw FitPro WORKOUT_MODE to the gRPC-style state the C++ layer understands. */
    private fun toQzState(rawMode: Int?): Int {
        if (rawMode == null) return currentWorkoutState
        return if (protocolIsV1) {
            // V1 WorkoutMode: IDLE=1 RUNNING=2 PAUSE=3 DMK=8 WARM_UP=10 COOL_DOWN=11
            when (rawMode) {
                2 -> STATE_RUNNING
                3 -> STATE_PAUSED
                11 -> STATE_RESULTS
                else -> STATE_IDLE
            }
        } else {
            // V2 WorkoutMode: NONE=0 READY=1 WARM_UP=2 RUNNING=3 COOL_DOWN=4 PAUSED=5 RESULTS=6
            when (rawMode) {
                3 -> STATE_RUNNING
                5 -> STATE_PAUSED
                6 -> STATE_RESULTS
                else -> STATE_IDLE
            }
        }
    }

    private suspend fun openUsb(ctx: Context): Pair<UsbHidTransport, Int>? {
        val usbManager = ctx.getSystemService(Context.USB_SERVICE) as? UsbManager ?: return null

        val device: UsbDevice = usbManager.deviceList.values.firstOrNull {
            it.vendorId == FITPRO_VENDOR_ID && it.productId in PRODUCT_IDS
        } ?: run {
            QLog.e(TAG, "No FitPro USB device (vendor=$FITPRO_VENDOR_ID) attached")
            return null
        }

        if (!usbManager.hasPermission(device)) {
            QLog.i(TAG, "Requesting USB permission for ${device.deviceName}")
            if (!requestUsbPermission(ctx, usbManager, device)) {
                QLog.e(TAG, "USB permission denied for ${device.deviceName}")
                return null
            }
        }

        val usbInterface: UsbInterface = device.getInterface(0)
        var inEndpoint: UsbEndpoint? = null
        var outEndpoint: UsbEndpoint? = null
        for (i in 0 until usbInterface.endpointCount) {
            val ep = usbInterface.getEndpoint(i)
            if (ep.direction == UsbConstants.USB_DIR_IN) inEndpoint = ep else outEndpoint = ep
        }
        if (inEndpoint == null || outEndpoint == null) {
            QLog.e(TAG, "FitPro device missing bulk IN/OUT endpoints")
            return null
        }

        val connection: UsbDeviceConnection = usbManager.openDevice(device) ?: run {
            QLog.e(TAG, "Failed to open FitPro USB device")
            return null
        }

        val transport = UsbHidTransport(connection, usbInterface, inEndpoint, outEndpoint, logger)
        transport.open()
        return Pair(transport, device.productId)
    }

    private suspend fun requestUsbPermission(
        ctx: Context,
        usbManager: UsbManager,
        device: UsbDevice,
    ): Boolean = suspendCancellableCoroutine { cont ->
        val receiver = object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                if (intent.action != ACTION_USB_PERMISSION) return
                try { ctx.unregisterReceiver(this) } catch (_: Exception) {}
                val granted = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)
                if (cont.isActive) cont.resume(granted)
            }
        }
        val flags = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) PendingIntent.FLAG_IMMUTABLE else 0
        val permissionIntent =
            PendingIntent.getBroadcast(ctx, 0, Intent(ACTION_USB_PERMISSION).setPackage(ctx.packageName), flags)
        ContextCompat.registerReceiver(
            ctx, receiver, IntentFilter(ACTION_USB_PERMISSION), ContextCompat.RECEIVER_EXPORTED,
        )
        cont.invokeOnCancellation { try { ctx.unregisterReceiver(receiver) } catch (_: Exception) {} }
        usbManager.requestPermission(device, permissionIntent)
    }
}

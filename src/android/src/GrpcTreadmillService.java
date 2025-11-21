package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.security.KeyFactory;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.PKCS8EncodedKeySpec;
import android.util.Base64;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;

import io.grpc.ManagedChannel;
import io.grpc.Metadata;
import io.grpc.okhttp.OkHttpChannelBuilder;
import io.grpc.stub.MetadataUtils;

import com.ifit.glassos.util.Empty;
import com.ifit.glassos.workout.SpeedMetric;
import com.ifit.glassos.workout.SpeedServiceGrpc;
import com.ifit.glassos.workout.SpeedRequest;
import com.ifit.glassos.workout.InclineMetric;
import com.ifit.glassos.workout.InclineServiceGrpc;
import com.ifit.glassos.workout.InclineRequest;
import com.ifit.glassos.workout.WattsMetric;
import com.ifit.glassos.workout.WattsServiceGrpc;
import com.ifit.glassos.console.constantwatts.ConstantWattsMessage;
import com.ifit.glassos.console.constantwatts.ConstantWattsServiceGrpc;
import com.ifit.glassos.workout.ResistanceMetric;
import com.ifit.glassos.workout.ResistanceServiceGrpc;
import com.ifit.glassos.workout.ResistanceRequest;
import com.ifit.glassos.workout.CadenceMetric;
import com.ifit.glassos.workout.CadenceServiceGrpc;
import com.ifit.glassos.workout.RpmMetric;
import com.ifit.glassos.workout.RpmServiceGrpc;
import com.ifit.glassos.settings.FanState;
import com.ifit.glassos.settings.FanStateMessage;
import com.ifit.glassos.settings.FanStateServiceGrpc;
import com.ifit.glassos.workout.WorkoutServiceGrpc;
import com.ifit.glassos.workout.WorkoutResult;
import com.ifit.glassos.workout.StartWorkoutResponse;
import com.ifit.glassos.workout.WorkoutState;
import com.ifit.glassos.workout.WorkoutStateMessage;
import com.ifit.glassos.workout.StrokesMetric;
import com.ifit.glassos.workout.StrokesServiceGrpc;
import com.ifit.glassos.workout.PaceMetric;
import com.ifit.glassos.workout.PaceServiceGrpc;

import org.cagnulen.qdomyoszwift.QLog;

public class GrpcTreadmillService {

    private static final String TAG = "GrpcTreadmillService";
    
    // Singleton instance for static access
    private static GrpcTreadmillService instance = null;
    private static Context staticContext = null;
    private static String serverHost = "localhost";
    private static final int SERVER_PORT = 54321;
    private static final int UPDATE_INTERVAL_MS = 500;

    // Threading components
    private Handler mainHandler;
    private ExecutorService executorService;
    private ExecutorService workoutStateExecutor;
    private Runnable metricsUpdateRunnable;

    // gRPC components
    private ManagedChannel channel;
    private SpeedServiceGrpc.SpeedServiceBlockingStub speedStub;
    private InclineServiceGrpc.InclineServiceBlockingStub inclineStub;
    private WattsServiceGrpc.WattsServiceBlockingStub wattsStub;
    private ConstantWattsServiceGrpc.ConstantWattsServiceBlockingStub constantWattsStub;
    private ResistanceServiceGrpc.ResistanceServiceBlockingStub resistanceStub;
    private CadenceServiceGrpc.CadenceServiceBlockingStub cadenceStub;
    private RpmServiceGrpc.RpmServiceBlockingStub rpmStub;
    private FanStateServiceGrpc.FanStateServiceBlockingStub fanStub;
    private WorkoutServiceGrpc.WorkoutServiceBlockingStub workoutStub;
    private StrokesServiceGrpc.StrokesServiceBlockingStub strokesStub;
    private PaceServiceGrpc.PaceServiceBlockingStub paceStub;

    // Control flags and current values
    private volatile boolean isUpdating = false;
    private volatile boolean isMonitoringWorkoutState = false;
    private volatile int currentWorkoutState = 1; // WORKOUT_STATE_IDLE
    private volatile double currentSpeed = 0.0;
    private volatile double currentIncline = 0.0;
    private volatile double currentResistance = 0.0;
    private volatile double currentWatts = 0.0;
    private volatile double currentCadence = 0.0;
    private volatile double currentRpm = 0.0;
    private volatile int currentFanSpeed = 0;
    private volatile double currentStrokesCount = 0.0;
    private volatile double currentStrokesLength = 0.0;
    private volatile int currentPaceSeconds = 0;
    private volatile int currentLast500mPaceSeconds = 0;

    // Context for accessing assets
    private Context context;

    // Metrics listener interface
    public interface MetricsListener {
        void onSpeedUpdated(double speed);
        void onInclineUpdated(double incline);
        void onWattsUpdated(double watts);
        void onResistanceUpdated(double resistance);
        void onCadenceUpdated(double cadence);
        void onRpmUpdated(double rpm);
        void onFanSpeedUpdated(int fanSpeed);
        void onStrokesUpdated(double strokesCount, double strokesLength);
        void onPaceUpdated(int paceSeconds, int last500mPaceSeconds);
        void onWorkoutStateChanged(int workoutState);
        void onError(String metric, String error);
    }

    private MetricsListener metricsListener;

    public GrpcTreadmillService(Context context) {
        this.context = context;
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.executorService = Executors.newSingleThreadExecutor();
        this.workoutStateExecutor = Executors.newSingleThreadExecutor();
    }

    public void setMetricsListener(MetricsListener listener) {
        this.metricsListener = listener;
    }

    private void initializeInstance() throws Exception {
        initializeGrpcConnection();
    }

    private void startMetricsUpdatesInstance() {
        if (isUpdating) return;

        isUpdating = true;

        metricsUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                if (!isUpdating) return;

                executorService.execute(() -> {
                    fetchAllMetricsFromServer();

                    if (isUpdating) {
                        mainHandler.postDelayed(metricsUpdateRunnable, UPDATE_INTERVAL_MS);
                    }
                });
            }
        };

        mainHandler.post(metricsUpdateRunnable);
        QLog.i(TAG, "Started periodic metrics updates");
    }

    private void stopMetricsUpdatesInstance() {
        isUpdating = false;

        if (metricsUpdateRunnable != null) {
            mainHandler.removeCallbacks(metricsUpdateRunnable);
        }

        QLog.i(TAG, "Stopped periodic metrics updates");
    }

    private void startWorkoutStateMonitoringInstance() {
        if (isMonitoringWorkoutState) return;

        isMonitoringWorkoutState = true;

        workoutStateExecutor.execute(() -> {
            try {
                QLog.i(TAG, "Starting WorkoutStateChanged stream monitoring");

                Metadata headers = createHeaders();
                WorkoutServiceGrpc.WorkoutServiceBlockingStub stubWithHeaders = workoutStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );

                Empty request = Empty.newBuilder().build();
                java.util.Iterator<WorkoutStateMessage> stateStream = stubWithHeaders.workoutStateChanged(request);

                while (isMonitoringWorkoutState && stateStream.hasNext()) {
                    WorkoutStateMessage stateMsg = stateStream.next();
                    int newState = stateMsg.getWorkoutState().getNumber();

                    if (newState != currentWorkoutState) {
                        int oldState = currentWorkoutState;
                        currentWorkoutState = newState;

                        QLog.i(TAG, String.format("Workout state changed: %d -> %d", oldState, newState));

                        if (metricsListener != null) {
                            mainHandler.post(() -> metricsListener.onWorkoutStateChanged(newState));
                        }
                    }
                }

                QLog.i(TAG, "WorkoutStateChanged stream monitoring stopped");

            } catch (Exception e) {
                QLog.e(TAG, "Error in WorkoutStateChanged stream", e);
                isMonitoringWorkoutState = false;
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("workout_state", e.getMessage()));
                }
            }
        });
    }

    private void stopWorkoutStateMonitoringInstance() {
        isMonitoringWorkoutState = false;
        QLog.i(TAG, "Stopping WorkoutStateChanged stream monitoring");
    }

    private void adjustSpeedInstance(double delta) {
        executorService.execute(() -> {
            try {
                double newSpeed = Math.max(0.0, currentSpeed + delta);
                
                Metadata headers = createHeaders();
                SpeedServiceGrpc.SpeedServiceBlockingStub stubWithHeaders = speedStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                SpeedRequest request = SpeedRequest.newBuilder().setKph(newSpeed).build();
                stubWithHeaders.setSpeed(request);
                
                QLog.d(TAG, String.format("Set speed to %.1f km/h", newSpeed));
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to set speed", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("speed", e.getMessage()));
                }
            }
        });
    }

    private void adjustInclineInstance(double delta) {
        executorService.execute(() -> {
            try {
                double newIncline = Math.max(-50.0, currentIncline + delta);
                
                Metadata headers = createHeaders();
                InclineServiceGrpc.InclineServiceBlockingStub stubWithHeaders = inclineStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                InclineRequest request = InclineRequest.newBuilder().setPercent(newIncline).build();
                stubWithHeaders.setIncline(request);
                
                QLog.d(TAG, String.format("Set incline to %.1f%%", newIncline));
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to set incline", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("incline", e.getMessage()));
                }
            }
        });
    }

    private void adjustResistanceInstance(double delta) {
        executorService.execute(() -> {
            try {
                double newResistance = Math.max(0.0, currentResistance + delta);
                
                Metadata headers = createHeaders();
                ResistanceServiceGrpc.ResistanceServiceBlockingStub stubWithHeaders = resistanceStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                ResistanceRequest request = ResistanceRequest.newBuilder().setResistance(newResistance).build();
                stubWithHeaders.setResistance(request);
                
                QLog.d(TAG, String.format("Set resistance to %.0f level", newResistance));
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to set resistance", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("resistance", e.getMessage()));
                }
            }
        });
    }

    private void setWattsInstance(double watts) {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                ConstantWattsServiceGrpc.ConstantWattsServiceBlockingStub stubWithHeaders = constantWattsStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                if (watts <= 0) {
                    // Disable constant watts mode when watts is 0 or negative
                    stubWithHeaders.disable(Empty.newBuilder().build());
                    QLog.d(TAG, "Disabled constant watts mode");
                } else {
                    // Set target watts
                    int targetWatts = (int) watts;
                    ConstantWattsMessage request = ConstantWattsMessage.newBuilder().setWatts(targetWatts).build();
                    stubWithHeaders.setConstantWatts(request);
                    QLog.d(TAG, String.format("Set constant watts to %d", targetWatts));
                }
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to set watts", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("watts", e.getMessage()));
                }
            }
        });
    }

    private void disableConstantWattsInstance() {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                ConstantWattsServiceGrpc.ConstantWattsServiceBlockingStub stubWithHeaders = constantWattsStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                stubWithHeaders.disable(Empty.newBuilder().build());
                QLog.d(TAG, "Explicitly disabled constant watts mode");
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to disable constant watts", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("watts", e.getMessage()));
                }
            }
        });
    }

    private void setFanSpeedInstance(int fanSpeed) {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                FanStateServiceGrpc.FanStateServiceBlockingStub stubWithHeaders = fanStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                FanState fanState;
                switch (fanSpeed) {
                    case 0:
                        fanState = FanState.FAN_STATE_OFF;
                        break;
                    case 1:
                        fanState = FanState.FAN_STATE_LOW;
                        break;
                    case 2:
                        fanState = FanState.FAN_STATE_MEDIUM;
                        break;
                    case 3:
                        fanState = FanState.FAN_STATE_HIGH;
                        break;
                    case 4:
                        fanState = FanState.FAN_STATE_AUTO;
                        break;
                    default:
                        fanState = FanState.FAN_STATE_OFF;
                        break;
                }
                
                FanStateMessage request = FanStateMessage.newBuilder().setState(fanState).build();
                stubWithHeaders.setFanState(request);
                
                QLog.d(TAG, String.format("Set fan speed to %d (%s)", fanSpeed, fanState.name()));
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to set fan speed", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("fan", e.getMessage()));
                }
            }
        });
    }

    private void shutdownInstance() {
        stopMetricsUpdates();
        stopWorkoutStateMonitoringInstance();

        if (channel != null) {
            try {
                channel.shutdown();
                if (!channel.awaitTermination(5, TimeUnit.SECONDS)) {
                    channel.shutdownNow();
                }
            } catch (InterruptedException e) {
                QLog.e(TAG, "Error shutting down gRPC channel", e);
                channel.shutdownNow();
            }
        }

        if (executorService != null) {
            executorService.shutdown();
            try {
                if (!executorService.awaitTermination(2, TimeUnit.SECONDS)) {
                    executorService.shutdownNow();
                }
            } catch (InterruptedException e) {
                QLog.e(TAG, "Error shutting down executor service", e);
                executorService.shutdownNow();
            }
        }

        if (workoutStateExecutor != null) {
            workoutStateExecutor.shutdown();
            try {
                if (!workoutStateExecutor.awaitTermination(2, TimeUnit.SECONDS)) {
                    workoutStateExecutor.shutdownNow();
                }
            } catch (InterruptedException e) {
                QLog.e(TAG, "Error shutting down workout state executor", e);
                workoutStateExecutor.shutdownNow();
            }
        }
    }

    private void initializeGrpcConnection() throws Exception {
        AssetManager assets = context.getAssets();
        
        String[] requiredFiles = {"client_cert.pem", "client_key.pem"};
        for (String file : requiredFiles) {
            try {
                assets.open(file).close();
            } catch (Exception e) {
                throw new RuntimeException("Required certificate file missing: " + file +
                        ". Please add it to app/src/main/assets/");
            }
        }

        InputStream caCertStream = null;
        try {
            caCertStream = assets.open("ca_cert.pem");
        } catch (Exception e) {
            QLog.w(TAG, "ca_cert.pem not found, continuing with insecure mode");
        }
        InputStream clientCertStream = assets.open("client_cert.pem");
        InputStream clientKeyStream = assets.open("client_key.pem");

        QLog.i(TAG, "Loading TLS certificates (insecure server validation mode)...");

        SSLContext sslContext = createSSLContext(caCertStream, clientCertStream, clientKeyStream);

        channel = OkHttpChannelBuilder.forAddress(serverHost, SERVER_PORT)
                .sslSocketFactory(sslContext.getSocketFactory())
                .build();

        if (caCertStream != null) caCertStream.close();
        clientCertStream.close();
        clientKeyStream.close();

        speedStub = SpeedServiceGrpc.newBlockingStub(channel);
        inclineStub = InclineServiceGrpc.newBlockingStub(channel);
        wattsStub = WattsServiceGrpc.newBlockingStub(channel);
        constantWattsStub = ConstantWattsServiceGrpc.newBlockingStub(channel);
        resistanceStub = ResistanceServiceGrpc.newBlockingStub(channel);
        cadenceStub = CadenceServiceGrpc.newBlockingStub(channel);
        rpmStub = RpmServiceGrpc.newBlockingStub(channel);
        fanStub = FanStateServiceGrpc.newBlockingStub(channel);
        workoutStub = WorkoutServiceGrpc.newBlockingStub(channel);
        strokesStub = StrokesServiceGrpc.newBlockingStub(channel);
        paceStub = PaceServiceGrpc.newBlockingStub(channel);

        QLog.i(TAG, "gRPC connection initialized with client certificates");
    }

    private SSLContext createSSLContext(InputStream caCertStream, InputStream clientCertStream,
                                        InputStream clientKeyStream) throws Exception {

        QLog.d(TAG, "Creating SSL context with client certificates (insecure server validation)...");

        CertificateFactory cf = CertificateFactory.getInstance("X.509");
        X509Certificate clientCert = (X509Certificate) cf.generateCertificate(clientCertStream);
        QLog.d(TAG, "Loaded client certificate: " + clientCert.getSubjectDN());

        byte[] keyData = readAllBytesCompat(clientKeyStream);
        String keyString = new String(keyData, StandardCharsets.UTF_8);
        keyString = keyString.replace("-----BEGIN PRIVATE KEY-----", "")
                .replace("-----END PRIVATE KEY-----", "")
                .replaceAll("\\s", "");

        byte[] keyBytes = Base64.decode(keyString, Base64.DEFAULT);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(keyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        PrivateKey privateKey = keyFactory.generatePrivate(keySpec);
        QLog.d(TAG, "Loaded private key");

        KeyStore keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
        keyStore.load(null, null);
        keyStore.setKeyEntry("client", privateKey, "".toCharArray(), new Certificate[]{clientCert});

        KeyManagerFactory kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        kmf.init(keyStore, "".toCharArray());

        javax.net.ssl.TrustManager[] insecureTrustManagers = new javax.net.ssl.TrustManager[] {
                new javax.net.ssl.X509TrustManager() {
                    @Override
                    public void checkClientTrusted(X509Certificate[] chain, String authType) {
                    }

                    @Override
                    public void checkServerTrusted(X509Certificate[] chain, String authType) {
                        QLog.d(TAG, "Accepting server certificate without validation (insecure mode)");
                    }

                    @Override
                    public X509Certificate[] getAcceptedIssuers() {
                        return new X509Certificate[0];
                    }
                }
        };

        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(kmf.getKeyManagers(), insecureTrustManagers, new SecureRandom());

        QLog.i(TAG, "SSL context created with client authentication but insecure server validation");
        return sslContext;
    }

    private byte[] readAllBytesCompat(InputStream inputStream) throws Exception {
        byte[] buffer = new byte[8192];
        int bytesRead;
        java.io.ByteArrayOutputStream output = new java.io.ByteArrayOutputStream();

        while ((bytesRead = inputStream.read(buffer)) != -1) {
            output.write(buffer, 0, bytesRead);
        }

        return output.toByteArray();
    }

    private Metadata createHeaders() {
        Metadata headers = new Metadata();
        headers.put(Metadata.Key.of("client_id", Metadata.ASCII_STRING_MARSHALLER),
                "com.ifit.eriador");
        return headers;
    }

    private void fetchAllMetricsFromServer() {
        long startTime = System.currentTimeMillis();
        try {
            QLog.d(TAG, "Making gRPC calls for all metrics...");

            long headersStartTime = System.currentTimeMillis();
            Metadata headers = createHeaders();
            Empty request = Empty.newBuilder().build();
            long headersEndTime = System.currentTimeMillis();
            QLog.d(TAG, "Headers creation took: " + (headersEndTime - headersStartTime) + "ms");

            // Fetch speed
            try {
                long speedStartTime = System.currentTimeMillis();
                SpeedServiceGrpc.SpeedServiceBlockingStub speedStubWithHeaders = speedStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long speedInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Speed interceptor setup took: " + (speedInterceptorTime - speedStartTime) + "ms");
                
                SpeedMetric speedResponse = speedStubWithHeaders.getSpeed(request);
                long speedCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Speed gRPC call took: " + (speedCallTime - speedInterceptorTime) + "ms");
                
                currentSpeed = speedResponse.getLastKph();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onSpeedUpdated(currentSpeed));
                }
                long speedEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Speed total processing took: " + (speedEndTime - speedStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch speed", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("speed", "Error"));
                }
            }

            // Fetch inclination
            try {
                long inclineStartTime = System.currentTimeMillis();
                InclineServiceGrpc.InclineServiceBlockingStub inclineStubWithHeaders = inclineStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long inclineInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Incline interceptor setup took: " + (inclineInterceptorTime - inclineStartTime) + "ms");
                
                InclineMetric inclineResponse = inclineStubWithHeaders.getIncline(request);
                long inclineCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Incline gRPC call took: " + (inclineCallTime - inclineInterceptorTime) + "ms");
                
                currentIncline = inclineResponse.getLastInclinePercent();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onInclineUpdated(currentIncline));
                }
                long inclineEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Incline total processing took: " + (inclineEndTime - inclineStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch inclination", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("inclination", "Error"));
                }
            }

            // Fetch watts
            try {
                long wattsStartTime = System.currentTimeMillis();
                WattsServiceGrpc.WattsServiceBlockingStub wattsStubWithHeaders = wattsStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long wattsInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Watts interceptor setup took: " + (wattsInterceptorTime - wattsStartTime) + "ms");
                
                WattsMetric wattsResponse = wattsStubWithHeaders.getWatts(request);
                long wattsCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Watts gRPC call took: " + (wattsCallTime - wattsInterceptorTime) + "ms");
                
                currentWatts = wattsResponse.getLastWatts();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onWattsUpdated(currentWatts));
                }
                long wattsEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Watts total processing took: " + (wattsEndTime - wattsStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch watts", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("watts", "Error"));
                }
            }

            // Fetch resistance
            try {
                long resistanceStartTime = System.currentTimeMillis();
                ResistanceServiceGrpc.ResistanceServiceBlockingStub resistanceStubWithHeaders = resistanceStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long resistanceInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Resistance interceptor setup took: " + (resistanceInterceptorTime - resistanceStartTime) + "ms");
                
                ResistanceMetric resistanceResponse = resistanceStubWithHeaders.getResistance(request);
                long resistanceCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Resistance gRPC call took: " + (resistanceCallTime - resistanceInterceptorTime) + "ms");
                
                currentResistance = resistanceResponse.getLastResistance();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onResistanceUpdated(currentResistance));
                }
                long resistanceEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Resistance total processing took: " + (resistanceEndTime - resistanceStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch resistance", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("resistance", "Error"));
                }
            }

            // Fetch RPM (for bikes)
            try {
                long rpmStartTime = System.currentTimeMillis();
                RpmServiceGrpc.RpmServiceBlockingStub rpmStubWithHeaders = rpmStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long rpmInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "RPM interceptor setup took: " + (rpmInterceptorTime - rpmStartTime) + "ms");
                
                RpmMetric rpmResponse = rpmStubWithHeaders.getRpm(request);
                long rpmCallTime = System.currentTimeMillis();
                QLog.d(TAG, "RPM gRPC call took: " + (rpmCallTime - rpmInterceptorTime) + "ms");
                
                currentRpm = rpmResponse.getLastRpm();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onRpmUpdated(currentRpm));
                }
                long rpmEndTime = System.currentTimeMillis();
                QLog.d(TAG, "RPM total processing took: " + (rpmEndTime - rpmStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch RPM", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("rpm", "Error"));
                }
            }

            // Fetch cadence (for treadmills)
            try {
                long cadenceStartTime = System.currentTimeMillis();
                CadenceServiceGrpc.CadenceServiceBlockingStub cadenceStubWithHeaders = cadenceStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long cadenceInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Cadence interceptor setup took: " + (cadenceInterceptorTime - cadenceStartTime) + "ms");
                
                CadenceMetric cadenceResponse = cadenceStubWithHeaders.getCadence(request);
                long cadenceCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Cadence gRPC call took: " + (cadenceCallTime - cadenceInterceptorTime) + "ms");
                
                currentCadence = cadenceResponse.getLastStepsPerMinute();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onCadenceUpdated(currentCadence));
                }
                long cadenceEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Cadence total processing took: " + (cadenceEndTime - cadenceStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch cadence", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("cadence", "Error"));
                }
            }

            // Fetch strokes (for rowers)
            try {
                long strokesStartTime = System.currentTimeMillis();
                StrokesServiceGrpc.StrokesServiceBlockingStub strokesStubWithHeaders = strokesStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long strokesInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Strokes interceptor setup took: " + (strokesInterceptorTime - strokesStartTime) + "ms");

                StrokesMetric strokesResponse = strokesStubWithHeaders.getStrokes(request);
                long strokesCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Strokes gRPC call took: " + (strokesCallTime - strokesInterceptorTime) + "ms");

                currentStrokesCount = strokesResponse.getStrokesCount();
                currentStrokesLength = strokesResponse.getStrokesLength();

                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onStrokesUpdated(currentStrokesCount, currentStrokesLength));
                }
                long strokesEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Strokes total processing took: " + (strokesEndTime - strokesStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch strokes", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("strokes", "Error"));
                }
            }

            // Fetch pace (for rowers)
            try {
                long paceStartTime = System.currentTimeMillis();
                PaceServiceGrpc.PaceServiceBlockingStub paceStubWithHeaders = paceStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long paceInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Pace interceptor setup took: " + (paceInterceptorTime - paceStartTime) + "ms");

                PaceMetric paceResponse = paceStubWithHeaders.getPace(request);
                long paceCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Pace gRPC call took: " + (paceCallTime - paceInterceptorTime) + "ms");

                currentPaceSeconds = paceResponse.getCurrentPaceSeconds();
                currentLast500mPaceSeconds = paceResponse.getLastPaceSeconds();

                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onPaceUpdated(currentPaceSeconds, currentLast500mPaceSeconds));
                }
                long paceEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Pace total processing took: " + (paceEndTime - paceStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch pace", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("pace", "Error"));
                }
            }

            // Fetch fan speed
            /*
            try {
                long fanStartTime = System.currentTimeMillis();
                FanStateServiceGrpc.FanStateServiceBlockingStub fanStubWithHeaders = fanStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                long fanInterceptorTime = System.currentTimeMillis();
                QLog.d(TAG, "Fan interceptor setup took: " + (fanInterceptorTime - fanStartTime) + "ms");
                
                FanStateMessage fanResponse = fanStubWithHeaders.getFanState(request);
                long fanCallTime = System.currentTimeMillis();
                QLog.d(TAG, "Fan gRPC call took: " + (fanCallTime - fanInterceptorTime) + "ms");
                
                int fanSpeed;
                switch (fanResponse.getState()) {
                    case FAN_STATE_OFF:
                        fanSpeed = 0;
                        break;
                    case FAN_STATE_LOW:
                        fanSpeed = 1;
                        break;
                    case FAN_STATE_MEDIUM:
                        fanSpeed = 2;
                        break;
                    case FAN_STATE_HIGH:
                        fanSpeed = 3;
                        break;
                    case FAN_STATE_AUTO:
                        fanSpeed = 4;
                        break;
                    default:
                        fanSpeed = 0;
                        break;
                }
                
                currentFanSpeed = fanSpeed;
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onFanSpeedUpdated(currentFanSpeed));
                }
                long fanEndTime = System.currentTimeMillis();
                QLog.d(TAG, "Fan total processing took: " + (fanEndTime - fanStartTime) + "ms");
            } catch (Exception e) {
                QLog.w(TAG, "Failed to fetch fan speed", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("fan", "Error"));
                }
            }
            */

            long totalEndTime = System.currentTimeMillis();
            long totalTime = totalEndTime - startTime;
            QLog.d(TAG, "=== TIMING SUMMARY ===");
            QLog.d(TAG, "Total fetchAllMetricsFromServer execution time: " + totalTime + "ms");
            QLog.d(TAG, "Completed all metrics fetch");

        } catch (Exception e) {
            long totalEndTime = System.currentTimeMillis();
            long totalTime = totalEndTime - startTime;
            QLog.e(TAG, "Failed to fetch metrics after " + totalTime + "ms", e);
            if (metricsListener != null) {
                mainHandler.post(() -> {
                    metricsListener.onError("speed", "Error");
                    metricsListener.onError("inclination", "Error");
                    metricsListener.onError("watts", "Error");
                    metricsListener.onError("resistance", "Error");
                    metricsListener.onError("cadence", "Error");
                });
            }
        }
    }
    
    // Static wrapper methods for JNI calls
    public static void initialize() {
        initialize("localhost");
    }
    
    public static void initialize(String host) {
        try {
            if (staticContext == null) {
                QLog.e(TAG, "Context not set. Call setContext() first.");
                return;
            }
            
            serverHost = host;
            
            if (instance == null) {
                instance = new GrpcTreadmillService(staticContext);
            }
            
            instance.initializeInstance();
            QLog.i(TAG, "Static initialize completed with host: " + host);
        } catch (Exception e) {
            QLog.e(TAG, "Static initialize failed", e);
        }
    }
    
    public static void setContext(Context context) {
        staticContext = context;
    }
    
    public static void startMetricsUpdates() {
        if (instance != null) {
            instance.startMetricsUpdatesInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void stopMetricsUpdates() {
        if (instance != null) {
            instance.stopMetricsUpdatesInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static double getCurrentSpeed() {
        if (instance != null) {
            return instance.currentSpeed;
        }
        return 0.0;
    }
    
    public static double getCurrentIncline() {
        if (instance != null) {
            return instance.currentIncline;
        }
        return 0.0;
    }
    
    public static double getCurrentWatts() {
        if (instance != null) {
            return instance.currentWatts;
        }
        return 0.0;
    }
    
    public static double getCurrentCadence() {
        if (instance != null) {
            return instance.currentCadence;
        }
        return 0.0;
    }
    
    public static double getCurrentRpm() {
        if (instance != null) {
            return instance.currentRpm;
        }
        return 0.0;
    }
    
    public static int getCurrentFanSpeed() {
        if (instance != null) {
            return instance.currentFanSpeed;
        }
        return 0;
    }
    
    public static double getCurrentResistance() {
        if (instance != null) {
            return instance.currentResistance;
        }
        return 0.0;
    }

    public static double getCurrentStrokesCount() {
        if (instance != null) {
            return instance.currentStrokesCount;
        }
        return 0.0;
    }

    public static double getCurrentStrokesLength() {
        if (instance != null) {
            return instance.currentStrokesLength;
        }
        return 0.0;
    }

    public static int getCurrentPaceSeconds() {
        if (instance != null) {
            return instance.currentPaceSeconds;
        }
        return 0;
    }

    public static int getCurrentLast500mPaceSeconds() {
        if (instance != null) {
            return instance.currentLast500mPaceSeconds;
        }
        return 0;
    }

    public static void adjustSpeed(double delta) {
        if (instance != null) {
            instance.adjustSpeedInstance(delta);
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void adjustIncline(double delta) {
        if (instance != null) {
            instance.adjustInclineInstance(delta);
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void adjustResistance(double delta) {
        if (instance != null) {
            instance.adjustResistanceInstance(delta);
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void setWatts(double watts) {
        if (instance != null) {
            instance.setWattsInstance(watts);
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void disableConstantWatts() {
        if (instance != null) {
            instance.disableConstantWattsInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void setFanSpeed(int fanSpeed) {
        if (instance != null) {
            instance.setFanSpeedInstance(fanSpeed);
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    private void startWorkoutInstance() {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                WorkoutServiceGrpc.WorkoutServiceBlockingStub stubWithHeaders = workoutStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );

                Empty request = Empty.newBuilder().build();
                StartWorkoutResponse response = stubWithHeaders.startNewWorkout(request);

                QLog.i(TAG, "Started workout via WorkoutService");
                if (response.hasResult()) {
                    QLog.d(TAG, "Workout started with result: " + response.getResult());
                }

            } catch (Exception e) {
                QLog.e(TAG, "Failed to start workout", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("workout_start", e.getMessage()));
                }
            }
        });
    }

    private void stopWorkoutInstance() {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                WorkoutServiceGrpc.WorkoutServiceBlockingStub stubWithHeaders = workoutStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );

                Empty request = Empty.newBuilder().build();
                WorkoutResult result = stubWithHeaders.stop(request);

                QLog.i(TAG, "Stopped workout via WorkoutService");

            } catch (Exception e) {
                QLog.e(TAG, "Failed to stop workout", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("workout_stop", e.getMessage()));
                }
            }
        });
    }

    private void pauseWorkoutInstance() {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                WorkoutServiceGrpc.WorkoutServiceBlockingStub stubWithHeaders = workoutStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );

                Empty request = Empty.newBuilder().build();
                WorkoutResult result = stubWithHeaders.pause(request);

                QLog.i(TAG, "Paused workout via WorkoutService");

            } catch (Exception e) {
                QLog.e(TAG, "Failed to pause workout", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("workout_pause", e.getMessage()));
                }
            }
        });
    }

    private void resumeWorkoutInstance() {
        executorService.execute(() -> {
            try {
                Metadata headers = createHeaders();
                WorkoutServiceGrpc.WorkoutServiceBlockingStub stubWithHeaders = workoutStub.withInterceptors(
                        MetadataUtils.newAttachHeadersInterceptor(headers)
                );

                Empty request = Empty.newBuilder().build();
                WorkoutResult result = stubWithHeaders.resume(request);

                QLog.i(TAG, "Resumed workout via WorkoutService");

            } catch (Exception e) {
                QLog.e(TAG, "Failed to resume workout", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("workout_resume", e.getMessage()));
                }
            }
        });
    }

    public static void startWorkout() {
        if (instance != null) {
            instance.startWorkoutInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }

    public static void stopWorkout() {
        if (instance != null) {
            instance.stopWorkoutInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }

    public static void pauseWorkout() {
        if (instance != null) {
            instance.pauseWorkoutInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }

    public static void resumeWorkout() {
        if (instance != null) {
            instance.resumeWorkoutInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }

    public static void startWorkoutStateMonitoring() {
        if (instance != null) {
            instance.startWorkoutStateMonitoringInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }

    public static void stopWorkoutStateMonitoring() {
        if (instance != null) {
            instance.stopWorkoutStateMonitoringInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }

    public static int getWorkoutState() {
        if (instance != null) {
            return instance.currentWorkoutState;
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
            return 1; // WORKOUT_STATE_IDLE
        }
    }

    public static void shutdown() {
        if (instance != null) {
            instance.shutdownInstance();
            instance = null;
        }
    }
}

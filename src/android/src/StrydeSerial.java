package org.cagnulen.qdomyoszwift;

import android.content.Context;
import java.io.File;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.concurrent.ConcurrentLinkedQueue;
import org.qtproject.qt5.android.QtNative;

public final class StrydeSerial {
    private static final ConcurrentLinkedQueue<String> events = new ConcurrentLinkedQueue<>();
    private static volatile boolean running;
    private static volatile Object port;
    private static Thread reader;
    private static Thread writer;

    private static void event(String message) {
        QLog.d("StrydeSerial", message);
        events.add(message);
        while (events.size() > 1000) events.poll();
    }

    public static String nextEvent() {
        String value = events.poll();
        return value == null ? "" : value;
    }

    public static synchronized void start() {
        if (running) return;
        if (reader != null && reader.isAlive()) { event("ERROR previous serial reader is still stopping"); return; }
        events.clear();
        running = true;
        reader = new Thread(() -> {
            Object opened = null;
            try {
                Context context = QtNative.activity();
                if (context == null) throw new Exception("Android activity unavailable");
                Object service = context.getSystemService("serial");
                if (service == null) throw new Exception("OEM serial service unavailable");
                String[] ports = (String[]) service.getClass().getMethod("getSerialPorts").invoke(service);
                String path = ports != null && ports.length > 0 ? ports[0] : "/dev/ttyMT1";
                event("PORT " + path + " baud=19200");
                if (!new File(path).exists()) throw new Exception("Serial device does not exist: " + path);
                opened = service.getClass().getMethod("openSerialPort", String.class, int.class)
                        .invoke(service, path, 19200);
                if (opened == null) throw new Exception("Serial service returned no port");
                synchronized (StrydeSerial.class) {
                    if (!running) { opened.getClass().getMethod("close").invoke(opened); return; }
                    port = opened;
                }
                final Object activePort = opened;
                final Method write = activePort.getClass().getMethod("write", ByteBuffer.class, int.class);
                Method read = activePort.getClass().getMethod("read", ByteBuffer.class);
                event("OPEN");
                writer = new Thread(() -> {
                    byte[] request = {0x5b, 0x02, 0x00, (byte) 0xfe, 0x5d};
                    try {
                        while (running) {
                            Thread.sleep(1000);
                            synchronized (StrydeSerial.class) {
                                if (!running) break;
                                write.invoke(activePort, ByteBuffer.wrap(request), request.length);
                                event("TX 5b0200fe5d");
                            }
                        }
                    } catch (InterruptedException ignored) {
                        Thread.currentThread().interrupt();
                    } catch (Exception e) { if (running) { event("ERROR write: " + e); stop(); } }
                }, "StrydeSerialWriter");
                writer.start();
                while (running) {
                    ByteBuffer buffer = ByteBuffer.allocate(1024);
                    int count = ((Number) read.invoke(activePort, buffer)).intValue();
                    if (count < 0) throw new Exception("Serial read returned " + count);
                    if (count > 0) {
                        StringBuilder hex = new StringBuilder(count * 2);
                        byte[] data = buffer.array();
                        for (int i = 0; i < count; i++) hex.append(String.format(java.util.Locale.US, "%02x", data[i] & 255));
                        event("RX " + hex);
                    } else Thread.sleep(100);
                }
            } catch (Exception e) { if (running) event("ERROR serial: " + e); }
            finally { stop(); }
        }, "StrydeSerialReader");
        reader.start();
    }

    public static void stop() {
        Object closing;
        Thread writing;
        synchronized (StrydeSerial.class) {
            if (!running && port == null) return;
            running = false;
            closing = port;
            port = null;
            writing = writer;
            writer = null;
        }
        if (writing != null) writing.interrupt();
        if (closing != null) {
            try { closing.getClass().getMethod("close").invoke(closing); }
            catch (Exception e) { event("ERROR close: " + e); }
        }
        event("CLOSED");
    }
}

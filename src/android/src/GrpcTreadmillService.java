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
import java.util.Base64;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;

import io.grpc.ManagedChannel;
import io.grpc.Metadata;
import io.grpc.okhttp.OkHttpChannelBuilder;

import com.ifit.glassos.util.Empty;
import com.ifit.glassos.workout.SpeedMetric;
import com.ifit.glassos.workout.SpeedServiceGrpc;
import com.ifit.glassos.workout.SpeedRequest;
import com.ifit.glassos.workout.InclineMetric;
import com.ifit.glassos.workout.InclineServiceGrpc;
import com.ifit.glassos.workout.InclineRequest;
import com.ifit.glassos.workout.WattsMetric;
import com.ifit.glassos.workout.WattsServiceGrpc;
import com.ifit.glassos.workout.ResistanceMetric;
import com.ifit.glassos.workout.ResistanceServiceGrpc;
import com.ifit.glassos.workout.ResistanceRequest;
import com.ifit.glassos.workout.CadenceMetric;
import com.ifit.glassos.workout.CadenceServiceGrpc;

public class GrpcTreadmillService {

    private static final String TAG = "GrpcTreadmillService";
    private static final String SERVER_HOST = "localhost";
    private static final int SERVER_PORT = 54321;
    private static final int UPDATE_INTERVAL_MS = 500;

    // Threading components
    private Handler mainHandler;
    private ExecutorService executorService;
    private Runnable metricsUpdateRunnable;

    // gRPC components
    private ManagedChannel channel;
    private SpeedServiceGrpc.SpeedServiceBlockingStub speedStub;
    private InclineServiceGrpc.InclineServiceBlockingStub inclineStub;
    private WattsServiceGrpc.WattsServiceBlockingStub wattsStub;
    private ResistanceServiceGrpc.ResistanceServiceBlockingStub resistanceStub;
    private CadenceServiceGrpc.CadenceServiceBlockingStub cadenceStub;

    // Control flags and current values
    private volatile boolean isUpdating = false;
    private volatile double currentSpeed = 0.0;
    private volatile double currentIncline = 0.0;
    private volatile double currentResistance = 0.0;

    // Context for accessing assets
    private Context context;

    // Metrics listener interface
    public interface MetricsListener {
        void onSpeedUpdated(double speed);
        void onInclineUpdated(double incline);
        void onWattsUpdated(double watts);
        void onResistanceUpdated(double resistance);
        void onCadenceUpdated(double cadence);
        void onError(String metric, String error);
    }

    private MetricsListener metricsListener;

    public GrpcTreadmillService(Context context) {
        this.context = context;
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.executorService = Executors.newSingleThreadExecutor();
    }

    public void setMetricsListener(MetricsListener listener) {
        this.metricsListener = listener;
    }

    public void initialize() throws Exception {
        initializeGrpcConnection();
    }

    public void startMetricsUpdates() {
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
        Log.i(TAG, "Started periodic metrics updates");
    }

    public void stopMetricsUpdates() {
        isUpdating = false;

        if (metricsUpdateRunnable != null) {
            mainHandler.removeCallbacks(metricsUpdateRunnable);
        }

        Log.i(TAG, "Stopped periodic metrics updates");
    }

    public void adjustSpeed(double delta) {
        executorService.execute(() -> {
            try {
                double newSpeed = Math.max(0.0, currentSpeed + delta);
                
                Metadata headers = createHeaders();
                SpeedServiceGrpc.SpeedServiceBlockingStub stubWithHeaders = speedStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                SpeedRequest request = SpeedRequest.newBuilder().setKph(newSpeed).build();
                stubWithHeaders.setSpeed(request);
                
                Log.d(TAG, String.format("Set speed to %.1f km/h", newSpeed));
                
            } catch (Exception e) {
                Log.e(TAG, "Failed to set speed", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("speed", e.getMessage()));
                }
            }
        });
    }

    public void adjustIncline(double delta) {
        executorService.execute(() -> {
            try {
                double newIncline = Math.max(0.0, currentIncline + delta);
                
                Metadata headers = createHeaders();
                InclineServiceGrpc.InclineServiceBlockingStub stubWithHeaders = inclineStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                InclineRequest request = InclineRequest.newBuilder().setPercent(newIncline).build();
                stubWithHeaders.setIncline(request);
                
                Log.d(TAG, String.format("Set incline to %.1f%%", newIncline));
                
            } catch (Exception e) {
                Log.e(TAG, "Failed to set incline", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("incline", e.getMessage()));
                }
            }
        });
    }

    public void adjustResistance(double delta) {
        executorService.execute(() -> {
            try {
                double newResistance = Math.max(0.0, currentResistance + delta);
                
                Metadata headers = createHeaders();
                ResistanceServiceGrpc.ResistanceServiceBlockingStub stubWithHeaders = resistanceStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                
                ResistanceRequest request = ResistanceRequest.newBuilder().setResistance(newResistance).build();
                stubWithHeaders.setResistance(request);
                
                Log.d(TAG, String.format("Set resistance to %.0f level", newResistance));
                
            } catch (Exception e) {
                Log.e(TAG, "Failed to set resistance", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("resistance", e.getMessage()));
                }
            }
        });
    }

    public void shutdown() {
        stopMetricsUpdates();

        if (channel != null) {
            try {
                channel.shutdown();
                if (!channel.awaitTermination(5, TimeUnit.SECONDS)) {
                    channel.shutdownNow();
                }
            } catch (InterruptedException e) {
                Log.e(TAG, "Error shutting down gRPC channel", e);
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
                Log.e(TAG, "Error shutting down executor service", e);
                executorService.shutdownNow();
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
            Log.w(TAG, "ca_cert.pem not found, continuing with insecure mode");
        }
        InputStream clientCertStream = assets.open("client_cert.pem");
        InputStream clientKeyStream = assets.open("client_key.pem");

        Log.i(TAG, "Loading TLS certificates (insecure server validation mode)...");

        SSLContext sslContext = createSSLContext(caCertStream, clientCertStream, clientKeyStream);

        channel = OkHttpChannelBuilder.forAddress(SERVER_HOST, SERVER_PORT)
                .sslSocketFactory(sslContext.getSocketFactory())
                .build();

        if (caCertStream != null) caCertStream.close();
        clientCertStream.close();
        clientKeyStream.close();

        speedStub = SpeedServiceGrpc.newBlockingStub(channel);
        inclineStub = InclineServiceGrpc.newBlockingStub(channel);
        wattsStub = WattsServiceGrpc.newBlockingStub(channel);
        resistanceStub = ResistanceServiceGrpc.newBlockingStub(channel);
        cadenceStub = CadenceServiceGrpc.newBlockingStub(channel);

        Log.i(TAG, "gRPC connection initialized with client certificates");
    }

    private SSLContext createSSLContext(InputStream caCertStream, InputStream clientCertStream,
                                        InputStream clientKeyStream) throws Exception {

        Log.d(TAG, "Creating SSL context with client certificates (insecure server validation)...");

        CertificateFactory cf = CertificateFactory.getInstance("X.509");
        X509Certificate clientCert = (X509Certificate) cf.generateCertificate(clientCertStream);
        Log.d(TAG, "Loaded client certificate: " + clientCert.getSubjectDN());

        byte[] keyData = readAllBytesCompat(clientKeyStream);
        String keyString = new String(keyData, StandardCharsets.UTF_8);
        keyString = keyString.replace("-----BEGIN PRIVATE KEY-----", "")
                .replace("-----END PRIVATE KEY-----", "")
                .replaceAll("\\s", "");

        byte[] keyBytes = Base64.getDecoder().decode(keyString);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(keyBytes);
        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        PrivateKey privateKey = keyFactory.generatePrivate(keySpec);
        Log.d(TAG, "Loaded private key");

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
                        Log.d(TAG, "Accepting server certificate without validation (insecure mode)");
                    }

                    @Override
                    public X509Certificate[] getAcceptedIssuers() {
                        return new X509Certificate[0];
                    }
                }
        };

        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(kmf.getKeyManagers(), insecureTrustManagers, new SecureRandom());

        Log.i(TAG, "SSL context created with client authentication but insecure server validation");
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
        try {
            Log.d(TAG, "Making gRPC calls for all metrics...");

            Metadata headers = createHeaders();
            Empty request = Empty.newBuilder().build();

            // Fetch speed
            try {
                SpeedServiceGrpc.SpeedServiceBlockingStub speedStubWithHeaders = speedStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                SpeedMetric speedResponse = speedStubWithHeaders.getSpeed(request);
                currentSpeed = speedResponse.getLastKph();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onSpeedUpdated(currentSpeed));
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to fetch speed", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("speed", "Error"));
                }
            }

            // Fetch inclination
            try {
                InclineServiceGrpc.InclineServiceBlockingStub inclineStubWithHeaders = inclineStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                InclineMetric inclineResponse = inclineStubWithHeaders.getIncline(request);
                currentIncline = inclineResponse.getLastInclinePercent();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onInclineUpdated(currentIncline));
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to fetch inclination", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("inclination", "Error"));
                }
            }

            // Fetch watts
            try {
                WattsServiceGrpc.WattsServiceBlockingStub wattsStubWithHeaders = wattsStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                WattsMetric wattsResponse = wattsStubWithHeaders.getWatts(request);
                double currentWatts = wattsResponse.getLastWatts();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onWattsUpdated(currentWatts));
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to fetch watts", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("watts", "Error"));
                }
            }

            // Fetch resistance
            try {
                ResistanceServiceGrpc.ResistanceServiceBlockingStub resistanceStubWithHeaders = resistanceStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                ResistanceMetric resistanceResponse = resistanceStubWithHeaders.getResistance(request);
                currentResistance = resistanceResponse.getLastResistance();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onResistanceUpdated(currentResistance));
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to fetch resistance", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("resistance", "Error"));
                }
            }

            // Fetch cadence
            try {
                CadenceServiceGrpc.CadenceServiceBlockingStub cadenceStubWithHeaders = cadenceStub.withInterceptors(
                        io.grpc.stub.MetadataUtils.newAttachHeadersInterceptor(headers)
                );
                CadenceMetric cadenceResponse = cadenceStubWithHeaders.getCadence(request);
                double currentCadence = cadenceResponse.getLastStepsPerMinute();
                
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onCadenceUpdated(currentCadence));
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to fetch cadence", e);
                if (metricsListener != null) {
                    mainHandler.post(() -> metricsListener.onError("cadence", "Error"));
                }
            }

            Log.d(TAG, "Completed all metrics fetch");

        } catch (Exception e) {
            Log.e(TAG, "Failed to fetch metrics", e);
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
}

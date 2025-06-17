package com.ifit.glassos.settings.heartrate;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/heartrate/HeartRateSettingsService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class HeartRateSettingsServiceGrpc {

  private HeartRateSettingsServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.HeartRateSettingsService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getHeartRateSettingsSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "HeartRateSettingsSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.heartrate.HeartRateSettings.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getHeartRateSettingsSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.heartrate.HeartRateSettings> getHeartRateSettingsSubscriptionMethod;
    if ((getHeartRateSettingsSubscriptionMethod = HeartRateSettingsServiceGrpc.getHeartRateSettingsSubscriptionMethod) == null) {
      synchronized (HeartRateSettingsServiceGrpc.class) {
        if ((getHeartRateSettingsSubscriptionMethod = HeartRateSettingsServiceGrpc.getHeartRateSettingsSubscriptionMethod) == null) {
          HeartRateSettingsServiceGrpc.getHeartRateSettingsSubscriptionMethod = getHeartRateSettingsSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.heartrate.HeartRateSettings>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "HeartRateSettingsSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.heartrate.HeartRateSettings.getDefaultInstance()))
              .build();
        }
      }
    }
    return getHeartRateSettingsSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getResetHeartRateSettingsToDefaultMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ResetHeartRateSettingsToDefault",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.heartrate.HeartRateSettings.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getResetHeartRateSettingsToDefaultMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.heartrate.HeartRateSettings> getResetHeartRateSettingsToDefaultMethod;
    if ((getResetHeartRateSettingsToDefaultMethod = HeartRateSettingsServiceGrpc.getResetHeartRateSettingsToDefaultMethod) == null) {
      synchronized (HeartRateSettingsServiceGrpc.class) {
        if ((getResetHeartRateSettingsToDefaultMethod = HeartRateSettingsServiceGrpc.getResetHeartRateSettingsToDefaultMethod) == null) {
          HeartRateSettingsServiceGrpc.getResetHeartRateSettingsToDefaultMethod = getResetHeartRateSettingsToDefaultMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.heartrate.HeartRateSettings>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ResetHeartRateSettingsToDefault"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.heartrate.HeartRateSettings.getDefaultInstance()))
              .build();
        }
      }
    }
    return getResetHeartRateSettingsToDefaultMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getUpdateHeartRateSettingsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "UpdateHeartRateSettings",
      requestType = com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate.class,
      responseType = com.ifit.glassos.settings.heartrate.HeartRateSettings.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getUpdateHeartRateSettingsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate, com.ifit.glassos.settings.heartrate.HeartRateSettings> getUpdateHeartRateSettingsMethod;
    if ((getUpdateHeartRateSettingsMethod = HeartRateSettingsServiceGrpc.getUpdateHeartRateSettingsMethod) == null) {
      synchronized (HeartRateSettingsServiceGrpc.class) {
        if ((getUpdateHeartRateSettingsMethod = HeartRateSettingsServiceGrpc.getUpdateHeartRateSettingsMethod) == null) {
          HeartRateSettingsServiceGrpc.getUpdateHeartRateSettingsMethod = getUpdateHeartRateSettingsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate, com.ifit.glassos.settings.heartrate.HeartRateSettings>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "UpdateHeartRateSettings"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.heartrate.HeartRateSettings.getDefaultInstance()))
              .build();
        }
      }
    }
    return getUpdateHeartRateSettingsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getGetHeartRateSettingsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetHeartRateSettings",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.heartrate.HeartRateSettings.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.heartrate.HeartRateSettings> getGetHeartRateSettingsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.heartrate.HeartRateSettings> getGetHeartRateSettingsMethod;
    if ((getGetHeartRateSettingsMethod = HeartRateSettingsServiceGrpc.getGetHeartRateSettingsMethod) == null) {
      synchronized (HeartRateSettingsServiceGrpc.class) {
        if ((getGetHeartRateSettingsMethod = HeartRateSettingsServiceGrpc.getGetHeartRateSettingsMethod) == null) {
          HeartRateSettingsServiceGrpc.getGetHeartRateSettingsMethod = getGetHeartRateSettingsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.heartrate.HeartRateSettings>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetHeartRateSettings"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.heartrate.HeartRateSettings.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetHeartRateSettingsMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static HeartRateSettingsServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HeartRateSettingsServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HeartRateSettingsServiceStub>() {
        @java.lang.Override
        public HeartRateSettingsServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HeartRateSettingsServiceStub(channel, callOptions);
        }
      };
    return HeartRateSettingsServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static HeartRateSettingsServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HeartRateSettingsServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HeartRateSettingsServiceBlockingStub>() {
        @java.lang.Override
        public HeartRateSettingsServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HeartRateSettingsServiceBlockingStub(channel, callOptions);
        }
      };
    return HeartRateSettingsServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static HeartRateSettingsServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HeartRateSettingsServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HeartRateSettingsServiceFutureStub>() {
        @java.lang.Override
        public HeartRateSettingsServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HeartRateSettingsServiceFutureStub(channel, callOptions);
        }
      };
    return HeartRateSettingsServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * subscribe to heart rate settings changes
     * </pre>
     */
    default void heartRateSettingsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getHeartRateSettingsSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * reset heart rate settings to default values
     * </pre>
     */
    default void resetHeartRateSettingsToDefault(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getResetHeartRateSettingsToDefaultMethod(), responseObserver);
    }

    /**
     * <pre>
     * updates heart rate settings
     * </pre>
     */
    default void updateHeartRateSettings(com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getUpdateHeartRateSettingsMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current heart rate settings
     * </pre>
     */
    default void getHeartRateSettings(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetHeartRateSettingsMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service HeartRateSettingsService.
   */
  public static abstract class HeartRateSettingsServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return HeartRateSettingsServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service HeartRateSettingsService.
   */
  public static final class HeartRateSettingsServiceStub
      extends io.grpc.stub.AbstractAsyncStub<HeartRateSettingsServiceStub> {
    private HeartRateSettingsServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HeartRateSettingsServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HeartRateSettingsServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * subscribe to heart rate settings changes
     * </pre>
     */
    public void heartRateSettingsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getHeartRateSettingsSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * reset heart rate settings to default values
     * </pre>
     */
    public void resetHeartRateSettingsToDefault(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getResetHeartRateSettingsToDefaultMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * updates heart rate settings
     * </pre>
     */
    public void updateHeartRateSettings(com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getUpdateHeartRateSettingsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current heart rate settings
     * </pre>
     */
    public void getHeartRateSettings(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetHeartRateSettingsMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service HeartRateSettingsService.
   */
  public static final class HeartRateSettingsServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<HeartRateSettingsServiceBlockingStub> {
    private HeartRateSettingsServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HeartRateSettingsServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HeartRateSettingsServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * subscribe to heart rate settings changes
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.settings.heartrate.HeartRateSettings> heartRateSettingsSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getHeartRateSettingsSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * reset heart rate settings to default values
     * </pre>
     */
    public com.ifit.glassos.settings.heartrate.HeartRateSettings resetHeartRateSettingsToDefault(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getResetHeartRateSettingsToDefaultMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * updates heart rate settings
     * </pre>
     */
    public com.ifit.glassos.settings.heartrate.HeartRateSettings updateHeartRateSettings(com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getUpdateHeartRateSettingsMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current heart rate settings
     * </pre>
     */
    public com.ifit.glassos.settings.heartrate.HeartRateSettings getHeartRateSettings(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetHeartRateSettingsMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service HeartRateSettingsService.
   */
  public static final class HeartRateSettingsServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<HeartRateSettingsServiceFutureStub> {
    private HeartRateSettingsServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HeartRateSettingsServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HeartRateSettingsServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * reset heart rate settings to default values
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.heartrate.HeartRateSettings> resetHeartRateSettingsToDefault(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getResetHeartRateSettingsToDefaultMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * updates heart rate settings
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.heartrate.HeartRateSettings> updateHeartRateSettings(
        com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getUpdateHeartRateSettingsMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current heart rate settings
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.heartrate.HeartRateSettings> getHeartRateSettings(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetHeartRateSettingsMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_HEART_RATE_SETTINGS_SUBSCRIPTION = 0;
  private static final int METHODID_RESET_HEART_RATE_SETTINGS_TO_DEFAULT = 1;
  private static final int METHODID_UPDATE_HEART_RATE_SETTINGS = 2;
  private static final int METHODID_GET_HEART_RATE_SETTINGS = 3;

  private static final class MethodHandlers<Req, Resp> implements
      io.grpc.stub.ServerCalls.UnaryMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ServerStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.ClientStreamingMethod<Req, Resp>,
      io.grpc.stub.ServerCalls.BidiStreamingMethod<Req, Resp> {
    private final AsyncService serviceImpl;
    private final int methodId;

    MethodHandlers(AsyncService serviceImpl, int methodId) {
      this.serviceImpl = serviceImpl;
      this.methodId = methodId;
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public void invoke(Req request, io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        case METHODID_HEART_RATE_SETTINGS_SUBSCRIPTION:
          serviceImpl.heartRateSettingsSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings>) responseObserver);
          break;
        case METHODID_RESET_HEART_RATE_SETTINGS_TO_DEFAULT:
          serviceImpl.resetHeartRateSettingsToDefault((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings>) responseObserver);
          break;
        case METHODID_UPDATE_HEART_RATE_SETTINGS:
          serviceImpl.updateHeartRateSettings((com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings>) responseObserver);
          break;
        case METHODID_GET_HEART_RATE_SETTINGS:
          serviceImpl.getHeartRateSettings((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.heartrate.HeartRateSettings>) responseObserver);
          break;
        default:
          throw new AssertionError();
      }
    }

    @java.lang.Override
    @java.lang.SuppressWarnings("unchecked")
    public io.grpc.stub.StreamObserver<Req> invoke(
        io.grpc.stub.StreamObserver<Resp> responseObserver) {
      switch (methodId) {
        default:
          throw new AssertionError();
      }
    }
  }

  public static final io.grpc.ServerServiceDefinition bindService(AsyncService service) {
    return io.grpc.ServerServiceDefinition.builder(getServiceDescriptor())
        .addMethod(
          getHeartRateSettingsSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.heartrate.HeartRateSettings>(
                service, METHODID_HEART_RATE_SETTINGS_SUBSCRIPTION)))
        .addMethod(
          getResetHeartRateSettingsToDefaultMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.heartrate.HeartRateSettings>(
                service, METHODID_RESET_HEART_RATE_SETTINGS_TO_DEFAULT)))
        .addMethod(
          getUpdateHeartRateSettingsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.heartrate.HeartRateSettingsUpdate,
              com.ifit.glassos.settings.heartrate.HeartRateSettings>(
                service, METHODID_UPDATE_HEART_RATE_SETTINGS)))
        .addMethod(
          getGetHeartRateSettingsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.heartrate.HeartRateSettings>(
                service, METHODID_GET_HEART_RATE_SETTINGS)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (HeartRateSettingsServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getHeartRateSettingsSubscriptionMethod())
              .addMethod(getResetHeartRateSettingsToDefaultMethod())
              .addMethod(getUpdateHeartRateSettingsMethod())
              .addMethod(getGetHeartRateSettingsMethod())
              .build();
        }
      }
    }
    return result;
  }
}

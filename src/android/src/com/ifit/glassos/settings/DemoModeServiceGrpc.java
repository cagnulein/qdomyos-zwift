package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/DemoModeService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class DemoModeServiceGrpc {

  private DemoModeServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.DemoModeService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.DemoMode> getDemoModeChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "DemoModeChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.DemoMode.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.DemoMode> getDemoModeChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.DemoMode> getDemoModeChangedMethod;
    if ((getDemoModeChangedMethod = DemoModeServiceGrpc.getDemoModeChangedMethod) == null) {
      synchronized (DemoModeServiceGrpc.class) {
        if ((getDemoModeChangedMethod = DemoModeServiceGrpc.getDemoModeChangedMethod) == null) {
          DemoModeServiceGrpc.getDemoModeChangedMethod = getDemoModeChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.DemoMode>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "DemoModeChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoMode.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDemoModeChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.DemoMode,
      com.ifit.glassos.settings.DemoModeResult> getSetDemoModeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetDemoMode",
      requestType = com.ifit.glassos.settings.DemoMode.class,
      responseType = com.ifit.glassos.settings.DemoModeResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.DemoMode,
      com.ifit.glassos.settings.DemoModeResult> getSetDemoModeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.DemoMode, com.ifit.glassos.settings.DemoModeResult> getSetDemoModeMethod;
    if ((getSetDemoModeMethod = DemoModeServiceGrpc.getSetDemoModeMethod) == null) {
      synchronized (DemoModeServiceGrpc.class) {
        if ((getSetDemoModeMethod = DemoModeServiceGrpc.getSetDemoModeMethod) == null) {
          DemoModeServiceGrpc.getSetDemoModeMethod = getSetDemoModeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.DemoMode, com.ifit.glassos.settings.DemoModeResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetDemoMode"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoMode.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoModeResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetDemoModeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.DemoModeResult> getGetDemoModeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetDemoMode",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.DemoModeResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.DemoModeResult> getGetDemoModeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.DemoModeResult> getGetDemoModeMethod;
    if ((getGetDemoModeMethod = DemoModeServiceGrpc.getGetDemoModeMethod) == null) {
      synchronized (DemoModeServiceGrpc.class) {
        if ((getGetDemoModeMethod = DemoModeServiceGrpc.getGetDemoModeMethod) == null) {
          DemoModeServiceGrpc.getGetDemoModeMethod = getGetDemoModeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.DemoModeResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetDemoMode"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoModeResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetDemoModeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.DemoModeScreensaverVolume,
      com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getSetDemoModeScreensaverVolumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetDemoModeScreensaverVolume",
      requestType = com.ifit.glassos.settings.DemoModeScreensaverVolume.class,
      responseType = com.ifit.glassos.settings.DemoModeScreensaverVolumeResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.DemoModeScreensaverVolume,
      com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getSetDemoModeScreensaverVolumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.DemoModeScreensaverVolume, com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getSetDemoModeScreensaverVolumeMethod;
    if ((getSetDemoModeScreensaverVolumeMethod = DemoModeServiceGrpc.getSetDemoModeScreensaverVolumeMethod) == null) {
      synchronized (DemoModeServiceGrpc.class) {
        if ((getSetDemoModeScreensaverVolumeMethod = DemoModeServiceGrpc.getSetDemoModeScreensaverVolumeMethod) == null) {
          DemoModeServiceGrpc.getSetDemoModeScreensaverVolumeMethod = getSetDemoModeScreensaverVolumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.DemoModeScreensaverVolume, com.ifit.glassos.settings.DemoModeScreensaverVolumeResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetDemoModeScreensaverVolume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoModeScreensaverVolume.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoModeScreensaverVolumeResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetDemoModeScreensaverVolumeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getGetDemoModeScreensaverVolumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetDemoModeScreensaverVolume",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.DemoModeScreensaverVolumeResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getGetDemoModeScreensaverVolumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getGetDemoModeScreensaverVolumeMethod;
    if ((getGetDemoModeScreensaverVolumeMethod = DemoModeServiceGrpc.getGetDemoModeScreensaverVolumeMethod) == null) {
      synchronized (DemoModeServiceGrpc.class) {
        if ((getGetDemoModeScreensaverVolumeMethod = DemoModeServiceGrpc.getGetDemoModeScreensaverVolumeMethod) == null) {
          DemoModeServiceGrpc.getGetDemoModeScreensaverVolumeMethod = getGetDemoModeScreensaverVolumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.DemoModeScreensaverVolumeResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetDemoModeScreensaverVolume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.DemoModeScreensaverVolumeResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetDemoModeScreensaverVolumeMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static DemoModeServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<DemoModeServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<DemoModeServiceStub>() {
        @java.lang.Override
        public DemoModeServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new DemoModeServiceStub(channel, callOptions);
        }
      };
    return DemoModeServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static DemoModeServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<DemoModeServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<DemoModeServiceBlockingStub>() {
        @java.lang.Override
        public DemoModeServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new DemoModeServiceBlockingStub(channel, callOptions);
        }
      };
    return DemoModeServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static DemoModeServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<DemoModeServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<DemoModeServiceFutureStub>() {
        @java.lang.Override
        public DemoModeServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new DemoModeServiceFutureStub(channel, callOptions);
        }
      };
    return DemoModeServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void demoModeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoMode> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDemoModeChangedMethod(), responseObserver);
    }

    /**
     */
    default void setDemoMode(com.ifit.glassos.settings.DemoMode request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetDemoModeMethod(), responseObserver);
    }

    /**
     */
    default void getDemoMode(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetDemoModeMethod(), responseObserver);
    }

    /**
     */
    default void setDemoModeScreensaverVolume(com.ifit.glassos.settings.DemoModeScreensaverVolume request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetDemoModeScreensaverVolumeMethod(), responseObserver);
    }

    /**
     */
    default void getDemoModeScreensaverVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetDemoModeScreensaverVolumeMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service DemoModeService.
   */
  public static abstract class DemoModeServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return DemoModeServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service DemoModeService.
   */
  public static final class DemoModeServiceStub
      extends io.grpc.stub.AbstractAsyncStub<DemoModeServiceStub> {
    private DemoModeServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected DemoModeServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new DemoModeServiceStub(channel, callOptions);
    }

    /**
     */
    public void demoModeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoMode> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getDemoModeChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setDemoMode(com.ifit.glassos.settings.DemoMode request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetDemoModeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getDemoMode(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetDemoModeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setDemoModeScreensaverVolume(com.ifit.glassos.settings.DemoModeScreensaverVolume request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetDemoModeScreensaverVolumeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getDemoModeScreensaverVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetDemoModeScreensaverVolumeMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service DemoModeService.
   */
  public static final class DemoModeServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<DemoModeServiceBlockingStub> {
    private DemoModeServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected DemoModeServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new DemoModeServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.DemoMode> demoModeChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getDemoModeChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.DemoModeResult setDemoMode(com.ifit.glassos.settings.DemoMode request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetDemoModeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.DemoModeResult getDemoMode(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetDemoModeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.DemoModeScreensaverVolumeResult setDemoModeScreensaverVolume(com.ifit.glassos.settings.DemoModeScreensaverVolume request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetDemoModeScreensaverVolumeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.DemoModeScreensaverVolumeResult getDemoModeScreensaverVolume(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetDemoModeScreensaverVolumeMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service DemoModeService.
   */
  public static final class DemoModeServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<DemoModeServiceFutureStub> {
    private DemoModeServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected DemoModeServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new DemoModeServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.DemoModeResult> setDemoMode(
        com.ifit.glassos.settings.DemoMode request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetDemoModeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.DemoModeResult> getDemoMode(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetDemoModeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> setDemoModeScreensaverVolume(
        com.ifit.glassos.settings.DemoModeScreensaverVolume request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetDemoModeScreensaverVolumeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult> getDemoModeScreensaverVolume(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetDemoModeScreensaverVolumeMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_DEMO_MODE_CHANGED = 0;
  private static final int METHODID_SET_DEMO_MODE = 1;
  private static final int METHODID_GET_DEMO_MODE = 2;
  private static final int METHODID_SET_DEMO_MODE_SCREENSAVER_VOLUME = 3;
  private static final int METHODID_GET_DEMO_MODE_SCREENSAVER_VOLUME = 4;

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
        case METHODID_DEMO_MODE_CHANGED:
          serviceImpl.demoModeChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoMode>) responseObserver);
          break;
        case METHODID_SET_DEMO_MODE:
          serviceImpl.setDemoMode((com.ifit.glassos.settings.DemoMode) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeResult>) responseObserver);
          break;
        case METHODID_GET_DEMO_MODE:
          serviceImpl.getDemoMode((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeResult>) responseObserver);
          break;
        case METHODID_SET_DEMO_MODE_SCREENSAVER_VOLUME:
          serviceImpl.setDemoModeScreensaverVolume((com.ifit.glassos.settings.DemoModeScreensaverVolume) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult>) responseObserver);
          break;
        case METHODID_GET_DEMO_MODE_SCREENSAVER_VOLUME:
          serviceImpl.getDemoModeScreensaverVolume((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.DemoModeScreensaverVolumeResult>) responseObserver);
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
          getDemoModeChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.DemoMode>(
                service, METHODID_DEMO_MODE_CHANGED)))
        .addMethod(
          getSetDemoModeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.DemoMode,
              com.ifit.glassos.settings.DemoModeResult>(
                service, METHODID_SET_DEMO_MODE)))
        .addMethod(
          getGetDemoModeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.DemoModeResult>(
                service, METHODID_GET_DEMO_MODE)))
        .addMethod(
          getSetDemoModeScreensaverVolumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.DemoModeScreensaverVolume,
              com.ifit.glassos.settings.DemoModeScreensaverVolumeResult>(
                service, METHODID_SET_DEMO_MODE_SCREENSAVER_VOLUME)))
        .addMethod(
          getGetDemoModeScreensaverVolumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.DemoModeScreensaverVolumeResult>(
                service, METHODID_GET_DEMO_MODE_SCREENSAVER_VOLUME)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (DemoModeServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getDemoModeChangedMethod())
              .addMethod(getSetDemoModeMethod())
              .addMethod(getGetDemoModeMethod())
              .addMethod(getSetDemoModeScreensaverVolumeMethod())
              .addMethod(getGetDemoModeScreensaverVolumeMethod())
              .build();
        }
      }
    }
    return result;
  }
}

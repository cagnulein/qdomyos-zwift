package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/BrightnessService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class BrightnessServiceGrpc {

  private BrightnessServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.BrightnessService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.Brightness> getBrightnessChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "BrightnessChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.Brightness.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.Brightness> getBrightnessChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.Brightness> getBrightnessChangedMethod;
    if ((getBrightnessChangedMethod = BrightnessServiceGrpc.getBrightnessChangedMethod) == null) {
      synchronized (BrightnessServiceGrpc.class) {
        if ((getBrightnessChangedMethod = BrightnessServiceGrpc.getBrightnessChangedMethod) == null) {
          BrightnessServiceGrpc.getBrightnessChangedMethod = getBrightnessChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.Brightness>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "BrightnessChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.Brightness.getDefaultInstance()))
              .build();
        }
      }
    }
    return getBrightnessChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.Brightness,
      com.ifit.glassos.settings.BrightnessResult> getSetBrightnessMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetBrightness",
      requestType = com.ifit.glassos.settings.Brightness.class,
      responseType = com.ifit.glassos.settings.BrightnessResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.Brightness,
      com.ifit.glassos.settings.BrightnessResult> getSetBrightnessMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.Brightness, com.ifit.glassos.settings.BrightnessResult> getSetBrightnessMethod;
    if ((getSetBrightnessMethod = BrightnessServiceGrpc.getSetBrightnessMethod) == null) {
      synchronized (BrightnessServiceGrpc.class) {
        if ((getSetBrightnessMethod = BrightnessServiceGrpc.getSetBrightnessMethod) == null) {
          BrightnessServiceGrpc.getSetBrightnessMethod = getSetBrightnessMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.Brightness, com.ifit.glassos.settings.BrightnessResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetBrightness"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.Brightness.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.BrightnessResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetBrightnessMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.BrightnessResult> getGetBrightnessMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetBrightness",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.BrightnessResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.BrightnessResult> getGetBrightnessMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.BrightnessResult> getGetBrightnessMethod;
    if ((getGetBrightnessMethod = BrightnessServiceGrpc.getGetBrightnessMethod) == null) {
      synchronized (BrightnessServiceGrpc.class) {
        if ((getGetBrightnessMethod = BrightnessServiceGrpc.getGetBrightnessMethod) == null) {
          BrightnessServiceGrpc.getGetBrightnessMethod = getGetBrightnessMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.BrightnessResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetBrightness"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.BrightnessResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetBrightnessMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static BrightnessServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<BrightnessServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<BrightnessServiceStub>() {
        @java.lang.Override
        public BrightnessServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new BrightnessServiceStub(channel, callOptions);
        }
      };
    return BrightnessServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static BrightnessServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<BrightnessServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<BrightnessServiceBlockingStub>() {
        @java.lang.Override
        public BrightnessServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new BrightnessServiceBlockingStub(channel, callOptions);
        }
      };
    return BrightnessServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static BrightnessServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<BrightnessServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<BrightnessServiceFutureStub>() {
        @java.lang.Override
        public BrightnessServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new BrightnessServiceFutureStub(channel, callOptions);
        }
      };
    return BrightnessServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void brightnessChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.Brightness> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getBrightnessChangedMethod(), responseObserver);
    }

    /**
     */
    default void setBrightness(com.ifit.glassos.settings.Brightness request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.BrightnessResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetBrightnessMethod(), responseObserver);
    }

    /**
     */
    default void getBrightness(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.BrightnessResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetBrightnessMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service BrightnessService.
   */
  public static abstract class BrightnessServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return BrightnessServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service BrightnessService.
   */
  public static final class BrightnessServiceStub
      extends io.grpc.stub.AbstractAsyncStub<BrightnessServiceStub> {
    private BrightnessServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected BrightnessServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new BrightnessServiceStub(channel, callOptions);
    }

    /**
     */
    public void brightnessChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.Brightness> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getBrightnessChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setBrightness(com.ifit.glassos.settings.Brightness request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.BrightnessResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetBrightnessMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getBrightness(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.BrightnessResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetBrightnessMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service BrightnessService.
   */
  public static final class BrightnessServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<BrightnessServiceBlockingStub> {
    private BrightnessServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected BrightnessServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new BrightnessServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.Brightness> brightnessChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getBrightnessChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.BrightnessResult setBrightness(com.ifit.glassos.settings.Brightness request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetBrightnessMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.BrightnessResult getBrightness(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetBrightnessMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service BrightnessService.
   */
  public static final class BrightnessServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<BrightnessServiceFutureStub> {
    private BrightnessServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected BrightnessServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new BrightnessServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.BrightnessResult> setBrightness(
        com.ifit.glassos.settings.Brightness request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetBrightnessMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.BrightnessResult> getBrightness(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetBrightnessMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_BRIGHTNESS_CHANGED = 0;
  private static final int METHODID_SET_BRIGHTNESS = 1;
  private static final int METHODID_GET_BRIGHTNESS = 2;

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
        case METHODID_BRIGHTNESS_CHANGED:
          serviceImpl.brightnessChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.Brightness>) responseObserver);
          break;
        case METHODID_SET_BRIGHTNESS:
          serviceImpl.setBrightness((com.ifit.glassos.settings.Brightness) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.BrightnessResult>) responseObserver);
          break;
        case METHODID_GET_BRIGHTNESS:
          serviceImpl.getBrightness((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.BrightnessResult>) responseObserver);
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
          getBrightnessChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.Brightness>(
                service, METHODID_BRIGHTNESS_CHANGED)))
        .addMethod(
          getSetBrightnessMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.Brightness,
              com.ifit.glassos.settings.BrightnessResult>(
                service, METHODID_SET_BRIGHTNESS)))
        .addMethod(
          getGetBrightnessMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.BrightnessResult>(
                service, METHODID_GET_BRIGHTNESS)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (BrightnessServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getBrightnessChangedMethod())
              .addMethod(getSetBrightnessMethod())
              .addMethod(getGetBrightnessMethod())
              .build();
        }
      }
    }
    return result;
  }
}

package com.ifit.glassos.console.calibration;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/calibration/InclineCalibrationService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class InclineCalibrationServiceGrpc {

  private InclineCalibrationServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.InclineCalibrationService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getCalibrateInclineMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CalibrateIncline",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getCalibrateInclineMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getCalibrateInclineMethod;
    if ((getCalibrateInclineMethod = InclineCalibrationServiceGrpc.getCalibrateInclineMethod) == null) {
      synchronized (InclineCalibrationServiceGrpc.class) {
        if ((getCalibrateInclineMethod = InclineCalibrationServiceGrpc.getCalibrateInclineMethod) == null) {
          InclineCalibrationServiceGrpc.getCalibrateInclineMethod = getCalibrateInclineMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CalibrateIncline"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCalibrateInclineMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.InclineCalibrationStateResult> getInclineCalibrationStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "InclineCalibrationStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.calibration.InclineCalibrationStateResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.InclineCalibrationStateResult> getInclineCalibrationStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.InclineCalibrationStateResult> getInclineCalibrationStateChangedMethod;
    if ((getInclineCalibrationStateChangedMethod = InclineCalibrationServiceGrpc.getInclineCalibrationStateChangedMethod) == null) {
      synchronized (InclineCalibrationServiceGrpc.class) {
        if ((getInclineCalibrationStateChangedMethod = InclineCalibrationServiceGrpc.getInclineCalibrationStateChangedMethod) == null) {
          InclineCalibrationServiceGrpc.getInclineCalibrationStateChangedMethod = getInclineCalibrationStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.InclineCalibrationStateResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "InclineCalibrationStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.calibration.InclineCalibrationStateResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getInclineCalibrationStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.InclineCalibrationStartedResult> getInclineCalibrationStartedChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "InclineCalibrationStartedChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.calibration.InclineCalibrationStartedResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.InclineCalibrationStartedResult> getInclineCalibrationStartedChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.InclineCalibrationStartedResult> getInclineCalibrationStartedChangedMethod;
    if ((getInclineCalibrationStartedChangedMethod = InclineCalibrationServiceGrpc.getInclineCalibrationStartedChangedMethod) == null) {
      synchronized (InclineCalibrationServiceGrpc.class) {
        if ((getInclineCalibrationStartedChangedMethod = InclineCalibrationServiceGrpc.getInclineCalibrationStartedChangedMethod) == null) {
          InclineCalibrationServiceGrpc.getInclineCalibrationStartedChangedMethod = getInclineCalibrationStartedChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.InclineCalibrationStartedResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "InclineCalibrationStartedChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.calibration.InclineCalibrationStartedResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getInclineCalibrationStartedChangedMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static InclineCalibrationServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<InclineCalibrationServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<InclineCalibrationServiceStub>() {
        @java.lang.Override
        public InclineCalibrationServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new InclineCalibrationServiceStub(channel, callOptions);
        }
      };
    return InclineCalibrationServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static InclineCalibrationServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<InclineCalibrationServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<InclineCalibrationServiceBlockingStub>() {
        @java.lang.Override
        public InclineCalibrationServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new InclineCalibrationServiceBlockingStub(channel, callOptions);
        }
      };
    return InclineCalibrationServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static InclineCalibrationServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<InclineCalibrationServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<InclineCalibrationServiceFutureStub>() {
        @java.lang.Override
        public InclineCalibrationServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new InclineCalibrationServiceFutureStub(channel, callOptions);
        }
      };
    return InclineCalibrationServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void calibrateIncline(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCalibrateInclineMethod(), responseObserver);
    }

    /**
     */
    default void inclineCalibrationStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.InclineCalibrationStateResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getInclineCalibrationStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void inclineCalibrationStartedChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.InclineCalibrationStartedResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getInclineCalibrationStartedChangedMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service InclineCalibrationService.
   */
  public static abstract class InclineCalibrationServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return InclineCalibrationServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service InclineCalibrationService.
   */
  public static final class InclineCalibrationServiceStub
      extends io.grpc.stub.AbstractAsyncStub<InclineCalibrationServiceStub> {
    private InclineCalibrationServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected InclineCalibrationServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new InclineCalibrationServiceStub(channel, callOptions);
    }

    /**
     */
    public void calibrateIncline(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCalibrateInclineMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void inclineCalibrationStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.InclineCalibrationStateResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getInclineCalibrationStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void inclineCalibrationStartedChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.InclineCalibrationStartedResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getInclineCalibrationStartedChangedMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service InclineCalibrationService.
   */
  public static final class InclineCalibrationServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<InclineCalibrationServiceBlockingStub> {
    private InclineCalibrationServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected InclineCalibrationServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new InclineCalibrationServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.Empty calibrateIncline(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCalibrateInclineMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.calibration.InclineCalibrationStateResult> inclineCalibrationStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getInclineCalibrationStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.calibration.InclineCalibrationStartedResult> inclineCalibrationStartedChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getInclineCalibrationStartedChangedMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service InclineCalibrationService.
   */
  public static final class InclineCalibrationServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<InclineCalibrationServiceFutureStub> {
    private InclineCalibrationServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected InclineCalibrationServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new InclineCalibrationServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> calibrateIncline(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCalibrateInclineMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CALIBRATE_INCLINE = 0;
  private static final int METHODID_INCLINE_CALIBRATION_STATE_CHANGED = 1;
  private static final int METHODID_INCLINE_CALIBRATION_STARTED_CHANGED = 2;

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
        case METHODID_CALIBRATE_INCLINE:
          serviceImpl.calibrateIncline((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_INCLINE_CALIBRATION_STATE_CHANGED:
          serviceImpl.inclineCalibrationStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.InclineCalibrationStateResult>) responseObserver);
          break;
        case METHODID_INCLINE_CALIBRATION_STARTED_CHANGED:
          serviceImpl.inclineCalibrationStartedChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.InclineCalibrationStartedResult>) responseObserver);
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
          getCalibrateInclineMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_CALIBRATE_INCLINE)))
        .addMethod(
          getInclineCalibrationStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.calibration.InclineCalibrationStateResult>(
                service, METHODID_INCLINE_CALIBRATION_STATE_CHANGED)))
        .addMethod(
          getInclineCalibrationStartedChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.calibration.InclineCalibrationStartedResult>(
                service, METHODID_INCLINE_CALIBRATION_STARTED_CHANGED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (InclineCalibrationServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCalibrateInclineMethod())
              .addMethod(getInclineCalibrationStateChangedMethod())
              .addMethod(getInclineCalibrationStartedChangedMethod())
              .build();
        }
      }
    }
    return result;
  }
}

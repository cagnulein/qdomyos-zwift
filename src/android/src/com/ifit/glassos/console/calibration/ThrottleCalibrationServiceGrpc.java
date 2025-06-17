package com.ifit.glassos.console.calibration;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/calibration/ThrottleCalibrationService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ThrottleCalibrationServiceGrpc {

  private ThrottleCalibrationServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ThrottleCalibrationService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getIsThrottleCalibrationAvailableMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsThrottleCalibrationAvailable",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getIsThrottleCalibrationAvailableMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getIsThrottleCalibrationAvailableMethod;
    if ((getIsThrottleCalibrationAvailableMethod = ThrottleCalibrationServiceGrpc.getIsThrottleCalibrationAvailableMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getIsThrottleCalibrationAvailableMethod = ThrottleCalibrationServiceGrpc.getIsThrottleCalibrationAvailableMethod) == null) {
          ThrottleCalibrationServiceGrpc.getIsThrottleCalibrationAvailableMethod = getIsThrottleCalibrationAvailableMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsThrottleCalibrationAvailable"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsThrottleCalibrationAvailableMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getCalibrateThrottlesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CalibrateThrottles",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getCalibrateThrottlesMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getCalibrateThrottlesMethod;
    if ((getCalibrateThrottlesMethod = ThrottleCalibrationServiceGrpc.getCalibrateThrottlesMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getCalibrateThrottlesMethod = ThrottleCalibrationServiceGrpc.getCalibrateThrottlesMethod) == null) {
          ThrottleCalibrationServiceGrpc.getCalibrateThrottlesMethod = getCalibrateThrottlesMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CalibrateThrottles"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCalibrateThrottlesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getConfirmThrottleStateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConfirmThrottleState",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getConfirmThrottleStateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getConfirmThrottleStateMethod;
    if ((getConfirmThrottleStateMethod = ThrottleCalibrationServiceGrpc.getConfirmThrottleStateMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getConfirmThrottleStateMethod = ThrottleCalibrationServiceGrpc.getConfirmThrottleStateMethod) == null) {
          ThrottleCalibrationServiceGrpc.getConfirmThrottleStateMethod = getConfirmThrottleStateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConfirmThrottleState"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConfirmThrottleStateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getAbortCalibrateThrottlesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AbortCalibrateThrottles",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getAbortCalibrateThrottlesMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getAbortCalibrateThrottlesMethod;
    if ((getAbortCalibrateThrottlesMethod = ThrottleCalibrationServiceGrpc.getAbortCalibrateThrottlesMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getAbortCalibrateThrottlesMethod = ThrottleCalibrationServiceGrpc.getAbortCalibrateThrottlesMethod) == null) {
          ThrottleCalibrationServiceGrpc.getAbortCalibrateThrottlesMethod = getAbortCalibrateThrottlesMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AbortCalibrateThrottles"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAbortCalibrateThrottlesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult> getThrottleCalibrationStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ThrottleCalibrationStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult> getThrottleCalibrationStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult> getThrottleCalibrationStateChangedMethod;
    if ((getThrottleCalibrationStateChangedMethod = ThrottleCalibrationServiceGrpc.getThrottleCalibrationStateChangedMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getThrottleCalibrationStateChangedMethod = ThrottleCalibrationServiceGrpc.getThrottleCalibrationStateChangedMethod) == null) {
          ThrottleCalibrationServiceGrpc.getThrottleCalibrationStateChangedMethod = getThrottleCalibrationStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ThrottleCalibrationStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getThrottleCalibrationStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getGetThrottleCalibrationValuesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetThrottleCalibrationValues",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.calibration.ThrottleCalibrationValues.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getGetThrottleCalibrationValuesMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getGetThrottleCalibrationValuesMethod;
    if ((getGetThrottleCalibrationValuesMethod = ThrottleCalibrationServiceGrpc.getGetThrottleCalibrationValuesMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getGetThrottleCalibrationValuesMethod = ThrottleCalibrationServiceGrpc.getGetThrottleCalibrationValuesMethod) == null) {
          ThrottleCalibrationServiceGrpc.getGetThrottleCalibrationValuesMethod = getGetThrottleCalibrationValuesMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.ThrottleCalibrationValues>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetThrottleCalibrationValues"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.calibration.ThrottleCalibrationValues.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetThrottleCalibrationValuesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getThrottleCalibrationValuesChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ThrottleCalibrationValuesChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.calibration.ThrottleCalibrationValues.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getThrottleCalibrationValuesChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getThrottleCalibrationValuesChangedMethod;
    if ((getThrottleCalibrationValuesChangedMethod = ThrottleCalibrationServiceGrpc.getThrottleCalibrationValuesChangedMethod) == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        if ((getThrottleCalibrationValuesChangedMethod = ThrottleCalibrationServiceGrpc.getThrottleCalibrationValuesChangedMethod) == null) {
          ThrottleCalibrationServiceGrpc.getThrottleCalibrationValuesChangedMethod = getThrottleCalibrationValuesChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.calibration.ThrottleCalibrationValues>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ThrottleCalibrationValuesChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.calibration.ThrottleCalibrationValues.getDefaultInstance()))
              .build();
        }
      }
    }
    return getThrottleCalibrationValuesChangedMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ThrottleCalibrationServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ThrottleCalibrationServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ThrottleCalibrationServiceStub>() {
        @java.lang.Override
        public ThrottleCalibrationServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ThrottleCalibrationServiceStub(channel, callOptions);
        }
      };
    return ThrottleCalibrationServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ThrottleCalibrationServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ThrottleCalibrationServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ThrottleCalibrationServiceBlockingStub>() {
        @java.lang.Override
        public ThrottleCalibrationServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ThrottleCalibrationServiceBlockingStub(channel, callOptions);
        }
      };
    return ThrottleCalibrationServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ThrottleCalibrationServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ThrottleCalibrationServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ThrottleCalibrationServiceFutureStub>() {
        @java.lang.Override
        public ThrottleCalibrationServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ThrottleCalibrationServiceFutureStub(channel, callOptions);
        }
      };
    return ThrottleCalibrationServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void isThrottleCalibrationAvailable(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsThrottleCalibrationAvailableMethod(), responseObserver);
    }

    /**
     */
    default void calibrateThrottles(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCalibrateThrottlesMethod(), responseObserver);
    }

    /**
     */
    default void confirmThrottleState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConfirmThrottleStateMethod(), responseObserver);
    }

    /**
     */
    default void abortCalibrateThrottles(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAbortCalibrateThrottlesMethod(), responseObserver);
    }

    /**
     */
    default void throttleCalibrationStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getThrottleCalibrationStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void getThrottleCalibrationValues(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationValues> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetThrottleCalibrationValuesMethod(), responseObserver);
    }

    /**
     */
    default void throttleCalibrationValuesChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationValues> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getThrottleCalibrationValuesChangedMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ThrottleCalibrationService.
   */
  public static abstract class ThrottleCalibrationServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ThrottleCalibrationServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ThrottleCalibrationService.
   */
  public static final class ThrottleCalibrationServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ThrottleCalibrationServiceStub> {
    private ThrottleCalibrationServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ThrottleCalibrationServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ThrottleCalibrationServiceStub(channel, callOptions);
    }

    /**
     */
    public void isThrottleCalibrationAvailable(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsThrottleCalibrationAvailableMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void calibrateThrottles(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCalibrateThrottlesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void confirmThrottleState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getConfirmThrottleStateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void abortCalibrateThrottles(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getAbortCalibrateThrottlesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void throttleCalibrationStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getThrottleCalibrationStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getThrottleCalibrationValues(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationValues> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetThrottleCalibrationValuesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void throttleCalibrationValuesChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationValues> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getThrottleCalibrationValuesChangedMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ThrottleCalibrationService.
   */
  public static final class ThrottleCalibrationServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ThrottleCalibrationServiceBlockingStub> {
    private ThrottleCalibrationServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ThrottleCalibrationServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ThrottleCalibrationServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.AvailabilityResponse isThrottleCalibrationAvailable(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsThrottleCalibrationAvailableMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty calibrateThrottles(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCalibrateThrottlesMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty confirmThrottleState(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getConfirmThrottleStateMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty abortCalibrateThrottles(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getAbortCalibrateThrottlesMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult> throttleCalibrationStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getThrottleCalibrationStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.calibration.ThrottleCalibrationValues getThrottleCalibrationValues(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetThrottleCalibrationValuesMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.calibration.ThrottleCalibrationValues> throttleCalibrationValuesChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getThrottleCalibrationValuesChangedMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ThrottleCalibrationService.
   */
  public static final class ThrottleCalibrationServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ThrottleCalibrationServiceFutureStub> {
    private ThrottleCalibrationServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ThrottleCalibrationServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ThrottleCalibrationServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> isThrottleCalibrationAvailable(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsThrottleCalibrationAvailableMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> calibrateThrottles(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCalibrateThrottlesMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> confirmThrottleState(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getConfirmThrottleStateMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> abortCalibrateThrottles(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getAbortCalibrateThrottlesMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.calibration.ThrottleCalibrationValues> getThrottleCalibrationValues(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetThrottleCalibrationValuesMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_IS_THROTTLE_CALIBRATION_AVAILABLE = 0;
  private static final int METHODID_CALIBRATE_THROTTLES = 1;
  private static final int METHODID_CONFIRM_THROTTLE_STATE = 2;
  private static final int METHODID_ABORT_CALIBRATE_THROTTLES = 3;
  private static final int METHODID_THROTTLE_CALIBRATION_STATE_CHANGED = 4;
  private static final int METHODID_GET_THROTTLE_CALIBRATION_VALUES = 5;
  private static final int METHODID_THROTTLE_CALIBRATION_VALUES_CHANGED = 6;

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
        case METHODID_IS_THROTTLE_CALIBRATION_AVAILABLE:
          serviceImpl.isThrottleCalibrationAvailable((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_CALIBRATE_THROTTLES:
          serviceImpl.calibrateThrottles((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_CONFIRM_THROTTLE_STATE:
          serviceImpl.confirmThrottleState((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_ABORT_CALIBRATE_THROTTLES:
          serviceImpl.abortCalibrateThrottles((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_THROTTLE_CALIBRATION_STATE_CHANGED:
          serviceImpl.throttleCalibrationStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult>) responseObserver);
          break;
        case METHODID_GET_THROTTLE_CALIBRATION_VALUES:
          serviceImpl.getThrottleCalibrationValues((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationValues>) responseObserver);
          break;
        case METHODID_THROTTLE_CALIBRATION_VALUES_CHANGED:
          serviceImpl.throttleCalibrationValuesChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.calibration.ThrottleCalibrationValues>) responseObserver);
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
          getIsThrottleCalibrationAvailableMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_IS_THROTTLE_CALIBRATION_AVAILABLE)))
        .addMethod(
          getCalibrateThrottlesMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_CALIBRATE_THROTTLES)))
        .addMethod(
          getConfirmThrottleStateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_CONFIRM_THROTTLE_STATE)))
        .addMethod(
          getAbortCalibrateThrottlesMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_ABORT_CALIBRATE_THROTTLES)))
        .addMethod(
          getThrottleCalibrationStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.calibration.ThrottleCalibrationStateResult>(
                service, METHODID_THROTTLE_CALIBRATION_STATE_CHANGED)))
        .addMethod(
          getGetThrottleCalibrationValuesMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.calibration.ThrottleCalibrationValues>(
                service, METHODID_GET_THROTTLE_CALIBRATION_VALUES)))
        .addMethod(
          getThrottleCalibrationValuesChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.calibration.ThrottleCalibrationValues>(
                service, METHODID_THROTTLE_CALIBRATION_VALUES_CHANGED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ThrottleCalibrationServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getIsThrottleCalibrationAvailableMethod())
              .addMethod(getCalibrateThrottlesMethod())
              .addMethod(getConfirmThrottleStateMethod())
              .addMethod(getAbortCalibrateThrottlesMethod())
              .addMethod(getThrottleCalibrationStateChangedMethod())
              .addMethod(getGetThrottleCalibrationValuesMethod())
              .addMethod(getThrottleCalibrationValuesChangedMethod())
              .build();
        }
      }
    }
    return result;
  }
}

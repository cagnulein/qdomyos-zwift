package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/HeartRateService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class HeartRateServiceGrpc {

  private HeartRateServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.HeartRateService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanReadMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanRead",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanReadMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanReadMethod;
    if ((getCanReadMethod = HeartRateServiceGrpc.getCanReadMethod) == null) {
      synchronized (HeartRateServiceGrpc.class) {
        if ((getCanReadMethod = HeartRateServiceGrpc.getCanReadMethod) == null) {
          HeartRateServiceGrpc.getCanReadMethod = getCanReadMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanRead"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanReadMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanWrite",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanWriteMethod;
    if ((getCanWriteMethod = HeartRateServiceGrpc.getCanWriteMethod) == null) {
      synchronized (HeartRateServiceGrpc.class) {
        if ((getCanWriteMethod = HeartRateServiceGrpc.getCanWriteMethod) == null) {
          HeartRateServiceGrpc.getCanWriteMethod = getCanWriteMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanWrite"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanWriteMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.HeartRateMetric> getGetHeartRateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetHeartRate",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.HeartRateMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.HeartRateMetric> getGetHeartRateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.HeartRateMetric> getGetHeartRateMethod;
    if ((getGetHeartRateMethod = HeartRateServiceGrpc.getGetHeartRateMethod) == null) {
      synchronized (HeartRateServiceGrpc.class) {
        if ((getGetHeartRateMethod = HeartRateServiceGrpc.getGetHeartRateMethod) == null) {
          HeartRateServiceGrpc.getGetHeartRateMethod = getGetHeartRateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.HeartRateMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetHeartRate"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.HeartRateMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetHeartRateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.HeartRateRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetHeartRateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetHeartRate",
      requestType = com.ifit.glassos.workout.HeartRateRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.HeartRateRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetHeartRateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.HeartRateRequest, com.ifit.glassos.workout.WorkoutResult> getSetHeartRateMethod;
    if ((getSetHeartRateMethod = HeartRateServiceGrpc.getSetHeartRateMethod) == null) {
      synchronized (HeartRateServiceGrpc.class) {
        if ((getSetHeartRateMethod = HeartRateServiceGrpc.getSetHeartRateMethod) == null) {
          HeartRateServiceGrpc.getSetHeartRateMethod = getSetHeartRateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.HeartRateRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetHeartRate"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.HeartRateRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetHeartRateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.HeartRateMetric> getHeartRateSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "HeartRateSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.HeartRateMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.HeartRateMetric> getHeartRateSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.HeartRateMetric> getHeartRateSubscriptionMethod;
    if ((getHeartRateSubscriptionMethod = HeartRateServiceGrpc.getHeartRateSubscriptionMethod) == null) {
      synchronized (HeartRateServiceGrpc.class) {
        if ((getHeartRateSubscriptionMethod = HeartRateServiceGrpc.getHeartRateSubscriptionMethod) == null) {
          HeartRateServiceGrpc.getHeartRateSubscriptionMethod = getHeartRateSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.HeartRateMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "HeartRateSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.HeartRateMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getHeartRateSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.HeartRateMetricList> getGetHeartRateHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetHeartRateHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.HeartRateMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.HeartRateMetricList> getGetHeartRateHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.HeartRateMetricList> getGetHeartRateHistoryMethod;
    if ((getGetHeartRateHistoryMethod = HeartRateServiceGrpc.getGetHeartRateHistoryMethod) == null) {
      synchronized (HeartRateServiceGrpc.class) {
        if ((getGetHeartRateHistoryMethod = HeartRateServiceGrpc.getGetHeartRateHistoryMethod) == null) {
          HeartRateServiceGrpc.getGetHeartRateHistoryMethod = getGetHeartRateHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.HeartRateMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetHeartRateHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.HeartRateMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetHeartRateHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static HeartRateServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HeartRateServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HeartRateServiceStub>() {
        @java.lang.Override
        public HeartRateServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HeartRateServiceStub(channel, callOptions);
        }
      };
    return HeartRateServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static HeartRateServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HeartRateServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HeartRateServiceBlockingStub>() {
        @java.lang.Override
        public HeartRateServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HeartRateServiceBlockingStub(channel, callOptions);
        }
      };
    return HeartRateServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static HeartRateServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HeartRateServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HeartRateServiceFutureStub>() {
        @java.lang.Override
        public HeartRateServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HeartRateServiceFutureStub(channel, callOptions);
        }
      };
    return HeartRateServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether HeartRate is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * check whether HeartRate is Writable, like for Virtual Console or from outside apps like Apple Watch
     * </pre>
     */
    default void canWrite(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanWriteMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Heart Rate Metric relative to the current workout
     * </pre>
     */
    default void getHeartRate(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetHeartRateMethod(), responseObserver);
    }

    /**
     */
    default void setHeartRate(com.ifit.glassos.workout.HeartRateRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetHeartRateMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to HeartRate updates relative to the current workout
     * </pre>
     */
    default void heartRateSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getHeartRateSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the HeartRate changes for the given workout ID
     * </pre>
     */
    default void getHeartRateHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetHeartRateHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service HeartRateService.
   */
  public static abstract class HeartRateServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return HeartRateServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service HeartRateService.
   */
  public static final class HeartRateServiceStub
      extends io.grpc.stub.AbstractAsyncStub<HeartRateServiceStub> {
    private HeartRateServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HeartRateServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HeartRateServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether HeartRate is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * check whether HeartRate is Writable, like for Virtual Console or from outside apps like Apple Watch
     * </pre>
     */
    public void canWrite(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanWriteMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Heart Rate Metric relative to the current workout
     * </pre>
     */
    public void getHeartRate(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetHeartRateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setHeartRate(com.ifit.glassos.workout.HeartRateRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetHeartRateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to HeartRate updates relative to the current workout
     * </pre>
     */
    public void heartRateSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getHeartRateSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the HeartRate changes for the given workout ID
     * </pre>
     */
    public void getHeartRateHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetHeartRateHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service HeartRateService.
   */
  public static final class HeartRateServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<HeartRateServiceBlockingStub> {
    private HeartRateServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HeartRateServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HeartRateServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether HeartRate is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * check whether HeartRate is Writable, like for Virtual Console or from outside apps like Apple Watch
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canWrite(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanWriteMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Heart Rate Metric relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.HeartRateMetric getHeartRate(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetHeartRateMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutResult setHeartRate(com.ifit.glassos.workout.HeartRateRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetHeartRateMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to HeartRate updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.HeartRateMetric> heartRateSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getHeartRateSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the HeartRate changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.HeartRateMetricList getHeartRateHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetHeartRateHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service HeartRateService.
   */
  public static final class HeartRateServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<HeartRateServiceFutureStub> {
    private HeartRateServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HeartRateServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HeartRateServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether HeartRate is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * check whether HeartRate is Writable, like for Virtual Console or from outside apps like Apple Watch
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canWrite(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanWriteMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Heart Rate Metric relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.HeartRateMetric> getHeartRate(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetHeartRateMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setHeartRate(
        com.ifit.glassos.workout.HeartRateRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetHeartRateMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the HeartRate changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.HeartRateMetricList> getHeartRateHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetHeartRateHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE = 1;
  private static final int METHODID_GET_HEART_RATE = 2;
  private static final int METHODID_SET_HEART_RATE = 3;
  private static final int METHODID_HEART_RATE_SUBSCRIPTION = 4;
  private static final int METHODID_GET_HEART_RATE_HISTORY = 5;

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
        case METHODID_CAN_READ:
          serviceImpl.canRead((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_CAN_WRITE:
          serviceImpl.canWrite((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_GET_HEART_RATE:
          serviceImpl.getHeartRate((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetric>) responseObserver);
          break;
        case METHODID_SET_HEART_RATE:
          serviceImpl.setHeartRate((com.ifit.glassos.workout.HeartRateRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_HEART_RATE_SUBSCRIPTION:
          serviceImpl.heartRateSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetric>) responseObserver);
          break;
        case METHODID_GET_HEART_RATE_HISTORY:
          serviceImpl.getHeartRateHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.HeartRateMetricList>) responseObserver);
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
          getCanReadMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_READ)))
        .addMethod(
          getCanWriteMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_WRITE)))
        .addMethod(
          getGetHeartRateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.HeartRateMetric>(
                service, METHODID_GET_HEART_RATE)))
        .addMethod(
          getSetHeartRateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.HeartRateRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_HEART_RATE)))
        .addMethod(
          getHeartRateSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.HeartRateMetric>(
                service, METHODID_HEART_RATE_SUBSCRIPTION)))
        .addMethod(
          getGetHeartRateHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.HeartRateMetricList>(
                service, METHODID_GET_HEART_RATE_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (HeartRateServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetHeartRateMethod())
              .addMethod(getSetHeartRateMethod())
              .addMethod(getHeartRateSubscriptionMethod())
              .addMethod(getGetHeartRateHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

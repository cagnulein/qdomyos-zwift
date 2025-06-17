package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/StepCountService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class StepCountServiceGrpc {

  private StepCountServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.StepCountService";

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
    if ((getCanReadMethod = StepCountServiceGrpc.getCanReadMethod) == null) {
      synchronized (StepCountServiceGrpc.class) {
        if ((getCanReadMethod = StepCountServiceGrpc.getCanReadMethod) == null) {
          StepCountServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.StepCountMetric> getGetStepCountMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStepCount",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StepCountMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StepCountMetric> getGetStepCountMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StepCountMetric> getGetStepCountMethod;
    if ((getGetStepCountMethod = StepCountServiceGrpc.getGetStepCountMethod) == null) {
      synchronized (StepCountServiceGrpc.class) {
        if ((getGetStepCountMethod = StepCountServiceGrpc.getGetStepCountMethod) == null) {
          StepCountServiceGrpc.getGetStepCountMethod = getGetStepCountMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StepCountMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStepCount"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StepCountMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStepCountMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StepCountMetric> getStepCountSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StepCountSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StepCountMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StepCountMetric> getStepCountSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StepCountMetric> getStepCountSubscriptionMethod;
    if ((getStepCountSubscriptionMethod = StepCountServiceGrpc.getStepCountSubscriptionMethod) == null) {
      synchronized (StepCountServiceGrpc.class) {
        if ((getStepCountSubscriptionMethod = StepCountServiceGrpc.getStepCountSubscriptionMethod) == null) {
          StepCountServiceGrpc.getStepCountSubscriptionMethod = getStepCountSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StepCountMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StepCountSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StepCountMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStepCountSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StepCountMetricList> getGetStepCountHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStepCountHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.StepCountMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StepCountMetricList> getGetStepCountHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StepCountMetricList> getGetStepCountHistoryMethod;
    if ((getGetStepCountHistoryMethod = StepCountServiceGrpc.getGetStepCountHistoryMethod) == null) {
      synchronized (StepCountServiceGrpc.class) {
        if ((getGetStepCountHistoryMethod = StepCountServiceGrpc.getGetStepCountHistoryMethod) == null) {
          StepCountServiceGrpc.getGetStepCountHistoryMethod = getGetStepCountHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StepCountMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStepCountHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StepCountMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStepCountHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static StepCountServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StepCountServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StepCountServiceStub>() {
        @java.lang.Override
        public StepCountServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StepCountServiceStub(channel, callOptions);
        }
      };
    return StepCountServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static StepCountServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StepCountServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StepCountServiceBlockingStub>() {
        @java.lang.Override
        public StepCountServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StepCountServiceBlockingStub(channel, callOptions);
        }
      };
    return StepCountServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static StepCountServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StepCountServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StepCountServiceFutureStub>() {
        @java.lang.Override
        public StepCountServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StepCountServiceFutureStub(channel, callOptions);
        }
      };
    return StepCountServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether StepCount is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current StepCount relative to the current workout
     * </pre>
     */
    default void getStepCount(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStepCountMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to StepCount updates relative to the current workout
     * </pre>
     */
    default void stepCountSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStepCountSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the StepCount changes for the given workout ID
     * </pre>
     */
    default void getStepCountHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStepCountHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service StepCountService.
   */
  public static abstract class StepCountServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return StepCountServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service StepCountService.
   */
  public static final class StepCountServiceStub
      extends io.grpc.stub.AbstractAsyncStub<StepCountServiceStub> {
    private StepCountServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StepCountServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StepCountServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether StepCount is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current StepCount relative to the current workout
     * </pre>
     */
    public void getStepCount(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStepCountMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to StepCount updates relative to the current workout
     * </pre>
     */
    public void stepCountSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getStepCountSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the StepCount changes for the given workout ID
     * </pre>
     */
    public void getStepCountHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStepCountHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service StepCountService.
   */
  public static final class StepCountServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<StepCountServiceBlockingStub> {
    private StepCountServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StepCountServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StepCountServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether StepCount is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current StepCount relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.StepCountMetric getStepCount(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStepCountMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to StepCount updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.StepCountMetric> stepCountSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getStepCountSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the StepCount changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.StepCountMetricList getStepCountHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStepCountHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service StepCountService.
   */
  public static final class StepCountServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<StepCountServiceFutureStub> {
    private StepCountServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StepCountServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StepCountServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether StepCount is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current StepCount relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StepCountMetric> getStepCount(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStepCountMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the StepCount changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StepCountMetricList> getStepCountHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStepCountHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_STEP_COUNT = 1;
  private static final int METHODID_STEP_COUNT_SUBSCRIPTION = 2;
  private static final int METHODID_GET_STEP_COUNT_HISTORY = 3;

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
        case METHODID_GET_STEP_COUNT:
          serviceImpl.getStepCount((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetric>) responseObserver);
          break;
        case METHODID_STEP_COUNT_SUBSCRIPTION:
          serviceImpl.stepCountSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetric>) responseObserver);
          break;
        case METHODID_GET_STEP_COUNT_HISTORY:
          serviceImpl.getStepCountHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StepCountMetricList>) responseObserver);
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
          getGetStepCountMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StepCountMetric>(
                service, METHODID_GET_STEP_COUNT)))
        .addMethod(
          getStepCountSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StepCountMetric>(
                service, METHODID_STEP_COUNT_SUBSCRIPTION)))
        .addMethod(
          getGetStepCountHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.StepCountMetricList>(
                service, METHODID_GET_STEP_COUNT_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (StepCountServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetStepCountMethod())
              .addMethod(getStepCountSubscriptionMethod())
              .addMethod(getGetStepCountHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

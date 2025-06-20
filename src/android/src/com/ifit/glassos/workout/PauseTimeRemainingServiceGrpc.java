package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/PauseTimeRemainingService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class PauseTimeRemainingServiceGrpc {

  private PauseTimeRemainingServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.PauseTimeRemainingService";

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
    if ((getCanReadMethod = PauseTimeRemainingServiceGrpc.getCanReadMethod) == null) {
      synchronized (PauseTimeRemainingServiceGrpc.class) {
        if ((getCanReadMethod = PauseTimeRemainingServiceGrpc.getCanReadMethod) == null) {
          PauseTimeRemainingServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.PauseTimeRemainingMetric> getGetPauseTimeRemainingMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetPauseTimeRemaining",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.PauseTimeRemainingMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.PauseTimeRemainingMetric> getGetPauseTimeRemainingMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.PauseTimeRemainingMetric> getGetPauseTimeRemainingMethod;
    if ((getGetPauseTimeRemainingMethod = PauseTimeRemainingServiceGrpc.getGetPauseTimeRemainingMethod) == null) {
      synchronized (PauseTimeRemainingServiceGrpc.class) {
        if ((getGetPauseTimeRemainingMethod = PauseTimeRemainingServiceGrpc.getGetPauseTimeRemainingMethod) == null) {
          PauseTimeRemainingServiceGrpc.getGetPauseTimeRemainingMethod = getGetPauseTimeRemainingMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.PauseTimeRemainingMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetPauseTimeRemaining"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.PauseTimeRemainingMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetPauseTimeRemainingMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.PauseTimeRemainingMetric> getPauseTimeRemainingSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "PauseTimeRemainingSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.PauseTimeRemainingMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.PauseTimeRemainingMetric> getPauseTimeRemainingSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.PauseTimeRemainingMetric> getPauseTimeRemainingSubscriptionMethod;
    if ((getPauseTimeRemainingSubscriptionMethod = PauseTimeRemainingServiceGrpc.getPauseTimeRemainingSubscriptionMethod) == null) {
      synchronized (PauseTimeRemainingServiceGrpc.class) {
        if ((getPauseTimeRemainingSubscriptionMethod = PauseTimeRemainingServiceGrpc.getPauseTimeRemainingSubscriptionMethod) == null) {
          PauseTimeRemainingServiceGrpc.getPauseTimeRemainingSubscriptionMethod = getPauseTimeRemainingSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.PauseTimeRemainingMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "PauseTimeRemainingSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.PauseTimeRemainingMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPauseTimeRemainingSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.PauseTimeRemainingMetricList> getGetPauseTimeRemainingHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetPauseTimeRemainingHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.PauseTimeRemainingMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.PauseTimeRemainingMetricList> getGetPauseTimeRemainingHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.PauseTimeRemainingMetricList> getGetPauseTimeRemainingHistoryMethod;
    if ((getGetPauseTimeRemainingHistoryMethod = PauseTimeRemainingServiceGrpc.getGetPauseTimeRemainingHistoryMethod) == null) {
      synchronized (PauseTimeRemainingServiceGrpc.class) {
        if ((getGetPauseTimeRemainingHistoryMethod = PauseTimeRemainingServiceGrpc.getGetPauseTimeRemainingHistoryMethod) == null) {
          PauseTimeRemainingServiceGrpc.getGetPauseTimeRemainingHistoryMethod = getGetPauseTimeRemainingHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.PauseTimeRemainingMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetPauseTimeRemainingHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.PauseTimeRemainingMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetPauseTimeRemainingHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static PauseTimeRemainingServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<PauseTimeRemainingServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<PauseTimeRemainingServiceStub>() {
        @java.lang.Override
        public PauseTimeRemainingServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new PauseTimeRemainingServiceStub(channel, callOptions);
        }
      };
    return PauseTimeRemainingServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static PauseTimeRemainingServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<PauseTimeRemainingServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<PauseTimeRemainingServiceBlockingStub>() {
        @java.lang.Override
        public PauseTimeRemainingServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new PauseTimeRemainingServiceBlockingStub(channel, callOptions);
        }
      };
    return PauseTimeRemainingServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static PauseTimeRemainingServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<PauseTimeRemainingServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<PauseTimeRemainingServiceFutureStub>() {
        @java.lang.Override
        public PauseTimeRemainingServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new PauseTimeRemainingServiceFutureStub(channel, callOptions);
        }
      };
    return PauseTimeRemainingServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether PauseTimeRemaining is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Pause Time Remaining relative to the current workout
     * </pre>
     */
    default void getPauseTimeRemaining(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetPauseTimeRemainingMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to PauseTimeRemaining updates relative to the current workout
     * </pre>
     */
    default void pauseTimeRemainingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPauseTimeRemainingSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the PauseTimeRemaining changes for the given workout ID
     * </pre>
     */
    default void getPauseTimeRemainingHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetPauseTimeRemainingHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service PauseTimeRemainingService.
   */
  public static abstract class PauseTimeRemainingServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return PauseTimeRemainingServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service PauseTimeRemainingService.
   */
  public static final class PauseTimeRemainingServiceStub
      extends io.grpc.stub.AbstractAsyncStub<PauseTimeRemainingServiceStub> {
    private PauseTimeRemainingServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected PauseTimeRemainingServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new PauseTimeRemainingServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether PauseTimeRemaining is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Pause Time Remaining relative to the current workout
     * </pre>
     */
    public void getPauseTimeRemaining(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetPauseTimeRemainingMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to PauseTimeRemaining updates relative to the current workout
     * </pre>
     */
    public void pauseTimeRemainingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getPauseTimeRemainingSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the PauseTimeRemaining changes for the given workout ID
     * </pre>
     */
    public void getPauseTimeRemainingHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetPauseTimeRemainingHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service PauseTimeRemainingService.
   */
  public static final class PauseTimeRemainingServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<PauseTimeRemainingServiceBlockingStub> {
    private PauseTimeRemainingServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected PauseTimeRemainingServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new PauseTimeRemainingServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether PauseTimeRemaining is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Pause Time Remaining relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.PauseTimeRemainingMetric getPauseTimeRemaining(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetPauseTimeRemainingMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to PauseTimeRemaining updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.PauseTimeRemainingMetric> pauseTimeRemainingSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getPauseTimeRemainingSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the PauseTimeRemaining changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.PauseTimeRemainingMetricList getPauseTimeRemainingHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetPauseTimeRemainingHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service PauseTimeRemainingService.
   */
  public static final class PauseTimeRemainingServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<PauseTimeRemainingServiceFutureStub> {
    private PauseTimeRemainingServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected PauseTimeRemainingServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new PauseTimeRemainingServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether PauseTimeRemaining is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Pause Time Remaining relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.PauseTimeRemainingMetric> getPauseTimeRemaining(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetPauseTimeRemainingMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the PauseTimeRemaining changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.PauseTimeRemainingMetricList> getPauseTimeRemainingHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetPauseTimeRemainingHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_PAUSE_TIME_REMAINING = 1;
  private static final int METHODID_PAUSE_TIME_REMAINING_SUBSCRIPTION = 2;
  private static final int METHODID_GET_PAUSE_TIME_REMAINING_HISTORY = 3;

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
        case METHODID_GET_PAUSE_TIME_REMAINING:
          serviceImpl.getPauseTimeRemaining((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetric>) responseObserver);
          break;
        case METHODID_PAUSE_TIME_REMAINING_SUBSCRIPTION:
          serviceImpl.pauseTimeRemainingSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetric>) responseObserver);
          break;
        case METHODID_GET_PAUSE_TIME_REMAINING_HISTORY:
          serviceImpl.getPauseTimeRemainingHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.PauseTimeRemainingMetricList>) responseObserver);
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
          getGetPauseTimeRemainingMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.PauseTimeRemainingMetric>(
                service, METHODID_GET_PAUSE_TIME_REMAINING)))
        .addMethod(
          getPauseTimeRemainingSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.PauseTimeRemainingMetric>(
                service, METHODID_PAUSE_TIME_REMAINING_SUBSCRIPTION)))
        .addMethod(
          getGetPauseTimeRemainingHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.PauseTimeRemainingMetricList>(
                service, METHODID_GET_PAUSE_TIME_REMAINING_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (PauseTimeRemainingServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetPauseTimeRemainingMethod())
              .addMethod(getPauseTimeRemainingSubscriptionMethod())
              .addMethod(getGetPauseTimeRemainingHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/LapTimeService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class LapTimeServiceGrpc {

  private LapTimeServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.LapTimeService";

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
    if ((getCanReadMethod = LapTimeServiceGrpc.getCanReadMethod) == null) {
      synchronized (LapTimeServiceGrpc.class) {
        if ((getCanReadMethod = LapTimeServiceGrpc.getCanReadMethod) == null) {
          LapTimeServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.LapTimeMetric> getGetLapTimeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetLapTime",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.LapTimeMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.LapTimeMetric> getGetLapTimeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.LapTimeMetric> getGetLapTimeMethod;
    if ((getGetLapTimeMethod = LapTimeServiceGrpc.getGetLapTimeMethod) == null) {
      synchronized (LapTimeServiceGrpc.class) {
        if ((getGetLapTimeMethod = LapTimeServiceGrpc.getGetLapTimeMethod) == null) {
          LapTimeServiceGrpc.getGetLapTimeMethod = getGetLapTimeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.LapTimeMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetLapTime"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.LapTimeMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetLapTimeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.LapTimeMetric> getLapTimeSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "LapTimeSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.LapTimeMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.LapTimeMetric> getLapTimeSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.LapTimeMetric> getLapTimeSubscriptionMethod;
    if ((getLapTimeSubscriptionMethod = LapTimeServiceGrpc.getLapTimeSubscriptionMethod) == null) {
      synchronized (LapTimeServiceGrpc.class) {
        if ((getLapTimeSubscriptionMethod = LapTimeServiceGrpc.getLapTimeSubscriptionMethod) == null) {
          LapTimeServiceGrpc.getLapTimeSubscriptionMethod = getLapTimeSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.LapTimeMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "LapTimeSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.LapTimeMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getLapTimeSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.LapTimeMetricList> getGetLapTimeHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetLapTimeHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.LapTimeMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.LapTimeMetricList> getGetLapTimeHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.LapTimeMetricList> getGetLapTimeHistoryMethod;
    if ((getGetLapTimeHistoryMethod = LapTimeServiceGrpc.getGetLapTimeHistoryMethod) == null) {
      synchronized (LapTimeServiceGrpc.class) {
        if ((getGetLapTimeHistoryMethod = LapTimeServiceGrpc.getGetLapTimeHistoryMethod) == null) {
          LapTimeServiceGrpc.getGetLapTimeHistoryMethod = getGetLapTimeHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.LapTimeMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetLapTimeHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.LapTimeMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetLapTimeHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static LapTimeServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<LapTimeServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<LapTimeServiceStub>() {
        @java.lang.Override
        public LapTimeServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new LapTimeServiceStub(channel, callOptions);
        }
      };
    return LapTimeServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static LapTimeServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<LapTimeServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<LapTimeServiceBlockingStub>() {
        @java.lang.Override
        public LapTimeServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new LapTimeServiceBlockingStub(channel, callOptions);
        }
      };
    return LapTimeServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static LapTimeServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<LapTimeServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<LapTimeServiceFutureStub>() {
        @java.lang.Override
        public LapTimeServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new LapTimeServiceFutureStub(channel, callOptions);
        }
      };
    return LapTimeServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether LapTime is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Lap Time relative to the current workout
     * </pre>
     */
    default void getLapTime(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetLapTimeMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to LapTime updates relative to the current workout
     * </pre>
     */
    default void lapTimeSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getLapTimeSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the LapTime changes for the given workout ID
     * </pre>
     */
    default void getLapTimeHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetLapTimeHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service LapTimeService.
   */
  public static abstract class LapTimeServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return LapTimeServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service LapTimeService.
   */
  public static final class LapTimeServiceStub
      extends io.grpc.stub.AbstractAsyncStub<LapTimeServiceStub> {
    private LapTimeServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected LapTimeServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new LapTimeServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether LapTime is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Lap Time relative to the current workout
     * </pre>
     */
    public void getLapTime(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetLapTimeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to LapTime updates relative to the current workout
     * </pre>
     */
    public void lapTimeSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getLapTimeSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the LapTime changes for the given workout ID
     * </pre>
     */
    public void getLapTimeHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetLapTimeHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service LapTimeService.
   */
  public static final class LapTimeServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<LapTimeServiceBlockingStub> {
    private LapTimeServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected LapTimeServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new LapTimeServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether LapTime is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Lap Time relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.LapTimeMetric getLapTime(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetLapTimeMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to LapTime updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.LapTimeMetric> lapTimeSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getLapTimeSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the LapTime changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.LapTimeMetricList getLapTimeHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetLapTimeHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service LapTimeService.
   */
  public static final class LapTimeServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<LapTimeServiceFutureStub> {
    private LapTimeServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected LapTimeServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new LapTimeServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether LapTime is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Lap Time relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.LapTimeMetric> getLapTime(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetLapTimeMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the LapTime changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.LapTimeMetricList> getLapTimeHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetLapTimeHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_LAP_TIME = 1;
  private static final int METHODID_LAP_TIME_SUBSCRIPTION = 2;
  private static final int METHODID_GET_LAP_TIME_HISTORY = 3;

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
        case METHODID_GET_LAP_TIME:
          serviceImpl.getLapTime((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetric>) responseObserver);
          break;
        case METHODID_LAP_TIME_SUBSCRIPTION:
          serviceImpl.lapTimeSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetric>) responseObserver);
          break;
        case METHODID_GET_LAP_TIME_HISTORY:
          serviceImpl.getLapTimeHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.LapTimeMetricList>) responseObserver);
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
          getGetLapTimeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.LapTimeMetric>(
                service, METHODID_GET_LAP_TIME)))
        .addMethod(
          getLapTimeSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.LapTimeMetric>(
                service, METHODID_LAP_TIME_SUBSCRIPTION)))
        .addMethod(
          getGetLapTimeHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.LapTimeMetricList>(
                service, METHODID_GET_LAP_TIME_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (LapTimeServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetLapTimeMethod())
              .addMethod(getLapTimeSubscriptionMethod())
              .addMethod(getGetLapTimeHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

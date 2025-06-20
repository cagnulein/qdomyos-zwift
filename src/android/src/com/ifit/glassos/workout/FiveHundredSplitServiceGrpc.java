package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/FiveHundredSplitService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class FiveHundredSplitServiceGrpc {

  private FiveHundredSplitServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.FiveHundredSplitService";

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
    if ((getCanReadMethod = FiveHundredSplitServiceGrpc.getCanReadMethod) == null) {
      synchronized (FiveHundredSplitServiceGrpc.class) {
        if ((getCanReadMethod = FiveHundredSplitServiceGrpc.getCanReadMethod) == null) {
          FiveHundredSplitServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.FiveHundredSplitMetric> getGetFiveHundredSplitMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetFiveHundredSplit",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.FiveHundredSplitMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.FiveHundredSplitMetric> getGetFiveHundredSplitMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.FiveHundredSplitMetric> getGetFiveHundredSplitMethod;
    if ((getGetFiveHundredSplitMethod = FiveHundredSplitServiceGrpc.getGetFiveHundredSplitMethod) == null) {
      synchronized (FiveHundredSplitServiceGrpc.class) {
        if ((getGetFiveHundredSplitMethod = FiveHundredSplitServiceGrpc.getGetFiveHundredSplitMethod) == null) {
          FiveHundredSplitServiceGrpc.getGetFiveHundredSplitMethod = getGetFiveHundredSplitMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.FiveHundredSplitMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetFiveHundredSplit"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.FiveHundredSplitMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetFiveHundredSplitMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.FiveHundredSplitMetric> getFiveHundredSplitSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "FiveHundredSplitSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.FiveHundredSplitMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.FiveHundredSplitMetric> getFiveHundredSplitSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.FiveHundredSplitMetric> getFiveHundredSplitSubscriptionMethod;
    if ((getFiveHundredSplitSubscriptionMethod = FiveHundredSplitServiceGrpc.getFiveHundredSplitSubscriptionMethod) == null) {
      synchronized (FiveHundredSplitServiceGrpc.class) {
        if ((getFiveHundredSplitSubscriptionMethod = FiveHundredSplitServiceGrpc.getFiveHundredSplitSubscriptionMethod) == null) {
          FiveHundredSplitServiceGrpc.getFiveHundredSplitSubscriptionMethod = getFiveHundredSplitSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.FiveHundredSplitMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "FiveHundredSplitSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.FiveHundredSplitMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getFiveHundredSplitSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.FiveHundredSplitMetricList> getGetFiveHundredSplitHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetFiveHundredSplitHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.FiveHundredSplitMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.FiveHundredSplitMetricList> getGetFiveHundredSplitHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.FiveHundredSplitMetricList> getGetFiveHundredSplitHistoryMethod;
    if ((getGetFiveHundredSplitHistoryMethod = FiveHundredSplitServiceGrpc.getGetFiveHundredSplitHistoryMethod) == null) {
      synchronized (FiveHundredSplitServiceGrpc.class) {
        if ((getGetFiveHundredSplitHistoryMethod = FiveHundredSplitServiceGrpc.getGetFiveHundredSplitHistoryMethod) == null) {
          FiveHundredSplitServiceGrpc.getGetFiveHundredSplitHistoryMethod = getGetFiveHundredSplitHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.FiveHundredSplitMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetFiveHundredSplitHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.FiveHundredSplitMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetFiveHundredSplitHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static FiveHundredSplitServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FiveHundredSplitServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FiveHundredSplitServiceStub>() {
        @java.lang.Override
        public FiveHundredSplitServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FiveHundredSplitServiceStub(channel, callOptions);
        }
      };
    return FiveHundredSplitServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static FiveHundredSplitServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FiveHundredSplitServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FiveHundredSplitServiceBlockingStub>() {
        @java.lang.Override
        public FiveHundredSplitServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FiveHundredSplitServiceBlockingStub(channel, callOptions);
        }
      };
    return FiveHundredSplitServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static FiveHundredSplitServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FiveHundredSplitServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FiveHundredSplitServiceFutureStub>() {
        @java.lang.Override
        public FiveHundredSplitServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FiveHundredSplitServiceFutureStub(channel, callOptions);
        }
      };
    return FiveHundredSplitServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether FiveHundredSplit is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current FiveHundredSplit relative to the current workout
     * </pre>
     */
    default void getFiveHundredSplit(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetFiveHundredSplitMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to FiveHundredSplit updates relative to the current workout
     * </pre>
     */
    default void fiveHundredSplitSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getFiveHundredSplitSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the FiveHundredSplit changes for the given workout ID
     * </pre>
     */
    default void getFiveHundredSplitHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetFiveHundredSplitHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service FiveHundredSplitService.
   */
  public static abstract class FiveHundredSplitServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return FiveHundredSplitServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service FiveHundredSplitService.
   */
  public static final class FiveHundredSplitServiceStub
      extends io.grpc.stub.AbstractAsyncStub<FiveHundredSplitServiceStub> {
    private FiveHundredSplitServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FiveHundredSplitServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FiveHundredSplitServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether FiveHundredSplit is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current FiveHundredSplit relative to the current workout
     * </pre>
     */
    public void getFiveHundredSplit(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetFiveHundredSplitMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to FiveHundredSplit updates relative to the current workout
     * </pre>
     */
    public void fiveHundredSplitSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getFiveHundredSplitSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the FiveHundredSplit changes for the given workout ID
     * </pre>
     */
    public void getFiveHundredSplitHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetFiveHundredSplitHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service FiveHundredSplitService.
   */
  public static final class FiveHundredSplitServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<FiveHundredSplitServiceBlockingStub> {
    private FiveHundredSplitServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FiveHundredSplitServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FiveHundredSplitServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether FiveHundredSplit is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current FiveHundredSplit relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.FiveHundredSplitMetric getFiveHundredSplit(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetFiveHundredSplitMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to FiveHundredSplit updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.FiveHundredSplitMetric> fiveHundredSplitSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getFiveHundredSplitSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the FiveHundredSplit changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.FiveHundredSplitMetricList getFiveHundredSplitHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetFiveHundredSplitHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service FiveHundredSplitService.
   */
  public static final class FiveHundredSplitServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<FiveHundredSplitServiceFutureStub> {
    private FiveHundredSplitServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FiveHundredSplitServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FiveHundredSplitServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether FiveHundredSplit is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current FiveHundredSplit relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.FiveHundredSplitMetric> getFiveHundredSplit(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetFiveHundredSplitMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the FiveHundredSplit changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.FiveHundredSplitMetricList> getFiveHundredSplitHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetFiveHundredSplitHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_FIVE_HUNDRED_SPLIT = 1;
  private static final int METHODID_FIVE_HUNDRED_SPLIT_SUBSCRIPTION = 2;
  private static final int METHODID_GET_FIVE_HUNDRED_SPLIT_HISTORY = 3;

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
        case METHODID_GET_FIVE_HUNDRED_SPLIT:
          serviceImpl.getFiveHundredSplit((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetric>) responseObserver);
          break;
        case METHODID_FIVE_HUNDRED_SPLIT_SUBSCRIPTION:
          serviceImpl.fiveHundredSplitSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetric>) responseObserver);
          break;
        case METHODID_GET_FIVE_HUNDRED_SPLIT_HISTORY:
          serviceImpl.getFiveHundredSplitHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.FiveHundredSplitMetricList>) responseObserver);
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
          getGetFiveHundredSplitMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.FiveHundredSplitMetric>(
                service, METHODID_GET_FIVE_HUNDRED_SPLIT)))
        .addMethod(
          getFiveHundredSplitSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.FiveHundredSplitMetric>(
                service, METHODID_FIVE_HUNDRED_SPLIT_SUBSCRIPTION)))
        .addMethod(
          getGetFiveHundredSplitHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.FiveHundredSplitMetricList>(
                service, METHODID_GET_FIVE_HUNDRED_SPLIT_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (FiveHundredSplitServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetFiveHundredSplitMethod())
              .addMethod(getFiveHundredSplitSubscriptionMethod())
              .addMethod(getGetFiveHundredSplitHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

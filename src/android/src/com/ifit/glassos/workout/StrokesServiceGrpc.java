package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/StrokesService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class StrokesServiceGrpc {

  private StrokesServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.StrokesService";

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
    if ((getCanReadMethod = StrokesServiceGrpc.getCanReadMethod) == null) {
      synchronized (StrokesServiceGrpc.class) {
        if ((getCanReadMethod = StrokesServiceGrpc.getCanReadMethod) == null) {
          StrokesServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.StrokesMetric> getGetStrokesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStrokes",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StrokesMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StrokesMetric> getGetStrokesMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesMetric> getGetStrokesMethod;
    if ((getGetStrokesMethod = StrokesServiceGrpc.getGetStrokesMethod) == null) {
      synchronized (StrokesServiceGrpc.class) {
        if ((getGetStrokesMethod = StrokesServiceGrpc.getGetStrokesMethod) == null) {
          StrokesServiceGrpc.getGetStrokesMethod = getGetStrokesMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStrokes"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStrokesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StrokesMetric> getStrokesSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StrokesSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StrokesMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StrokesMetric> getStrokesSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesMetric> getStrokesSubscriptionMethod;
    if ((getStrokesSubscriptionMethod = StrokesServiceGrpc.getStrokesSubscriptionMethod) == null) {
      synchronized (StrokesServiceGrpc.class) {
        if ((getStrokesSubscriptionMethod = StrokesServiceGrpc.getStrokesSubscriptionMethod) == null) {
          StrokesServiceGrpc.getStrokesSubscriptionMethod = getStrokesSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StrokesSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStrokesSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StrokesMetricList> getGetStrokesHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStrokesHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.StrokesMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StrokesMetricList> getGetStrokesHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StrokesMetricList> getGetStrokesHistoryMethod;
    if ((getGetStrokesHistoryMethod = StrokesServiceGrpc.getGetStrokesHistoryMethod) == null) {
      synchronized (StrokesServiceGrpc.class) {
        if ((getGetStrokesHistoryMethod = StrokesServiceGrpc.getGetStrokesHistoryMethod) == null) {
          StrokesServiceGrpc.getGetStrokesHistoryMethod = getGetStrokesHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StrokesMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStrokesHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStrokesHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static StrokesServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StrokesServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StrokesServiceStub>() {
        @java.lang.Override
        public StrokesServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StrokesServiceStub(channel, callOptions);
        }
      };
    return StrokesServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static StrokesServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StrokesServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StrokesServiceBlockingStub>() {
        @java.lang.Override
        public StrokesServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StrokesServiceBlockingStub(channel, callOptions);
        }
      };
    return StrokesServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static StrokesServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StrokesServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StrokesServiceFutureStub>() {
        @java.lang.Override
        public StrokesServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StrokesServiceFutureStub(channel, callOptions);
        }
      };
    return StrokesServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Strokes is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Strokes relative to the current workout
     * </pre>
     */
    default void getStrokes(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStrokesMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Strokes updates relative to the current workout
     * </pre>
     */
    default void strokesSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStrokesSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Strokes changes for the given workout ID
     * </pre>
     */
    default void getStrokesHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStrokesHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service StrokesService.
   */
  public static abstract class StrokesServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return StrokesServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service StrokesService.
   */
  public static final class StrokesServiceStub
      extends io.grpc.stub.AbstractAsyncStub<StrokesServiceStub> {
    private StrokesServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StrokesServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StrokesServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Strokes is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Strokes relative to the current workout
     * </pre>
     */
    public void getStrokes(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStrokesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Strokes updates relative to the current workout
     * </pre>
     */
    public void strokesSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getStrokesSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Strokes changes for the given workout ID
     * </pre>
     */
    public void getStrokesHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStrokesHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service StrokesService.
   */
  public static final class StrokesServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<StrokesServiceBlockingStub> {
    private StrokesServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StrokesServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StrokesServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Strokes is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Strokes relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.StrokesMetric getStrokes(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStrokesMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Strokes updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.StrokesMetric> strokesSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getStrokesSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Strokes changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.StrokesMetricList getStrokesHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStrokesHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service StrokesService.
   */
  public static final class StrokesServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<StrokesServiceFutureStub> {
    private StrokesServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StrokesServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StrokesServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Strokes is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Strokes relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StrokesMetric> getStrokes(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStrokesMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Strokes changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StrokesMetricList> getStrokesHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStrokesHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_STROKES = 1;
  private static final int METHODID_STROKES_SUBSCRIPTION = 2;
  private static final int METHODID_GET_STROKES_HISTORY = 3;

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
        case METHODID_GET_STROKES:
          serviceImpl.getStrokes((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetric>) responseObserver);
          break;
        case METHODID_STROKES_SUBSCRIPTION:
          serviceImpl.strokesSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetric>) responseObserver);
          break;
        case METHODID_GET_STROKES_HISTORY:
          serviceImpl.getStrokesHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesMetricList>) responseObserver);
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
          getGetStrokesMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StrokesMetric>(
                service, METHODID_GET_STROKES)))
        .addMethod(
          getStrokesSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StrokesMetric>(
                service, METHODID_STROKES_SUBSCRIPTION)))
        .addMethod(
          getGetStrokesHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.StrokesMetricList>(
                service, METHODID_GET_STROKES_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (StrokesServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetStrokesMethod())
              .addMethod(getStrokesSubscriptionMethod())
              .addMethod(getGetStrokesHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

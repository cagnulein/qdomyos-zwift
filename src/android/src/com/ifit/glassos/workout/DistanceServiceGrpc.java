package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/DistanceService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class DistanceServiceGrpc {

  private DistanceServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.DistanceService";

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
    if ((getCanReadMethod = DistanceServiceGrpc.getCanReadMethod) == null) {
      synchronized (DistanceServiceGrpc.class) {
        if ((getCanReadMethod = DistanceServiceGrpc.getCanReadMethod) == null) {
          DistanceServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.DistanceMetric> getGetDistanceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetDistance",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.DistanceMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.DistanceMetric> getGetDistanceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.DistanceMetric> getGetDistanceMethod;
    if ((getGetDistanceMethod = DistanceServiceGrpc.getGetDistanceMethod) == null) {
      synchronized (DistanceServiceGrpc.class) {
        if ((getGetDistanceMethod = DistanceServiceGrpc.getGetDistanceMethod) == null) {
          DistanceServiceGrpc.getGetDistanceMethod = getGetDistanceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.DistanceMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetDistance"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.DistanceMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetDistanceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.DistanceMetric> getDistanceSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "DistanceSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.DistanceMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.DistanceMetric> getDistanceSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.DistanceMetric> getDistanceSubscriptionMethod;
    if ((getDistanceSubscriptionMethod = DistanceServiceGrpc.getDistanceSubscriptionMethod) == null) {
      synchronized (DistanceServiceGrpc.class) {
        if ((getDistanceSubscriptionMethod = DistanceServiceGrpc.getDistanceSubscriptionMethod) == null) {
          DistanceServiceGrpc.getDistanceSubscriptionMethod = getDistanceSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.DistanceMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "DistanceSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.DistanceMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDistanceSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.DistanceMetricList> getGetDistanceHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetDistanceHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.DistanceMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.DistanceMetricList> getGetDistanceHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.DistanceMetricList> getGetDistanceHistoryMethod;
    if ((getGetDistanceHistoryMethod = DistanceServiceGrpc.getGetDistanceHistoryMethod) == null) {
      synchronized (DistanceServiceGrpc.class) {
        if ((getGetDistanceHistoryMethod = DistanceServiceGrpc.getGetDistanceHistoryMethod) == null) {
          DistanceServiceGrpc.getGetDistanceHistoryMethod = getGetDistanceHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.DistanceMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetDistanceHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.DistanceMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetDistanceHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static DistanceServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<DistanceServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<DistanceServiceStub>() {
        @java.lang.Override
        public DistanceServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new DistanceServiceStub(channel, callOptions);
        }
      };
    return DistanceServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static DistanceServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<DistanceServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<DistanceServiceBlockingStub>() {
        @java.lang.Override
        public DistanceServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new DistanceServiceBlockingStub(channel, callOptions);
        }
      };
    return DistanceServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static DistanceServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<DistanceServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<DistanceServiceFutureStub>() {
        @java.lang.Override
        public DistanceServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new DistanceServiceFutureStub(channel, callOptions);
        }
      };
    return DistanceServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Distance is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Distance relative to the current workout
     * </pre>
     */
    default void getDistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetDistanceMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Distance updates relative to the current workout
     * </pre>
     */
    default void distanceSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDistanceSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Distance changes for the given workout ID
     * </pre>
     */
    default void getDistanceHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetDistanceHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service DistanceService.
   */
  public static abstract class DistanceServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return DistanceServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service DistanceService.
   */
  public static final class DistanceServiceStub
      extends io.grpc.stub.AbstractAsyncStub<DistanceServiceStub> {
    private DistanceServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected DistanceServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new DistanceServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Distance is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Distance relative to the current workout
     * </pre>
     */
    public void getDistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetDistanceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Distance updates relative to the current workout
     * </pre>
     */
    public void distanceSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getDistanceSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Distance changes for the given workout ID
     * </pre>
     */
    public void getDistanceHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetDistanceHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service DistanceService.
   */
  public static final class DistanceServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<DistanceServiceBlockingStub> {
    private DistanceServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected DistanceServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new DistanceServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Distance is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Distance relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.DistanceMetric getDistance(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetDistanceMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Distance updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.DistanceMetric> distanceSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getDistanceSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Distance changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.DistanceMetricList getDistanceHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetDistanceHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service DistanceService.
   */
  public static final class DistanceServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<DistanceServiceFutureStub> {
    private DistanceServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected DistanceServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new DistanceServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Distance is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Distance relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.DistanceMetric> getDistance(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetDistanceMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Distance changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.DistanceMetricList> getDistanceHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetDistanceHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_DISTANCE = 1;
  private static final int METHODID_DISTANCE_SUBSCRIPTION = 2;
  private static final int METHODID_GET_DISTANCE_HISTORY = 3;

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
        case METHODID_GET_DISTANCE:
          serviceImpl.getDistance((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetric>) responseObserver);
          break;
        case METHODID_DISTANCE_SUBSCRIPTION:
          serviceImpl.distanceSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetric>) responseObserver);
          break;
        case METHODID_GET_DISTANCE_HISTORY:
          serviceImpl.getDistanceHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.DistanceMetricList>) responseObserver);
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
          getGetDistanceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.DistanceMetric>(
                service, METHODID_GET_DISTANCE)))
        .addMethod(
          getDistanceSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.DistanceMetric>(
                service, METHODID_DISTANCE_SUBSCRIPTION)))
        .addMethod(
          getGetDistanceHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.DistanceMetricList>(
                service, METHODID_GET_DISTANCE_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (DistanceServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetDistanceMethod())
              .addMethod(getDistanceSubscriptionMethod())
              .addMethod(getGetDistanceHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

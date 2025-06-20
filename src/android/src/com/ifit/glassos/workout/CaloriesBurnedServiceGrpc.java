package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/CaloriesBurnedService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class CaloriesBurnedServiceGrpc {

  private CaloriesBurnedServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.CaloriesBurnedService";

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
    if ((getCanReadMethod = CaloriesBurnedServiceGrpc.getCanReadMethod) == null) {
      synchronized (CaloriesBurnedServiceGrpc.class) {
        if ((getCanReadMethod = CaloriesBurnedServiceGrpc.getCanReadMethod) == null) {
          CaloriesBurnedServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.CaloriesBurnedMetric> getGetCaloriesBurnedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCaloriesBurned",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.CaloriesBurnedMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.CaloriesBurnedMetric> getGetCaloriesBurnedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesBurnedMetric> getGetCaloriesBurnedMethod;
    if ((getGetCaloriesBurnedMethod = CaloriesBurnedServiceGrpc.getGetCaloriesBurnedMethod) == null) {
      synchronized (CaloriesBurnedServiceGrpc.class) {
        if ((getGetCaloriesBurnedMethod = CaloriesBurnedServiceGrpc.getGetCaloriesBurnedMethod) == null) {
          CaloriesBurnedServiceGrpc.getGetCaloriesBurnedMethod = getGetCaloriesBurnedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesBurnedMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCaloriesBurned"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.CaloriesBurnedMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCaloriesBurnedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.CaloriesBurnedMetric> getCaloriesBurnedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CaloriesBurnedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.CaloriesBurnedMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.CaloriesBurnedMetric> getCaloriesBurnedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesBurnedMetric> getCaloriesBurnedSubscriptionMethod;
    if ((getCaloriesBurnedSubscriptionMethod = CaloriesBurnedServiceGrpc.getCaloriesBurnedSubscriptionMethod) == null) {
      synchronized (CaloriesBurnedServiceGrpc.class) {
        if ((getCaloriesBurnedSubscriptionMethod = CaloriesBurnedServiceGrpc.getCaloriesBurnedSubscriptionMethod) == null) {
          CaloriesBurnedServiceGrpc.getCaloriesBurnedSubscriptionMethod = getCaloriesBurnedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesBurnedMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CaloriesBurnedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.CaloriesBurnedMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCaloriesBurnedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.CaloriesBurnedMetricList> getGetCaloriesBurnedHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCaloriesBurnedHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.CaloriesBurnedMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.CaloriesBurnedMetricList> getGetCaloriesBurnedHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.CaloriesBurnedMetricList> getGetCaloriesBurnedHistoryMethod;
    if ((getGetCaloriesBurnedHistoryMethod = CaloriesBurnedServiceGrpc.getGetCaloriesBurnedHistoryMethod) == null) {
      synchronized (CaloriesBurnedServiceGrpc.class) {
        if ((getGetCaloriesBurnedHistoryMethod = CaloriesBurnedServiceGrpc.getGetCaloriesBurnedHistoryMethod) == null) {
          CaloriesBurnedServiceGrpc.getGetCaloriesBurnedHistoryMethod = getGetCaloriesBurnedHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.CaloriesBurnedMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCaloriesBurnedHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.CaloriesBurnedMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCaloriesBurnedHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static CaloriesBurnedServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CaloriesBurnedServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CaloriesBurnedServiceStub>() {
        @java.lang.Override
        public CaloriesBurnedServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CaloriesBurnedServiceStub(channel, callOptions);
        }
      };
    return CaloriesBurnedServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static CaloriesBurnedServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CaloriesBurnedServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CaloriesBurnedServiceBlockingStub>() {
        @java.lang.Override
        public CaloriesBurnedServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CaloriesBurnedServiceBlockingStub(channel, callOptions);
        }
      };
    return CaloriesBurnedServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static CaloriesBurnedServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CaloriesBurnedServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CaloriesBurnedServiceFutureStub>() {
        @java.lang.Override
        public CaloriesBurnedServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CaloriesBurnedServiceFutureStub(channel, callOptions);
        }
      };
    return CaloriesBurnedServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether CaloriesBurned is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current CaloriesBurned relative to the current workout
     * </pre>
     */
    default void getCaloriesBurned(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCaloriesBurnedMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to CaloriesBurned updates relative to the current workout
     * </pre>
     */
    default void caloriesBurnedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCaloriesBurnedSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesBurned changes for the given workout ID
     * </pre>
     */
    default void getCaloriesBurnedHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCaloriesBurnedHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service CaloriesBurnedService.
   */
  public static abstract class CaloriesBurnedServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return CaloriesBurnedServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service CaloriesBurnedService.
   */
  public static final class CaloriesBurnedServiceStub
      extends io.grpc.stub.AbstractAsyncStub<CaloriesBurnedServiceStub> {
    private CaloriesBurnedServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CaloriesBurnedServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CaloriesBurnedServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether CaloriesBurned is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current CaloriesBurned relative to the current workout
     * </pre>
     */
    public void getCaloriesBurned(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCaloriesBurnedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to CaloriesBurned updates relative to the current workout
     * </pre>
     */
    public void caloriesBurnedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getCaloriesBurnedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesBurned changes for the given workout ID
     * </pre>
     */
    public void getCaloriesBurnedHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCaloriesBurnedHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service CaloriesBurnedService.
   */
  public static final class CaloriesBurnedServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<CaloriesBurnedServiceBlockingStub> {
    private CaloriesBurnedServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CaloriesBurnedServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CaloriesBurnedServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether CaloriesBurned is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current CaloriesBurned relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.CaloriesBurnedMetric getCaloriesBurned(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCaloriesBurnedMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to CaloriesBurned updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.CaloriesBurnedMetric> caloriesBurnedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getCaloriesBurnedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesBurned changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.CaloriesBurnedMetricList getCaloriesBurnedHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCaloriesBurnedHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service CaloriesBurnedService.
   */
  public static final class CaloriesBurnedServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<CaloriesBurnedServiceFutureStub> {
    private CaloriesBurnedServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CaloriesBurnedServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CaloriesBurnedServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether CaloriesBurned is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current CaloriesBurned relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.CaloriesBurnedMetric> getCaloriesBurned(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCaloriesBurnedMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesBurned changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.CaloriesBurnedMetricList> getCaloriesBurnedHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCaloriesBurnedHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_CALORIES_BURNED = 1;
  private static final int METHODID_CALORIES_BURNED_SUBSCRIPTION = 2;
  private static final int METHODID_GET_CALORIES_BURNED_HISTORY = 3;

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
        case METHODID_GET_CALORIES_BURNED:
          serviceImpl.getCaloriesBurned((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetric>) responseObserver);
          break;
        case METHODID_CALORIES_BURNED_SUBSCRIPTION:
          serviceImpl.caloriesBurnedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetric>) responseObserver);
          break;
        case METHODID_GET_CALORIES_BURNED_HISTORY:
          serviceImpl.getCaloriesBurnedHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesBurnedMetricList>) responseObserver);
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
          getGetCaloriesBurnedMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.CaloriesBurnedMetric>(
                service, METHODID_GET_CALORIES_BURNED)))
        .addMethod(
          getCaloriesBurnedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.CaloriesBurnedMetric>(
                service, METHODID_CALORIES_BURNED_SUBSCRIPTION)))
        .addMethod(
          getGetCaloriesBurnedHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.CaloriesBurnedMetricList>(
                service, METHODID_GET_CALORIES_BURNED_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (CaloriesBurnedServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetCaloriesBurnedMethod())
              .addMethod(getCaloriesBurnedSubscriptionMethod())
              .addMethod(getGetCaloriesBurnedHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

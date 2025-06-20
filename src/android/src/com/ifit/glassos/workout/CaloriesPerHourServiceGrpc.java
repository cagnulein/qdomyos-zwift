package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/CaloriesPerHourService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class CaloriesPerHourServiceGrpc {

  private CaloriesPerHourServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.CaloriesPerHourService";

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
    if ((getCanReadMethod = CaloriesPerHourServiceGrpc.getCanReadMethod) == null) {
      synchronized (CaloriesPerHourServiceGrpc.class) {
        if ((getCanReadMethod = CaloriesPerHourServiceGrpc.getCanReadMethod) == null) {
          CaloriesPerHourServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.CaloriesPerHourMetric> getGetCaloriesPerHourMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCaloriesPerHour",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.CaloriesPerHourMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.CaloriesPerHourMetric> getGetCaloriesPerHourMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesPerHourMetric> getGetCaloriesPerHourMethod;
    if ((getGetCaloriesPerHourMethod = CaloriesPerHourServiceGrpc.getGetCaloriesPerHourMethod) == null) {
      synchronized (CaloriesPerHourServiceGrpc.class) {
        if ((getGetCaloriesPerHourMethod = CaloriesPerHourServiceGrpc.getGetCaloriesPerHourMethod) == null) {
          CaloriesPerHourServiceGrpc.getGetCaloriesPerHourMethod = getGetCaloriesPerHourMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesPerHourMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCaloriesPerHour"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.CaloriesPerHourMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCaloriesPerHourMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.CaloriesPerHourMetric> getCaloriesPerHourSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CaloriesPerHourSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.CaloriesPerHourMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.CaloriesPerHourMetric> getCaloriesPerHourSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesPerHourMetric> getCaloriesPerHourSubscriptionMethod;
    if ((getCaloriesPerHourSubscriptionMethod = CaloriesPerHourServiceGrpc.getCaloriesPerHourSubscriptionMethod) == null) {
      synchronized (CaloriesPerHourServiceGrpc.class) {
        if ((getCaloriesPerHourSubscriptionMethod = CaloriesPerHourServiceGrpc.getCaloriesPerHourSubscriptionMethod) == null) {
          CaloriesPerHourServiceGrpc.getCaloriesPerHourSubscriptionMethod = getCaloriesPerHourSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.CaloriesPerHourMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CaloriesPerHourSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.CaloriesPerHourMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCaloriesPerHourSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.CaloriesPerHourMetricList> getGetCaloriesPerHourHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCaloriesPerHourHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.CaloriesPerHourMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.CaloriesPerHourMetricList> getGetCaloriesPerHourHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.CaloriesPerHourMetricList> getGetCaloriesPerHourHistoryMethod;
    if ((getGetCaloriesPerHourHistoryMethod = CaloriesPerHourServiceGrpc.getGetCaloriesPerHourHistoryMethod) == null) {
      synchronized (CaloriesPerHourServiceGrpc.class) {
        if ((getGetCaloriesPerHourHistoryMethod = CaloriesPerHourServiceGrpc.getGetCaloriesPerHourHistoryMethod) == null) {
          CaloriesPerHourServiceGrpc.getGetCaloriesPerHourHistoryMethod = getGetCaloriesPerHourHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.CaloriesPerHourMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCaloriesPerHourHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.CaloriesPerHourMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCaloriesPerHourHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static CaloriesPerHourServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CaloriesPerHourServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CaloriesPerHourServiceStub>() {
        @java.lang.Override
        public CaloriesPerHourServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CaloriesPerHourServiceStub(channel, callOptions);
        }
      };
    return CaloriesPerHourServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static CaloriesPerHourServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CaloriesPerHourServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CaloriesPerHourServiceBlockingStub>() {
        @java.lang.Override
        public CaloriesPerHourServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CaloriesPerHourServiceBlockingStub(channel, callOptions);
        }
      };
    return CaloriesPerHourServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static CaloriesPerHourServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<CaloriesPerHourServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<CaloriesPerHourServiceFutureStub>() {
        @java.lang.Override
        public CaloriesPerHourServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new CaloriesPerHourServiceFutureStub(channel, callOptions);
        }
      };
    return CaloriesPerHourServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether CaloriesPerHour is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current CaloriesPerHour relative to the current workout
     * </pre>
     */
    default void getCaloriesPerHour(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCaloriesPerHourMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to CaloriesPerHour updates relative to the current workout
     * </pre>
     */
    default void caloriesPerHourSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCaloriesPerHourSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesPerHour changes for the given workout ID
     * </pre>
     */
    default void getCaloriesPerHourHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCaloriesPerHourHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service CaloriesPerHourService.
   */
  public static abstract class CaloriesPerHourServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return CaloriesPerHourServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service CaloriesPerHourService.
   */
  public static final class CaloriesPerHourServiceStub
      extends io.grpc.stub.AbstractAsyncStub<CaloriesPerHourServiceStub> {
    private CaloriesPerHourServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CaloriesPerHourServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CaloriesPerHourServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether CaloriesPerHour is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current CaloriesPerHour relative to the current workout
     * </pre>
     */
    public void getCaloriesPerHour(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCaloriesPerHourMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to CaloriesPerHour updates relative to the current workout
     * </pre>
     */
    public void caloriesPerHourSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getCaloriesPerHourSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesPerHour changes for the given workout ID
     * </pre>
     */
    public void getCaloriesPerHourHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCaloriesPerHourHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service CaloriesPerHourService.
   */
  public static final class CaloriesPerHourServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<CaloriesPerHourServiceBlockingStub> {
    private CaloriesPerHourServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CaloriesPerHourServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CaloriesPerHourServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether CaloriesPerHour is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current CaloriesPerHour relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.CaloriesPerHourMetric getCaloriesPerHour(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCaloriesPerHourMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to CaloriesPerHour updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.CaloriesPerHourMetric> caloriesPerHourSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getCaloriesPerHourSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesPerHour changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.CaloriesPerHourMetricList getCaloriesPerHourHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCaloriesPerHourHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service CaloriesPerHourService.
   */
  public static final class CaloriesPerHourServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<CaloriesPerHourServiceFutureStub> {
    private CaloriesPerHourServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected CaloriesPerHourServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new CaloriesPerHourServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether CaloriesPerHour is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current CaloriesPerHour relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.CaloriesPerHourMetric> getCaloriesPerHour(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCaloriesPerHourMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the CaloriesPerHour changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.CaloriesPerHourMetricList> getCaloriesPerHourHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCaloriesPerHourHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_CALORIES_PER_HOUR = 1;
  private static final int METHODID_CALORIES_PER_HOUR_SUBSCRIPTION = 2;
  private static final int METHODID_GET_CALORIES_PER_HOUR_HISTORY = 3;

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
        case METHODID_GET_CALORIES_PER_HOUR:
          serviceImpl.getCaloriesPerHour((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetric>) responseObserver);
          break;
        case METHODID_CALORIES_PER_HOUR_SUBSCRIPTION:
          serviceImpl.caloriesPerHourSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetric>) responseObserver);
          break;
        case METHODID_GET_CALORIES_PER_HOUR_HISTORY:
          serviceImpl.getCaloriesPerHourHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.CaloriesPerHourMetricList>) responseObserver);
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
          getGetCaloriesPerHourMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.CaloriesPerHourMetric>(
                service, METHODID_GET_CALORIES_PER_HOUR)))
        .addMethod(
          getCaloriesPerHourSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.CaloriesPerHourMetric>(
                service, METHODID_CALORIES_PER_HOUR_SUBSCRIPTION)))
        .addMethod(
          getGetCaloriesPerHourHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.CaloriesPerHourMetricList>(
                service, METHODID_GET_CALORIES_PER_HOUR_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (CaloriesPerHourServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetCaloriesPerHourMethod())
              .addMethod(getCaloriesPerHourSubscriptionMethod())
              .addMethod(getGetCaloriesPerHourHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

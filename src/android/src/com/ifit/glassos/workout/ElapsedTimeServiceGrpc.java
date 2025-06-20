package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/ElapsedTimeService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ElapsedTimeServiceGrpc {

  private ElapsedTimeServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ElapsedTimeService";

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
    if ((getCanReadMethod = ElapsedTimeServiceGrpc.getCanReadMethod) == null) {
      synchronized (ElapsedTimeServiceGrpc.class) {
        if ((getCanReadMethod = ElapsedTimeServiceGrpc.getCanReadMethod) == null) {
          ElapsedTimeServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.ElapsedTimeMetric> getGetElapsedTimeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetElapsedTime",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ElapsedTimeMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ElapsedTimeMetric> getGetElapsedTimeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElapsedTimeMetric> getGetElapsedTimeMethod;
    if ((getGetElapsedTimeMethod = ElapsedTimeServiceGrpc.getGetElapsedTimeMethod) == null) {
      synchronized (ElapsedTimeServiceGrpc.class) {
        if ((getGetElapsedTimeMethod = ElapsedTimeServiceGrpc.getGetElapsedTimeMethod) == null) {
          ElapsedTimeServiceGrpc.getGetElapsedTimeMethod = getGetElapsedTimeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElapsedTimeMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetElapsedTime"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ElapsedTimeMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetElapsedTimeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ElapsedTimeMetric> getElapsedTimeSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ElapsedTimeSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ElapsedTimeMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ElapsedTimeMetric> getElapsedTimeSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElapsedTimeMetric> getElapsedTimeSubscriptionMethod;
    if ((getElapsedTimeSubscriptionMethod = ElapsedTimeServiceGrpc.getElapsedTimeSubscriptionMethod) == null) {
      synchronized (ElapsedTimeServiceGrpc.class) {
        if ((getElapsedTimeSubscriptionMethod = ElapsedTimeServiceGrpc.getElapsedTimeSubscriptionMethod) == null) {
          ElapsedTimeServiceGrpc.getElapsedTimeSubscriptionMethod = getElapsedTimeSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElapsedTimeMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ElapsedTimeSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ElapsedTimeMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getElapsedTimeSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.ElapsedTimeMetricList> getGetElapsedTimeHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetElapsedTimeHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.ElapsedTimeMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.ElapsedTimeMetricList> getGetElapsedTimeHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.ElapsedTimeMetricList> getGetElapsedTimeHistoryMethod;
    if ((getGetElapsedTimeHistoryMethod = ElapsedTimeServiceGrpc.getGetElapsedTimeHistoryMethod) == null) {
      synchronized (ElapsedTimeServiceGrpc.class) {
        if ((getGetElapsedTimeHistoryMethod = ElapsedTimeServiceGrpc.getGetElapsedTimeHistoryMethod) == null) {
          ElapsedTimeServiceGrpc.getGetElapsedTimeHistoryMethod = getGetElapsedTimeHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.ElapsedTimeMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetElapsedTimeHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ElapsedTimeMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetElapsedTimeHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ElapsedTimeServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ElapsedTimeServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ElapsedTimeServiceStub>() {
        @java.lang.Override
        public ElapsedTimeServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ElapsedTimeServiceStub(channel, callOptions);
        }
      };
    return ElapsedTimeServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ElapsedTimeServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ElapsedTimeServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ElapsedTimeServiceBlockingStub>() {
        @java.lang.Override
        public ElapsedTimeServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ElapsedTimeServiceBlockingStub(channel, callOptions);
        }
      };
    return ElapsedTimeServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ElapsedTimeServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ElapsedTimeServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ElapsedTimeServiceFutureStub>() {
        @java.lang.Override
        public ElapsedTimeServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ElapsedTimeServiceFutureStub(channel, callOptions);
        }
      };
    return ElapsedTimeServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether ElapsedTime is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Elapsed Time relative to the current workout
     * </pre>
     */
    default void getElapsedTime(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetElapsedTimeMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to ElapsedTime updates relative to the current workout
     * </pre>
     */
    default void elapsedTimeSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getElapsedTimeSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the ElapsedTime changes for the given workout ID
     * </pre>
     */
    default void getElapsedTimeHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetElapsedTimeHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ElapsedTimeService.
   */
  public static abstract class ElapsedTimeServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ElapsedTimeServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ElapsedTimeService.
   */
  public static final class ElapsedTimeServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ElapsedTimeServiceStub> {
    private ElapsedTimeServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ElapsedTimeServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ElapsedTimeServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether ElapsedTime is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Elapsed Time relative to the current workout
     * </pre>
     */
    public void getElapsedTime(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetElapsedTimeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to ElapsedTime updates relative to the current workout
     * </pre>
     */
    public void elapsedTimeSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getElapsedTimeSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the ElapsedTime changes for the given workout ID
     * </pre>
     */
    public void getElapsedTimeHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetElapsedTimeHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ElapsedTimeService.
   */
  public static final class ElapsedTimeServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ElapsedTimeServiceBlockingStub> {
    private ElapsedTimeServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ElapsedTimeServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ElapsedTimeServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether ElapsedTime is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Elapsed Time relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.ElapsedTimeMetric getElapsedTime(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetElapsedTimeMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to ElapsedTime updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.ElapsedTimeMetric> elapsedTimeSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getElapsedTimeSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the ElapsedTime changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.ElapsedTimeMetricList getElapsedTimeHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetElapsedTimeHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ElapsedTimeService.
   */
  public static final class ElapsedTimeServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ElapsedTimeServiceFutureStub> {
    private ElapsedTimeServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ElapsedTimeServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ElapsedTimeServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether ElapsedTime is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Elapsed Time relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ElapsedTimeMetric> getElapsedTime(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetElapsedTimeMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the ElapsedTime changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ElapsedTimeMetricList> getElapsedTimeHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetElapsedTimeHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_ELAPSED_TIME = 1;
  private static final int METHODID_ELAPSED_TIME_SUBSCRIPTION = 2;
  private static final int METHODID_GET_ELAPSED_TIME_HISTORY = 3;

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
        case METHODID_GET_ELAPSED_TIME:
          serviceImpl.getElapsedTime((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetric>) responseObserver);
          break;
        case METHODID_ELAPSED_TIME_SUBSCRIPTION:
          serviceImpl.elapsedTimeSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetric>) responseObserver);
          break;
        case METHODID_GET_ELAPSED_TIME_HISTORY:
          serviceImpl.getElapsedTimeHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElapsedTimeMetricList>) responseObserver);
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
          getGetElapsedTimeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ElapsedTimeMetric>(
                service, METHODID_GET_ELAPSED_TIME)))
        .addMethod(
          getElapsedTimeSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ElapsedTimeMetric>(
                service, METHODID_ELAPSED_TIME_SUBSCRIPTION)))
        .addMethod(
          getGetElapsedTimeHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.ElapsedTimeMetricList>(
                service, METHODID_GET_ELAPSED_TIME_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ElapsedTimeServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetElapsedTimeMethod())
              .addMethod(getElapsedTimeSubscriptionMethod())
              .addMethod(getGetElapsedTimeHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

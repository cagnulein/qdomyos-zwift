package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/ElevationService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ElevationServiceGrpc {

  private ElevationServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ElevationService";

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
    if ((getCanReadMethod = ElevationServiceGrpc.getCanReadMethod) == null) {
      synchronized (ElevationServiceGrpc.class) {
        if ((getCanReadMethod = ElevationServiceGrpc.getCanReadMethod) == null) {
          ElevationServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.workout.ElevationMetric> getGetElevationMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetElevation",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ElevationMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ElevationMetric> getGetElevationMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElevationMetric> getGetElevationMethod;
    if ((getGetElevationMethod = ElevationServiceGrpc.getGetElevationMethod) == null) {
      synchronized (ElevationServiceGrpc.class) {
        if ((getGetElevationMethod = ElevationServiceGrpc.getGetElevationMethod) == null) {
          ElevationServiceGrpc.getGetElevationMethod = getGetElevationMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElevationMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetElevation"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ElevationMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetElevationMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ElevationMetric> getElevationSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ElevationSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ElevationMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ElevationMetric> getElevationSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElevationMetric> getElevationSubscriptionMethod;
    if ((getElevationSubscriptionMethod = ElevationServiceGrpc.getElevationSubscriptionMethod) == null) {
      synchronized (ElevationServiceGrpc.class) {
        if ((getElevationSubscriptionMethod = ElevationServiceGrpc.getElevationSubscriptionMethod) == null) {
          ElevationServiceGrpc.getElevationSubscriptionMethod = getElevationSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ElevationMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ElevationSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ElevationMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getElevationSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.ElevationMetricList> getGetElevationHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetElevationHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.ElevationMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.ElevationMetricList> getGetElevationHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.ElevationMetricList> getGetElevationHistoryMethod;
    if ((getGetElevationHistoryMethod = ElevationServiceGrpc.getGetElevationHistoryMethod) == null) {
      synchronized (ElevationServiceGrpc.class) {
        if ((getGetElevationHistoryMethod = ElevationServiceGrpc.getGetElevationHistoryMethod) == null) {
          ElevationServiceGrpc.getGetElevationHistoryMethod = getGetElevationHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.ElevationMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetElevationHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ElevationMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetElevationHistoryMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ElevationServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ElevationServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ElevationServiceStub>() {
        @java.lang.Override
        public ElevationServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ElevationServiceStub(channel, callOptions);
        }
      };
    return ElevationServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ElevationServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ElevationServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ElevationServiceBlockingStub>() {
        @java.lang.Override
        public ElevationServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ElevationServiceBlockingStub(channel, callOptions);
        }
      };
    return ElevationServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ElevationServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ElevationServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ElevationServiceFutureStub>() {
        @java.lang.Override
        public ElevationServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ElevationServiceFutureStub(channel, callOptions);
        }
      };
    return ElevationServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Elevation is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Elevation relative to the current workout
     * </pre>
     */
    default void getElevation(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetElevationMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Elevation updates relative to the current workout
     * </pre>
     */
    default void elevationSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getElevationSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Elevation changes for the given workout ID
     * </pre>
     */
    default void getElevationHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetElevationHistoryMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ElevationService.
   */
  public static abstract class ElevationServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ElevationServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ElevationService.
   */
  public static final class ElevationServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ElevationServiceStub> {
    private ElevationServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ElevationServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ElevationServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Elevation is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Elevation relative to the current workout
     * </pre>
     */
    public void getElevation(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetElevationMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Elevation updates relative to the current workout
     * </pre>
     */
    public void elevationSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getElevationSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Elevation changes for the given workout ID
     * </pre>
     */
    public void getElevationHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetElevationHistoryMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ElevationService.
   */
  public static final class ElevationServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ElevationServiceBlockingStub> {
    private ElevationServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ElevationServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ElevationServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Elevation is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Elevation relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.ElevationMetric getElevation(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetElevationMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Elevation updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.ElevationMetric> elevationSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getElevationSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Elevation changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.ElevationMetricList getElevationHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetElevationHistoryMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ElevationService.
   */
  public static final class ElevationServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ElevationServiceFutureStub> {
    private ElevationServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ElevationServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ElevationServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Elevation is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Elevation relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ElevationMetric> getElevation(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetElevationMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Elevation changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ElevationMetricList> getElevationHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetElevationHistoryMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_GET_ELEVATION = 1;
  private static final int METHODID_ELEVATION_SUBSCRIPTION = 2;
  private static final int METHODID_GET_ELEVATION_HISTORY = 3;

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
        case METHODID_GET_ELEVATION:
          serviceImpl.getElevation((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetric>) responseObserver);
          break;
        case METHODID_ELEVATION_SUBSCRIPTION:
          serviceImpl.elevationSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetric>) responseObserver);
          break;
        case METHODID_GET_ELEVATION_HISTORY:
          serviceImpl.getElevationHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ElevationMetricList>) responseObserver);
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
          getGetElevationMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ElevationMetric>(
                service, METHODID_GET_ELEVATION)))
        .addMethod(
          getElevationSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ElevationMetric>(
                service, METHODID_ELEVATION_SUBSCRIPTION)))
        .addMethod(
          getGetElevationHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.ElevationMetricList>(
                service, METHODID_GET_ELEVATION_HISTORY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ElevationServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getGetElevationMethod())
              .addMethod(getElevationSubscriptionMethod())
              .addMethod(getGetElevationHistoryMethod())
              .build();
        }
      }
    }
    return result;
  }
}

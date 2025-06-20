package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/RpmService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class RpmServiceGrpc {

  private RpmServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.RpmService";

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
    if ((getCanReadMethod = RpmServiceGrpc.getCanReadMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getCanReadMethod = RpmServiceGrpc.getCanReadMethod) == null) {
          RpmServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteVirtualMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanWriteVirtual",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteVirtualMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanWriteVirtualMethod;
    if ((getCanWriteVirtualMethod = RpmServiceGrpc.getCanWriteVirtualMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getCanWriteVirtualMethod = RpmServiceGrpc.getCanWriteVirtualMethod) == null) {
          RpmServiceGrpc.getCanWriteVirtualMethod = getCanWriteVirtualMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanWriteVirtual"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanWriteVirtualMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RpmMetric> getGetRpmMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetRpm",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.RpmMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RpmMetric> getGetRpmMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RpmMetric> getGetRpmMethod;
    if ((getGetRpmMethod = RpmServiceGrpc.getGetRpmMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getGetRpmMethod = RpmServiceGrpc.getGetRpmMethod) == null) {
          RpmServiceGrpc.getGetRpmMethod = getGetRpmMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RpmMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetRpm"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.RpmMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetRpmMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RpmMetric> getRpmSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RpmSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.RpmMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RpmMetric> getRpmSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RpmMetric> getRpmSubscriptionMethod;
    if ((getRpmSubscriptionMethod = RpmServiceGrpc.getRpmSubscriptionMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getRpmSubscriptionMethod = RpmServiceGrpc.getRpmSubscriptionMethod) == null) {
          RpmServiceGrpc.getRpmSubscriptionMethod = getRpmSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RpmMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RpmSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.RpmMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRpmSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getGetAllTargetsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetAllTargets",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.ControlList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getGetAllTargetsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList> getGetAllTargetsMethod;
    if ((getGetAllTargetsMethod = RpmServiceGrpc.getGetAllTargetsMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getGetAllTargetsMethod = RpmServiceGrpc.getGetAllTargetsMethod) == null) {
          RpmServiceGrpc.getGetAllTargetsMethod = getGetAllTargetsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetAllTargets"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.ControlList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetAllTargetsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getAllTargetsSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AllTargetsSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.ControlList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getAllTargetsSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList> getAllTargetsSubscriptionMethod;
    if ((getAllTargetsSubscriptionMethod = RpmServiceGrpc.getAllTargetsSubscriptionMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getAllTargetsSubscriptionMethod = RpmServiceGrpc.getAllTargetsSubscriptionMethod) == null) {
          RpmServiceGrpc.getAllTargetsSubscriptionMethod = getAllTargetsSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AllTargetsSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.ControlList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAllTargetsSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.RpmMetricList> getGetRpmHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetRpmHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.RpmMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.RpmMetricList> getGetRpmHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.RpmMetricList> getGetRpmHistoryMethod;
    if ((getGetRpmHistoryMethod = RpmServiceGrpc.getGetRpmHistoryMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getGetRpmHistoryMethod = RpmServiceGrpc.getGetRpmHistoryMethod) == null) {
          RpmServiceGrpc.getGetRpmHistoryMethod = getGetRpmHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.RpmMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetRpmHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.RpmMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetRpmHistoryMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.RpmRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetVirtualRpmMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetVirtualRpm",
      requestType = com.ifit.glassos.workout.RpmRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.RpmRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetVirtualRpmMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.RpmRequest, com.ifit.glassos.workout.WorkoutResult> getSetVirtualRpmMethod;
    if ((getSetVirtualRpmMethod = RpmServiceGrpc.getSetVirtualRpmMethod) == null) {
      synchronized (RpmServiceGrpc.class) {
        if ((getSetVirtualRpmMethod = RpmServiceGrpc.getSetVirtualRpmMethod) == null) {
          RpmServiceGrpc.getSetVirtualRpmMethod = getSetVirtualRpmMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.RpmRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetVirtualRpm"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.RpmRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetVirtualRpmMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static RpmServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<RpmServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<RpmServiceStub>() {
        @java.lang.Override
        public RpmServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new RpmServiceStub(channel, callOptions);
        }
      };
    return RpmServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static RpmServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<RpmServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<RpmServiceBlockingStub>() {
        @java.lang.Override
        public RpmServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new RpmServiceBlockingStub(channel, callOptions);
        }
      };
    return RpmServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static RpmServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<RpmServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<RpmServiceFutureStub>() {
        @java.lang.Override
        public RpmServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new RpmServiceFutureStub(channel, callOptions);
        }
      };
    return RpmServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Rpm is Readable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     */
    default void canWriteVirtual(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanWriteVirtualMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Rpm relative to the current workout
     * </pre>
     */
    default void getRpm(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetRpmMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Rpm updates relative to the current workout
     * </pre>
     */
    default void rpmSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRpmSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    default void getAllTargets(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetAllTargetsMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to workout control updates
     * </pre>
     */
    default void allTargetsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAllTargetsSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Rpm changes for the given workout ID
     * </pre>
     */
    default void getRpmHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetRpmHistoryMethod(), responseObserver);
    }

    /**
     */
    default void setVirtualRpm(com.ifit.glassos.workout.RpmRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetVirtualRpmMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service RpmService.
   */
  public static abstract class RpmServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return RpmServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service RpmService.
   */
  public static final class RpmServiceStub
      extends io.grpc.stub.AbstractAsyncStub<RpmServiceStub> {
    private RpmServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected RpmServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new RpmServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Rpm is Readable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void canWriteVirtual(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanWriteVirtualMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Rpm relative to the current workout
     * </pre>
     */
    public void getRpm(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetRpmMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Rpm updates relative to the current workout
     * </pre>
     */
    public void rpmSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getRpmSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    public void getAllTargets(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetAllTargetsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to workout control updates
     * </pre>
     */
    public void allTargetsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getAllTargetsSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Rpm changes for the given workout ID
     * </pre>
     */
    public void getRpmHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetRpmHistoryMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setVirtualRpm(com.ifit.glassos.workout.RpmRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetVirtualRpmMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service RpmService.
   */
  public static final class RpmServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<RpmServiceBlockingStub> {
    private RpmServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected RpmServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new RpmServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Rpm is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.AvailabilityResponse canWriteVirtual(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanWriteVirtualMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Rpm relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.RpmMetric getRpm(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetRpmMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Rpm updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.RpmMetric> rpmSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getRpmSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    public com.ifit.glassos.workout.data.ControlList getAllTargets(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetAllTargetsMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to workout control updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.ControlList> allTargetsSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getAllTargetsSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Rpm changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.RpmMetricList getRpmHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetRpmHistoryMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutResult setVirtualRpm(com.ifit.glassos.workout.RpmRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetVirtualRpmMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service RpmService.
   */
  public static final class RpmServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<RpmServiceFutureStub> {
    private RpmServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected RpmServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new RpmServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Rpm is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canWriteVirtual(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanWriteVirtualMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Rpm relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.RpmMetric> getRpm(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetRpmMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.data.ControlList> getAllTargets(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetAllTargetsMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Rpm changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.RpmMetricList> getRpmHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetRpmHistoryMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setVirtualRpm(
        com.ifit.glassos.workout.RpmRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetVirtualRpmMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE_VIRTUAL = 1;
  private static final int METHODID_GET_RPM = 2;
  private static final int METHODID_RPM_SUBSCRIPTION = 3;
  private static final int METHODID_GET_ALL_TARGETS = 4;
  private static final int METHODID_ALL_TARGETS_SUBSCRIPTION = 5;
  private static final int METHODID_GET_RPM_HISTORY = 6;
  private static final int METHODID_SET_VIRTUAL_RPM = 7;

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
        case METHODID_CAN_WRITE_VIRTUAL:
          serviceImpl.canWriteVirtual((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_GET_RPM:
          serviceImpl.getRpm((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetric>) responseObserver);
          break;
        case METHODID_RPM_SUBSCRIPTION:
          serviceImpl.rpmSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetric>) responseObserver);
          break;
        case METHODID_GET_ALL_TARGETS:
          serviceImpl.getAllTargets((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList>) responseObserver);
          break;
        case METHODID_ALL_TARGETS_SUBSCRIPTION:
          serviceImpl.allTargetsSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList>) responseObserver);
          break;
        case METHODID_GET_RPM_HISTORY:
          serviceImpl.getRpmHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RpmMetricList>) responseObserver);
          break;
        case METHODID_SET_VIRTUAL_RPM:
          serviceImpl.setVirtualRpm((com.ifit.glassos.workout.RpmRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
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
          getCanWriteVirtualMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_WRITE_VIRTUAL)))
        .addMethod(
          getGetRpmMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.RpmMetric>(
                service, METHODID_GET_RPM)))
        .addMethod(
          getRpmSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.RpmMetric>(
                service, METHODID_RPM_SUBSCRIPTION)))
        .addMethod(
          getGetAllTargetsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.ControlList>(
                service, METHODID_GET_ALL_TARGETS)))
        .addMethod(
          getAllTargetsSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.ControlList>(
                service, METHODID_ALL_TARGETS_SUBSCRIPTION)))
        .addMethod(
          getGetRpmHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.RpmMetricList>(
                service, METHODID_GET_RPM_HISTORY)))
        .addMethod(
          getSetVirtualRpmMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.RpmRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_VIRTUAL_RPM)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (RpmServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteVirtualMethod())
              .addMethod(getGetRpmMethod())
              .addMethod(getRpmSubscriptionMethod())
              .addMethod(getGetAllTargetsMethod())
              .addMethod(getAllTargetsSubscriptionMethod())
              .addMethod(getGetRpmHistoryMethod())
              .addMethod(getSetVirtualRpmMethod())
              .build();
        }
      }
    }
    return result;
  }
}

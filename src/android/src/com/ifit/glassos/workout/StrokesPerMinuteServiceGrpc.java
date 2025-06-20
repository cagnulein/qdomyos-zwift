package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/StrokesPerMinuteService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class StrokesPerMinuteServiceGrpc {

  private StrokesPerMinuteServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.StrokesPerMinuteService";

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
    if ((getCanReadMethod = StrokesPerMinuteServiceGrpc.getCanReadMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getCanReadMethod = StrokesPerMinuteServiceGrpc.getCanReadMethod) == null) {
          StrokesPerMinuteServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteVirtualMethod = StrokesPerMinuteServiceGrpc.getCanWriteVirtualMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getCanWriteVirtualMethod = StrokesPerMinuteServiceGrpc.getCanWriteVirtualMethod) == null) {
          StrokesPerMinuteServiceGrpc.getCanWriteVirtualMethod = getCanWriteVirtualMethod =
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
      com.ifit.glassos.workout.StrokesPerMinuteMetric> getGetStrokesPerMinuteMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStrokesPerMinute",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StrokesPerMinuteMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StrokesPerMinuteMetric> getGetStrokesPerMinuteMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesPerMinuteMetric> getGetStrokesPerMinuteMethod;
    if ((getGetStrokesPerMinuteMethod = StrokesPerMinuteServiceGrpc.getGetStrokesPerMinuteMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getGetStrokesPerMinuteMethod = StrokesPerMinuteServiceGrpc.getGetStrokesPerMinuteMethod) == null) {
          StrokesPerMinuteServiceGrpc.getGetStrokesPerMinuteMethod = getGetStrokesPerMinuteMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesPerMinuteMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStrokesPerMinute"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesPerMinuteMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStrokesPerMinuteMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StrokesPerMinuteMetric> getStrokesPerMinuteSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StrokesPerMinuteSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StrokesPerMinuteMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StrokesPerMinuteMetric> getStrokesPerMinuteSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesPerMinuteMetric> getStrokesPerMinuteSubscriptionMethod;
    if ((getStrokesPerMinuteSubscriptionMethod = StrokesPerMinuteServiceGrpc.getStrokesPerMinuteSubscriptionMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getStrokesPerMinuteSubscriptionMethod = StrokesPerMinuteServiceGrpc.getStrokesPerMinuteSubscriptionMethod) == null) {
          StrokesPerMinuteServiceGrpc.getStrokesPerMinuteSubscriptionMethod = getStrokesPerMinuteSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StrokesPerMinuteMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StrokesPerMinuteSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesPerMinuteMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStrokesPerMinuteSubscriptionMethod;
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
    if ((getGetAllTargetsMethod = StrokesPerMinuteServiceGrpc.getGetAllTargetsMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getGetAllTargetsMethod = StrokesPerMinuteServiceGrpc.getGetAllTargetsMethod) == null) {
          StrokesPerMinuteServiceGrpc.getGetAllTargetsMethod = getGetAllTargetsMethod =
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
    if ((getAllTargetsSubscriptionMethod = StrokesPerMinuteServiceGrpc.getAllTargetsSubscriptionMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getAllTargetsSubscriptionMethod = StrokesPerMinuteServiceGrpc.getAllTargetsSubscriptionMethod) == null) {
          StrokesPerMinuteServiceGrpc.getAllTargetsSubscriptionMethod = getAllTargetsSubscriptionMethod =
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
      com.ifit.glassos.workout.StrokesPerMinuteMetricList> getGetStrokesPerMinuteHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStrokesPerMinuteHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.StrokesPerMinuteMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StrokesPerMinuteMetricList> getGetStrokesPerMinuteHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StrokesPerMinuteMetricList> getGetStrokesPerMinuteHistoryMethod;
    if ((getGetStrokesPerMinuteHistoryMethod = StrokesPerMinuteServiceGrpc.getGetStrokesPerMinuteHistoryMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getGetStrokesPerMinuteHistoryMethod = StrokesPerMinuteServiceGrpc.getGetStrokesPerMinuteHistoryMethod) == null) {
          StrokesPerMinuteServiceGrpc.getGetStrokesPerMinuteHistoryMethod = getGetStrokesPerMinuteHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StrokesPerMinuteMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStrokesPerMinuteHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesPerMinuteMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStrokesPerMinuteHistoryMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.StrokesPerMinuteRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetVirtualStrokesPerMinuteMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetVirtualStrokesPerMinute",
      requestType = com.ifit.glassos.workout.StrokesPerMinuteRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.StrokesPerMinuteRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetVirtualStrokesPerMinuteMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.StrokesPerMinuteRequest, com.ifit.glassos.workout.WorkoutResult> getSetVirtualStrokesPerMinuteMethod;
    if ((getSetVirtualStrokesPerMinuteMethod = StrokesPerMinuteServiceGrpc.getSetVirtualStrokesPerMinuteMethod) == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        if ((getSetVirtualStrokesPerMinuteMethod = StrokesPerMinuteServiceGrpc.getSetVirtualStrokesPerMinuteMethod) == null) {
          StrokesPerMinuteServiceGrpc.getSetVirtualStrokesPerMinuteMethod = getSetVirtualStrokesPerMinuteMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.StrokesPerMinuteRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetVirtualStrokesPerMinute"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StrokesPerMinuteRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetVirtualStrokesPerMinuteMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static StrokesPerMinuteServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StrokesPerMinuteServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StrokesPerMinuteServiceStub>() {
        @java.lang.Override
        public StrokesPerMinuteServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StrokesPerMinuteServiceStub(channel, callOptions);
        }
      };
    return StrokesPerMinuteServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static StrokesPerMinuteServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StrokesPerMinuteServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StrokesPerMinuteServiceBlockingStub>() {
        @java.lang.Override
        public StrokesPerMinuteServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StrokesPerMinuteServiceBlockingStub(channel, callOptions);
        }
      };
    return StrokesPerMinuteServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static StrokesPerMinuteServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<StrokesPerMinuteServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<StrokesPerMinuteServiceFutureStub>() {
        @java.lang.Override
        public StrokesPerMinuteServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new StrokesPerMinuteServiceFutureStub(channel, callOptions);
        }
      };
    return StrokesPerMinuteServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether StrokesPerMinute is Readable
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
     * get the current StrokesPerMinute relative to the current workout
     * </pre>
     */
    default void getStrokesPerMinute(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStrokesPerMinuteMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to StrokesPerMinute updates relative to the current workout
     * </pre>
     */
    default void strokesPerMinuteSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStrokesPerMinuteSubscriptionMethod(), responseObserver);
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
     * gets a list of all of the StrokesPerMinute changes for the given workout ID
     * </pre>
     */
    default void getStrokesPerMinuteHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStrokesPerMinuteHistoryMethod(), responseObserver);
    }

    /**
     */
    default void setVirtualStrokesPerMinute(com.ifit.glassos.workout.StrokesPerMinuteRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetVirtualStrokesPerMinuteMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service StrokesPerMinuteService.
   */
  public static abstract class StrokesPerMinuteServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return StrokesPerMinuteServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service StrokesPerMinuteService.
   */
  public static final class StrokesPerMinuteServiceStub
      extends io.grpc.stub.AbstractAsyncStub<StrokesPerMinuteServiceStub> {
    private StrokesPerMinuteServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StrokesPerMinuteServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StrokesPerMinuteServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether StrokesPerMinute is Readable
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
     * get the current StrokesPerMinute relative to the current workout
     * </pre>
     */
    public void getStrokesPerMinute(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStrokesPerMinuteMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to StrokesPerMinute updates relative to the current workout
     * </pre>
     */
    public void strokesPerMinuteSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getStrokesPerMinuteSubscriptionMethod(), getCallOptions()), request, responseObserver);
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
     * gets a list of all of the StrokesPerMinute changes for the given workout ID
     * </pre>
     */
    public void getStrokesPerMinuteHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStrokesPerMinuteHistoryMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setVirtualStrokesPerMinute(com.ifit.glassos.workout.StrokesPerMinuteRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetVirtualStrokesPerMinuteMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service StrokesPerMinuteService.
   */
  public static final class StrokesPerMinuteServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<StrokesPerMinuteServiceBlockingStub> {
    private StrokesPerMinuteServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StrokesPerMinuteServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StrokesPerMinuteServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether StrokesPerMinute is Readable
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
     * get the current StrokesPerMinute relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.StrokesPerMinuteMetric getStrokesPerMinute(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStrokesPerMinuteMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to StrokesPerMinute updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.StrokesPerMinuteMetric> strokesPerMinuteSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getStrokesPerMinuteSubscriptionMethod(), getCallOptions(), request);
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
     * gets a list of all of the StrokesPerMinute changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.StrokesPerMinuteMetricList getStrokesPerMinuteHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStrokesPerMinuteHistoryMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutResult setVirtualStrokesPerMinute(com.ifit.glassos.workout.StrokesPerMinuteRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetVirtualStrokesPerMinuteMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service StrokesPerMinuteService.
   */
  public static final class StrokesPerMinuteServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<StrokesPerMinuteServiceFutureStub> {
    private StrokesPerMinuteServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected StrokesPerMinuteServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new StrokesPerMinuteServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether StrokesPerMinute is Readable
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
     * get the current StrokesPerMinute relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StrokesPerMinuteMetric> getStrokesPerMinute(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStrokesPerMinuteMethod(), getCallOptions()), request);
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
     * gets a list of all of the StrokesPerMinute changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StrokesPerMinuteMetricList> getStrokesPerMinuteHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStrokesPerMinuteHistoryMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setVirtualStrokesPerMinute(
        com.ifit.glassos.workout.StrokesPerMinuteRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetVirtualStrokesPerMinuteMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE_VIRTUAL = 1;
  private static final int METHODID_GET_STROKES_PER_MINUTE = 2;
  private static final int METHODID_STROKES_PER_MINUTE_SUBSCRIPTION = 3;
  private static final int METHODID_GET_ALL_TARGETS = 4;
  private static final int METHODID_ALL_TARGETS_SUBSCRIPTION = 5;
  private static final int METHODID_GET_STROKES_PER_MINUTE_HISTORY = 6;
  private static final int METHODID_SET_VIRTUAL_STROKES_PER_MINUTE = 7;

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
        case METHODID_GET_STROKES_PER_MINUTE:
          serviceImpl.getStrokesPerMinute((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetric>) responseObserver);
          break;
        case METHODID_STROKES_PER_MINUTE_SUBSCRIPTION:
          serviceImpl.strokesPerMinuteSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetric>) responseObserver);
          break;
        case METHODID_GET_ALL_TARGETS:
          serviceImpl.getAllTargets((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList>) responseObserver);
          break;
        case METHODID_ALL_TARGETS_SUBSCRIPTION:
          serviceImpl.allTargetsSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList>) responseObserver);
          break;
        case METHODID_GET_STROKES_PER_MINUTE_HISTORY:
          serviceImpl.getStrokesPerMinuteHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StrokesPerMinuteMetricList>) responseObserver);
          break;
        case METHODID_SET_VIRTUAL_STROKES_PER_MINUTE:
          serviceImpl.setVirtualStrokesPerMinute((com.ifit.glassos.workout.StrokesPerMinuteRequest) request,
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
          getGetStrokesPerMinuteMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StrokesPerMinuteMetric>(
                service, METHODID_GET_STROKES_PER_MINUTE)))
        .addMethod(
          getStrokesPerMinuteSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StrokesPerMinuteMetric>(
                service, METHODID_STROKES_PER_MINUTE_SUBSCRIPTION)))
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
          getGetStrokesPerMinuteHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.StrokesPerMinuteMetricList>(
                service, METHODID_GET_STROKES_PER_MINUTE_HISTORY)))
        .addMethod(
          getSetVirtualStrokesPerMinuteMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.StrokesPerMinuteRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_VIRTUAL_STROKES_PER_MINUTE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (StrokesPerMinuteServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteVirtualMethod())
              .addMethod(getGetStrokesPerMinuteMethod())
              .addMethod(getStrokesPerMinuteSubscriptionMethod())
              .addMethod(getGetAllTargetsMethod())
              .addMethod(getAllTargetsSubscriptionMethod())
              .addMethod(getGetStrokesPerMinuteHistoryMethod())
              .addMethod(getSetVirtualStrokesPerMinuteMethod())
              .build();
        }
      }
    }
    return result;
  }
}

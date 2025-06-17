package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/InclineService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class InclineServiceGrpc {

  private InclineServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.InclineService";

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
    if ((getCanReadMethod = InclineServiceGrpc.getCanReadMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getCanReadMethod = InclineServiceGrpc.getCanReadMethod) == null) {
          InclineServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanWrite",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanWriteMethod;
    if ((getCanWriteMethod = InclineServiceGrpc.getCanWriteMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getCanWriteMethod = InclineServiceGrpc.getCanWriteMethod) == null) {
          InclineServiceGrpc.getCanWriteMethod = getCanWriteMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanWrite"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanWriteMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.InclineMetric> getGetInclineMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetIncline",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.InclineMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.InclineMetric> getGetInclineMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.InclineMetric> getGetInclineMethod;
    if ((getGetInclineMethod = InclineServiceGrpc.getGetInclineMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getGetInclineMethod = InclineServiceGrpc.getGetInclineMethod) == null) {
          InclineServiceGrpc.getGetInclineMethod = getGetInclineMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.InclineMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetIncline"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.InclineMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetInclineMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.InclineRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetInclineMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetIncline",
      requestType = com.ifit.glassos.workout.InclineRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.InclineRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetInclineMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.InclineRequest, com.ifit.glassos.workout.WorkoutResult> getSetInclineMethod;
    if ((getSetInclineMethod = InclineServiceGrpc.getSetInclineMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getSetInclineMethod = InclineServiceGrpc.getSetInclineMethod) == null) {
          InclineServiceGrpc.getSetInclineMethod = getSetInclineMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.InclineRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetIncline"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.InclineRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetInclineMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.InclineMetric> getInclineSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "InclineSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.InclineMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.InclineMetric> getInclineSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.InclineMetric> getInclineSubscriptionMethod;
    if ((getInclineSubscriptionMethod = InclineServiceGrpc.getInclineSubscriptionMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getInclineSubscriptionMethod = InclineServiceGrpc.getInclineSubscriptionMethod) == null) {
          InclineServiceGrpc.getInclineSubscriptionMethod = getInclineSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.InclineMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "InclineSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.InclineMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getInclineSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.InclineMetricList> getGetInclineHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetInclineHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.InclineMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.InclineMetricList> getGetInclineHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.InclineMetricList> getGetInclineHistoryMethod;
    if ((getGetInclineHistoryMethod = InclineServiceGrpc.getGetInclineHistoryMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getGetInclineHistoryMethod = InclineServiceGrpc.getGetInclineHistoryMethod) == null) {
          InclineServiceGrpc.getGetInclineHistoryMethod = getGetInclineHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.InclineMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetInclineHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.InclineMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetInclineHistoryMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getFollowWorkoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "FollowWorkout",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getFollowWorkoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult> getFollowWorkoutMethod;
    if ((getFollowWorkoutMethod = InclineServiceGrpc.getFollowWorkoutMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getFollowWorkoutMethod = InclineServiceGrpc.getFollowWorkoutMethod) == null) {
          InclineServiceGrpc.getFollowWorkoutMethod = getFollowWorkoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "FollowWorkout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getFollowWorkoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getStopFollowingMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StopFollowing",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getStopFollowingMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getStopFollowingMethod;
    if ((getStopFollowingMethod = InclineServiceGrpc.getStopFollowingMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getStopFollowingMethod = InclineServiceGrpc.getStopFollowingMethod) == null) {
          InclineServiceGrpc.getStopFollowingMethod = getStopFollowingMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StopFollowing"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStopFollowingMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetIsFollowingMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetIsFollowing",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetIsFollowingMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getGetIsFollowingMethod;
    if ((getGetIsFollowingMethod = InclineServiceGrpc.getGetIsFollowingMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getGetIsFollowingMethod = InclineServiceGrpc.getGetIsFollowingMethod) == null) {
          InclineServiceGrpc.getGetIsFollowingMethod = getGetIsFollowingMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetIsFollowing"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetIsFollowingMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsFollowingSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsFollowingSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsFollowingSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsFollowingSubscriptionMethod;
    if ((getIsFollowingSubscriptionMethod = InclineServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getIsFollowingSubscriptionMethod = InclineServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
          InclineServiceGrpc.getIsFollowingSubscriptionMethod = getIsFollowingSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsFollowingSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsFollowingSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getGetControlsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetControls",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.ControlList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getGetControlsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList> getGetControlsMethod;
    if ((getGetControlsMethod = InclineServiceGrpc.getGetControlsMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getGetControlsMethod = InclineServiceGrpc.getGetControlsMethod) == null) {
          InclineServiceGrpc.getGetControlsMethod = getGetControlsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetControls"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.ControlList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetControlsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getControlsSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ControlsSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.ControlList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.ControlList> getControlsSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList> getControlsSubscriptionMethod;
    if ((getControlsSubscriptionMethod = InclineServiceGrpc.getControlsSubscriptionMethod) == null) {
      synchronized (InclineServiceGrpc.class) {
        if ((getControlsSubscriptionMethod = InclineServiceGrpc.getControlsSubscriptionMethod) == null) {
          InclineServiceGrpc.getControlsSubscriptionMethod = getControlsSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.ControlList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ControlsSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.ControlList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getControlsSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static InclineServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<InclineServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<InclineServiceStub>() {
        @java.lang.Override
        public InclineServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new InclineServiceStub(channel, callOptions);
        }
      };
    return InclineServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static InclineServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<InclineServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<InclineServiceBlockingStub>() {
        @java.lang.Override
        public InclineServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new InclineServiceBlockingStub(channel, callOptions);
        }
      };
    return InclineServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static InclineServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<InclineServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<InclineServiceFutureStub>() {
        @java.lang.Override
        public InclineServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new InclineServiceFutureStub(channel, callOptions);
        }
      };
    return InclineServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Incline is Readable and Writeable
     * </pre>
     */
    default void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMethod(), responseObserver);
    }

    /**
     */
    default void canWrite(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanWriteMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Incline relative to the current workout
     * </pre>
     */
    default void getIncline(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetInclineMethod(), responseObserver);
    }

    /**
     * <pre>
     * sets the Incline to the given value
     * </pre>
     */
    default void setIncline(com.ifit.glassos.workout.InclineRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetInclineMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Incline updates relative to the current workout
     * </pre>
     */
    default void inclineSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getInclineSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Incline changes for the given workout ID
     * </pre>
     */
    default void getInclineHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetInclineHistoryMethod(), responseObserver);
    }

    /**
     * <pre>
     * follow the current workout
     * </pre>
     */
    default void followWorkout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getFollowWorkoutMethod(), responseObserver);
    }

    /**
     * <pre>
     * stop following the current workout
     * </pre>
     */
    default void stopFollowing(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStopFollowingMethod(), responseObserver);
    }

    /**
     * <pre>
     * check whether the current workout is being followed
     * </pre>
     */
    default void getIsFollowing(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetIsFollowingMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to following updates
     * </pre>
     */
    default void isFollowingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsFollowingSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    default void getControls(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetControlsMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to workout control updates
     * </pre>
     */
    default void controlsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getControlsSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service InclineService.
   */
  public static abstract class InclineServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return InclineServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service InclineService.
   */
  public static final class InclineServiceStub
      extends io.grpc.stub.AbstractAsyncStub<InclineServiceStub> {
    private InclineServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected InclineServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new InclineServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Incline is Readable and Writeable
     * </pre>
     */
    public void canRead(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void canWrite(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanWriteMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Incline relative to the current workout
     * </pre>
     */
    public void getIncline(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetInclineMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * sets the Incline to the given value
     * </pre>
     */
    public void setIncline(com.ifit.glassos.workout.InclineRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetInclineMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Incline updates relative to the current workout
     * </pre>
     */
    public void inclineSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getInclineSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Incline changes for the given workout ID
     * </pre>
     */
    public void getInclineHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetInclineHistoryMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * follow the current workout
     * </pre>
     */
    public void followWorkout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getFollowWorkoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * stop following the current workout
     * </pre>
     */
    public void stopFollowing(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStopFollowingMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * check whether the current workout is being followed
     * </pre>
     */
    public void getIsFollowing(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetIsFollowingMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to following updates
     * </pre>
     */
    public void isFollowingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getIsFollowingSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    public void getControls(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetControlsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to workout control updates
     * </pre>
     */
    public void controlsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getControlsSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service InclineService.
   */
  public static final class InclineServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<InclineServiceBlockingStub> {
    private InclineServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected InclineServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new InclineServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Incline is Readable and Writeable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.AvailabilityResponse canWrite(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanWriteMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Incline relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.InclineMetric getIncline(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetInclineMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * sets the Incline to the given value
     * </pre>
     */
    public com.ifit.glassos.workout.WorkoutResult setIncline(com.ifit.glassos.workout.InclineRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetInclineMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Incline updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.InclineMetric> inclineSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getInclineSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Incline changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.InclineMetricList getInclineHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetInclineHistoryMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * follow the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.WorkoutResult followWorkout(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getFollowWorkoutMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * stop following the current workout
     * </pre>
     */
    public com.ifit.glassos.util.Empty stopFollowing(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStopFollowingMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * check whether the current workout is being followed
     * </pre>
     */
    public com.ifit.glassos.util.BooleanResponse getIsFollowing(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetIsFollowingMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to following updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> isFollowingSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getIsFollowingSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    public com.ifit.glassos.workout.data.ControlList getControls(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetControlsMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to workout control updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.ControlList> controlsSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getControlsSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service InclineService.
   */
  public static final class InclineServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<InclineServiceFutureStub> {
    private InclineServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected InclineServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new InclineServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Incline is Readable and Writeable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canRead(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canWrite(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanWriteMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Incline relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.InclineMetric> getIncline(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetInclineMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * sets the Incline to the given value
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setIncline(
        com.ifit.glassos.workout.InclineRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetInclineMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Incline changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.InclineMetricList> getInclineHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetInclineHistoryMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * follow the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> followWorkout(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getFollowWorkoutMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * stop following the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> stopFollowing(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStopFollowingMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * check whether the current workout is being followed
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> getIsFollowing(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetIsFollowingMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current workout controls
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.data.ControlList> getControls(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetControlsMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE = 1;
  private static final int METHODID_GET_INCLINE = 2;
  private static final int METHODID_SET_INCLINE = 3;
  private static final int METHODID_INCLINE_SUBSCRIPTION = 4;
  private static final int METHODID_GET_INCLINE_HISTORY = 5;
  private static final int METHODID_FOLLOW_WORKOUT = 6;
  private static final int METHODID_STOP_FOLLOWING = 7;
  private static final int METHODID_GET_IS_FOLLOWING = 8;
  private static final int METHODID_IS_FOLLOWING_SUBSCRIPTION = 9;
  private static final int METHODID_GET_CONTROLS = 10;
  private static final int METHODID_CONTROLS_SUBSCRIPTION = 11;

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
        case METHODID_CAN_WRITE:
          serviceImpl.canWrite((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_GET_INCLINE:
          serviceImpl.getIncline((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetric>) responseObserver);
          break;
        case METHODID_SET_INCLINE:
          serviceImpl.setIncline((com.ifit.glassos.workout.InclineRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_INCLINE_SUBSCRIPTION:
          serviceImpl.inclineSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetric>) responseObserver);
          break;
        case METHODID_GET_INCLINE_HISTORY:
          serviceImpl.getInclineHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.InclineMetricList>) responseObserver);
          break;
        case METHODID_FOLLOW_WORKOUT:
          serviceImpl.followWorkout((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_STOP_FOLLOWING:
          serviceImpl.stopFollowing((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_IS_FOLLOWING:
          serviceImpl.getIsFollowing((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_IS_FOLLOWING_SUBSCRIPTION:
          serviceImpl.isFollowingSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_GET_CONTROLS:
          serviceImpl.getControls((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList>) responseObserver);
          break;
        case METHODID_CONTROLS_SUBSCRIPTION:
          serviceImpl.controlsSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.ControlList>) responseObserver);
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
          getCanWriteMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_WRITE)))
        .addMethod(
          getGetInclineMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.InclineMetric>(
                service, METHODID_GET_INCLINE)))
        .addMethod(
          getSetInclineMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.InclineRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_INCLINE)))
        .addMethod(
          getInclineSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.InclineMetric>(
                service, METHODID_INCLINE_SUBSCRIPTION)))
        .addMethod(
          getGetInclineHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.InclineMetricList>(
                service, METHODID_GET_INCLINE_HISTORY)))
        .addMethod(
          getFollowWorkoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_FOLLOW_WORKOUT)))
        .addMethod(
          getStopFollowingMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_STOP_FOLLOWING)))
        .addMethod(
          getGetIsFollowingMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_GET_IS_FOLLOWING)))
        .addMethod(
          getIsFollowingSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_FOLLOWING_SUBSCRIPTION)))
        .addMethod(
          getGetControlsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.ControlList>(
                service, METHODID_GET_CONTROLS)))
        .addMethod(
          getControlsSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.ControlList>(
                service, METHODID_CONTROLS_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (InclineServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetInclineMethod())
              .addMethod(getSetInclineMethod())
              .addMethod(getInclineSubscriptionMethod())
              .addMethod(getGetInclineHistoryMethod())
              .addMethod(getFollowWorkoutMethod())
              .addMethod(getStopFollowingMethod())
              .addMethod(getGetIsFollowingMethod())
              .addMethod(getIsFollowingSubscriptionMethod())
              .addMethod(getGetControlsMethod())
              .addMethod(getControlsSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

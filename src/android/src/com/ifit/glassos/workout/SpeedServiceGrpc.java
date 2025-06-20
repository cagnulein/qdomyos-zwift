package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/SpeedService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class SpeedServiceGrpc {

  private SpeedServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.SpeedService";

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
    if ((getCanReadMethod = SpeedServiceGrpc.getCanReadMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getCanReadMethod = SpeedServiceGrpc.getCanReadMethod) == null) {
          SpeedServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteMethod = SpeedServiceGrpc.getCanWriteMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getCanWriteMethod = SpeedServiceGrpc.getCanWriteMethod) == null) {
          SpeedServiceGrpc.getCanWriteMethod = getCanWriteMethod =
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
      com.ifit.glassos.workout.SpeedMetric> getGetSpeedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSpeed",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.SpeedMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.SpeedMetric> getGetSpeedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.SpeedMetric> getGetSpeedMethod;
    if ((getGetSpeedMethod = SpeedServiceGrpc.getGetSpeedMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getGetSpeedMethod = SpeedServiceGrpc.getGetSpeedMethod) == null) {
          SpeedServiceGrpc.getGetSpeedMethod = getGetSpeedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.SpeedMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSpeed"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.SpeedMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSpeedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.SpeedRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetSpeedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetSpeed",
      requestType = com.ifit.glassos.workout.SpeedRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.SpeedRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetSpeedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.SpeedRequest, com.ifit.glassos.workout.WorkoutResult> getSetSpeedMethod;
    if ((getSetSpeedMethod = SpeedServiceGrpc.getSetSpeedMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getSetSpeedMethod = SpeedServiceGrpc.getSetSpeedMethod) == null) {
          SpeedServiceGrpc.getSetSpeedMethod = getSetSpeedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.SpeedRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetSpeed"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.SpeedRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetSpeedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.SpeedMetric> getSpeedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SpeedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.SpeedMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.SpeedMetric> getSpeedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.SpeedMetric> getSpeedSubscriptionMethod;
    if ((getSpeedSubscriptionMethod = SpeedServiceGrpc.getSpeedSubscriptionMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getSpeedSubscriptionMethod = SpeedServiceGrpc.getSpeedSubscriptionMethod) == null) {
          SpeedServiceGrpc.getSpeedSubscriptionMethod = getSpeedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.SpeedMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SpeedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.SpeedMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSpeedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.SpeedMetricList> getGetSpeedHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSpeedHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.SpeedMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.SpeedMetricList> getGetSpeedHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.SpeedMetricList> getGetSpeedHistoryMethod;
    if ((getGetSpeedHistoryMethod = SpeedServiceGrpc.getGetSpeedHistoryMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getGetSpeedHistoryMethod = SpeedServiceGrpc.getGetSpeedHistoryMethod) == null) {
          SpeedServiceGrpc.getGetSpeedHistoryMethod = getGetSpeedHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.SpeedMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSpeedHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.SpeedMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSpeedHistoryMethod;
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
    if ((getFollowWorkoutMethod = SpeedServiceGrpc.getFollowWorkoutMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getFollowWorkoutMethod = SpeedServiceGrpc.getFollowWorkoutMethod) == null) {
          SpeedServiceGrpc.getFollowWorkoutMethod = getFollowWorkoutMethod =
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
    if ((getStopFollowingMethod = SpeedServiceGrpc.getStopFollowingMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getStopFollowingMethod = SpeedServiceGrpc.getStopFollowingMethod) == null) {
          SpeedServiceGrpc.getStopFollowingMethod = getStopFollowingMethod =
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
    if ((getGetIsFollowingMethod = SpeedServiceGrpc.getGetIsFollowingMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getGetIsFollowingMethod = SpeedServiceGrpc.getGetIsFollowingMethod) == null) {
          SpeedServiceGrpc.getGetIsFollowingMethod = getGetIsFollowingMethod =
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
    if ((getIsFollowingSubscriptionMethod = SpeedServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getIsFollowingSubscriptionMethod = SpeedServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
          SpeedServiceGrpc.getIsFollowingSubscriptionMethod = getIsFollowingSubscriptionMethod =
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
    if ((getGetControlsMethod = SpeedServiceGrpc.getGetControlsMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getGetControlsMethod = SpeedServiceGrpc.getGetControlsMethod) == null) {
          SpeedServiceGrpc.getGetControlsMethod = getGetControlsMethod =
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
    if ((getControlsSubscriptionMethod = SpeedServiceGrpc.getControlsSubscriptionMethod) == null) {
      synchronized (SpeedServiceGrpc.class) {
        if ((getControlsSubscriptionMethod = SpeedServiceGrpc.getControlsSubscriptionMethod) == null) {
          SpeedServiceGrpc.getControlsSubscriptionMethod = getControlsSubscriptionMethod =
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
  public static SpeedServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SpeedServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SpeedServiceStub>() {
        @java.lang.Override
        public SpeedServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SpeedServiceStub(channel, callOptions);
        }
      };
    return SpeedServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static SpeedServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SpeedServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SpeedServiceBlockingStub>() {
        @java.lang.Override
        public SpeedServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SpeedServiceBlockingStub(channel, callOptions);
        }
      };
    return SpeedServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static SpeedServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SpeedServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SpeedServiceFutureStub>() {
        @java.lang.Override
        public SpeedServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SpeedServiceFutureStub(channel, callOptions);
        }
      };
    return SpeedServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Speed is Readable and Writeable
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
     * get the current Speed relative to the current workout
     * </pre>
     */
    default void getSpeed(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSpeedMethod(), responseObserver);
    }

    /**
     * <pre>
     * sets the Speed to the given value
     * </pre>
     */
    default void setSpeed(com.ifit.glassos.workout.SpeedRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetSpeedMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Speed updates relative to the current workout
     * </pre>
     */
    default void speedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSpeedSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Speed changes for the given workout ID
     * </pre>
     */
    default void getSpeedHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSpeedHistoryMethod(), responseObserver);
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
   * Base class for the server implementation of the service SpeedService.
   */
  public static abstract class SpeedServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return SpeedServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service SpeedService.
   */
  public static final class SpeedServiceStub
      extends io.grpc.stub.AbstractAsyncStub<SpeedServiceStub> {
    private SpeedServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SpeedServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SpeedServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Speed is Readable and Writeable
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
     * get the current Speed relative to the current workout
     * </pre>
     */
    public void getSpeed(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSpeedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * sets the Speed to the given value
     * </pre>
     */
    public void setSpeed(com.ifit.glassos.workout.SpeedRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetSpeedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Speed updates relative to the current workout
     * </pre>
     */
    public void speedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getSpeedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Speed changes for the given workout ID
     * </pre>
     */
    public void getSpeedHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSpeedHistoryMethod(), getCallOptions()), request, responseObserver);
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
   * A stub to allow clients to do synchronous rpc calls to service SpeedService.
   */
  public static final class SpeedServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<SpeedServiceBlockingStub> {
    private SpeedServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SpeedServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SpeedServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Speed is Readable and Writeable
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
     * get the current Speed relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.SpeedMetric getSpeed(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSpeedMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * sets the Speed to the given value
     * </pre>
     */
    public com.ifit.glassos.workout.WorkoutResult setSpeed(com.ifit.glassos.workout.SpeedRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetSpeedMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Speed updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.SpeedMetric> speedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getSpeedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Speed changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.SpeedMetricList getSpeedHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSpeedHistoryMethod(), getCallOptions(), request);
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
   * A stub to allow clients to do ListenableFuture-style rpc calls to service SpeedService.
   */
  public static final class SpeedServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<SpeedServiceFutureStub> {
    private SpeedServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SpeedServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SpeedServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Speed is Readable and Writeable
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
     * get the current Speed relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.SpeedMetric> getSpeed(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSpeedMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * sets the Speed to the given value
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setSpeed(
        com.ifit.glassos.workout.SpeedRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetSpeedMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Speed changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.SpeedMetricList> getSpeedHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSpeedHistoryMethod(), getCallOptions()), request);
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
  private static final int METHODID_GET_SPEED = 2;
  private static final int METHODID_SET_SPEED = 3;
  private static final int METHODID_SPEED_SUBSCRIPTION = 4;
  private static final int METHODID_GET_SPEED_HISTORY = 5;
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
        case METHODID_GET_SPEED:
          serviceImpl.getSpeed((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetric>) responseObserver);
          break;
        case METHODID_SET_SPEED:
          serviceImpl.setSpeed((com.ifit.glassos.workout.SpeedRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_SPEED_SUBSCRIPTION:
          serviceImpl.speedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetric>) responseObserver);
          break;
        case METHODID_GET_SPEED_HISTORY:
          serviceImpl.getSpeedHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SpeedMetricList>) responseObserver);
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
          getGetSpeedMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.SpeedMetric>(
                service, METHODID_GET_SPEED)))
        .addMethod(
          getSetSpeedMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.SpeedRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_SPEED)))
        .addMethod(
          getSpeedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.SpeedMetric>(
                service, METHODID_SPEED_SUBSCRIPTION)))
        .addMethod(
          getGetSpeedHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.SpeedMetricList>(
                service, METHODID_GET_SPEED_HISTORY)))
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
      synchronized (SpeedServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetSpeedMethod())
              .addMethod(getSetSpeedMethod())
              .addMethod(getSpeedSubscriptionMethod())
              .addMethod(getGetSpeedHistoryMethod())
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

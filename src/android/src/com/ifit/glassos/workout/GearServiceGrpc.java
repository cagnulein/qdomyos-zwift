package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/GearService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class GearServiceGrpc {

  private GearServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.GearService";

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
    if ((getCanReadMethod = GearServiceGrpc.getCanReadMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getCanReadMethod = GearServiceGrpc.getCanReadMethod) == null) {
          GearServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteMethod = GearServiceGrpc.getCanWriteMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getCanWriteMethod = GearServiceGrpc.getCanWriteMethod) == null) {
          GearServiceGrpc.getCanWriteMethod = getCanWriteMethod =
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
      com.ifit.glassos.workout.GearMetric> getGetGearMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetGear",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.GearMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.GearMetric> getGetGearMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.GearMetric> getGetGearMethod;
    if ((getGetGearMethod = GearServiceGrpc.getGetGearMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getGetGearMethod = GearServiceGrpc.getGetGearMethod) == null) {
          GearServiceGrpc.getGetGearMethod = getGetGearMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.GearMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetGear"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.GearMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetGearMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.GearRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetGearMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetGear",
      requestType = com.ifit.glassos.workout.GearRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.GearRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetGearMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.GearRequest, com.ifit.glassos.workout.WorkoutResult> getSetGearMethod;
    if ((getSetGearMethod = GearServiceGrpc.getSetGearMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getSetGearMethod = GearServiceGrpc.getSetGearMethod) == null) {
          GearServiceGrpc.getSetGearMethod = getSetGearMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.GearRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetGear"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.GearRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetGearMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.GearMetric> getGearSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GearSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.GearMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.GearMetric> getGearSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.GearMetric> getGearSubscriptionMethod;
    if ((getGearSubscriptionMethod = GearServiceGrpc.getGearSubscriptionMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getGearSubscriptionMethod = GearServiceGrpc.getGearSubscriptionMethod) == null) {
          GearServiceGrpc.getGearSubscriptionMethod = getGearSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.GearMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GearSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.GearMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGearSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.GearMetricList> getGetGearHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetGearHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.GearMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.GearMetricList> getGetGearHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.GearMetricList> getGetGearHistoryMethod;
    if ((getGetGearHistoryMethod = GearServiceGrpc.getGetGearHistoryMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getGetGearHistoryMethod = GearServiceGrpc.getGetGearHistoryMethod) == null) {
          GearServiceGrpc.getGetGearHistoryMethod = getGetGearHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.GearMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetGearHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.GearMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetGearHistoryMethod;
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
    if ((getFollowWorkoutMethod = GearServiceGrpc.getFollowWorkoutMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getFollowWorkoutMethod = GearServiceGrpc.getFollowWorkoutMethod) == null) {
          GearServiceGrpc.getFollowWorkoutMethod = getFollowWorkoutMethod =
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
    if ((getStopFollowingMethod = GearServiceGrpc.getStopFollowingMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getStopFollowingMethod = GearServiceGrpc.getStopFollowingMethod) == null) {
          GearServiceGrpc.getStopFollowingMethod = getStopFollowingMethod =
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
    if ((getGetIsFollowingMethod = GearServiceGrpc.getGetIsFollowingMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getGetIsFollowingMethod = GearServiceGrpc.getGetIsFollowingMethod) == null) {
          GearServiceGrpc.getGetIsFollowingMethod = getGetIsFollowingMethod =
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
    if ((getIsFollowingSubscriptionMethod = GearServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getIsFollowingSubscriptionMethod = GearServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
          GearServiceGrpc.getIsFollowingSubscriptionMethod = getIsFollowingSubscriptionMethod =
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
    if ((getGetControlsMethod = GearServiceGrpc.getGetControlsMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getGetControlsMethod = GearServiceGrpc.getGetControlsMethod) == null) {
          GearServiceGrpc.getGetControlsMethod = getGetControlsMethod =
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
    if ((getControlsSubscriptionMethod = GearServiceGrpc.getControlsSubscriptionMethod) == null) {
      synchronized (GearServiceGrpc.class) {
        if ((getControlsSubscriptionMethod = GearServiceGrpc.getControlsSubscriptionMethod) == null) {
          GearServiceGrpc.getControlsSubscriptionMethod = getControlsSubscriptionMethod =
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
  public static GearServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<GearServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<GearServiceStub>() {
        @java.lang.Override
        public GearServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new GearServiceStub(channel, callOptions);
        }
      };
    return GearServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static GearServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<GearServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<GearServiceBlockingStub>() {
        @java.lang.Override
        public GearServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new GearServiceBlockingStub(channel, callOptions);
        }
      };
    return GearServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static GearServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<GearServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<GearServiceFutureStub>() {
        @java.lang.Override
        public GearServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new GearServiceFutureStub(channel, callOptions);
        }
      };
    return GearServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Gear is Readable and Writeable
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
     * get the current Gear relative to the current workout
     * </pre>
     */
    default void getGear(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetGearMethod(), responseObserver);
    }

    /**
     * <pre>
     * sets the Gear to the given value
     * </pre>
     */
    default void setGear(com.ifit.glassos.workout.GearRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetGearMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Gear updates relative to the current workout
     * </pre>
     */
    default void gearSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGearSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Gear changes for the given workout ID
     * </pre>
     */
    default void getGearHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetGearHistoryMethod(), responseObserver);
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
   * Base class for the server implementation of the service GearService.
   */
  public static abstract class GearServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return GearServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service GearService.
   */
  public static final class GearServiceStub
      extends io.grpc.stub.AbstractAsyncStub<GearServiceStub> {
    private GearServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected GearServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new GearServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Gear is Readable and Writeable
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
     * get the current Gear relative to the current workout
     * </pre>
     */
    public void getGear(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetGearMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * sets the Gear to the given value
     * </pre>
     */
    public void setGear(com.ifit.glassos.workout.GearRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetGearMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Gear updates relative to the current workout
     * </pre>
     */
    public void gearSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getGearSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Gear changes for the given workout ID
     * </pre>
     */
    public void getGearHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetGearHistoryMethod(), getCallOptions()), request, responseObserver);
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
   * A stub to allow clients to do synchronous rpc calls to service GearService.
   */
  public static final class GearServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<GearServiceBlockingStub> {
    private GearServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected GearServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new GearServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Gear is Readable and Writeable
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
     * get the current Gear relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.GearMetric getGear(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetGearMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * sets the Gear to the given value
     * </pre>
     */
    public com.ifit.glassos.workout.WorkoutResult setGear(com.ifit.glassos.workout.GearRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetGearMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Gear updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.GearMetric> gearSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getGearSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Gear changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.GearMetricList getGearHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetGearHistoryMethod(), getCallOptions(), request);
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
   * A stub to allow clients to do ListenableFuture-style rpc calls to service GearService.
   */
  public static final class GearServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<GearServiceFutureStub> {
    private GearServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected GearServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new GearServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Gear is Readable and Writeable
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
     * get the current Gear relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.GearMetric> getGear(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetGearMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * sets the Gear to the given value
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setGear(
        com.ifit.glassos.workout.GearRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetGearMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Gear changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.GearMetricList> getGearHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetGearHistoryMethod(), getCallOptions()), request);
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
  private static final int METHODID_GET_GEAR = 2;
  private static final int METHODID_SET_GEAR = 3;
  private static final int METHODID_GEAR_SUBSCRIPTION = 4;
  private static final int METHODID_GET_GEAR_HISTORY = 5;
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
        case METHODID_GET_GEAR:
          serviceImpl.getGear((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetric>) responseObserver);
          break;
        case METHODID_SET_GEAR:
          serviceImpl.setGear((com.ifit.glassos.workout.GearRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_GEAR_SUBSCRIPTION:
          serviceImpl.gearSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetric>) responseObserver);
          break;
        case METHODID_GET_GEAR_HISTORY:
          serviceImpl.getGearHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.GearMetricList>) responseObserver);
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
          getGetGearMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.GearMetric>(
                service, METHODID_GET_GEAR)))
        .addMethod(
          getSetGearMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.GearRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_GEAR)))
        .addMethod(
          getGearSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.GearMetric>(
                service, METHODID_GEAR_SUBSCRIPTION)))
        .addMethod(
          getGetGearHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.GearMetricList>(
                service, METHODID_GET_GEAR_HISTORY)))
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
      synchronized (GearServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetGearMethod())
              .addMethod(getSetGearMethod())
              .addMethod(getGearSubscriptionMethod())
              .addMethod(getGetGearHistoryMethod())
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

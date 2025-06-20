package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/ResistanceService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ResistanceServiceGrpc {

  private ResistanceServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ResistanceService";

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
    if ((getCanReadMethod = ResistanceServiceGrpc.getCanReadMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getCanReadMethod = ResistanceServiceGrpc.getCanReadMethod) == null) {
          ResistanceServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteMethod = ResistanceServiceGrpc.getCanWriteMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getCanWriteMethod = ResistanceServiceGrpc.getCanWriteMethod) == null) {
          ResistanceServiceGrpc.getCanWriteMethod = getCanWriteMethod =
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
      com.ifit.glassos.workout.ResistanceMetric> getGetResistanceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetResistance",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ResistanceMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ResistanceMetric> getGetResistanceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ResistanceMetric> getGetResistanceMethod;
    if ((getGetResistanceMethod = ResistanceServiceGrpc.getGetResistanceMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getGetResistanceMethod = ResistanceServiceGrpc.getGetResistanceMethod) == null) {
          ResistanceServiceGrpc.getGetResistanceMethod = getGetResistanceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ResistanceMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetResistance"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ResistanceMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetResistanceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.ResistanceRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetResistanceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetResistance",
      requestType = com.ifit.glassos.workout.ResistanceRequest.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.ResistanceRequest,
      com.ifit.glassos.workout.WorkoutResult> getSetResistanceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.ResistanceRequest, com.ifit.glassos.workout.WorkoutResult> getSetResistanceMethod;
    if ((getSetResistanceMethod = ResistanceServiceGrpc.getSetResistanceMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getSetResistanceMethod = ResistanceServiceGrpc.getSetResistanceMethod) == null) {
          ResistanceServiceGrpc.getSetResistanceMethod = getSetResistanceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.ResistanceRequest, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetResistance"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ResistanceRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetResistanceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ResistanceMetric> getResistanceSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ResistanceSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ResistanceMetric.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ResistanceMetric> getResistanceSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ResistanceMetric> getResistanceSubscriptionMethod;
    if ((getResistanceSubscriptionMethod = ResistanceServiceGrpc.getResistanceSubscriptionMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getResistanceSubscriptionMethod = ResistanceServiceGrpc.getResistanceSubscriptionMethod) == null) {
          ResistanceServiceGrpc.getResistanceSubscriptionMethod = getResistanceSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ResistanceMetric>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ResistanceSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ResistanceMetric.getDefaultInstance()))
              .build();
        }
      }
    }
    return getResistanceSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.ResistanceMetricList> getGetResistanceHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetResistanceHistory",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.ResistanceMetricList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.ResistanceMetricList> getGetResistanceHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.ResistanceMetricList> getGetResistanceHistoryMethod;
    if ((getGetResistanceHistoryMethod = ResistanceServiceGrpc.getGetResistanceHistoryMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getGetResistanceHistoryMethod = ResistanceServiceGrpc.getGetResistanceHistoryMethod) == null) {
          ResistanceServiceGrpc.getGetResistanceHistoryMethod = getGetResistanceHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.ResistanceMetricList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetResistanceHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ResistanceMetricList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetResistanceHistoryMethod;
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
    if ((getFollowWorkoutMethod = ResistanceServiceGrpc.getFollowWorkoutMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getFollowWorkoutMethod = ResistanceServiceGrpc.getFollowWorkoutMethod) == null) {
          ResistanceServiceGrpc.getFollowWorkoutMethod = getFollowWorkoutMethod =
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
    if ((getStopFollowingMethod = ResistanceServiceGrpc.getStopFollowingMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getStopFollowingMethod = ResistanceServiceGrpc.getStopFollowingMethod) == null) {
          ResistanceServiceGrpc.getStopFollowingMethod = getStopFollowingMethod =
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
    if ((getGetIsFollowingMethod = ResistanceServiceGrpc.getGetIsFollowingMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getGetIsFollowingMethod = ResistanceServiceGrpc.getGetIsFollowingMethod) == null) {
          ResistanceServiceGrpc.getGetIsFollowingMethod = getGetIsFollowingMethod =
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
    if ((getIsFollowingSubscriptionMethod = ResistanceServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getIsFollowingSubscriptionMethod = ResistanceServiceGrpc.getIsFollowingSubscriptionMethod) == null) {
          ResistanceServiceGrpc.getIsFollowingSubscriptionMethod = getIsFollowingSubscriptionMethod =
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
    if ((getGetControlsMethod = ResistanceServiceGrpc.getGetControlsMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getGetControlsMethod = ResistanceServiceGrpc.getGetControlsMethod) == null) {
          ResistanceServiceGrpc.getGetControlsMethod = getGetControlsMethod =
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
    if ((getControlsSubscriptionMethod = ResistanceServiceGrpc.getControlsSubscriptionMethod) == null) {
      synchronized (ResistanceServiceGrpc.class) {
        if ((getControlsSubscriptionMethod = ResistanceServiceGrpc.getControlsSubscriptionMethod) == null) {
          ResistanceServiceGrpc.getControlsSubscriptionMethod = getControlsSubscriptionMethod =
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
  public static ResistanceServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ResistanceServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ResistanceServiceStub>() {
        @java.lang.Override
        public ResistanceServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ResistanceServiceStub(channel, callOptions);
        }
      };
    return ResistanceServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ResistanceServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ResistanceServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ResistanceServiceBlockingStub>() {
        @java.lang.Override
        public ResistanceServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ResistanceServiceBlockingStub(channel, callOptions);
        }
      };
    return ResistanceServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ResistanceServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ResistanceServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ResistanceServiceFutureStub>() {
        @java.lang.Override
        public ResistanceServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ResistanceServiceFutureStub(channel, callOptions);
        }
      };
    return ResistanceServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Resistance is Readable and Writeable
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
     * get the current Resistance relative to the current workout
     * </pre>
     */
    default void getResistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetResistanceMethod(), responseObserver);
    }

    /**
     * <pre>
     * sets the Resistance to the given value
     * </pre>
     */
    default void setResistance(com.ifit.glassos.workout.ResistanceRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetResistanceMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Resistance updates relative to the current workout
     * </pre>
     */
    default void resistanceSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetric> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getResistanceSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Resistance changes for the given workout ID
     * </pre>
     */
    default void getResistanceHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetricList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetResistanceHistoryMethod(), responseObserver);
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
   * Base class for the server implementation of the service ResistanceService.
   */
  public static abstract class ResistanceServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ResistanceServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ResistanceService.
   */
  public static final class ResistanceServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ResistanceServiceStub> {
    private ResistanceServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ResistanceServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ResistanceServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Resistance is Readable and Writeable
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
     * get the current Resistance relative to the current workout
     * </pre>
     */
    public void getResistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetResistanceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * sets the Resistance to the given value
     * </pre>
     */
    public void setResistance(com.ifit.glassos.workout.ResistanceRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetResistanceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Resistance updates relative to the current workout
     * </pre>
     */
    public void resistanceSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetric> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getResistanceSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * gets a list of all of the Resistance changes for the given workout ID
     * </pre>
     */
    public void getResistanceHistory(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetricList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetResistanceHistoryMethod(), getCallOptions()), request, responseObserver);
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
   * A stub to allow clients to do synchronous rpc calls to service ResistanceService.
   */
  public static final class ResistanceServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ResistanceServiceBlockingStub> {
    private ResistanceServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ResistanceServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ResistanceServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Resistance is Readable and Writeable
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
     * get the current Resistance relative to the current workout
     * </pre>
     */
    public com.ifit.glassos.workout.ResistanceMetric getResistance(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetResistanceMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * sets the Resistance to the given value
     * </pre>
     */
    public com.ifit.glassos.workout.WorkoutResult setResistance(com.ifit.glassos.workout.ResistanceRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetResistanceMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Resistance updates relative to the current workout
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.ResistanceMetric> resistanceSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getResistanceSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * gets a list of all of the Resistance changes for the given workout ID
     * </pre>
     */
    public com.ifit.glassos.workout.ResistanceMetricList getResistanceHistory(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetResistanceHistoryMethod(), getCallOptions(), request);
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
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ResistanceService.
   */
  public static final class ResistanceServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ResistanceServiceFutureStub> {
    private ResistanceServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ResistanceServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ResistanceServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Resistance is Readable and Writeable
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
     * get the current Resistance relative to the current workout
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ResistanceMetric> getResistance(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetResistanceMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * sets the Resistance to the given value
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> setResistance(
        com.ifit.glassos.workout.ResistanceRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetResistanceMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * gets a list of all of the Resistance changes for the given workout ID
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ResistanceMetricList> getResistanceHistory(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetResistanceHistoryMethod(), getCallOptions()), request);
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
  private static final int METHODID_GET_RESISTANCE = 2;
  private static final int METHODID_SET_RESISTANCE = 3;
  private static final int METHODID_RESISTANCE_SUBSCRIPTION = 4;
  private static final int METHODID_GET_RESISTANCE_HISTORY = 5;
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
        case METHODID_GET_RESISTANCE:
          serviceImpl.getResistance((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetric>) responseObserver);
          break;
        case METHODID_SET_RESISTANCE:
          serviceImpl.setResistance((com.ifit.glassos.workout.ResistanceRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_RESISTANCE_SUBSCRIPTION:
          serviceImpl.resistanceSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetric>) responseObserver);
          break;
        case METHODID_GET_RESISTANCE_HISTORY:
          serviceImpl.getResistanceHistory((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ResistanceMetricList>) responseObserver);
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
          getGetResistanceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ResistanceMetric>(
                service, METHODID_GET_RESISTANCE)))
        .addMethod(
          getSetResistanceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.ResistanceRequest,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_SET_RESISTANCE)))
        .addMethod(
          getResistanceSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ResistanceMetric>(
                service, METHODID_RESISTANCE_SUBSCRIPTION)))
        .addMethod(
          getGetResistanceHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.ResistanceMetricList>(
                service, METHODID_GET_RESISTANCE_HISTORY)))
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
      synchronized (ResistanceServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetResistanceMethod())
              .addMethod(getSetResistanceMethod())
              .addMethod(getResistanceSubscriptionMethod())
              .addMethod(getGetResistanceHistoryMethod())
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

package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/ProgrammedWorkoutSessionService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ProgrammedWorkoutSessionServiceGrpc {

  private ProgrammedWorkoutSessionServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ProgrammedWorkoutSessionService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.WorkoutSegmentDescriptor,
      com.ifit.glassos.util.Empty> getPreloadWorkoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "PreloadWorkout",
      requestType = com.ifit.glassos.workout.data.WorkoutSegmentDescriptor.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.WorkoutSegmentDescriptor,
      com.ifit.glassos.util.Empty> getPreloadWorkoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.WorkoutSegmentDescriptor, com.ifit.glassos.util.Empty> getPreloadWorkoutMethod;
    if ((getPreloadWorkoutMethod = ProgrammedWorkoutSessionServiceGrpc.getPreloadWorkoutMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getPreloadWorkoutMethod = ProgrammedWorkoutSessionServiceGrpc.getPreloadWorkoutMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getPreloadWorkoutMethod = getPreloadWorkoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.data.WorkoutSegmentDescriptor, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "PreloadWorkout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.WorkoutSegmentDescriptor.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPreloadWorkoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutList> getPreloadedWorkoutsChangedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "PreloadedWorkoutsChangedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.WorkoutList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutList> getPreloadedWorkoutsChangedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutList> getPreloadedWorkoutsChangedSubscriptionMethod;
    if ((getPreloadedWorkoutsChangedSubscriptionMethod = ProgrammedWorkoutSessionServiceGrpc.getPreloadedWorkoutsChangedSubscriptionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getPreloadedWorkoutsChangedSubscriptionMethod = ProgrammedWorkoutSessionServiceGrpc.getPreloadedWorkoutsChangedSubscriptionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getPreloadedWorkoutsChangedSubscriptionMethod = getPreloadedWorkoutsChangedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "PreloadedWorkoutsChangedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.WorkoutList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPreloadedWorkoutsChangedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getSessionIsStartingSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SessionIsStartingSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getSessionIsStartingSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getSessionIsStartingSubscriptionMethod;
    if ((getSessionIsStartingSubscriptionMethod = ProgrammedWorkoutSessionServiceGrpc.getSessionIsStartingSubscriptionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getSessionIsStartingSubscriptionMethod = ProgrammedWorkoutSessionServiceGrpc.getSessionIsStartingSubscriptionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getSessionIsStartingSubscriptionMethod = getSessionIsStartingSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SessionIsStartingSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSessionIsStartingSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getManualStartRequestedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ManualStartRequestedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getManualStartRequestedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getManualStartRequestedSubscriptionMethod;
    if ((getManualStartRequestedSubscriptionMethod = ProgrammedWorkoutSessionServiceGrpc.getManualStartRequestedSubscriptionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getManualStartRequestedSubscriptionMethod = ProgrammedWorkoutSessionServiceGrpc.getManualStartRequestedSubscriptionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getManualStartRequestedSubscriptionMethod = getManualStartRequestedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ManualStartRequestedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getManualStartRequestedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAddAndStartMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AddAndStart",
      requestType = com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAddAndStartMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAddAndStartMethod;
    if ((getAddAndStartMethod = ProgrammedWorkoutSessionServiceGrpc.getAddAndStartMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getAddAndStartMethod = ProgrammedWorkoutSessionServiceGrpc.getAddAndStartMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getAddAndStartMethod = getAddAndStartMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AddAndStart"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAddAndStartMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAddAllWorkoutSegmentsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AddAllWorkoutSegments",
      requestType = com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAddAllWorkoutSegmentsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAddAllWorkoutSegmentsMethod;
    if ((getAddAllWorkoutSegmentsMethod = ProgrammedWorkoutSessionServiceGrpc.getAddAllWorkoutSegmentsMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getAddAllWorkoutSegmentsMethod = ProgrammedWorkoutSessionServiceGrpc.getAddAllWorkoutSegmentsMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getAddAllWorkoutSegmentsMethod = getAddAllWorkoutSegmentsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AddAllWorkoutSegments"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAddAllWorkoutSegmentsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getClearRemainingWorkoutSegmentsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ClearRemainingWorkoutSegments",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getClearRemainingWorkoutSegmentsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getClearRemainingWorkoutSegmentsMethod;
    if ((getClearRemainingWorkoutSegmentsMethod = ProgrammedWorkoutSessionServiceGrpc.getClearRemainingWorkoutSegmentsMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getClearRemainingWorkoutSegmentsMethod = ProgrammedWorkoutSessionServiceGrpc.getClearRemainingWorkoutSegmentsMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getClearRemainingWorkoutSegmentsMethod = getClearRemainingWorkoutSegmentsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ClearRemainingWorkoutSegments"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getClearRemainingWorkoutSegmentsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getStartMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Start",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getStartMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getStartMethod;
    if ((getStartMethod = ProgrammedWorkoutSessionServiceGrpc.getStartMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getStartMethod = ProgrammedWorkoutSessionServiceGrpc.getStartMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getStartMethod = getStartMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Start"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStartMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getStopMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Stop",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getStopMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getStopMethod;
    if ((getStopMethod = ProgrammedWorkoutSessionServiceGrpc.getStopMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getStopMethod = ProgrammedWorkoutSessionServiceGrpc.getStopMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getStopMethod = getStopMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Stop"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStopMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getNextMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Next",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getNextMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getNextMethod;
    if ((getNextMethod = ProgrammedWorkoutSessionServiceGrpc.getNextMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getNextMethod = ProgrammedWorkoutSessionServiceGrpc.getNextMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getNextMethod = getNextMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Next"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getNextMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getPauseMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Pause",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getPauseMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getPauseMethod;
    if ((getPauseMethod = ProgrammedWorkoutSessionServiceGrpc.getPauseMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getPauseMethod = ProgrammedWorkoutSessionServiceGrpc.getPauseMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getPauseMethod = getPauseMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Pause"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPauseMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getResumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Resume",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getResumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getResumeMethod;
    if ((getResumeMethod = ProgrammedWorkoutSessionServiceGrpc.getResumeMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getResumeMethod = ProgrammedWorkoutSessionServiceGrpc.getResumeMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getResumeMethod = getResumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Resume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getResumeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionItemList> getGetCurrentSessionWorkoutsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentSessionWorkouts",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.WorkoutSessionItemList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionItemList> getGetCurrentSessionWorkoutsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionItemList> getGetCurrentSessionWorkoutsMethod;
    if ((getGetCurrentSessionWorkoutsMethod = ProgrammedWorkoutSessionServiceGrpc.getGetCurrentSessionWorkoutsMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getGetCurrentSessionWorkoutsMethod = ProgrammedWorkoutSessionServiceGrpc.getGetCurrentSessionWorkoutsMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getGetCurrentSessionWorkoutsMethod = getGetCurrentSessionWorkoutsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionItemList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentSessionWorkouts"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.WorkoutSessionItemList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentSessionWorkoutsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionItemList> getSessionWorkoutsChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SessionWorkoutsChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.WorkoutSessionItemList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionItemList> getSessionWorkoutsChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionItemList> getSessionWorkoutsChangedMethod;
    if ((getSessionWorkoutsChangedMethod = ProgrammedWorkoutSessionServiceGrpc.getSessionWorkoutsChangedMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getSessionWorkoutsChangedMethod = ProgrammedWorkoutSessionServiceGrpc.getSessionWorkoutsChangedMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getSessionWorkoutsChangedMethod = getSessionWorkoutsChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionItemList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SessionWorkoutsChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.WorkoutSessionItemList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSessionWorkoutsChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionState> getGetCurrentWorkoutSessionStateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentWorkoutSessionState",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.WorkoutSessionState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionState> getGetCurrentWorkoutSessionStateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionState> getGetCurrentWorkoutSessionStateMethod;
    if ((getGetCurrentWorkoutSessionStateMethod = ProgrammedWorkoutSessionServiceGrpc.getGetCurrentWorkoutSessionStateMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getGetCurrentWorkoutSessionStateMethod = ProgrammedWorkoutSessionServiceGrpc.getGetCurrentWorkoutSessionStateMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getGetCurrentWorkoutSessionStateMethod = getGetCurrentWorkoutSessionStateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentWorkoutSessionState"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.WorkoutSessionState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentWorkoutSessionStateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionState> getWorkoutSessionStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "WorkoutSessionStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.WorkoutSessionState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.WorkoutSessionState> getWorkoutSessionStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionState> getWorkoutSessionStateChangedMethod;
    if ((getWorkoutSessionStateChangedMethod = ProgrammedWorkoutSessionServiceGrpc.getWorkoutSessionStateChangedMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getWorkoutSessionStateChangedMethod = ProgrammedWorkoutSessionServiceGrpc.getWorkoutSessionStateChangedMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getWorkoutSessionStateChangedMethod = getWorkoutSessionStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.WorkoutSessionState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "WorkoutSessionStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.WorkoutSessionState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getWorkoutSessionStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAssertPositionControlMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AssertPositionControl",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAssertPositionControlMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getAssertPositionControlMethod;
    if ((getAssertPositionControlMethod = ProgrammedWorkoutSessionServiceGrpc.getAssertPositionControlMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getAssertPositionControlMethod = ProgrammedWorkoutSessionServiceGrpc.getAssertPositionControlMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getAssertPositionControlMethod = getAssertPositionControlMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AssertPositionControl"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAssertPositionControlMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getReleasePositionControlMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ReleasePositionControl",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getReleasePositionControlMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getReleasePositionControlMethod;
    if ((getReleasePositionControlMethod = ProgrammedWorkoutSessionServiceGrpc.getReleasePositionControlMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getReleasePositionControlMethod = ProgrammedWorkoutSessionServiceGrpc.getReleasePositionControlMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getReleasePositionControlMethod = getReleasePositionControlMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ReleasePositionControl"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getReleasePositionControlMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.StringResponse> getGetPositionControllerMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetPositionController",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.StringResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.StringResponse> getGetPositionControllerMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.StringResponse> getGetPositionControllerMethod;
    if ((getGetPositionControllerMethod = ProgrammedWorkoutSessionServiceGrpc.getGetPositionControllerMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getGetPositionControllerMethod = ProgrammedWorkoutSessionServiceGrpc.getGetPositionControllerMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getGetPositionControllerMethod = getGetPositionControllerMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.StringResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetPositionController"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.StringResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetPositionControllerMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.SetAtPositionRequest,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getSetAtPositionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetAtPosition",
      requestType = com.ifit.glassos.workout.SetAtPositionRequest.class,
      responseType = com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.SetAtPositionRequest,
      com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getSetAtPositionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.SetAtPositionRequest, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> getSetAtPositionMethod;
    if ((getSetAtPositionMethod = ProgrammedWorkoutSessionServiceGrpc.getSetAtPositionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getSetAtPositionMethod = ProgrammedWorkoutSessionServiceGrpc.getSetAtPositionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getSetAtPositionMethod = getSetAtPositionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.SetAtPositionRequest, com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetAtPosition"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.SetAtPositionRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetAtPositionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgramPosition> getGetCurrentProgramPositionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentProgramPosition",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgramPosition.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgramPosition> getGetCurrentProgramPositionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgramPosition> getGetCurrentProgramPositionMethod;
    if ((getGetCurrentProgramPositionMethod = ProgrammedWorkoutSessionServiceGrpc.getGetCurrentProgramPositionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getGetCurrentProgramPositionMethod = ProgrammedWorkoutSessionServiceGrpc.getGetCurrentProgramPositionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getGetCurrentProgramPositionMethod = getGetCurrentProgramPositionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgramPosition>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentProgramPosition"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgramPosition.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentProgramPositionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgramPosition> getProgramPositionChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ProgramPositionChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.ProgramPosition.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.ProgramPosition> getProgramPositionChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgramPosition> getProgramPositionChangedMethod;
    if ((getProgramPositionChangedMethod = ProgrammedWorkoutSessionServiceGrpc.getProgramPositionChangedMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getProgramPositionChangedMethod = ProgrammedWorkoutSessionServiceGrpc.getProgramPositionChangedMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getProgramPositionChangedMethod = getProgramPositionChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.ProgramPosition>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ProgramPositionChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.ProgramPosition.getDefaultInstance()))
              .build();
        }
      }
    }
    return getProgramPositionChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RecoveredSessionResponse> getGetLatestUnfinishedWorkoutSessionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetLatestUnfinishedWorkoutSession",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.RecoveredSessionResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RecoveredSessionResponse> getGetLatestUnfinishedWorkoutSessionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RecoveredSessionResponse> getGetLatestUnfinishedWorkoutSessionMethod;
    if ((getGetLatestUnfinishedWorkoutSessionMethod = ProgrammedWorkoutSessionServiceGrpc.getGetLatestUnfinishedWorkoutSessionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getGetLatestUnfinishedWorkoutSessionMethod = ProgrammedWorkoutSessionServiceGrpc.getGetLatestUnfinishedWorkoutSessionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getGetLatestUnfinishedWorkoutSessionMethod = getGetLatestUnfinishedWorkoutSessionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RecoveredSessionResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetLatestUnfinishedWorkoutSession"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.RecoveredSessionResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetLatestUnfinishedWorkoutSessionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getHasUnfinishedSessionAvailableMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "HasUnfinishedSessionAvailable",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getHasUnfinishedSessionAvailableMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getHasUnfinishedSessionAvailableMethod;
    if ((getHasUnfinishedSessionAvailableMethod = ProgrammedWorkoutSessionServiceGrpc.getHasUnfinishedSessionAvailableMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getHasUnfinishedSessionAvailableMethod = ProgrammedWorkoutSessionServiceGrpc.getHasUnfinishedSessionAvailableMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getHasUnfinishedSessionAvailableMethod = getHasUnfinishedSessionAvailableMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "HasUnfinishedSessionAvailable"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getHasUnfinishedSessionAvailableMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.recovery.RecoveredSession,
      com.ifit.glassos.util.Empty> getRunUnfinishedSessionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RunUnfinishedSession",
      requestType = com.ifit.glassos.workout.data.recovery.RecoveredSession.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.recovery.RecoveredSession,
      com.ifit.glassos.util.Empty> getRunUnfinishedSessionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.recovery.RecoveredSession, com.ifit.glassos.util.Empty> getRunUnfinishedSessionMethod;
    if ((getRunUnfinishedSessionMethod = ProgrammedWorkoutSessionServiceGrpc.getRunUnfinishedSessionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getRunUnfinishedSessionMethod = ProgrammedWorkoutSessionServiceGrpc.getRunUnfinishedSessionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getRunUnfinishedSessionMethod = getRunUnfinishedSessionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.data.recovery.RecoveredSession, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RunUnfinishedSession"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.recovery.RecoveredSession.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRunUnfinishedSessionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.recovery.RecoveredSession,
      com.ifit.glassos.util.Empty> getMarkUnfinishedSessionAsCompleteMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "MarkUnfinishedSessionAsComplete",
      requestType = com.ifit.glassos.workout.data.recovery.RecoveredSession.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.recovery.RecoveredSession,
      com.ifit.glassos.util.Empty> getMarkUnfinishedSessionAsCompleteMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.data.recovery.RecoveredSession, com.ifit.glassos.util.Empty> getMarkUnfinishedSessionAsCompleteMethod;
    if ((getMarkUnfinishedSessionAsCompleteMethod = ProgrammedWorkoutSessionServiceGrpc.getMarkUnfinishedSessionAsCompleteMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getMarkUnfinishedSessionAsCompleteMethod = ProgrammedWorkoutSessionServiceGrpc.getMarkUnfinishedSessionAsCompleteMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getMarkUnfinishedSessionAsCompleteMethod = getMarkUnfinishedSessionAsCompleteMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.data.recovery.RecoveredSession, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "MarkUnfinishedSessionAsComplete"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.recovery.RecoveredSession.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getMarkUnfinishedSessionAsCompleteMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getClearUnfinishedSessionsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ClearUnfinishedSessions",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getClearUnfinishedSessionsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getClearUnfinishedSessionsMethod;
    if ((getClearUnfinishedSessionsMethod = ProgrammedWorkoutSessionServiceGrpc.getClearUnfinishedSessionsMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getClearUnfinishedSessionsMethod = ProgrammedWorkoutSessionServiceGrpc.getClearUnfinishedSessionsMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getClearUnfinishedSessionsMethod = getClearUnfinishedSessionsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ClearUnfinishedSessions"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getClearUnfinishedSessionsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RecoveredSessionResponse> getGetRunningUnfinishedSessionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetRunningUnfinishedSession",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.RecoveredSessionResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.RecoveredSessionResponse> getGetRunningUnfinishedSessionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RecoveredSessionResponse> getGetRunningUnfinishedSessionMethod;
    if ((getGetRunningUnfinishedSessionMethod = ProgrammedWorkoutSessionServiceGrpc.getGetRunningUnfinishedSessionMethod) == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        if ((getGetRunningUnfinishedSessionMethod = ProgrammedWorkoutSessionServiceGrpc.getGetRunningUnfinishedSessionMethod) == null) {
          ProgrammedWorkoutSessionServiceGrpc.getGetRunningUnfinishedSessionMethod = getGetRunningUnfinishedSessionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.RecoveredSessionResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetRunningUnfinishedSession"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.RecoveredSessionResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetRunningUnfinishedSessionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ProgrammedWorkoutSessionServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ProgrammedWorkoutSessionServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ProgrammedWorkoutSessionServiceStub>() {
        @java.lang.Override
        public ProgrammedWorkoutSessionServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ProgrammedWorkoutSessionServiceStub(channel, callOptions);
        }
      };
    return ProgrammedWorkoutSessionServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ProgrammedWorkoutSessionServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ProgrammedWorkoutSessionServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ProgrammedWorkoutSessionServiceBlockingStub>() {
        @java.lang.Override
        public ProgrammedWorkoutSessionServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ProgrammedWorkoutSessionServiceBlockingStub(channel, callOptions);
        }
      };
    return ProgrammedWorkoutSessionServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ProgrammedWorkoutSessionServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ProgrammedWorkoutSessionServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ProgrammedWorkoutSessionServiceFutureStub>() {
        @java.lang.Override
        public ProgrammedWorkoutSessionServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ProgrammedWorkoutSessionServiceFutureStub(channel, callOptions);
        }
      };
    return ProgrammedWorkoutSessionServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void preloadWorkout(com.ifit.glassos.workout.data.WorkoutSegmentDescriptor request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPreloadWorkoutMethod(), responseObserver);
    }

    /**
     */
    default void preloadedWorkoutsChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPreloadedWorkoutsChangedSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void sessionIsStartingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSessionIsStartingSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void manualStartRequestedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getManualStartRequestedSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void addAndStart(com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAddAndStartMethod(), responseObserver);
    }

    /**
     */
    default void addAllWorkoutSegments(com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAddAllWorkoutSegmentsMethod(), responseObserver);
    }

    /**
     */
    default void clearRemainingWorkoutSegments(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getClearRemainingWorkoutSegmentsMethod(), responseObserver);
    }

    /**
     */
    default void start(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStartMethod(), responseObserver);
    }

    /**
     */
    default void stop(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStopMethod(), responseObserver);
    }

    /**
     */
    default void next(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getNextMethod(), responseObserver);
    }

    /**
     */
    default void pause(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPauseMethod(), responseObserver);
    }

    /**
     */
    default void resume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getResumeMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentSessionWorkouts(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionItemList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentSessionWorkoutsMethod(), responseObserver);
    }

    /**
     */
    default void sessionWorkoutsChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionItemList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSessionWorkoutsChangedMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentWorkoutSessionState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentWorkoutSessionStateMethod(), responseObserver);
    }

    /**
     */
    default void workoutSessionStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getWorkoutSessionStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void assertPositionControl(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAssertPositionControlMethod(), responseObserver);
    }

    /**
     */
    default void releasePositionControl(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getReleasePositionControlMethod(), responseObserver);
    }

    /**
     */
    default void getPositionController(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.StringResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetPositionControllerMethod(), responseObserver);
    }

    /**
     */
    default void setAtPosition(com.ifit.glassos.workout.SetAtPositionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetAtPositionMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentProgramPosition(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgramPosition> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentProgramPositionMethod(), responseObserver);
    }

    /**
     */
    default void programPositionChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgramPosition> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getProgramPositionChangedMethod(), responseObserver);
    }

    /**
     */
    default void getLatestUnfinishedWorkoutSession(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RecoveredSessionResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetLatestUnfinishedWorkoutSessionMethod(), responseObserver);
    }

    /**
     */
    default void hasUnfinishedSessionAvailable(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getHasUnfinishedSessionAvailableMethod(), responseObserver);
    }

    /**
     */
    default void runUnfinishedSession(com.ifit.glassos.workout.data.recovery.RecoveredSession request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRunUnfinishedSessionMethod(), responseObserver);
    }

    /**
     */
    default void markUnfinishedSessionAsComplete(com.ifit.glassos.workout.data.recovery.RecoveredSession request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getMarkUnfinishedSessionAsCompleteMethod(), responseObserver);
    }

    /**
     */
    default void clearUnfinishedSessions(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getClearUnfinishedSessionsMethod(), responseObserver);
    }

    /**
     */
    default void getRunningUnfinishedSession(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RecoveredSessionResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetRunningUnfinishedSessionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ProgrammedWorkoutSessionService.
   */
  public static abstract class ProgrammedWorkoutSessionServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ProgrammedWorkoutSessionServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ProgrammedWorkoutSessionService.
   */
  public static final class ProgrammedWorkoutSessionServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ProgrammedWorkoutSessionServiceStub> {
    private ProgrammedWorkoutSessionServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ProgrammedWorkoutSessionServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ProgrammedWorkoutSessionServiceStub(channel, callOptions);
    }

    /**
     */
    public void preloadWorkout(com.ifit.glassos.workout.data.WorkoutSegmentDescriptor request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getPreloadWorkoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void preloadedWorkoutsChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getPreloadedWorkoutsChangedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void sessionIsStartingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getSessionIsStartingSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void manualStartRequestedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getManualStartRequestedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void addAndStart(com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getAddAndStartMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void addAllWorkoutSegments(com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getAddAllWorkoutSegmentsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void clearRemainingWorkoutSegments(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getClearRemainingWorkoutSegmentsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void start(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStartMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void stop(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStopMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void next(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getNextMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void pause(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getPauseMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void resume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getResumeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentSessionWorkouts(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionItemList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentSessionWorkoutsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void sessionWorkoutsChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionItemList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getSessionWorkoutsChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentWorkoutSessionState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentWorkoutSessionStateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void workoutSessionStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getWorkoutSessionStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void assertPositionControl(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getAssertPositionControlMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void releasePositionControl(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getReleasePositionControlMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getPositionController(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.StringResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetPositionControllerMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setAtPosition(com.ifit.glassos.workout.SetAtPositionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetAtPositionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentProgramPosition(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgramPosition> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentProgramPositionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void programPositionChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgramPosition> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getProgramPositionChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getLatestUnfinishedWorkoutSession(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RecoveredSessionResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetLatestUnfinishedWorkoutSessionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void hasUnfinishedSessionAvailable(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getHasUnfinishedSessionAvailableMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void runUnfinishedSession(com.ifit.glassos.workout.data.recovery.RecoveredSession request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRunUnfinishedSessionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void markUnfinishedSessionAsComplete(com.ifit.glassos.workout.data.recovery.RecoveredSession request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getMarkUnfinishedSessionAsCompleteMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void clearUnfinishedSessions(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getClearUnfinishedSessionsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getRunningUnfinishedSession(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RecoveredSessionResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetRunningUnfinishedSessionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ProgrammedWorkoutSessionService.
   */
  public static final class ProgrammedWorkoutSessionServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ProgrammedWorkoutSessionServiceBlockingStub> {
    private ProgrammedWorkoutSessionServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ProgrammedWorkoutSessionServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ProgrammedWorkoutSessionServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.Empty preloadWorkout(com.ifit.glassos.workout.data.WorkoutSegmentDescriptor request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getPreloadWorkoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.WorkoutList> preloadedWorkoutsChangedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getPreloadedWorkoutsChangedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> sessionIsStartingSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getSessionIsStartingSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> manualStartRequestedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getManualStartRequestedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse addAndStart(com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getAddAndStartMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse addAllWorkoutSegments(com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getAddAllWorkoutSegmentsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty clearRemainingWorkoutSegments(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getClearRemainingWorkoutSegmentsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse start(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStartMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse stop(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStopMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse next(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getNextMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse pause(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getPauseMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse resume(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getResumeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.data.WorkoutSessionItemList getCurrentSessionWorkouts(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentSessionWorkoutsMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.WorkoutSessionItemList> sessionWorkoutsChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getSessionWorkoutsChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.data.WorkoutSessionState getCurrentWorkoutSessionState(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentWorkoutSessionStateMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.WorkoutSessionState> workoutSessionStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getWorkoutSessionStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse assertPositionControl(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getAssertPositionControlMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty releasePositionControl(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getReleasePositionControlMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.StringResponse getPositionController(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetPositionControllerMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse setAtPosition(com.ifit.glassos.workout.SetAtPositionRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetAtPositionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.ProgramPosition getCurrentProgramPosition(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentProgramPositionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.ProgramPosition> programPositionChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getProgramPositionChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.RecoveredSessionResponse getLatestUnfinishedWorkoutSession(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetLatestUnfinishedWorkoutSessionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse hasUnfinishedSessionAvailable(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getHasUnfinishedSessionAvailableMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty runUnfinishedSession(com.ifit.glassos.workout.data.recovery.RecoveredSession request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRunUnfinishedSessionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty markUnfinishedSessionAsComplete(com.ifit.glassos.workout.data.recovery.RecoveredSession request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getMarkUnfinishedSessionAsCompleteMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty clearUnfinishedSessions(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getClearUnfinishedSessionsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.RecoveredSessionResponse getRunningUnfinishedSession(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetRunningUnfinishedSessionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ProgrammedWorkoutSessionService.
   */
  public static final class ProgrammedWorkoutSessionServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ProgrammedWorkoutSessionServiceFutureStub> {
    private ProgrammedWorkoutSessionServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ProgrammedWorkoutSessionServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ProgrammedWorkoutSessionServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> preloadWorkout(
        com.ifit.glassos.workout.data.WorkoutSegmentDescriptor request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getPreloadWorkoutMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> addAndStart(
        com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getAddAndStartMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> addAllWorkoutSegments(
        com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getAddAllWorkoutSegmentsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> clearRemainingWorkoutSegments(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getClearRemainingWorkoutSegmentsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> start(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStartMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> stop(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStopMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> next(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getNextMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> pause(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getPauseMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> resume(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getResumeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.data.WorkoutSessionItemList> getCurrentSessionWorkouts(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentSessionWorkoutsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.data.WorkoutSessionState> getCurrentWorkoutSessionState(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentWorkoutSessionStateMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> assertPositionControl(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getAssertPositionControlMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> releasePositionControl(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getReleasePositionControlMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.StringResponse> getPositionController(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetPositionControllerMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse> setAtPosition(
        com.ifit.glassos.workout.SetAtPositionRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetAtPositionMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.ProgramPosition> getCurrentProgramPosition(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentProgramPositionMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.RecoveredSessionResponse> getLatestUnfinishedWorkoutSession(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetLatestUnfinishedWorkoutSessionMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> hasUnfinishedSessionAvailable(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getHasUnfinishedSessionAvailableMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> runUnfinishedSession(
        com.ifit.glassos.workout.data.recovery.RecoveredSession request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRunUnfinishedSessionMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> markUnfinishedSessionAsComplete(
        com.ifit.glassos.workout.data.recovery.RecoveredSession request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getMarkUnfinishedSessionAsCompleteMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> clearUnfinishedSessions(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getClearUnfinishedSessionsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.RecoveredSessionResponse> getRunningUnfinishedSession(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetRunningUnfinishedSessionMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_PRELOAD_WORKOUT = 0;
  private static final int METHODID_PRELOADED_WORKOUTS_CHANGED_SUBSCRIPTION = 1;
  private static final int METHODID_SESSION_IS_STARTING_SUBSCRIPTION = 2;
  private static final int METHODID_MANUAL_START_REQUESTED_SUBSCRIPTION = 3;
  private static final int METHODID_ADD_AND_START = 4;
  private static final int METHODID_ADD_ALL_WORKOUT_SEGMENTS = 5;
  private static final int METHODID_CLEAR_REMAINING_WORKOUT_SEGMENTS = 6;
  private static final int METHODID_START = 7;
  private static final int METHODID_STOP = 8;
  private static final int METHODID_NEXT = 9;
  private static final int METHODID_PAUSE = 10;
  private static final int METHODID_RESUME = 11;
  private static final int METHODID_GET_CURRENT_SESSION_WORKOUTS = 12;
  private static final int METHODID_SESSION_WORKOUTS_CHANGED = 13;
  private static final int METHODID_GET_CURRENT_WORKOUT_SESSION_STATE = 14;
  private static final int METHODID_WORKOUT_SESSION_STATE_CHANGED = 15;
  private static final int METHODID_ASSERT_POSITION_CONTROL = 16;
  private static final int METHODID_RELEASE_POSITION_CONTROL = 17;
  private static final int METHODID_GET_POSITION_CONTROLLER = 18;
  private static final int METHODID_SET_AT_POSITION = 19;
  private static final int METHODID_GET_CURRENT_PROGRAM_POSITION = 20;
  private static final int METHODID_PROGRAM_POSITION_CHANGED = 21;
  private static final int METHODID_GET_LATEST_UNFINISHED_WORKOUT_SESSION = 22;
  private static final int METHODID_HAS_UNFINISHED_SESSION_AVAILABLE = 23;
  private static final int METHODID_RUN_UNFINISHED_SESSION = 24;
  private static final int METHODID_MARK_UNFINISHED_SESSION_AS_COMPLETE = 25;
  private static final int METHODID_CLEAR_UNFINISHED_SESSIONS = 26;
  private static final int METHODID_GET_RUNNING_UNFINISHED_SESSION = 27;

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
        case METHODID_PRELOAD_WORKOUT:
          serviceImpl.preloadWorkout((com.ifit.glassos.workout.data.WorkoutSegmentDescriptor) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_PRELOADED_WORKOUTS_CHANGED_SUBSCRIPTION:
          serviceImpl.preloadedWorkoutsChangedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutList>) responseObserver);
          break;
        case METHODID_SESSION_IS_STARTING_SUBSCRIPTION:
          serviceImpl.sessionIsStartingSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_MANUAL_START_REQUESTED_SUBSCRIPTION:
          serviceImpl.manualStartRequestedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_ADD_AND_START:
          serviceImpl.addAndStart((com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_ADD_ALL_WORKOUT_SEGMENTS:
          serviceImpl.addAllWorkoutSegments((com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_CLEAR_REMAINING_WORKOUT_SEGMENTS:
          serviceImpl.clearRemainingWorkoutSegments((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_START:
          serviceImpl.start((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_STOP:
          serviceImpl.stop((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_NEXT:
          serviceImpl.next((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_PAUSE:
          serviceImpl.pause((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_RESUME:
          serviceImpl.resume((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_GET_CURRENT_SESSION_WORKOUTS:
          serviceImpl.getCurrentSessionWorkouts((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionItemList>) responseObserver);
          break;
        case METHODID_SESSION_WORKOUTS_CHANGED:
          serviceImpl.sessionWorkoutsChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionItemList>) responseObserver);
          break;
        case METHODID_GET_CURRENT_WORKOUT_SESSION_STATE:
          serviceImpl.getCurrentWorkoutSessionState((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionState>) responseObserver);
          break;
        case METHODID_WORKOUT_SESSION_STATE_CHANGED:
          serviceImpl.workoutSessionStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.WorkoutSessionState>) responseObserver);
          break;
        case METHODID_ASSERT_POSITION_CONTROL:
          serviceImpl.assertPositionControl((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_RELEASE_POSITION_CONTROL:
          serviceImpl.releasePositionControl((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_POSITION_CONTROLLER:
          serviceImpl.getPositionController((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.StringResponse>) responseObserver);
          break;
        case METHODID_SET_AT_POSITION:
          serviceImpl.setAtPosition((com.ifit.glassos.workout.SetAtPositionRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>) responseObserver);
          break;
        case METHODID_GET_CURRENT_PROGRAM_POSITION:
          serviceImpl.getCurrentProgramPosition((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgramPosition>) responseObserver);
          break;
        case METHODID_PROGRAM_POSITION_CHANGED:
          serviceImpl.programPositionChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.ProgramPosition>) responseObserver);
          break;
        case METHODID_GET_LATEST_UNFINISHED_WORKOUT_SESSION:
          serviceImpl.getLatestUnfinishedWorkoutSession((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RecoveredSessionResponse>) responseObserver);
          break;
        case METHODID_HAS_UNFINISHED_SESSION_AVAILABLE:
          serviceImpl.hasUnfinishedSessionAvailable((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_RUN_UNFINISHED_SESSION:
          serviceImpl.runUnfinishedSession((com.ifit.glassos.workout.data.recovery.RecoveredSession) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_MARK_UNFINISHED_SESSION_AS_COMPLETE:
          serviceImpl.markUnfinishedSessionAsComplete((com.ifit.glassos.workout.data.recovery.RecoveredSession) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_CLEAR_UNFINISHED_SESSIONS:
          serviceImpl.clearUnfinishedSessions((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_RUNNING_UNFINISHED_SESSION:
          serviceImpl.getRunningUnfinishedSession((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.RecoveredSessionResponse>) responseObserver);
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
          getPreloadWorkoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.data.WorkoutSegmentDescriptor,
              com.ifit.glassos.util.Empty>(
                service, METHODID_PRELOAD_WORKOUT)))
        .addMethod(
          getPreloadedWorkoutsChangedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.WorkoutList>(
                service, METHODID_PRELOADED_WORKOUTS_CHANGED_SUBSCRIPTION)))
        .addMethod(
          getSessionIsStartingSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_SESSION_IS_STARTING_SUBSCRIPTION)))
        .addMethod(
          getManualStartRequestedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_MANUAL_START_REQUESTED_SUBSCRIPTION)))
        .addMethod(
          getAddAndStartMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_ADD_AND_START)))
        .addMethod(
          getAddAllWorkoutSegmentsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.AddAllWorkoutSegmentsRequest,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_ADD_ALL_WORKOUT_SEGMENTS)))
        .addMethod(
          getClearRemainingWorkoutSegmentsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_CLEAR_REMAINING_WORKOUT_SEGMENTS)))
        .addMethod(
          getStartMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_START)))
        .addMethod(
          getStopMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_STOP)))
        .addMethod(
          getNextMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_NEXT)))
        .addMethod(
          getPauseMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_PAUSE)))
        .addMethod(
          getResumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_RESUME)))
        .addMethod(
          getGetCurrentSessionWorkoutsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.WorkoutSessionItemList>(
                service, METHODID_GET_CURRENT_SESSION_WORKOUTS)))
        .addMethod(
          getSessionWorkoutsChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.WorkoutSessionItemList>(
                service, METHODID_SESSION_WORKOUTS_CHANGED)))
        .addMethod(
          getGetCurrentWorkoutSessionStateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.WorkoutSessionState>(
                service, METHODID_GET_CURRENT_WORKOUT_SESSION_STATE)))
        .addMethod(
          getWorkoutSessionStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.WorkoutSessionState>(
                service, METHODID_WORKOUT_SESSION_STATE_CHANGED)))
        .addMethod(
          getAssertPositionControlMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_ASSERT_POSITION_CONTROL)))
        .addMethod(
          getReleasePositionControlMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_RELEASE_POSITION_CONTROL)))
        .addMethod(
          getGetPositionControllerMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.StringResponse>(
                service, METHODID_GET_POSITION_CONTROLLER)))
        .addMethod(
          getSetAtPositionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.SetAtPositionRequest,
              com.ifit.glassos.workout.ProgrammedWorkoutServiceResponse>(
                service, METHODID_SET_AT_POSITION)))
        .addMethod(
          getGetCurrentProgramPositionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgramPosition>(
                service, METHODID_GET_CURRENT_PROGRAM_POSITION)))
        .addMethod(
          getProgramPositionChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.ProgramPosition>(
                service, METHODID_PROGRAM_POSITION_CHANGED)))
        .addMethod(
          getGetLatestUnfinishedWorkoutSessionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.RecoveredSessionResponse>(
                service, METHODID_GET_LATEST_UNFINISHED_WORKOUT_SESSION)))
        .addMethod(
          getHasUnfinishedSessionAvailableMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_HAS_UNFINISHED_SESSION_AVAILABLE)))
        .addMethod(
          getRunUnfinishedSessionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.data.recovery.RecoveredSession,
              com.ifit.glassos.util.Empty>(
                service, METHODID_RUN_UNFINISHED_SESSION)))
        .addMethod(
          getMarkUnfinishedSessionAsCompleteMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.data.recovery.RecoveredSession,
              com.ifit.glassos.util.Empty>(
                service, METHODID_MARK_UNFINISHED_SESSION_AS_COMPLETE)))
        .addMethod(
          getClearUnfinishedSessionsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_CLEAR_UNFINISHED_SESSIONS)))
        .addMethod(
          getGetRunningUnfinishedSessionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.RecoveredSessionResponse>(
                service, METHODID_GET_RUNNING_UNFINISHED_SESSION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ProgrammedWorkoutSessionServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getPreloadWorkoutMethod())
              .addMethod(getPreloadedWorkoutsChangedSubscriptionMethod())
              .addMethod(getSessionIsStartingSubscriptionMethod())
              .addMethod(getManualStartRequestedSubscriptionMethod())
              .addMethod(getAddAndStartMethod())
              .addMethod(getAddAllWorkoutSegmentsMethod())
              .addMethod(getClearRemainingWorkoutSegmentsMethod())
              .addMethod(getStartMethod())
              .addMethod(getStopMethod())
              .addMethod(getNextMethod())
              .addMethod(getPauseMethod())
              .addMethod(getResumeMethod())
              .addMethod(getGetCurrentSessionWorkoutsMethod())
              .addMethod(getSessionWorkoutsChangedMethod())
              .addMethod(getGetCurrentWorkoutSessionStateMethod())
              .addMethod(getWorkoutSessionStateChangedMethod())
              .addMethod(getAssertPositionControlMethod())
              .addMethod(getReleasePositionControlMethod())
              .addMethod(getGetPositionControllerMethod())
              .addMethod(getSetAtPositionMethod())
              .addMethod(getGetCurrentProgramPositionMethod())
              .addMethod(getProgramPositionChangedMethod())
              .addMethod(getGetLatestUnfinishedWorkoutSessionMethod())
              .addMethod(getHasUnfinishedSessionAvailableMethod())
              .addMethod(getRunUnfinishedSessionMethod())
              .addMethod(getMarkUnfinishedSessionAsCompleteMethod())
              .addMethod(getClearUnfinishedSessionsMethod())
              .addMethod(getGetRunningUnfinishedSessionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

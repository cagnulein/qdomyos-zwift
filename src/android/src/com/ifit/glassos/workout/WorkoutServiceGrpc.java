package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/WorkoutService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class WorkoutServiceGrpc {

  private WorkoutServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.WorkoutService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StartWorkoutResponse> getStartNewWorkoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StartNewWorkout",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.StartWorkoutResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.StartWorkoutResponse> getStartNewWorkoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StartWorkoutResponse> getStartNewWorkoutMethod;
    if ((getStartNewWorkoutMethod = WorkoutServiceGrpc.getStartNewWorkoutMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getStartNewWorkoutMethod = WorkoutServiceGrpc.getStartNewWorkoutMethod) == null) {
          WorkoutServiceGrpc.getStartNewWorkoutMethod = getStartNewWorkoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.StartWorkoutResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StartNewWorkout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StartWorkoutResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStartNewWorkoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StartWorkoutResponse> getStartLoggedWorkoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StartLoggedWorkout",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.workout.StartWorkoutResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.workout.StartWorkoutResponse> getStartLoggedWorkoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StartWorkoutResponse> getStartLoggedWorkoutMethod;
    if ((getStartLoggedWorkoutMethod = WorkoutServiceGrpc.getStartLoggedWorkoutMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getStartLoggedWorkoutMethod = WorkoutServiceGrpc.getStartLoggedWorkoutMethod) == null) {
          WorkoutServiceGrpc.getStartLoggedWorkoutMethod = getStartLoggedWorkoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.workout.StartWorkoutResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StartLoggedWorkout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.StartWorkoutResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStartLoggedWorkoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getPauseMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Pause",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getPauseMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult> getPauseMethod;
    if ((getPauseMethod = WorkoutServiceGrpc.getPauseMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getPauseMethod = WorkoutServiceGrpc.getPauseMethod) == null) {
          WorkoutServiceGrpc.getPauseMethod = getPauseMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Pause"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPauseMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getResumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Resume",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getResumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult> getResumeMethod;
    if ((getResumeMethod = WorkoutServiceGrpc.getResumeMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getResumeMethod = WorkoutServiceGrpc.getResumeMethod) == null) {
          WorkoutServiceGrpc.getResumeMethod = getResumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Resume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getResumeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getStopMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Stop",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutResult> getStopMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult> getStopMethod;
    if ((getStopMethod = WorkoutServiceGrpc.getStopMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getStopMethod = WorkoutServiceGrpc.getStopMethod) == null) {
          WorkoutServiceGrpc.getStopMethod = getStopMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Stop"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStopMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutStateMessage> getGetWorkoutStateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetWorkoutState",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutStateMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutStateMessage> getGetWorkoutStateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutStateMessage> getGetWorkoutStateMethod;
    if ((getGetWorkoutStateMethod = WorkoutServiceGrpc.getGetWorkoutStateMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getGetWorkoutStateMethod = WorkoutServiceGrpc.getGetWorkoutStateMethod) == null) {
          WorkoutServiceGrpc.getGetWorkoutStateMethod = getGetWorkoutStateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutStateMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetWorkoutState"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutStateMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetWorkoutStateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutStateMessage> getWorkoutStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "WorkoutStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutStateMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutStateMessage> getWorkoutStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutStateMessage> getWorkoutStateChangedMethod;
    if ((getWorkoutStateChangedMethod = WorkoutServiceGrpc.getWorkoutStateChangedMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getWorkoutStateChangedMethod = WorkoutServiceGrpc.getWorkoutStateChangedMethod) == null) {
          WorkoutServiceGrpc.getWorkoutStateChangedMethod = getWorkoutStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutStateMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "WorkoutStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutStateMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getWorkoutStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.WorkoutID> getGetCurrentWorkoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentWorkout",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.WorkoutID.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.WorkoutID> getGetCurrentWorkoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.WorkoutID> getGetCurrentWorkoutMethod;
    if ((getGetCurrentWorkoutMethod = WorkoutServiceGrpc.getGetCurrentWorkoutMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getGetCurrentWorkoutMethod = WorkoutServiceGrpc.getGetCurrentWorkoutMethod) == null) {
          WorkoutServiceGrpc.getGetCurrentWorkoutMethod = getGetCurrentWorkoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.WorkoutID>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentWorkout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentWorkoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.WorkoutID> getCurrentWorkoutChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CurrentWorkoutChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.WorkoutID.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.WorkoutID> getCurrentWorkoutChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.WorkoutID> getCurrentWorkoutChangedMethod;
    if ((getCurrentWorkoutChangedMethod = WorkoutServiceGrpc.getCurrentWorkoutChangedMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getCurrentWorkoutChangedMethod = WorkoutServiceGrpc.getCurrentWorkoutChangedMethod) == null) {
          WorkoutServiceGrpc.getCurrentWorkoutChangedMethod = getCurrentWorkoutChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.WorkoutID>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CurrentWorkoutChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCurrentWorkoutChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutSourceMessage> getGetWorkoutSourceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetWorkoutSource",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.WorkoutSourceMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.WorkoutSourceMessage> getGetWorkoutSourceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutSourceMessage> getGetWorkoutSourceMethod;
    if ((getGetWorkoutSourceMethod = WorkoutServiceGrpc.getGetWorkoutSourceMethod) == null) {
      synchronized (WorkoutServiceGrpc.class) {
        if ((getGetWorkoutSourceMethod = WorkoutServiceGrpc.getGetWorkoutSourceMethod) == null) {
          WorkoutServiceGrpc.getGetWorkoutSourceMethod = getGetWorkoutSourceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.WorkoutSourceMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetWorkoutSource"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.WorkoutSourceMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetWorkoutSourceMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static WorkoutServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<WorkoutServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<WorkoutServiceStub>() {
        @java.lang.Override
        public WorkoutServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new WorkoutServiceStub(channel, callOptions);
        }
      };
    return WorkoutServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static WorkoutServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<WorkoutServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<WorkoutServiceBlockingStub>() {
        @java.lang.Override
        public WorkoutServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new WorkoutServiceBlockingStub(channel, callOptions);
        }
      };
    return WorkoutServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static WorkoutServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<WorkoutServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<WorkoutServiceFutureStub>() {
        @java.lang.Override
        public WorkoutServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new WorkoutServiceFutureStub(channel, callOptions);
        }
      };
    return WorkoutServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void startNewWorkout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StartWorkoutResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStartNewWorkoutMethod(), responseObserver);
    }

    /**
     */
    default void startLoggedWorkout(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StartWorkoutResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStartLoggedWorkoutMethod(), responseObserver);
    }

    /**
     */
    default void pause(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPauseMethod(), responseObserver);
    }

    /**
     */
    default void resume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getResumeMethod(), responseObserver);
    }

    /**
     */
    default void stop(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStopMethod(), responseObserver);
    }

    /**
     */
    default void getWorkoutState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutStateMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetWorkoutStateMethod(), responseObserver);
    }

    /**
     */
    default void workoutStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutStateMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getWorkoutStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentWorkout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.WorkoutID> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentWorkoutMethod(), responseObserver);
    }

    /**
     */
    default void currentWorkoutChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.WorkoutID> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCurrentWorkoutChangedMethod(), responseObserver);
    }

    /**
     */
    default void getWorkoutSource(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutSourceMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetWorkoutSourceMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service WorkoutService.
   */
  public static abstract class WorkoutServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return WorkoutServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service WorkoutService.
   */
  public static final class WorkoutServiceStub
      extends io.grpc.stub.AbstractAsyncStub<WorkoutServiceStub> {
    private WorkoutServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected WorkoutServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new WorkoutServiceStub(channel, callOptions);
    }

    /**
     */
    public void startNewWorkout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StartWorkoutResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStartNewWorkoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void startLoggedWorkout(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StartWorkoutResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStartLoggedWorkoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void pause(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getPauseMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void resume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getResumeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void stop(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStopMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getWorkoutState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutStateMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetWorkoutStateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void workoutStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutStateMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getWorkoutStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentWorkout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.WorkoutID> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentWorkoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void currentWorkoutChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.WorkoutID> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getCurrentWorkoutChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getWorkoutSource(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutSourceMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetWorkoutSourceMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service WorkoutService.
   */
  public static final class WorkoutServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<WorkoutServiceBlockingStub> {
    private WorkoutServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected WorkoutServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new WorkoutServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.workout.StartWorkoutResponse startNewWorkout(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStartNewWorkoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.StartWorkoutResponse startLoggedWorkout(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStartLoggedWorkoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutResult pause(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getPauseMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutResult resume(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getResumeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutResult stop(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStopMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutStateMessage getWorkoutState(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetWorkoutStateMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.WorkoutStateMessage> workoutStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getWorkoutStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.WorkoutID getCurrentWorkout(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentWorkoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.WorkoutID> currentWorkoutChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getCurrentWorkoutChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.WorkoutSourceMessage getWorkoutSource(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetWorkoutSourceMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service WorkoutService.
   */
  public static final class WorkoutServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<WorkoutServiceFutureStub> {
    private WorkoutServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected WorkoutServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new WorkoutServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StartWorkoutResponse> startNewWorkout(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStartNewWorkoutMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.StartWorkoutResponse> startLoggedWorkout(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStartLoggedWorkoutMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> pause(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getPauseMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> resume(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getResumeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutResult> stop(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStopMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutStateMessage> getWorkoutState(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetWorkoutStateMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.WorkoutID> getCurrentWorkout(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentWorkoutMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.WorkoutSourceMessage> getWorkoutSource(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetWorkoutSourceMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_START_NEW_WORKOUT = 0;
  private static final int METHODID_START_LOGGED_WORKOUT = 1;
  private static final int METHODID_PAUSE = 2;
  private static final int METHODID_RESUME = 3;
  private static final int METHODID_STOP = 4;
  private static final int METHODID_GET_WORKOUT_STATE = 5;
  private static final int METHODID_WORKOUT_STATE_CHANGED = 6;
  private static final int METHODID_GET_CURRENT_WORKOUT = 7;
  private static final int METHODID_CURRENT_WORKOUT_CHANGED = 8;
  private static final int METHODID_GET_WORKOUT_SOURCE = 9;

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
        case METHODID_START_NEW_WORKOUT:
          serviceImpl.startNewWorkout((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StartWorkoutResponse>) responseObserver);
          break;
        case METHODID_START_LOGGED_WORKOUT:
          serviceImpl.startLoggedWorkout((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.StartWorkoutResponse>) responseObserver);
          break;
        case METHODID_PAUSE:
          serviceImpl.pause((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_RESUME:
          serviceImpl.resume((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_STOP:
          serviceImpl.stop((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutResult>) responseObserver);
          break;
        case METHODID_GET_WORKOUT_STATE:
          serviceImpl.getWorkoutState((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutStateMessage>) responseObserver);
          break;
        case METHODID_WORKOUT_STATE_CHANGED:
          serviceImpl.workoutStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutStateMessage>) responseObserver);
          break;
        case METHODID_GET_CURRENT_WORKOUT:
          serviceImpl.getCurrentWorkout((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.WorkoutID>) responseObserver);
          break;
        case METHODID_CURRENT_WORKOUT_CHANGED:
          serviceImpl.currentWorkoutChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.WorkoutID>) responseObserver);
          break;
        case METHODID_GET_WORKOUT_SOURCE:
          serviceImpl.getWorkoutSource((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.WorkoutSourceMessage>) responseObserver);
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
          getStartNewWorkoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.StartWorkoutResponse>(
                service, METHODID_START_NEW_WORKOUT)))
        .addMethod(
          getStartLoggedWorkoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.workout.StartWorkoutResponse>(
                service, METHODID_START_LOGGED_WORKOUT)))
        .addMethod(
          getPauseMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_PAUSE)))
        .addMethod(
          getResumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_RESUME)))
        .addMethod(
          getStopMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutResult>(
                service, METHODID_STOP)))
        .addMethod(
          getGetWorkoutStateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutStateMessage>(
                service, METHODID_GET_WORKOUT_STATE)))
        .addMethod(
          getWorkoutStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutStateMessage>(
                service, METHODID_WORKOUT_STATE_CHANGED)))
        .addMethod(
          getGetCurrentWorkoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.WorkoutID>(
                service, METHODID_GET_CURRENT_WORKOUT)))
        .addMethod(
          getCurrentWorkoutChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.WorkoutID>(
                service, METHODID_CURRENT_WORKOUT_CHANGED)))
        .addMethod(
          getGetWorkoutSourceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.WorkoutSourceMessage>(
                service, METHODID_GET_WORKOUT_SOURCE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (WorkoutServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getStartNewWorkoutMethod())
              .addMethod(getStartLoggedWorkoutMethod())
              .addMethod(getPauseMethod())
              .addMethod(getResumeMethod())
              .addMethod(getStopMethod())
              .addMethod(getGetWorkoutStateMethod())
              .addMethod(getWorkoutStateChangedMethod())
              .addMethod(getGetCurrentWorkoutMethod())
              .addMethod(getCurrentWorkoutChangedMethod())
              .addMethod(getGetWorkoutSourceMethod())
              .build();
        }
      }
    }
    return result;
  }
}

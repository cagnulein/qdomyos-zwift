package com.ifit.glassos.activitylog;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: activitylog/ActivityLogService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ActivityLogServiceGrpc {

  private ActivityLogServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ActivityLogService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getHasUnprocessedUploadEventsSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "HasUnprocessedUploadEventsSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getHasUnprocessedUploadEventsSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getHasUnprocessedUploadEventsSubscriptionMethod;
    if ((getHasUnprocessedUploadEventsSubscriptionMethod = ActivityLogServiceGrpc.getHasUnprocessedUploadEventsSubscriptionMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getHasUnprocessedUploadEventsSubscriptionMethod = ActivityLogServiceGrpc.getHasUnprocessedUploadEventsSubscriptionMethod) == null) {
          ActivityLogServiceGrpc.getHasUnprocessedUploadEventsSubscriptionMethod = getHasUnprocessedUploadEventsSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "HasUnprocessedUploadEventsSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getHasUnprocessedUploadEventsSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.activitylog.ActivityLogEvent> getPopMostRecentUploadEventMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "PopMostRecentUploadEvent",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogEvent.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.activitylog.ActivityLogEvent> getPopMostRecentUploadEventMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.activitylog.ActivityLogEvent> getPopMostRecentUploadEventMethod;
    if ((getPopMostRecentUploadEventMethod = ActivityLogServiceGrpc.getPopMostRecentUploadEventMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getPopMostRecentUploadEventMethod = ActivityLogServiceGrpc.getPopMostRecentUploadEventMethod) == null) {
          ActivityLogServiceGrpc.getPopMostRecentUploadEventMethod = getPopMostRecentUploadEventMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.activitylog.ActivityLogEvent>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "PopMostRecentUploadEvent"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogEvent.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPopMostRecentUploadEventMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.activitylog.ActivityLogEvent> getActivityLogEventSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ActivityLogEventSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogEvent.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.activitylog.ActivityLogEvent> getActivityLogEventSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.activitylog.ActivityLogEvent> getActivityLogEventSubscriptionMethod;
    if ((getActivityLogEventSubscriptionMethod = ActivityLogServiceGrpc.getActivityLogEventSubscriptionMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getActivityLogEventSubscriptionMethod = ActivityLogServiceGrpc.getActivityLogEventSubscriptionMethod) == null) {
          ActivityLogServiceGrpc.getActivityLogEventSubscriptionMethod = getActivityLogEventSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.activitylog.ActivityLogEvent>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ActivityLogEventSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogEvent.getDefaultInstance()))
              .build();
        }
      }
    }
    return getActivityLogEventSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.activitylog.ActivityLogUploading> getActivityLogUploadingSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ActivityLogUploadingSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogUploading.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.activitylog.ActivityLogUploading> getActivityLogUploadingSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.activitylog.ActivityLogUploading> getActivityLogUploadingSubscriptionMethod;
    if ((getActivityLogUploadingSubscriptionMethod = ActivityLogServiceGrpc.getActivityLogUploadingSubscriptionMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getActivityLogUploadingSubscriptionMethod = ActivityLogServiceGrpc.getActivityLogUploadingSubscriptionMethod) == null) {
          ActivityLogServiceGrpc.getActivityLogUploadingSubscriptionMethod = getActivityLogUploadingSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.activitylog.ActivityLogUploading>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ActivityLogUploadingSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogUploading.getDefaultInstance()))
              .build();
        }
      }
    }
    return getActivityLogUploadingSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogMetadataByWorkoutIdMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetActivityLogMetadataByWorkoutId",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogMetadataByWorkoutIdMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogMetadataByWorkoutIdMethod;
    if ((getGetActivityLogMetadataByWorkoutIdMethod = ActivityLogServiceGrpc.getGetActivityLogMetadataByWorkoutIdMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getGetActivityLogMetadataByWorkoutIdMethod = ActivityLogServiceGrpc.getGetActivityLogMetadataByWorkoutIdMethod) == null) {
          ActivityLogServiceGrpc.getGetActivityLogMetadataByWorkoutIdMethod = getGetActivityLogMetadataByWorkoutIdMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetActivityLogMetadataByWorkoutId"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetActivityLogMetadataByWorkoutIdMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogMetadata,
      com.ifit.glassos.activitylog.ActivityLogResult> getChangeActivityLogMetadataMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ChangeActivityLogMetadata",
      requestType = com.ifit.glassos.activitylog.ActivityLogMetadata.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogMetadata,
      com.ifit.glassos.activitylog.ActivityLogResult> getChangeActivityLogMetadataMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogMetadata, com.ifit.glassos.activitylog.ActivityLogResult> getChangeActivityLogMetadataMethod;
    if ((getChangeActivityLogMetadataMethod = ActivityLogServiceGrpc.getChangeActivityLogMetadataMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getChangeActivityLogMetadataMethod = ActivityLogServiceGrpc.getChangeActivityLogMetadataMethod) == null) {
          ActivityLogServiceGrpc.getChangeActivityLogMetadataMethod = getChangeActivityLogMetadataMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.activitylog.ActivityLogMetadata, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ChangeActivityLogMetadata"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogMetadata.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getChangeActivityLogMetadataMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogByWorkoutIdMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetActivityLogByWorkoutId",
      requestType = com.ifit.glassos.util.WorkoutID.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogByWorkoutIdMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogByWorkoutIdMethod;
    if ((getGetActivityLogByWorkoutIdMethod = ActivityLogServiceGrpc.getGetActivityLogByWorkoutIdMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getGetActivityLogByWorkoutIdMethod = ActivityLogServiceGrpc.getGetActivityLogByWorkoutIdMethod) == null) {
          ActivityLogServiceGrpc.getGetActivityLogByWorkoutIdMethod = getGetActivityLogByWorkoutIdMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.WorkoutID, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetActivityLogByWorkoutId"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.WorkoutID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetActivityLogByWorkoutIdMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ContentID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetLatestActivityLogByContentIdMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetLatestActivityLogByContentId",
      requestType = com.ifit.glassos.activitylog.ContentID.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ContentID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetLatestActivityLogByContentIdMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ContentID, com.ifit.glassos.activitylog.ActivityLogResult> getGetLatestActivityLogByContentIdMethod;
    if ((getGetLatestActivityLogByContentIdMethod = ActivityLogServiceGrpc.getGetLatestActivityLogByContentIdMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getGetLatestActivityLogByContentIdMethod = ActivityLogServiceGrpc.getGetLatestActivityLogByContentIdMethod) == null) {
          ActivityLogServiceGrpc.getGetLatestActivityLogByContentIdMethod = getGetLatestActivityLogByContentIdMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.activitylog.ContentID, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetLatestActivityLogByContentId"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ContentID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetLatestActivityLogByContentIdMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogByActivityLogIdMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetActivityLogByActivityLogId",
      requestType = com.ifit.glassos.activitylog.ActivityLogID.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogID,
      com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogByActivityLogIdMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogID, com.ifit.glassos.activitylog.ActivityLogResult> getGetActivityLogByActivityLogIdMethod;
    if ((getGetActivityLogByActivityLogIdMethod = ActivityLogServiceGrpc.getGetActivityLogByActivityLogIdMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getGetActivityLogByActivityLogIdMethod = ActivityLogServiceGrpc.getGetActivityLogByActivityLogIdMethod) == null) {
          ActivityLogServiceGrpc.getGetActivityLogByActivityLogIdMethod = getGetActivityLogByActivityLogIdMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.activitylog.ActivityLogID, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetActivityLogByActivityLogId"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetActivityLogByActivityLogIdMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogID,
      com.ifit.glassos.activitylog.ActivityLogResult> getDeleteActivityLogByActivityLogIdMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "DeleteActivityLogByActivityLogId",
      requestType = com.ifit.glassos.activitylog.ActivityLogID.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogID,
      com.ifit.glassos.activitylog.ActivityLogResult> getDeleteActivityLogByActivityLogIdMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ActivityLogID, com.ifit.glassos.activitylog.ActivityLogResult> getDeleteActivityLogByActivityLogIdMethod;
    if ((getDeleteActivityLogByActivityLogIdMethod = ActivityLogServiceGrpc.getDeleteActivityLogByActivityLogIdMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getDeleteActivityLogByActivityLogIdMethod = ActivityLogServiceGrpc.getDeleteActivityLogByActivityLogIdMethod) == null) {
          ActivityLogServiceGrpc.getDeleteActivityLogByActivityLogIdMethod = getDeleteActivityLogByActivityLogIdMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.activitylog.ActivityLogID, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "DeleteActivityLogByActivityLogId"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogID.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDeleteActivityLogByActivityLogIdMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ExternalUploadRequest,
      com.ifit.glassos.activitylog.ActivityLogResult> getUploadActivityLogFromExternalSourceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "UploadActivityLogFromExternalSource",
      requestType = com.ifit.glassos.activitylog.ExternalUploadRequest.class,
      responseType = com.ifit.glassos.activitylog.ActivityLogResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ExternalUploadRequest,
      com.ifit.glassos.activitylog.ActivityLogResult> getUploadActivityLogFromExternalSourceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.activitylog.ExternalUploadRequest, com.ifit.glassos.activitylog.ActivityLogResult> getUploadActivityLogFromExternalSourceMethod;
    if ((getUploadActivityLogFromExternalSourceMethod = ActivityLogServiceGrpc.getUploadActivityLogFromExternalSourceMethod) == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        if ((getUploadActivityLogFromExternalSourceMethod = ActivityLogServiceGrpc.getUploadActivityLogFromExternalSourceMethod) == null) {
          ActivityLogServiceGrpc.getUploadActivityLogFromExternalSourceMethod = getUploadActivityLogFromExternalSourceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.activitylog.ExternalUploadRequest, com.ifit.glassos.activitylog.ActivityLogResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "UploadActivityLogFromExternalSource"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ExternalUploadRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.activitylog.ActivityLogResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getUploadActivityLogFromExternalSourceMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ActivityLogServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ActivityLogServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ActivityLogServiceStub>() {
        @java.lang.Override
        public ActivityLogServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ActivityLogServiceStub(channel, callOptions);
        }
      };
    return ActivityLogServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ActivityLogServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ActivityLogServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ActivityLogServiceBlockingStub>() {
        @java.lang.Override
        public ActivityLogServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ActivityLogServiceBlockingStub(channel, callOptions);
        }
      };
    return ActivityLogServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ActivityLogServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ActivityLogServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ActivityLogServiceFutureStub>() {
        @java.lang.Override
        public ActivityLogServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ActivityLogServiceFutureStub(channel, callOptions);
        }
      };
    return ActivityLogServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void hasUnprocessedUploadEventsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getHasUnprocessedUploadEventsSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void popMostRecentUploadEvent(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogEvent> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPopMostRecentUploadEventMethod(), responseObserver);
    }

    /**
     */
    default void activityLogEventSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogEvent> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getActivityLogEventSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void activityLogUploadingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogUploading> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getActivityLogUploadingSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void getActivityLogMetadataByWorkoutId(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetActivityLogMetadataByWorkoutIdMethod(), responseObserver);
    }

    /**
     */
    default void changeActivityLogMetadata(com.ifit.glassos.activitylog.ActivityLogMetadata request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getChangeActivityLogMetadataMethod(), responseObserver);
    }

    /**
     */
    default void getActivityLogByWorkoutId(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetActivityLogByWorkoutIdMethod(), responseObserver);
    }

    /**
     */
    default void getLatestActivityLogByContentId(com.ifit.glassos.activitylog.ContentID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetLatestActivityLogByContentIdMethod(), responseObserver);
    }

    /**
     */
    default void getActivityLogByActivityLogId(com.ifit.glassos.activitylog.ActivityLogID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetActivityLogByActivityLogIdMethod(), responseObserver);
    }

    /**
     */
    default void deleteActivityLogByActivityLogId(com.ifit.glassos.activitylog.ActivityLogID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDeleteActivityLogByActivityLogIdMethod(), responseObserver);
    }

    /**
     */
    default void uploadActivityLogFromExternalSource(com.ifit.glassos.activitylog.ExternalUploadRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getUploadActivityLogFromExternalSourceMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ActivityLogService.
   */
  public static abstract class ActivityLogServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ActivityLogServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ActivityLogService.
   */
  public static final class ActivityLogServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ActivityLogServiceStub> {
    private ActivityLogServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ActivityLogServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ActivityLogServiceStub(channel, callOptions);
    }

    /**
     */
    public void hasUnprocessedUploadEventsSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getHasUnprocessedUploadEventsSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void popMostRecentUploadEvent(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogEvent> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getPopMostRecentUploadEventMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void activityLogEventSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogEvent> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getActivityLogEventSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void activityLogUploadingSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogUploading> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getActivityLogUploadingSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getActivityLogMetadataByWorkoutId(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetActivityLogMetadataByWorkoutIdMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void changeActivityLogMetadata(com.ifit.glassos.activitylog.ActivityLogMetadata request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getChangeActivityLogMetadataMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getActivityLogByWorkoutId(com.ifit.glassos.util.WorkoutID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetActivityLogByWorkoutIdMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getLatestActivityLogByContentId(com.ifit.glassos.activitylog.ContentID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetLatestActivityLogByContentIdMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getActivityLogByActivityLogId(com.ifit.glassos.activitylog.ActivityLogID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetActivityLogByActivityLogIdMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void deleteActivityLogByActivityLogId(com.ifit.glassos.activitylog.ActivityLogID request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getDeleteActivityLogByActivityLogIdMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void uploadActivityLogFromExternalSource(com.ifit.glassos.activitylog.ExternalUploadRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getUploadActivityLogFromExternalSourceMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ActivityLogService.
   */
  public static final class ActivityLogServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ActivityLogServiceBlockingStub> {
    private ActivityLogServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ActivityLogServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ActivityLogServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> hasUnprocessedUploadEventsSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getHasUnprocessedUploadEventsSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogEvent popMostRecentUploadEvent(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getPopMostRecentUploadEventMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.activitylog.ActivityLogEvent> activityLogEventSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getActivityLogEventSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.activitylog.ActivityLogUploading> activityLogUploadingSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getActivityLogUploadingSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult getActivityLogMetadataByWorkoutId(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetActivityLogMetadataByWorkoutIdMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult changeActivityLogMetadata(com.ifit.glassos.activitylog.ActivityLogMetadata request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getChangeActivityLogMetadataMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult getActivityLogByWorkoutId(com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetActivityLogByWorkoutIdMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult getLatestActivityLogByContentId(com.ifit.glassos.activitylog.ContentID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetLatestActivityLogByContentIdMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult getActivityLogByActivityLogId(com.ifit.glassos.activitylog.ActivityLogID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetActivityLogByActivityLogIdMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult deleteActivityLogByActivityLogId(com.ifit.glassos.activitylog.ActivityLogID request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getDeleteActivityLogByActivityLogIdMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.activitylog.ActivityLogResult uploadActivityLogFromExternalSource(com.ifit.glassos.activitylog.ExternalUploadRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getUploadActivityLogFromExternalSourceMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ActivityLogService.
   */
  public static final class ActivityLogServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ActivityLogServiceFutureStub> {
    private ActivityLogServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ActivityLogServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ActivityLogServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogEvent> popMostRecentUploadEvent(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getPopMostRecentUploadEventMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> getActivityLogMetadataByWorkoutId(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetActivityLogMetadataByWorkoutIdMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> changeActivityLogMetadata(
        com.ifit.glassos.activitylog.ActivityLogMetadata request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getChangeActivityLogMetadataMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> getActivityLogByWorkoutId(
        com.ifit.glassos.util.WorkoutID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetActivityLogByWorkoutIdMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> getLatestActivityLogByContentId(
        com.ifit.glassos.activitylog.ContentID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetLatestActivityLogByContentIdMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> getActivityLogByActivityLogId(
        com.ifit.glassos.activitylog.ActivityLogID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetActivityLogByActivityLogIdMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> deleteActivityLogByActivityLogId(
        com.ifit.glassos.activitylog.ActivityLogID request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getDeleteActivityLogByActivityLogIdMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.activitylog.ActivityLogResult> uploadActivityLogFromExternalSource(
        com.ifit.glassos.activitylog.ExternalUploadRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getUploadActivityLogFromExternalSourceMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_HAS_UNPROCESSED_UPLOAD_EVENTS_SUBSCRIPTION = 0;
  private static final int METHODID_POP_MOST_RECENT_UPLOAD_EVENT = 1;
  private static final int METHODID_ACTIVITY_LOG_EVENT_SUBSCRIPTION = 2;
  private static final int METHODID_ACTIVITY_LOG_UPLOADING_SUBSCRIPTION = 3;
  private static final int METHODID_GET_ACTIVITY_LOG_METADATA_BY_WORKOUT_ID = 4;
  private static final int METHODID_CHANGE_ACTIVITY_LOG_METADATA = 5;
  private static final int METHODID_GET_ACTIVITY_LOG_BY_WORKOUT_ID = 6;
  private static final int METHODID_GET_LATEST_ACTIVITY_LOG_BY_CONTENT_ID = 7;
  private static final int METHODID_GET_ACTIVITY_LOG_BY_ACTIVITY_LOG_ID = 8;
  private static final int METHODID_DELETE_ACTIVITY_LOG_BY_ACTIVITY_LOG_ID = 9;
  private static final int METHODID_UPLOAD_ACTIVITY_LOG_FROM_EXTERNAL_SOURCE = 10;

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
        case METHODID_HAS_UNPROCESSED_UPLOAD_EVENTS_SUBSCRIPTION:
          serviceImpl.hasUnprocessedUploadEventsSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_POP_MOST_RECENT_UPLOAD_EVENT:
          serviceImpl.popMostRecentUploadEvent((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogEvent>) responseObserver);
          break;
        case METHODID_ACTIVITY_LOG_EVENT_SUBSCRIPTION:
          serviceImpl.activityLogEventSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogEvent>) responseObserver);
          break;
        case METHODID_ACTIVITY_LOG_UPLOADING_SUBSCRIPTION:
          serviceImpl.activityLogUploadingSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogUploading>) responseObserver);
          break;
        case METHODID_GET_ACTIVITY_LOG_METADATA_BY_WORKOUT_ID:
          serviceImpl.getActivityLogMetadataByWorkoutId((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
          break;
        case METHODID_CHANGE_ACTIVITY_LOG_METADATA:
          serviceImpl.changeActivityLogMetadata((com.ifit.glassos.activitylog.ActivityLogMetadata) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
          break;
        case METHODID_GET_ACTIVITY_LOG_BY_WORKOUT_ID:
          serviceImpl.getActivityLogByWorkoutId((com.ifit.glassos.util.WorkoutID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
          break;
        case METHODID_GET_LATEST_ACTIVITY_LOG_BY_CONTENT_ID:
          serviceImpl.getLatestActivityLogByContentId((com.ifit.glassos.activitylog.ContentID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
          break;
        case METHODID_GET_ACTIVITY_LOG_BY_ACTIVITY_LOG_ID:
          serviceImpl.getActivityLogByActivityLogId((com.ifit.glassos.activitylog.ActivityLogID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
          break;
        case METHODID_DELETE_ACTIVITY_LOG_BY_ACTIVITY_LOG_ID:
          serviceImpl.deleteActivityLogByActivityLogId((com.ifit.glassos.activitylog.ActivityLogID) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
          break;
        case METHODID_UPLOAD_ACTIVITY_LOG_FROM_EXTERNAL_SOURCE:
          serviceImpl.uploadActivityLogFromExternalSource((com.ifit.glassos.activitylog.ExternalUploadRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.activitylog.ActivityLogResult>) responseObserver);
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
          getHasUnprocessedUploadEventsSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_HAS_UNPROCESSED_UPLOAD_EVENTS_SUBSCRIPTION)))
        .addMethod(
          getPopMostRecentUploadEventMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.activitylog.ActivityLogEvent>(
                service, METHODID_POP_MOST_RECENT_UPLOAD_EVENT)))
        .addMethod(
          getActivityLogEventSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.activitylog.ActivityLogEvent>(
                service, METHODID_ACTIVITY_LOG_EVENT_SUBSCRIPTION)))
        .addMethod(
          getActivityLogUploadingSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.activitylog.ActivityLogUploading>(
                service, METHODID_ACTIVITY_LOG_UPLOADING_SUBSCRIPTION)))
        .addMethod(
          getGetActivityLogMetadataByWorkoutIdMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_GET_ACTIVITY_LOG_METADATA_BY_WORKOUT_ID)))
        .addMethod(
          getChangeActivityLogMetadataMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.activitylog.ActivityLogMetadata,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_CHANGE_ACTIVITY_LOG_METADATA)))
        .addMethod(
          getGetActivityLogByWorkoutIdMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.WorkoutID,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_GET_ACTIVITY_LOG_BY_WORKOUT_ID)))
        .addMethod(
          getGetLatestActivityLogByContentIdMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.activitylog.ContentID,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_GET_LATEST_ACTIVITY_LOG_BY_CONTENT_ID)))
        .addMethod(
          getGetActivityLogByActivityLogIdMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.activitylog.ActivityLogID,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_GET_ACTIVITY_LOG_BY_ACTIVITY_LOG_ID)))
        .addMethod(
          getDeleteActivityLogByActivityLogIdMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.activitylog.ActivityLogID,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_DELETE_ACTIVITY_LOG_BY_ACTIVITY_LOG_ID)))
        .addMethod(
          getUploadActivityLogFromExternalSourceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.activitylog.ExternalUploadRequest,
              com.ifit.glassos.activitylog.ActivityLogResult>(
                service, METHODID_UPLOAD_ACTIVITY_LOG_FROM_EXTERNAL_SOURCE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ActivityLogServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getHasUnprocessedUploadEventsSubscriptionMethod())
              .addMethod(getPopMostRecentUploadEventMethod())
              .addMethod(getActivityLogEventSubscriptionMethod())
              .addMethod(getActivityLogUploadingSubscriptionMethod())
              .addMethod(getGetActivityLogMetadataByWorkoutIdMethod())
              .addMethod(getChangeActivityLogMetadataMethod())
              .addMethod(getGetActivityLogByWorkoutIdMethod())
              .addMethod(getGetLatestActivityLogByContentIdMethod())
              .addMethod(getGetActivityLogByActivityLogIdMethod())
              .addMethod(getDeleteActivityLogByActivityLogIdMethod())
              .addMethod(getUploadActivityLogFromExternalSourceMethod())
              .build();
        }
      }
    }
    return result;
  }
}

package com.ifit.glassos.console.useractivity;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/useractivity/UserActivityService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class UserActivityServiceGrpc {

  private UserActivityServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.UserActivityService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.useractivity.DurationResult> getDurationSinceLastScreenTapMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "DurationSinceLastScreenTap",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.useractivity.DurationResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.useractivity.DurationResult> getDurationSinceLastScreenTapMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.useractivity.DurationResult> getDurationSinceLastScreenTapMethod;
    if ((getDurationSinceLastScreenTapMethod = UserActivityServiceGrpc.getDurationSinceLastScreenTapMethod) == null) {
      synchronized (UserActivityServiceGrpc.class) {
        if ((getDurationSinceLastScreenTapMethod = UserActivityServiceGrpc.getDurationSinceLastScreenTapMethod) == null) {
          UserActivityServiceGrpc.getDurationSinceLastScreenTapMethod = getDurationSinceLastScreenTapMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.useractivity.DurationResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "DurationSinceLastScreenTap"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.DurationResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDurationSinceLastScreenTapMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.useractivity.UserActivityOverrideMessage> getStartUserActivityOverrideMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StartUserActivityOverride",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.useractivity.UserActivityOverrideMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.useractivity.UserActivityOverrideMessage> getStartUserActivityOverrideMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.useractivity.UserActivityOverrideMessage> getStartUserActivityOverrideMethod;
    if ((getStartUserActivityOverrideMethod = UserActivityServiceGrpc.getStartUserActivityOverrideMethod) == null) {
      synchronized (UserActivityServiceGrpc.class) {
        if ((getStartUserActivityOverrideMethod = UserActivityServiceGrpc.getStartUserActivityOverrideMethod) == null) {
          UserActivityServiceGrpc.getStartUserActivityOverrideMethod = getStartUserActivityOverrideMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.useractivity.UserActivityOverrideMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StartUserActivityOverride"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityOverrideMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStartUserActivityOverrideMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getCompleteUserActivityOverrideMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CompleteUserActivityOverride",
      requestType = com.ifit.glassos.console.useractivity.UserActivityOverrideMessage.class,
      responseType = com.ifit.glassos.console.useractivity.UserActivityServiceResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getCompleteUserActivityOverrideMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage, com.ifit.glassos.console.useractivity.UserActivityServiceResult> getCompleteUserActivityOverrideMethod;
    if ((getCompleteUserActivityOverrideMethod = UserActivityServiceGrpc.getCompleteUserActivityOverrideMethod) == null) {
      synchronized (UserActivityServiceGrpc.class) {
        if ((getCompleteUserActivityOverrideMethod = UserActivityServiceGrpc.getCompleteUserActivityOverrideMethod) == null) {
          UserActivityServiceGrpc.getCompleteUserActivityOverrideMethod = getCompleteUserActivityOverrideMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage, com.ifit.glassos.console.useractivity.UserActivityServiceResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CompleteUserActivityOverride"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityOverrideMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityServiceResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCompleteUserActivityOverrideMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.useractivity.SetDurationRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetDurationSinceLastScreenTapMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetDurationSinceLastScreenTap",
      requestType = com.ifit.glassos.console.useractivity.SetDurationRequest.class,
      responseType = com.ifit.glassos.console.useractivity.UserActivityServiceResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.useractivity.SetDurationRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetDurationSinceLastScreenTapMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.useractivity.SetDurationRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetDurationSinceLastScreenTapMethod;
    if ((getSetDurationSinceLastScreenTapMethod = UserActivityServiceGrpc.getSetDurationSinceLastScreenTapMethod) == null) {
      synchronized (UserActivityServiceGrpc.class) {
        if ((getSetDurationSinceLastScreenTapMethod = UserActivityServiceGrpc.getSetDurationSinceLastScreenTapMethod) == null) {
          UserActivityServiceGrpc.getSetDurationSinceLastScreenTapMethod = getSetDurationSinceLastScreenTapMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.useractivity.SetDurationRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetDurationSinceLastScreenTap"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.SetDurationRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityServiceResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetDurationSinceLastScreenTapMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static UserActivityServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<UserActivityServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<UserActivityServiceStub>() {
        @java.lang.Override
        public UserActivityServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new UserActivityServiceStub(channel, callOptions);
        }
      };
    return UserActivityServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static UserActivityServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<UserActivityServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<UserActivityServiceBlockingStub>() {
        @java.lang.Override
        public UserActivityServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new UserActivityServiceBlockingStub(channel, callOptions);
        }
      };
    return UserActivityServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static UserActivityServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<UserActivityServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<UserActivityServiceFutureStub>() {
        @java.lang.Override
        public UserActivityServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new UserActivityServiceFutureStub(channel, callOptions);
        }
      };
    return UserActivityServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void durationSinceLastScreenTap(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.DurationResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDurationSinceLastScreenTapMethod(), responseObserver);
    }

    /**
     */
    default void startUserActivityOverride(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStartUserActivityOverrideMethod(), responseObserver);
    }

    /**
     */
    default void completeUserActivityOverride(com.ifit.glassos.console.useractivity.UserActivityOverrideMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCompleteUserActivityOverrideMethod(), responseObserver);
    }

    /**
     */
    default void setDurationSinceLastScreenTap(com.ifit.glassos.console.useractivity.SetDurationRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetDurationSinceLastScreenTapMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service UserActivityService.
   */
  public static abstract class UserActivityServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return UserActivityServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service UserActivityService.
   */
  public static final class UserActivityServiceStub
      extends io.grpc.stub.AbstractAsyncStub<UserActivityServiceStub> {
    private UserActivityServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected UserActivityServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new UserActivityServiceStub(channel, callOptions);
    }

    /**
     */
    public void durationSinceLastScreenTap(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.DurationResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getDurationSinceLastScreenTapMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void startUserActivityOverride(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStartUserActivityOverrideMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void completeUserActivityOverride(com.ifit.glassos.console.useractivity.UserActivityOverrideMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCompleteUserActivityOverrideMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setDurationSinceLastScreenTap(com.ifit.glassos.console.useractivity.SetDurationRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetDurationSinceLastScreenTapMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service UserActivityService.
   */
  public static final class UserActivityServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<UserActivityServiceBlockingStub> {
    private UserActivityServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected UserActivityServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new UserActivityServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.useractivity.DurationResult> durationSinceLastScreenTap(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getDurationSinceLastScreenTapMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.useractivity.UserActivityOverrideMessage startUserActivityOverride(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStartUserActivityOverrideMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.useractivity.UserActivityServiceResult completeUserActivityOverride(com.ifit.glassos.console.useractivity.UserActivityOverrideMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCompleteUserActivityOverrideMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.useractivity.UserActivityServiceResult setDurationSinceLastScreenTap(com.ifit.glassos.console.useractivity.SetDurationRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetDurationSinceLastScreenTapMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service UserActivityService.
   */
  public static final class UserActivityServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<UserActivityServiceFutureStub> {
    private UserActivityServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected UserActivityServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new UserActivityServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage> startUserActivityOverride(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStartUserActivityOverrideMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.useractivity.UserActivityServiceResult> completeUserActivityOverride(
        com.ifit.glassos.console.useractivity.UserActivityOverrideMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCompleteUserActivityOverrideMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.useractivity.UserActivityServiceResult> setDurationSinceLastScreenTap(
        com.ifit.glassos.console.useractivity.SetDurationRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetDurationSinceLastScreenTapMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_DURATION_SINCE_LAST_SCREEN_TAP = 0;
  private static final int METHODID_START_USER_ACTIVITY_OVERRIDE = 1;
  private static final int METHODID_COMPLETE_USER_ACTIVITY_OVERRIDE = 2;
  private static final int METHODID_SET_DURATION_SINCE_LAST_SCREEN_TAP = 3;

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
        case METHODID_DURATION_SINCE_LAST_SCREEN_TAP:
          serviceImpl.durationSinceLastScreenTap((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.DurationResult>) responseObserver);
          break;
        case METHODID_START_USER_ACTIVITY_OVERRIDE:
          serviceImpl.startUserActivityOverride((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityOverrideMessage>) responseObserver);
          break;
        case METHODID_COMPLETE_USER_ACTIVITY_OVERRIDE:
          serviceImpl.completeUserActivityOverride((com.ifit.glassos.console.useractivity.UserActivityOverrideMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult>) responseObserver);
          break;
        case METHODID_SET_DURATION_SINCE_LAST_SCREEN_TAP:
          serviceImpl.setDurationSinceLastScreenTap((com.ifit.glassos.console.useractivity.SetDurationRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult>) responseObserver);
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
          getDurationSinceLastScreenTapMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.useractivity.DurationResult>(
                service, METHODID_DURATION_SINCE_LAST_SCREEN_TAP)))
        .addMethod(
          getStartUserActivityOverrideMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.useractivity.UserActivityOverrideMessage>(
                service, METHODID_START_USER_ACTIVITY_OVERRIDE)))
        .addMethod(
          getCompleteUserActivityOverrideMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.useractivity.UserActivityOverrideMessage,
              com.ifit.glassos.console.useractivity.UserActivityServiceResult>(
                service, METHODID_COMPLETE_USER_ACTIVITY_OVERRIDE)))
        .addMethod(
          getSetDurationSinceLastScreenTapMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.useractivity.SetDurationRequest,
              com.ifit.glassos.console.useractivity.UserActivityServiceResult>(
                service, METHODID_SET_DURATION_SINCE_LAST_SCREEN_TAP)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (UserActivityServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getDurationSinceLastScreenTapMethod())
              .addMethod(getStartUserActivityOverrideMethod())
              .addMethod(getCompleteUserActivityOverrideMethod())
              .addMethod(getSetDurationSinceLastScreenTapMethod())
              .build();
        }
      }
    }
    return result;
  }
}

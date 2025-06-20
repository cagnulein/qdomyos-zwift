package com.ifit.glassos.console.idlelockout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/idlelockout/IdleModeLockoutService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class IdleModeLockoutServiceGrpc {

  private IdleModeLockoutServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.IdleModeLockoutService";

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
    if ((getCanReadMethod = IdleModeLockoutServiceGrpc.getCanReadMethod) == null) {
      synchronized (IdleModeLockoutServiceGrpc.class) {
        if ((getCanReadMethod = IdleModeLockoutServiceGrpc.getCanReadMethod) == null) {
          IdleModeLockoutServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteMethod = IdleModeLockoutServiceGrpc.getCanWriteMethod) == null) {
      synchronized (IdleModeLockoutServiceGrpc.class) {
        if ((getCanWriteMethod = IdleModeLockoutServiceGrpc.getCanWriteMethod) == null) {
          IdleModeLockoutServiceGrpc.getCanWriteMethod = getCanWriteMethod =
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
      com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getGetIdleModeLockoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetIdleModeLockout",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.idlelockout.IdleModeLockoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getGetIdleModeLockoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getGetIdleModeLockoutMethod;
    if ((getGetIdleModeLockoutMethod = IdleModeLockoutServiceGrpc.getGetIdleModeLockoutMethod) == null) {
      synchronized (IdleModeLockoutServiceGrpc.class) {
        if ((getGetIdleModeLockoutMethod = IdleModeLockoutServiceGrpc.getGetIdleModeLockoutMethod) == null) {
          IdleModeLockoutServiceGrpc.getGetIdleModeLockoutMethod = getGetIdleModeLockoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.idlelockout.IdleModeLockoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetIdleModeLockout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.idlelockout.IdleModeLockoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetIdleModeLockoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage,
      com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getSetIdleModeLockoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetIdleModeLockout",
      requestType = com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage.class,
      responseType = com.ifit.glassos.console.idlelockout.IdleModeLockoutResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage,
      com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getSetIdleModeLockoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage, com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getSetIdleModeLockoutMethod;
    if ((getSetIdleModeLockoutMethod = IdleModeLockoutServiceGrpc.getSetIdleModeLockoutMethod) == null) {
      synchronized (IdleModeLockoutServiceGrpc.class) {
        if ((getSetIdleModeLockoutMethod = IdleModeLockoutServiceGrpc.getSetIdleModeLockoutMethod) == null) {
          IdleModeLockoutServiceGrpc.getSetIdleModeLockoutMethod = getSetIdleModeLockoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage, com.ifit.glassos.console.idlelockout.IdleModeLockoutResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetIdleModeLockout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.idlelockout.IdleModeLockoutResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetIdleModeLockoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage> getIdleModeLockoutSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IdleModeLockoutSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage> getIdleModeLockoutSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage> getIdleModeLockoutSubscriptionMethod;
    if ((getIdleModeLockoutSubscriptionMethod = IdleModeLockoutServiceGrpc.getIdleModeLockoutSubscriptionMethod) == null) {
      synchronized (IdleModeLockoutServiceGrpc.class) {
        if ((getIdleModeLockoutSubscriptionMethod = IdleModeLockoutServiceGrpc.getIdleModeLockoutSubscriptionMethod) == null) {
          IdleModeLockoutServiceGrpc.getIdleModeLockoutSubscriptionMethod = getIdleModeLockoutSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IdleModeLockoutSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIdleModeLockoutSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static IdleModeLockoutServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<IdleModeLockoutServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<IdleModeLockoutServiceStub>() {
        @java.lang.Override
        public IdleModeLockoutServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new IdleModeLockoutServiceStub(channel, callOptions);
        }
      };
    return IdleModeLockoutServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static IdleModeLockoutServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<IdleModeLockoutServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<IdleModeLockoutServiceBlockingStub>() {
        @java.lang.Override
        public IdleModeLockoutServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new IdleModeLockoutServiceBlockingStub(channel, callOptions);
        }
      };
    return IdleModeLockoutServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static IdleModeLockoutServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<IdleModeLockoutServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<IdleModeLockoutServiceFutureStub>() {
        @java.lang.Override
        public IdleModeLockoutServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new IdleModeLockoutServiceFutureStub(channel, callOptions);
        }
      };
    return IdleModeLockoutServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
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
     */
    default void getIdleModeLockout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetIdleModeLockoutMethod(), responseObserver);
    }

    /**
     */
    default void setIdleModeLockout(com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetIdleModeLockoutMethod(), responseObserver);
    }

    /**
     */
    default void idleModeLockoutSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIdleModeLockoutSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service IdleModeLockoutService.
   */
  public static abstract class IdleModeLockoutServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return IdleModeLockoutServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service IdleModeLockoutService.
   */
  public static final class IdleModeLockoutServiceStub
      extends io.grpc.stub.AbstractAsyncStub<IdleModeLockoutServiceStub> {
    private IdleModeLockoutServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected IdleModeLockoutServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new IdleModeLockoutServiceStub(channel, callOptions);
    }

    /**
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
     */
    public void getIdleModeLockout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetIdleModeLockoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setIdleModeLockout(com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetIdleModeLockoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void idleModeLockoutSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getIdleModeLockoutSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service IdleModeLockoutService.
   */
  public static final class IdleModeLockoutServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<IdleModeLockoutServiceBlockingStub> {
    private IdleModeLockoutServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected IdleModeLockoutServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new IdleModeLockoutServiceBlockingStub(channel, callOptions);
    }

    /**
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
     */
    public com.ifit.glassos.console.idlelockout.IdleModeLockoutResult getIdleModeLockout(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetIdleModeLockoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.idlelockout.IdleModeLockoutResult setIdleModeLockout(com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetIdleModeLockoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage> idleModeLockoutSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getIdleModeLockoutSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service IdleModeLockoutService.
   */
  public static final class IdleModeLockoutServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<IdleModeLockoutServiceFutureStub> {
    private IdleModeLockoutServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected IdleModeLockoutServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new IdleModeLockoutServiceFutureStub(channel, callOptions);
    }

    /**
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
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> getIdleModeLockout(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetIdleModeLockoutMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult> setIdleModeLockout(
        com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetIdleModeLockoutMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE = 1;
  private static final int METHODID_GET_IDLE_MODE_LOCKOUT = 2;
  private static final int METHODID_SET_IDLE_MODE_LOCKOUT = 3;
  private static final int METHODID_IDLE_MODE_LOCKOUT_SUBSCRIPTION = 4;

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
        case METHODID_GET_IDLE_MODE_LOCKOUT:
          serviceImpl.getIdleModeLockout((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult>) responseObserver);
          break;
        case METHODID_SET_IDLE_MODE_LOCKOUT:
          serviceImpl.setIdleModeLockout((com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutResult>) responseObserver);
          break;
        case METHODID_IDLE_MODE_LOCKOUT_SUBSCRIPTION:
          serviceImpl.idleModeLockoutSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage>) responseObserver);
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
          getGetIdleModeLockoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.idlelockout.IdleModeLockoutResult>(
                service, METHODID_GET_IDLE_MODE_LOCKOUT)))
        .addMethod(
          getSetIdleModeLockoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage,
              com.ifit.glassos.console.idlelockout.IdleModeLockoutResult>(
                service, METHODID_SET_IDLE_MODE_LOCKOUT)))
        .addMethod(
          getIdleModeLockoutSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.idlelockout.IdleModeLockoutMessage>(
                service, METHODID_IDLE_MODE_LOCKOUT_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (IdleModeLockoutServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetIdleModeLockoutMethod())
              .addMethod(getSetIdleModeLockoutMethod())
              .addMethod(getIdleModeLockoutSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

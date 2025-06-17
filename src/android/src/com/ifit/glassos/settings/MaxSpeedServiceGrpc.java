package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/MaxSpeedService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class MaxSpeedServiceGrpc {

  private MaxSpeedServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.MaxSpeedService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.MaxSpeedMessage,
      com.ifit.glassos.util.Empty> getSetMaxSpeedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetMaxSpeed",
      requestType = com.ifit.glassos.settings.MaxSpeedMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.MaxSpeedMessage,
      com.ifit.glassos.util.Empty> getSetMaxSpeedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.MaxSpeedMessage, com.ifit.glassos.util.Empty> getSetMaxSpeedMethod;
    if ((getSetMaxSpeedMethod = MaxSpeedServiceGrpc.getSetMaxSpeedMethod) == null) {
      synchronized (MaxSpeedServiceGrpc.class) {
        if ((getSetMaxSpeedMethod = MaxSpeedServiceGrpc.getSetMaxSpeedMethod) == null) {
          MaxSpeedServiceGrpc.getSetMaxSpeedMethod = getSetMaxSpeedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.MaxSpeedMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetMaxSpeed"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.MaxSpeedMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetMaxSpeedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.MaxSpeedMessage> getMaxSpeedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "MaxSpeedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.MaxSpeedMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.MaxSpeedMessage> getMaxSpeedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.MaxSpeedMessage> getMaxSpeedSubscriptionMethod;
    if ((getMaxSpeedSubscriptionMethod = MaxSpeedServiceGrpc.getMaxSpeedSubscriptionMethod) == null) {
      synchronized (MaxSpeedServiceGrpc.class) {
        if ((getMaxSpeedSubscriptionMethod = MaxSpeedServiceGrpc.getMaxSpeedSubscriptionMethod) == null) {
          MaxSpeedServiceGrpc.getMaxSpeedSubscriptionMethod = getMaxSpeedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.MaxSpeedMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "MaxSpeedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.MaxSpeedMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getMaxSpeedSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static MaxSpeedServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<MaxSpeedServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<MaxSpeedServiceStub>() {
        @java.lang.Override
        public MaxSpeedServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new MaxSpeedServiceStub(channel, callOptions);
        }
      };
    return MaxSpeedServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static MaxSpeedServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<MaxSpeedServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<MaxSpeedServiceBlockingStub>() {
        @java.lang.Override
        public MaxSpeedServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new MaxSpeedServiceBlockingStub(channel, callOptions);
        }
      };
    return MaxSpeedServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static MaxSpeedServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<MaxSpeedServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<MaxSpeedServiceFutureStub>() {
        @java.lang.Override
        public MaxSpeedServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new MaxSpeedServiceFutureStub(channel, callOptions);
        }
      };
    return MaxSpeedServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * sets the Max Speed for scaling workouts, we will not auto scale to a control set that would
     * go above this value
     * </pre>
     */
    default void setMaxSpeed(com.ifit.glassos.settings.MaxSpeedMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetMaxSpeedMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to active pulse state updates
     * </pre>
     */
    default void maxSpeedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.MaxSpeedMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getMaxSpeedSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service MaxSpeedService.
   */
  public static abstract class MaxSpeedServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return MaxSpeedServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service MaxSpeedService.
   */
  public static final class MaxSpeedServiceStub
      extends io.grpc.stub.AbstractAsyncStub<MaxSpeedServiceStub> {
    private MaxSpeedServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected MaxSpeedServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new MaxSpeedServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * sets the Max Speed for scaling workouts, we will not auto scale to a control set that would
     * go above this value
     * </pre>
     */
    public void setMaxSpeed(com.ifit.glassos.settings.MaxSpeedMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetMaxSpeedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to active pulse state updates
     * </pre>
     */
    public void maxSpeedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.MaxSpeedMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getMaxSpeedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service MaxSpeedService.
   */
  public static final class MaxSpeedServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<MaxSpeedServiceBlockingStub> {
    private MaxSpeedServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected MaxSpeedServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new MaxSpeedServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * sets the Max Speed for scaling workouts, we will not auto scale to a control set that would
     * go above this value
     * </pre>
     */
    public com.ifit.glassos.util.Empty setMaxSpeed(com.ifit.glassos.settings.MaxSpeedMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetMaxSpeedMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to active pulse state updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.settings.MaxSpeedMessage> maxSpeedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getMaxSpeedSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service MaxSpeedService.
   */
  public static final class MaxSpeedServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<MaxSpeedServiceFutureStub> {
    private MaxSpeedServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected MaxSpeedServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new MaxSpeedServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * sets the Max Speed for scaling workouts, we will not auto scale to a control set that would
     * go above this value
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setMaxSpeed(
        com.ifit.glassos.settings.MaxSpeedMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetMaxSpeedMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_SET_MAX_SPEED = 0;
  private static final int METHODID_MAX_SPEED_SUBSCRIPTION = 1;

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
        case METHODID_SET_MAX_SPEED:
          serviceImpl.setMaxSpeed((com.ifit.glassos.settings.MaxSpeedMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_MAX_SPEED_SUBSCRIPTION:
          serviceImpl.maxSpeedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.MaxSpeedMessage>) responseObserver);
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
          getSetMaxSpeedMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.MaxSpeedMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_MAX_SPEED)))
        .addMethod(
          getMaxSpeedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.MaxSpeedMessage>(
                service, METHODID_MAX_SPEED_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (MaxSpeedServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getSetMaxSpeedMethod())
              .addMethod(getMaxSpeedSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

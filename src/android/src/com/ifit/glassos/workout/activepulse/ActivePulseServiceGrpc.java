package com.ifit.glassos.workout.activepulse;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/activepulse/ActivePulseService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ActivePulseServiceGrpc {

  private ActivePulseServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ActivePulseService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetIsActiveMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetIsActive",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetIsActiveMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getGetIsActiveMethod;
    if ((getGetIsActiveMethod = ActivePulseServiceGrpc.getGetIsActiveMethod) == null) {
      synchronized (ActivePulseServiceGrpc.class) {
        if ((getGetIsActiveMethod = ActivePulseServiceGrpc.getGetIsActiveMethod) == null) {
          ActivePulseServiceGrpc.getGetIsActiveMethod = getGetIsActiveMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetIsActive"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetIsActiveMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getSetIsActiveMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetIsActive",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getSetIsActiveMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty> getSetIsActiveMethod;
    if ((getSetIsActiveMethod = ActivePulseServiceGrpc.getSetIsActiveMethod) == null) {
      synchronized (ActivePulseServiceGrpc.class) {
        if ((getSetIsActiveMethod = ActivePulseServiceGrpc.getSetIsActiveMethod) == null) {
          ActivePulseServiceGrpc.getSetIsActiveMethod = getSetIsActiveMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetIsActive"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetIsActiveMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.activepulse.ActivePulseState> getActivePulseStateSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ActivePulseStateSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.activepulse.ActivePulseState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.activepulse.ActivePulseState> getActivePulseStateSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.activepulse.ActivePulseState> getActivePulseStateSubscriptionMethod;
    if ((getActivePulseStateSubscriptionMethod = ActivePulseServiceGrpc.getActivePulseStateSubscriptionMethod) == null) {
      synchronized (ActivePulseServiceGrpc.class) {
        if ((getActivePulseStateSubscriptionMethod = ActivePulseServiceGrpc.getActivePulseStateSubscriptionMethod) == null) {
          ActivePulseServiceGrpc.getActivePulseStateSubscriptionMethod = getActivePulseStateSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.activepulse.ActivePulseState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ActivePulseStateSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.activepulse.ActivePulseState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getActivePulseStateSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ActivePulseServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ActivePulseServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ActivePulseServiceStub>() {
        @java.lang.Override
        public ActivePulseServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ActivePulseServiceStub(channel, callOptions);
        }
      };
    return ActivePulseServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ActivePulseServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ActivePulseServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ActivePulseServiceBlockingStub>() {
        @java.lang.Override
        public ActivePulseServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ActivePulseServiceBlockingStub(channel, callOptions);
        }
      };
    return ActivePulseServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ActivePulseServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ActivePulseServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ActivePulseServiceFutureStub>() {
        @java.lang.Override
        public ActivePulseServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ActivePulseServiceFutureStub(channel, callOptions);
        }
      };
    return ActivePulseServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * gets the current active state of the service
     * </pre>
     */
    default void getIsActive(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetIsActiveMethod(), responseObserver);
    }

    /**
     * <pre>
     * sets the active state of the service
     * </pre>
     */
    default void setIsActive(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetIsActiveMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to active pulse state updates
     * </pre>
     */
    default void activePulseStateSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.activepulse.ActivePulseState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getActivePulseStateSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ActivePulseService.
   */
  public static abstract class ActivePulseServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ActivePulseServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ActivePulseService.
   */
  public static final class ActivePulseServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ActivePulseServiceStub> {
    private ActivePulseServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ActivePulseServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ActivePulseServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * gets the current active state of the service
     * </pre>
     */
    public void getIsActive(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetIsActiveMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * sets the active state of the service
     * </pre>
     */
    public void setIsActive(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetIsActiveMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to active pulse state updates
     * </pre>
     */
    public void activePulseStateSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.activepulse.ActivePulseState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getActivePulseStateSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ActivePulseService.
   */
  public static final class ActivePulseServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ActivePulseServiceBlockingStub> {
    private ActivePulseServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ActivePulseServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ActivePulseServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * gets the current active state of the service
     * </pre>
     */
    public com.ifit.glassos.util.BooleanResponse getIsActive(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetIsActiveMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * sets the active state of the service
     * </pre>
     */
    public com.ifit.glassos.util.Empty setIsActive(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetIsActiveMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to active pulse state updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.activepulse.ActivePulseState> activePulseStateSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getActivePulseStateSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ActivePulseService.
   */
  public static final class ActivePulseServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ActivePulseServiceFutureStub> {
    private ActivePulseServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ActivePulseServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ActivePulseServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * gets the current active state of the service
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> getIsActive(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetIsActiveMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * sets the active state of the service
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setIsActive(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetIsActiveMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_IS_ACTIVE = 0;
  private static final int METHODID_SET_IS_ACTIVE = 1;
  private static final int METHODID_ACTIVE_PULSE_STATE_SUBSCRIPTION = 2;

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
        case METHODID_GET_IS_ACTIVE:
          serviceImpl.getIsActive((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_SET_IS_ACTIVE:
          serviceImpl.setIsActive((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_ACTIVE_PULSE_STATE_SUBSCRIPTION:
          serviceImpl.activePulseStateSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.activepulse.ActivePulseState>) responseObserver);
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
          getGetIsActiveMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_GET_IS_ACTIVE)))
        .addMethod(
          getSetIsActiveMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_IS_ACTIVE)))
        .addMethod(
          getActivePulseStateSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.activepulse.ActivePulseState>(
                service, METHODID_ACTIVE_PULSE_STATE_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ActivePulseServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetIsActiveMethod())
              .addMethod(getSetIsActiveMethod())
              .addMethod(getActivePulseStateSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

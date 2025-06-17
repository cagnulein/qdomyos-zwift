package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/SmartAdjustService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class SmartAdjustServiceGrpc {

  private SmartAdjustServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.SmartAdjustService";

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
    if ((getGetIsActiveMethod = SmartAdjustServiceGrpc.getGetIsActiveMethod) == null) {
      synchronized (SmartAdjustServiceGrpc.class) {
        if ((getGetIsActiveMethod = SmartAdjustServiceGrpc.getGetIsActiveMethod) == null) {
          SmartAdjustServiceGrpc.getGetIsActiveMethod = getGetIsActiveMethod =
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
    if ((getSetIsActiveMethod = SmartAdjustServiceGrpc.getSetIsActiveMethod) == null) {
      synchronized (SmartAdjustServiceGrpc.class) {
        if ((getSetIsActiveMethod = SmartAdjustServiceGrpc.getSetIsActiveMethod) == null) {
          SmartAdjustServiceGrpc.getSetIsActiveMethod = getSetIsActiveMethod =
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
      com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse> getScaleChangeNotificationSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ScaleChangeNotificationSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse> getScaleChangeNotificationSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse> getScaleChangeNotificationSubscriptionMethod;
    if ((getScaleChangeNotificationSubscriptionMethod = SmartAdjustServiceGrpc.getScaleChangeNotificationSubscriptionMethod) == null) {
      synchronized (SmartAdjustServiceGrpc.class) {
        if ((getScaleChangeNotificationSubscriptionMethod = SmartAdjustServiceGrpc.getScaleChangeNotificationSubscriptionMethod) == null) {
          SmartAdjustServiceGrpc.getScaleChangeNotificationSubscriptionMethod = getScaleChangeNotificationSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ScaleChangeNotificationSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getScaleChangeNotificationSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static SmartAdjustServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SmartAdjustServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SmartAdjustServiceStub>() {
        @java.lang.Override
        public SmartAdjustServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SmartAdjustServiceStub(channel, callOptions);
        }
      };
    return SmartAdjustServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static SmartAdjustServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SmartAdjustServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SmartAdjustServiceBlockingStub>() {
        @java.lang.Override
        public SmartAdjustServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SmartAdjustServiceBlockingStub(channel, callOptions);
        }
      };
    return SmartAdjustServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static SmartAdjustServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SmartAdjustServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SmartAdjustServiceFutureStub>() {
        @java.lang.Override
        public SmartAdjustServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SmartAdjustServiceFutureStub(channel, callOptions);
        }
      };
    return SmartAdjustServiceFutureStub.newStub(factory, channel);
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
     * subscribe to notification updates
     * </pre>
     */
    default void scaleChangeNotificationSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getScaleChangeNotificationSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service SmartAdjustService.
   */
  public static abstract class SmartAdjustServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return SmartAdjustServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service SmartAdjustService.
   */
  public static final class SmartAdjustServiceStub
      extends io.grpc.stub.AbstractAsyncStub<SmartAdjustServiceStub> {
    private SmartAdjustServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SmartAdjustServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SmartAdjustServiceStub(channel, callOptions);
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
     * subscribe to notification updates
     * </pre>
     */
    public void scaleChangeNotificationSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getScaleChangeNotificationSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service SmartAdjustService.
   */
  public static final class SmartAdjustServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<SmartAdjustServiceBlockingStub> {
    private SmartAdjustServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SmartAdjustServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SmartAdjustServiceBlockingStub(channel, callOptions);
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
     * subscribe to notification updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse> scaleChangeNotificationSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getScaleChangeNotificationSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service SmartAdjustService.
   */
  public static final class SmartAdjustServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<SmartAdjustServiceFutureStub> {
    private SmartAdjustServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SmartAdjustServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SmartAdjustServiceFutureStub(channel, callOptions);
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
  private static final int METHODID_SCALE_CHANGE_NOTIFICATION_SUBSCRIPTION = 2;

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
        case METHODID_SCALE_CHANGE_NOTIFICATION_SUBSCRIPTION:
          serviceImpl.scaleChangeNotificationSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse>) responseObserver);
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
          getScaleChangeNotificationSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.SmartAdjustScaleChangeNotificationResponse>(
                service, METHODID_SCALE_CHANGE_NOTIFICATION_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (SmartAdjustServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetIsActiveMethod())
              .addMethod(getSetIsActiveMethod())
              .addMethod(getScaleChangeNotificationSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

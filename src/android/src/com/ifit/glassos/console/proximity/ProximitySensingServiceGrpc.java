package com.ifit.glassos.console.proximity;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/proximity/ProximitySensingService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ProximitySensingServiceGrpc {

  private ProximitySensingServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ProximitySensingService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanReadMovementDetectMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanReadMovementDetect",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanReadMovementDetectMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanReadMovementDetectMethod;
    if ((getCanReadMovementDetectMethod = ProximitySensingServiceGrpc.getCanReadMovementDetectMethod) == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        if ((getCanReadMovementDetectMethod = ProximitySensingServiceGrpc.getCanReadMovementDetectMethod) == null) {
          ProximitySensingServiceGrpc.getCanReadMovementDetectMethod = getCanReadMovementDetectMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanReadMovementDetect"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanReadMovementDetectMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanReadUserDistanceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanReadUserDistance",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanReadUserDistanceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanReadUserDistanceMethod;
    if ((getCanReadUserDistanceMethod = ProximitySensingServiceGrpc.getCanReadUserDistanceMethod) == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        if ((getCanReadUserDistanceMethod = ProximitySensingServiceGrpc.getCanReadUserDistanceMethod) == null) {
          ProximitySensingServiceGrpc.getCanReadUserDistanceMethod = getCanReadUserDistanceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanReadUserDistance"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanReadUserDistanceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetMovementDetectMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetMovementDetect",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetMovementDetectMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getGetMovementDetectMethod;
    if ((getGetMovementDetectMethod = ProximitySensingServiceGrpc.getGetMovementDetectMethod) == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        if ((getGetMovementDetectMethod = ProximitySensingServiceGrpc.getGetMovementDetectMethod) == null) {
          ProximitySensingServiceGrpc.getGetMovementDetectMethod = getGetMovementDetectMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetMovementDetect"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetMovementDetectMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getMovementDetectSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "MovementDetectSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getMovementDetectSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getMovementDetectSubscriptionMethod;
    if ((getMovementDetectSubscriptionMethod = ProximitySensingServiceGrpc.getMovementDetectSubscriptionMethod) == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        if ((getMovementDetectSubscriptionMethod = ProximitySensingServiceGrpc.getMovementDetectSubscriptionMethod) == null) {
          ProximitySensingServiceGrpc.getMovementDetectSubscriptionMethod = getMovementDetectSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "MovementDetectSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getMovementDetectSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getGetUserDistanceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetUserDistance",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.FloatResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getGetUserDistanceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse> getGetUserDistanceMethod;
    if ((getGetUserDistanceMethod = ProximitySensingServiceGrpc.getGetUserDistanceMethod) == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        if ((getGetUserDistanceMethod = ProximitySensingServiceGrpc.getGetUserDistanceMethod) == null) {
          ProximitySensingServiceGrpc.getGetUserDistanceMethod = getGetUserDistanceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetUserDistance"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.FloatResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetUserDistanceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getUserDistanceSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "UserDistanceSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.FloatResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getUserDistanceSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse> getUserDistanceSubscriptionMethod;
    if ((getUserDistanceSubscriptionMethod = ProximitySensingServiceGrpc.getUserDistanceSubscriptionMethod) == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        if ((getUserDistanceSubscriptionMethod = ProximitySensingServiceGrpc.getUserDistanceSubscriptionMethod) == null) {
          ProximitySensingServiceGrpc.getUserDistanceSubscriptionMethod = getUserDistanceSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "UserDistanceSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.FloatResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getUserDistanceSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ProximitySensingServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ProximitySensingServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ProximitySensingServiceStub>() {
        @java.lang.Override
        public ProximitySensingServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ProximitySensingServiceStub(channel, callOptions);
        }
      };
    return ProximitySensingServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ProximitySensingServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ProximitySensingServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ProximitySensingServiceBlockingStub>() {
        @java.lang.Override
        public ProximitySensingServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ProximitySensingServiceBlockingStub(channel, callOptions);
        }
      };
    return ProximitySensingServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ProximitySensingServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ProximitySensingServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ProximitySensingServiceFutureStub>() {
        @java.lang.Override
        public ProximitySensingServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ProximitySensingServiceFutureStub(channel, callOptions);
        }
      };
    return ProximitySensingServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     * <pre>
     * check whether Movement Detection (PIR) is Readable
     * </pre>
     */
    default void canReadMovementDetect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadMovementDetectMethod(), responseObserver);
    }

    /**
     * <pre>
     * check whether User Distance (LIDAR) is Readable
     * </pre>
     */
    default void canReadUserDistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanReadUserDistanceMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current Movement Detection (PIR) state
     * </pre>
     */
    default void getMovementDetect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetMovementDetectMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to Movement Detection (PIR) updates
     * </pre>
     */
    default void movementDetectSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getMovementDetectSubscriptionMethod(), responseObserver);
    }

    /**
     * <pre>
     * get the current User Distance (LIDAR) in centimeters
     * </pre>
     */
    default void getUserDistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetUserDistanceMethod(), responseObserver);
    }

    /**
     * <pre>
     * subscribe to User Distance (LIDAR) updates
     * </pre>
     */
    default void userDistanceSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getUserDistanceSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ProximitySensingService.
   */
  public static abstract class ProximitySensingServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ProximitySensingServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ProximitySensingService.
   */
  public static final class ProximitySensingServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ProximitySensingServiceStub> {
    private ProximitySensingServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ProximitySensingServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ProximitySensingServiceStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Movement Detection (PIR) is Readable
     * </pre>
     */
    public void canReadMovementDetect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadMovementDetectMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * check whether User Distance (LIDAR) is Readable
     * </pre>
     */
    public void canReadUserDistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanReadUserDistanceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current Movement Detection (PIR) state
     * </pre>
     */
    public void getMovementDetect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetMovementDetectMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to Movement Detection (PIR) updates
     * </pre>
     */
    public void movementDetectSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getMovementDetectSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * get the current User Distance (LIDAR) in centimeters
     * </pre>
     */
    public void getUserDistance(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetUserDistanceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     * <pre>
     * subscribe to User Distance (LIDAR) updates
     * </pre>
     */
    public void userDistanceSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getUserDistanceSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ProximitySensingService.
   */
  public static final class ProximitySensingServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ProximitySensingServiceBlockingStub> {
    private ProximitySensingServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ProximitySensingServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ProximitySensingServiceBlockingStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Movement Detection (PIR) is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canReadMovementDetect(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMovementDetectMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * check whether User Distance (LIDAR) is Readable
     * </pre>
     */
    public com.ifit.glassos.util.AvailabilityResponse canReadUserDistance(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadUserDistanceMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current Movement Detection (PIR) state
     * </pre>
     */
    public com.ifit.glassos.util.BooleanResponse getMovementDetect(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetMovementDetectMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to Movement Detection (PIR) updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> movementDetectSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getMovementDetectSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * get the current User Distance (LIDAR) in centimeters
     * </pre>
     */
    public com.ifit.glassos.util.FloatResponse getUserDistance(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetUserDistanceMethod(), getCallOptions(), request);
    }

    /**
     * <pre>
     * subscribe to User Distance (LIDAR) updates
     * </pre>
     */
    public java.util.Iterator<com.ifit.glassos.util.FloatResponse> userDistanceSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getUserDistanceSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ProximitySensingService.
   */
  public static final class ProximitySensingServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ProximitySensingServiceFutureStub> {
    private ProximitySensingServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ProximitySensingServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ProximitySensingServiceFutureStub(channel, callOptions);
    }

    /**
     * <pre>
     * check whether Movement Detection (PIR) is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canReadMovementDetect(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadMovementDetectMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * check whether User Distance (LIDAR) is Readable
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canReadUserDistance(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanReadUserDistanceMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current Movement Detection (PIR) state
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> getMovementDetect(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetMovementDetectMethod(), getCallOptions()), request);
    }

    /**
     * <pre>
     * get the current User Distance (LIDAR) in centimeters
     * </pre>
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.FloatResponse> getUserDistance(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetUserDistanceMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ_MOVEMENT_DETECT = 0;
  private static final int METHODID_CAN_READ_USER_DISTANCE = 1;
  private static final int METHODID_GET_MOVEMENT_DETECT = 2;
  private static final int METHODID_MOVEMENT_DETECT_SUBSCRIPTION = 3;
  private static final int METHODID_GET_USER_DISTANCE = 4;
  private static final int METHODID_USER_DISTANCE_SUBSCRIPTION = 5;

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
        case METHODID_CAN_READ_MOVEMENT_DETECT:
          serviceImpl.canReadMovementDetect((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_CAN_READ_USER_DISTANCE:
          serviceImpl.canReadUserDistance((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_GET_MOVEMENT_DETECT:
          serviceImpl.getMovementDetect((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_MOVEMENT_DETECT_SUBSCRIPTION:
          serviceImpl.movementDetectSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_GET_USER_DISTANCE:
          serviceImpl.getUserDistance((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse>) responseObserver);
          break;
        case METHODID_USER_DISTANCE_SUBSCRIPTION:
          serviceImpl.userDistanceSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse>) responseObserver);
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
          getCanReadMovementDetectMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_READ_MOVEMENT_DETECT)))
        .addMethod(
          getCanReadUserDistanceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_READ_USER_DISTANCE)))
        .addMethod(
          getGetMovementDetectMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_GET_MOVEMENT_DETECT)))
        .addMethod(
          getMovementDetectSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_MOVEMENT_DETECT_SUBSCRIPTION)))
        .addMethod(
          getGetUserDistanceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.FloatResponse>(
                service, METHODID_GET_USER_DISTANCE)))
        .addMethod(
          getUserDistanceSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.FloatResponse>(
                service, METHODID_USER_DISTANCE_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ProximitySensingServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMovementDetectMethod())
              .addMethod(getCanReadUserDistanceMethod())
              .addMethod(getGetMovementDetectMethod())
              .addMethod(getMovementDetectSubscriptionMethod())
              .addMethod(getGetUserDistanceMethod())
              .addMethod(getUserDistanceSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

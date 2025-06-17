package com.ifit.glassos.console.keypress;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/keypress/KeyPressService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class KeyPressServiceGrpc {

  private KeyPressServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.KeyPressService";

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
    if ((getCanReadMethod = KeyPressServiceGrpc.getCanReadMethod) == null) {
      synchronized (KeyPressServiceGrpc.class) {
        if ((getCanReadMethod = KeyPressServiceGrpc.getCanReadMethod) == null) {
          KeyPressServiceGrpc.getCanReadMethod = getCanReadMethod =
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
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteVirtualMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CanWriteVirtual",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getCanWriteVirtualMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getCanWriteVirtualMethod;
    if ((getCanWriteVirtualMethod = KeyPressServiceGrpc.getCanWriteVirtualMethod) == null) {
      synchronized (KeyPressServiceGrpc.class) {
        if ((getCanWriteVirtualMethod = KeyPressServiceGrpc.getCanWriteVirtualMethod) == null) {
          KeyPressServiceGrpc.getCanWriteVirtualMethod = getCanWriteVirtualMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CanWriteVirtual"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCanWriteVirtualMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.keypress.KeyPressResult> getGetKeyPressMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetKeyPress",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.keypress.KeyPressResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.keypress.KeyPressResult> getGetKeyPressMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.keypress.KeyPressResult> getGetKeyPressMethod;
    if ((getGetKeyPressMethod = KeyPressServiceGrpc.getGetKeyPressMethod) == null) {
      synchronized (KeyPressServiceGrpc.class) {
        if ((getGetKeyPressMethod = KeyPressServiceGrpc.getGetKeyPressMethod) == null) {
          KeyPressServiceGrpc.getGetKeyPressMethod = getGetKeyPressMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.keypress.KeyPressResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetKeyPress"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.keypress.KeyPressResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetKeyPressMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.keypress.KeyPress> getKeyPressSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "KeyPressSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.keypress.KeyPress.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.keypress.KeyPress> getKeyPressSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.keypress.KeyPress> getKeyPressSubscriptionMethod;
    if ((getKeyPressSubscriptionMethod = KeyPressServiceGrpc.getKeyPressSubscriptionMethod) == null) {
      synchronized (KeyPressServiceGrpc.class) {
        if ((getKeyPressSubscriptionMethod = KeyPressServiceGrpc.getKeyPressSubscriptionMethod) == null) {
          KeyPressServiceGrpc.getKeyPressSubscriptionMethod = getKeyPressSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.keypress.KeyPress>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "KeyPressSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.keypress.KeyPress.getDefaultInstance()))
              .build();
        }
      }
    }
    return getKeyPressSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.keypress.KeyPress,
      com.ifit.glassos.console.keypress.KeyPressResult> getSetVirtualKeyPressMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetVirtualKeyPress",
      requestType = com.ifit.glassos.console.keypress.KeyPress.class,
      responseType = com.ifit.glassos.console.keypress.KeyPressResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.keypress.KeyPress,
      com.ifit.glassos.console.keypress.KeyPressResult> getSetVirtualKeyPressMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.keypress.KeyPress, com.ifit.glassos.console.keypress.KeyPressResult> getSetVirtualKeyPressMethod;
    if ((getSetVirtualKeyPressMethod = KeyPressServiceGrpc.getSetVirtualKeyPressMethod) == null) {
      synchronized (KeyPressServiceGrpc.class) {
        if ((getSetVirtualKeyPressMethod = KeyPressServiceGrpc.getSetVirtualKeyPressMethod) == null) {
          KeyPressServiceGrpc.getSetVirtualKeyPressMethod = getSetVirtualKeyPressMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.keypress.KeyPress, com.ifit.glassos.console.keypress.KeyPressResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetVirtualKeyPress"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.keypress.KeyPress.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.keypress.KeyPressResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetVirtualKeyPressMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static KeyPressServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<KeyPressServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<KeyPressServiceStub>() {
        @java.lang.Override
        public KeyPressServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new KeyPressServiceStub(channel, callOptions);
        }
      };
    return KeyPressServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static KeyPressServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<KeyPressServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<KeyPressServiceBlockingStub>() {
        @java.lang.Override
        public KeyPressServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new KeyPressServiceBlockingStub(channel, callOptions);
        }
      };
    return KeyPressServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static KeyPressServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<KeyPressServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<KeyPressServiceFutureStub>() {
        @java.lang.Override
        public KeyPressServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new KeyPressServiceFutureStub(channel, callOptions);
        }
      };
    return KeyPressServiceFutureStub.newStub(factory, channel);
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
    default void canWriteVirtual(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCanWriteVirtualMethod(), responseObserver);
    }

    /**
     */
    default void getKeyPress(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPressResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetKeyPressMethod(), responseObserver);
    }

    /**
     */
    default void keyPressSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPress> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getKeyPressSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void setVirtualKeyPress(com.ifit.glassos.console.keypress.KeyPress request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPressResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetVirtualKeyPressMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service KeyPressService.
   */
  public static abstract class KeyPressServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return KeyPressServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service KeyPressService.
   */
  public static final class KeyPressServiceStub
      extends io.grpc.stub.AbstractAsyncStub<KeyPressServiceStub> {
    private KeyPressServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected KeyPressServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new KeyPressServiceStub(channel, callOptions);
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
    public void canWriteVirtual(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCanWriteVirtualMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getKeyPress(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPressResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetKeyPressMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void keyPressSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPress> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getKeyPressSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setVirtualKeyPress(com.ifit.glassos.console.keypress.KeyPress request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPressResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetVirtualKeyPressMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service KeyPressService.
   */
  public static final class KeyPressServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<KeyPressServiceBlockingStub> {
    private KeyPressServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected KeyPressServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new KeyPressServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.AvailabilityResponse canRead(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanReadMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.AvailabilityResponse canWriteVirtual(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCanWriteVirtualMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.keypress.KeyPressResult getKeyPress(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetKeyPressMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.keypress.KeyPress> keyPressSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getKeyPressSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.keypress.KeyPressResult setVirtualKeyPress(com.ifit.glassos.console.keypress.KeyPress request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetVirtualKeyPressMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service KeyPressService.
   */
  public static final class KeyPressServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<KeyPressServiceFutureStub> {
    private KeyPressServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected KeyPressServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new KeyPressServiceFutureStub(channel, callOptions);
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
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> canWriteVirtual(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCanWriteVirtualMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.keypress.KeyPressResult> getKeyPress(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetKeyPressMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.keypress.KeyPressResult> setVirtualKeyPress(
        com.ifit.glassos.console.keypress.KeyPress request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetVirtualKeyPressMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE_VIRTUAL = 1;
  private static final int METHODID_GET_KEY_PRESS = 2;
  private static final int METHODID_KEY_PRESS_SUBSCRIPTION = 3;
  private static final int METHODID_SET_VIRTUAL_KEY_PRESS = 4;

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
        case METHODID_CAN_WRITE_VIRTUAL:
          serviceImpl.canWriteVirtual((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_GET_KEY_PRESS:
          serviceImpl.getKeyPress((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPressResult>) responseObserver);
          break;
        case METHODID_KEY_PRESS_SUBSCRIPTION:
          serviceImpl.keyPressSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPress>) responseObserver);
          break;
        case METHODID_SET_VIRTUAL_KEY_PRESS:
          serviceImpl.setVirtualKeyPress((com.ifit.glassos.console.keypress.KeyPress) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.keypress.KeyPressResult>) responseObserver);
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
          getCanWriteVirtualMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_CAN_WRITE_VIRTUAL)))
        .addMethod(
          getGetKeyPressMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.keypress.KeyPressResult>(
                service, METHODID_GET_KEY_PRESS)))
        .addMethod(
          getKeyPressSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.keypress.KeyPress>(
                service, METHODID_KEY_PRESS_SUBSCRIPTION)))
        .addMethod(
          getSetVirtualKeyPressMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.keypress.KeyPress,
              com.ifit.glassos.console.keypress.KeyPressResult>(
                service, METHODID_SET_VIRTUAL_KEY_PRESS)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (KeyPressServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteVirtualMethod())
              .addMethod(getGetKeyPressMethod())
              .addMethod(getKeyPressSubscriptionMethod())
              .addMethod(getSetVirtualKeyPressMethod())
              .build();
        }
      }
    }
    return result;
  }
}

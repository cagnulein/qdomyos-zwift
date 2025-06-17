package com.ifit.glassos.console;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/ConsoleService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ConsoleServiceGrpc {

  private ConsoleServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ConsoleService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConnectionResult> getConnectMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Connect",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConnectionResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConnectionResult> getConnectMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConnectionResult> getConnectMethod;
    if ((getConnectMethod = ConsoleServiceGrpc.getConnectMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getConnectMethod = ConsoleServiceGrpc.getConnectMethod) == null) {
          ConsoleServiceGrpc.getConnectMethod = getConnectMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConnectionResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Connect"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConnectionResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConnectMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getDisconnectMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Disconnect",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getDisconnectMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getDisconnectMethod;
    if ((getDisconnectMethod = ConsoleServiceGrpc.getDisconnectMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getDisconnectMethod = ConsoleServiceGrpc.getDisconnectMethod) == null) {
          ConsoleServiceGrpc.getDisconnectMethod = getDisconnectMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Disconnect"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDisconnectMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getGetConsoleMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetConsole",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConsoleInfo.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getGetConsoleMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo> getGetConsoleMethod;
    if ((getGetConsoleMethod = ConsoleServiceGrpc.getGetConsoleMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getGetConsoleMethod = ConsoleServiceGrpc.getGetConsoleMethod) == null) {
          ConsoleServiceGrpc.getGetConsoleMethod = getGetConsoleMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetConsole"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConsoleInfo.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetConsoleMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getConsoleChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConsoleChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConsoleInfo.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getConsoleChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo> getConsoleChangedMethod;
    if ((getConsoleChangedMethod = ConsoleServiceGrpc.getConsoleChangedMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getConsoleChangedMethod = ConsoleServiceGrpc.getConsoleChangedMethod) == null) {
          ConsoleServiceGrpc.getConsoleChangedMethod = getConsoleChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConsoleChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConsoleInfo.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConsoleChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleStateMessage> getGetConsoleStateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetConsoleState",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConsoleStateMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleStateMessage> getGetConsoleStateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleStateMessage> getGetConsoleStateMethod;
    if ((getGetConsoleStateMethod = ConsoleServiceGrpc.getGetConsoleStateMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getGetConsoleStateMethod = ConsoleServiceGrpc.getGetConsoleStateMethod) == null) {
          ConsoleServiceGrpc.getGetConsoleStateMethod = getGetConsoleStateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleStateMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetConsoleState"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConsoleStateMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetConsoleStateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleStateMessage> getConsoleStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConsoleStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConsoleStateMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleStateMessage> getConsoleStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleStateMessage> getConsoleStateChangedMethod;
    if ((getConsoleStateChangedMethod = ConsoleServiceGrpc.getConsoleStateChangedMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getConsoleStateChangedMethod = ConsoleServiceGrpc.getConsoleStateChangedMethod) == null) {
          ConsoleServiceGrpc.getConsoleStateChangedMethod = getConsoleStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleStateMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConsoleStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConsoleStateMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConsoleStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getGetKnownConsoleInfoMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetKnownConsoleInfo",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConsoleInfo.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getGetKnownConsoleInfoMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo> getGetKnownConsoleInfoMethod;
    if ((getGetKnownConsoleInfoMethod = ConsoleServiceGrpc.getGetKnownConsoleInfoMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getGetKnownConsoleInfoMethod = ConsoleServiceGrpc.getGetKnownConsoleInfoMethod) == null) {
          ConsoleServiceGrpc.getGetKnownConsoleInfoMethod = getGetKnownConsoleInfoMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetKnownConsoleInfo"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConsoleInfo.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetKnownConsoleInfoMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getRefreshKnownConsoleInfoMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RefreshKnownConsoleInfo",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.ConsoleInfo.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.ConsoleInfo> getRefreshKnownConsoleInfoMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo> getRefreshKnownConsoleInfoMethod;
    if ((getRefreshKnownConsoleInfoMethod = ConsoleServiceGrpc.getRefreshKnownConsoleInfoMethod) == null) {
      synchronized (ConsoleServiceGrpc.class) {
        if ((getRefreshKnownConsoleInfoMethod = ConsoleServiceGrpc.getRefreshKnownConsoleInfoMethod) == null) {
          ConsoleServiceGrpc.getRefreshKnownConsoleInfoMethod = getRefreshKnownConsoleInfoMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.ConsoleInfo>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RefreshKnownConsoleInfo"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.ConsoleInfo.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRefreshKnownConsoleInfoMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ConsoleServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ConsoleServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ConsoleServiceStub>() {
        @java.lang.Override
        public ConsoleServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ConsoleServiceStub(channel, callOptions);
        }
      };
    return ConsoleServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ConsoleServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ConsoleServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ConsoleServiceBlockingStub>() {
        @java.lang.Override
        public ConsoleServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ConsoleServiceBlockingStub(channel, callOptions);
        }
      };
    return ConsoleServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ConsoleServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ConsoleServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ConsoleServiceFutureStub>() {
        @java.lang.Override
        public ConsoleServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ConsoleServiceFutureStub(channel, callOptions);
        }
      };
    return ConsoleServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void connect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConnectionResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConnectMethod(), responseObserver);
    }

    /**
     */
    default void disconnect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDisconnectMethod(), responseObserver);
    }

    /**
     */
    default void getConsole(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetConsoleMethod(), responseObserver);
    }

    /**
     */
    default void consoleChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConsoleChangedMethod(), responseObserver);
    }

    /**
     */
    default void getConsoleState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleStateMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetConsoleStateMethod(), responseObserver);
    }

    /**
     */
    default void consoleStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleStateMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConsoleStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void getKnownConsoleInfo(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetKnownConsoleInfoMethod(), responseObserver);
    }

    /**
     */
    default void refreshKnownConsoleInfo(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRefreshKnownConsoleInfoMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ConsoleService.
   */
  public static abstract class ConsoleServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ConsoleServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ConsoleService.
   */
  public static final class ConsoleServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ConsoleServiceStub> {
    private ConsoleServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ConsoleServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ConsoleServiceStub(channel, callOptions);
    }

    /**
     */
    public void connect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConnectionResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getConnectMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void disconnect(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getDisconnectMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getConsole(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetConsoleMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void consoleChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getConsoleChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getConsoleState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleStateMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetConsoleStateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void consoleStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleStateMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getConsoleStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getKnownConsoleInfo(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetKnownConsoleInfoMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void refreshKnownConsoleInfo(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRefreshKnownConsoleInfoMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ConsoleService.
   */
  public static final class ConsoleServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ConsoleServiceBlockingStub> {
    private ConsoleServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ConsoleServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ConsoleServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.console.ConnectionResult connect(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getConnectMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty disconnect(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getDisconnectMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.ConsoleInfo getConsole(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetConsoleMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.ConsoleInfo> consoleChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getConsoleChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.ConsoleStateMessage getConsoleState(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetConsoleStateMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.ConsoleStateMessage> consoleStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getConsoleStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.ConsoleInfo getKnownConsoleInfo(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetKnownConsoleInfoMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.ConsoleInfo refreshKnownConsoleInfo(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRefreshKnownConsoleInfoMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ConsoleService.
   */
  public static final class ConsoleServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ConsoleServiceFutureStub> {
    private ConsoleServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ConsoleServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ConsoleServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.ConnectionResult> connect(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getConnectMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> disconnect(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getDisconnectMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.ConsoleInfo> getConsole(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetConsoleMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.ConsoleStateMessage> getConsoleState(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetConsoleStateMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.ConsoleInfo> getKnownConsoleInfo(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetKnownConsoleInfoMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.ConsoleInfo> refreshKnownConsoleInfo(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRefreshKnownConsoleInfoMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CONNECT = 0;
  private static final int METHODID_DISCONNECT = 1;
  private static final int METHODID_GET_CONSOLE = 2;
  private static final int METHODID_CONSOLE_CHANGED = 3;
  private static final int METHODID_GET_CONSOLE_STATE = 4;
  private static final int METHODID_CONSOLE_STATE_CHANGED = 5;
  private static final int METHODID_GET_KNOWN_CONSOLE_INFO = 6;
  private static final int METHODID_REFRESH_KNOWN_CONSOLE_INFO = 7;

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
        case METHODID_CONNECT:
          serviceImpl.connect((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConnectionResult>) responseObserver);
          break;
        case METHODID_DISCONNECT:
          serviceImpl.disconnect((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_CONSOLE:
          serviceImpl.getConsole((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo>) responseObserver);
          break;
        case METHODID_CONSOLE_CHANGED:
          serviceImpl.consoleChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo>) responseObserver);
          break;
        case METHODID_GET_CONSOLE_STATE:
          serviceImpl.getConsoleState((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleStateMessage>) responseObserver);
          break;
        case METHODID_CONSOLE_STATE_CHANGED:
          serviceImpl.consoleStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleStateMessage>) responseObserver);
          break;
        case METHODID_GET_KNOWN_CONSOLE_INFO:
          serviceImpl.getKnownConsoleInfo((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo>) responseObserver);
          break;
        case METHODID_REFRESH_KNOWN_CONSOLE_INFO:
          serviceImpl.refreshKnownConsoleInfo((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.ConsoleInfo>) responseObserver);
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
          getConnectMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConnectionResult>(
                service, METHODID_CONNECT)))
        .addMethod(
          getDisconnectMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_DISCONNECT)))
        .addMethod(
          getGetConsoleMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConsoleInfo>(
                service, METHODID_GET_CONSOLE)))
        .addMethod(
          getConsoleChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConsoleInfo>(
                service, METHODID_CONSOLE_CHANGED)))
        .addMethod(
          getGetConsoleStateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConsoleStateMessage>(
                service, METHODID_GET_CONSOLE_STATE)))
        .addMethod(
          getConsoleStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConsoleStateMessage>(
                service, METHODID_CONSOLE_STATE_CHANGED)))
        .addMethod(
          getGetKnownConsoleInfoMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConsoleInfo>(
                service, METHODID_GET_KNOWN_CONSOLE_INFO)))
        .addMethod(
          getRefreshKnownConsoleInfoMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.ConsoleInfo>(
                service, METHODID_REFRESH_KNOWN_CONSOLE_INFO)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ConsoleServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getConnectMethod())
              .addMethod(getDisconnectMethod())
              .addMethod(getGetConsoleMethod())
              .addMethod(getConsoleChangedMethod())
              .addMethod(getGetConsoleStateMethod())
              .addMethod(getConsoleStateChangedMethod())
              .addMethod(getGetKnownConsoleInfoMethod())
              .addMethod(getRefreshKnownConsoleInfoMethod())
              .build();
        }
      }
    }
    return result;
  }
}

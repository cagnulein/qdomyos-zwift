package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/SystemUnitsService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class SystemUnitsServiceGrpc {

  private SystemUnitsServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.SystemUnitsService";

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
    if ((getCanReadMethod = SystemUnitsServiceGrpc.getCanReadMethod) == null) {
      synchronized (SystemUnitsServiceGrpc.class) {
        if ((getCanReadMethod = SystemUnitsServiceGrpc.getCanReadMethod) == null) {
          SystemUnitsServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteMethod = SystemUnitsServiceGrpc.getCanWriteMethod) == null) {
      synchronized (SystemUnitsServiceGrpc.class) {
        if ((getCanWriteMethod = SystemUnitsServiceGrpc.getCanWriteMethod) == null) {
          SystemUnitsServiceGrpc.getCanWriteMethod = getCanWriteMethod =
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
      com.ifit.glassos.settings.SystemUnitsMessage> getGetSystemUnitsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSystemUnits",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.SystemUnitsMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.SystemUnitsMessage> getGetSystemUnitsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.SystemUnitsMessage> getGetSystemUnitsMethod;
    if ((getGetSystemUnitsMethod = SystemUnitsServiceGrpc.getGetSystemUnitsMethod) == null) {
      synchronized (SystemUnitsServiceGrpc.class) {
        if ((getGetSystemUnitsMethod = SystemUnitsServiceGrpc.getGetSystemUnitsMethod) == null) {
          SystemUnitsServiceGrpc.getGetSystemUnitsMethod = getGetSystemUnitsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.SystemUnitsMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSystemUnits"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.SystemUnitsMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSystemUnitsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.SystemUnitsMessage> getSystemUnitsChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SystemUnitsChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.SystemUnitsMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.SystemUnitsMessage> getSystemUnitsChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.SystemUnitsMessage> getSystemUnitsChangedMethod;
    if ((getSystemUnitsChangedMethod = SystemUnitsServiceGrpc.getSystemUnitsChangedMethod) == null) {
      synchronized (SystemUnitsServiceGrpc.class) {
        if ((getSystemUnitsChangedMethod = SystemUnitsServiceGrpc.getSystemUnitsChangedMethod) == null) {
          SystemUnitsServiceGrpc.getSystemUnitsChangedMethod = getSystemUnitsChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.SystemUnitsMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SystemUnitsChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.SystemUnitsMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSystemUnitsChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage,
      com.ifit.glassos.util.Empty> getSetSystemUnitsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetSystemUnits",
      requestType = com.ifit.glassos.settings.SystemUnitsMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage,
      com.ifit.glassos.util.Empty> getSetSystemUnitsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage, com.ifit.glassos.util.Empty> getSetSystemUnitsMethod;
    if ((getSetSystemUnitsMethod = SystemUnitsServiceGrpc.getSetSystemUnitsMethod) == null) {
      synchronized (SystemUnitsServiceGrpc.class) {
        if ((getSetSystemUnitsMethod = SystemUnitsServiceGrpc.getSetSystemUnitsMethod) == null) {
          SystemUnitsServiceGrpc.getSetSystemUnitsMethod = getSetSystemUnitsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.SystemUnitsMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetSystemUnits"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.SystemUnitsMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetSystemUnitsMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static SystemUnitsServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SystemUnitsServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SystemUnitsServiceStub>() {
        @java.lang.Override
        public SystemUnitsServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SystemUnitsServiceStub(channel, callOptions);
        }
      };
    return SystemUnitsServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static SystemUnitsServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SystemUnitsServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SystemUnitsServiceBlockingStub>() {
        @java.lang.Override
        public SystemUnitsServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SystemUnitsServiceBlockingStub(channel, callOptions);
        }
      };
    return SystemUnitsServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static SystemUnitsServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<SystemUnitsServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<SystemUnitsServiceFutureStub>() {
        @java.lang.Override
        public SystemUnitsServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new SystemUnitsServiceFutureStub(channel, callOptions);
        }
      };
    return SystemUnitsServiceFutureStub.newStub(factory, channel);
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
    default void getSystemUnits(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSystemUnitsMethod(), responseObserver);
    }

    /**
     */
    default void systemUnitsChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSystemUnitsChangedMethod(), responseObserver);
    }

    /**
     */
    default void setSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetSystemUnitsMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service SystemUnitsService.
   */
  public static abstract class SystemUnitsServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return SystemUnitsServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service SystemUnitsService.
   */
  public static final class SystemUnitsServiceStub
      extends io.grpc.stub.AbstractAsyncStub<SystemUnitsServiceStub> {
    private SystemUnitsServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SystemUnitsServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SystemUnitsServiceStub(channel, callOptions);
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
    public void getSystemUnits(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSystemUnitsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void systemUnitsChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getSystemUnitsChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetSystemUnitsMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service SystemUnitsService.
   */
  public static final class SystemUnitsServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<SystemUnitsServiceBlockingStub> {
    private SystemUnitsServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SystemUnitsServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SystemUnitsServiceBlockingStub(channel, callOptions);
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
    public com.ifit.glassos.settings.SystemUnitsMessage getSystemUnits(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSystemUnitsMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.SystemUnitsMessage> systemUnitsChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getSystemUnitsChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetSystemUnitsMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service SystemUnitsService.
   */
  public static final class SystemUnitsServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<SystemUnitsServiceFutureStub> {
    private SystemUnitsServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected SystemUnitsServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new SystemUnitsServiceFutureStub(channel, callOptions);
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
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.SystemUnitsMessage> getSystemUnits(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSystemUnitsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setSystemUnits(
        com.ifit.glassos.settings.SystemUnitsMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetSystemUnitsMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE = 1;
  private static final int METHODID_GET_SYSTEM_UNITS = 2;
  private static final int METHODID_SYSTEM_UNITS_CHANGED = 3;
  private static final int METHODID_SET_SYSTEM_UNITS = 4;

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
        case METHODID_GET_SYSTEM_UNITS:
          serviceImpl.getSystemUnits((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsMessage>) responseObserver);
          break;
        case METHODID_SYSTEM_UNITS_CHANGED:
          serviceImpl.systemUnitsChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsMessage>) responseObserver);
          break;
        case METHODID_SET_SYSTEM_UNITS:
          serviceImpl.setSystemUnits((com.ifit.glassos.settings.SystemUnitsMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
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
          getGetSystemUnitsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.SystemUnitsMessage>(
                service, METHODID_GET_SYSTEM_UNITS)))
        .addMethod(
          getSystemUnitsChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.SystemUnitsMessage>(
                service, METHODID_SYSTEM_UNITS_CHANGED)))
        .addMethod(
          getSetSystemUnitsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.SystemUnitsMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_SYSTEM_UNITS)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (SystemUnitsServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetSystemUnitsMethod())
              .addMethod(getSystemUnitsChangedMethod())
              .addMethod(getSetSystemUnitsMethod())
              .build();
        }
      }
    }
    return result;
  }
}

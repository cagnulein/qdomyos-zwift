package com.ifit.glassos.console.update;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/update/FirmwareUpdateService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class FirmwareUpdateServiceGrpc {

  private FirmwareUpdateServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.FirmwareUpdateService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.update.FirmwareUpdateStatus> getGetFirmwareUpdateStatusMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetFirmwareUpdateStatus",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.update.FirmwareUpdateStatus.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.update.FirmwareUpdateStatus> getGetFirmwareUpdateStatusMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.update.FirmwareUpdateStatus> getGetFirmwareUpdateStatusMethod;
    if ((getGetFirmwareUpdateStatusMethod = FirmwareUpdateServiceGrpc.getGetFirmwareUpdateStatusMethod) == null) {
      synchronized (FirmwareUpdateServiceGrpc.class) {
        if ((getGetFirmwareUpdateStatusMethod = FirmwareUpdateServiceGrpc.getGetFirmwareUpdateStatusMethod) == null) {
          FirmwareUpdateServiceGrpc.getGetFirmwareUpdateStatusMethod = getGetFirmwareUpdateStatusMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.update.FirmwareUpdateStatus>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetFirmwareUpdateStatus"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.update.FirmwareUpdateStatus.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetFirmwareUpdateStatusMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.update.FirmwareUpdateStatus> getFirmwareUpdateStatusChangedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "FirmwareUpdateStatusChangedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.update.FirmwareUpdateStatus.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.update.FirmwareUpdateStatus> getFirmwareUpdateStatusChangedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.update.FirmwareUpdateStatus> getFirmwareUpdateStatusChangedSubscriptionMethod;
    if ((getFirmwareUpdateStatusChangedSubscriptionMethod = FirmwareUpdateServiceGrpc.getFirmwareUpdateStatusChangedSubscriptionMethod) == null) {
      synchronized (FirmwareUpdateServiceGrpc.class) {
        if ((getFirmwareUpdateStatusChangedSubscriptionMethod = FirmwareUpdateServiceGrpc.getFirmwareUpdateStatusChangedSubscriptionMethod) == null) {
          FirmwareUpdateServiceGrpc.getFirmwareUpdateStatusChangedSubscriptionMethod = getFirmwareUpdateStatusChangedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.update.FirmwareUpdateStatus>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "FirmwareUpdateStatusChangedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.update.FirmwareUpdateStatus.getDefaultInstance()))
              .build();
        }
      }
    }
    return getFirmwareUpdateStatusChangedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.update.FirmwareUpdateFile,
      com.ifit.glassos.console.update.FirmwareUpdateStatus> getStartFirmwareUpdateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StartFirmwareUpdate",
      requestType = com.ifit.glassos.console.update.FirmwareUpdateFile.class,
      responseType = com.ifit.glassos.console.update.FirmwareUpdateStatus.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.update.FirmwareUpdateFile,
      com.ifit.glassos.console.update.FirmwareUpdateStatus> getStartFirmwareUpdateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.update.FirmwareUpdateFile, com.ifit.glassos.console.update.FirmwareUpdateStatus> getStartFirmwareUpdateMethod;
    if ((getStartFirmwareUpdateMethod = FirmwareUpdateServiceGrpc.getStartFirmwareUpdateMethod) == null) {
      synchronized (FirmwareUpdateServiceGrpc.class) {
        if ((getStartFirmwareUpdateMethod = FirmwareUpdateServiceGrpc.getStartFirmwareUpdateMethod) == null) {
          FirmwareUpdateServiceGrpc.getStartFirmwareUpdateMethod = getStartFirmwareUpdateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.update.FirmwareUpdateFile, com.ifit.glassos.console.update.FirmwareUpdateStatus>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StartFirmwareUpdate"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.update.FirmwareUpdateFile.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.update.FirmwareUpdateStatus.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStartFirmwareUpdateMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static FirmwareUpdateServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FirmwareUpdateServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FirmwareUpdateServiceStub>() {
        @java.lang.Override
        public FirmwareUpdateServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FirmwareUpdateServiceStub(channel, callOptions);
        }
      };
    return FirmwareUpdateServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static FirmwareUpdateServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FirmwareUpdateServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FirmwareUpdateServiceBlockingStub>() {
        @java.lang.Override
        public FirmwareUpdateServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FirmwareUpdateServiceBlockingStub(channel, callOptions);
        }
      };
    return FirmwareUpdateServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static FirmwareUpdateServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FirmwareUpdateServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FirmwareUpdateServiceFutureStub>() {
        @java.lang.Override
        public FirmwareUpdateServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FirmwareUpdateServiceFutureStub(channel, callOptions);
        }
      };
    return FirmwareUpdateServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void getFirmwareUpdateStatus(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetFirmwareUpdateStatusMethod(), responseObserver);
    }

    /**
     */
    default void firmwareUpdateStatusChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getFirmwareUpdateStatusChangedSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void startFirmwareUpdate(com.ifit.glassos.console.update.FirmwareUpdateFile request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStartFirmwareUpdateMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service FirmwareUpdateService.
   */
  public static abstract class FirmwareUpdateServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return FirmwareUpdateServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service FirmwareUpdateService.
   */
  public static final class FirmwareUpdateServiceStub
      extends io.grpc.stub.AbstractAsyncStub<FirmwareUpdateServiceStub> {
    private FirmwareUpdateServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FirmwareUpdateServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FirmwareUpdateServiceStub(channel, callOptions);
    }

    /**
     */
    public void getFirmwareUpdateStatus(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetFirmwareUpdateStatusMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void firmwareUpdateStatusChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getFirmwareUpdateStatusChangedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void startFirmwareUpdate(com.ifit.glassos.console.update.FirmwareUpdateFile request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStartFirmwareUpdateMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service FirmwareUpdateService.
   */
  public static final class FirmwareUpdateServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<FirmwareUpdateServiceBlockingStub> {
    private FirmwareUpdateServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FirmwareUpdateServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FirmwareUpdateServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.console.update.FirmwareUpdateStatus getFirmwareUpdateStatus(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetFirmwareUpdateStatusMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.update.FirmwareUpdateStatus> firmwareUpdateStatusChangedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getFirmwareUpdateStatusChangedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.update.FirmwareUpdateStatus startFirmwareUpdate(com.ifit.glassos.console.update.FirmwareUpdateFile request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStartFirmwareUpdateMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service FirmwareUpdateService.
   */
  public static final class FirmwareUpdateServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<FirmwareUpdateServiceFutureStub> {
    private FirmwareUpdateServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FirmwareUpdateServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FirmwareUpdateServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.update.FirmwareUpdateStatus> getFirmwareUpdateStatus(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetFirmwareUpdateStatusMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.update.FirmwareUpdateStatus> startFirmwareUpdate(
        com.ifit.glassos.console.update.FirmwareUpdateFile request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStartFirmwareUpdateMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_FIRMWARE_UPDATE_STATUS = 0;
  private static final int METHODID_FIRMWARE_UPDATE_STATUS_CHANGED_SUBSCRIPTION = 1;
  private static final int METHODID_START_FIRMWARE_UPDATE = 2;

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
        case METHODID_GET_FIRMWARE_UPDATE_STATUS:
          serviceImpl.getFirmwareUpdateStatus((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus>) responseObserver);
          break;
        case METHODID_FIRMWARE_UPDATE_STATUS_CHANGED_SUBSCRIPTION:
          serviceImpl.firmwareUpdateStatusChangedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus>) responseObserver);
          break;
        case METHODID_START_FIRMWARE_UPDATE:
          serviceImpl.startFirmwareUpdate((com.ifit.glassos.console.update.FirmwareUpdateFile) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.update.FirmwareUpdateStatus>) responseObserver);
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
          getGetFirmwareUpdateStatusMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.update.FirmwareUpdateStatus>(
                service, METHODID_GET_FIRMWARE_UPDATE_STATUS)))
        .addMethod(
          getFirmwareUpdateStatusChangedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.update.FirmwareUpdateStatus>(
                service, METHODID_FIRMWARE_UPDATE_STATUS_CHANGED_SUBSCRIPTION)))
        .addMethod(
          getStartFirmwareUpdateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.update.FirmwareUpdateFile,
              com.ifit.glassos.console.update.FirmwareUpdateStatus>(
                service, METHODID_START_FIRMWARE_UPDATE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (FirmwareUpdateServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetFirmwareUpdateStatusMethod())
              .addMethod(getFirmwareUpdateStatusChangedSubscriptionMethod())
              .addMethod(getStartFirmwareUpdateMethod())
              .build();
        }
      }
    }
    return result;
  }
}

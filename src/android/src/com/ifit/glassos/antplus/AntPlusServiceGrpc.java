package com.ifit.glassos.antplus;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: antplus/AntPlusService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class AntPlusServiceGrpc {

  private AntPlusServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.AntPlusService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.antplus.AntPlusScanDurationMessage,
      com.ifit.glassos.util.Empty> getScanForDurationMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ScanForDuration",
      requestType = com.ifit.glassos.antplus.AntPlusScanDurationMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.antplus.AntPlusScanDurationMessage,
      com.ifit.glassos.util.Empty> getScanForDurationMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.antplus.AntPlusScanDurationMessage, com.ifit.glassos.util.Empty> getScanForDurationMethod;
    if ((getScanForDurationMethod = AntPlusServiceGrpc.getScanForDurationMethod) == null) {
      synchronized (AntPlusServiceGrpc.class) {
        if ((getScanForDurationMethod = AntPlusServiceGrpc.getScanForDurationMethod) == null) {
          AntPlusServiceGrpc.getScanForDurationMethod = getScanForDurationMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.antplus.AntPlusScanDurationMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ScanForDuration"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.antplus.AntPlusScanDurationMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getScanForDurationMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.antplus.AntPlusDeviceList> getFoundAntPlusDevicesSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "FoundAntPlusDevicesSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.antplus.AntPlusDeviceList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.antplus.AntPlusDeviceList> getFoundAntPlusDevicesSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.antplus.AntPlusDeviceList> getFoundAntPlusDevicesSubscriptionMethod;
    if ((getFoundAntPlusDevicesSubscriptionMethod = AntPlusServiceGrpc.getFoundAntPlusDevicesSubscriptionMethod) == null) {
      synchronized (AntPlusServiceGrpc.class) {
        if ((getFoundAntPlusDevicesSubscriptionMethod = AntPlusServiceGrpc.getFoundAntPlusDevicesSubscriptionMethod) == null) {
          AntPlusServiceGrpc.getFoundAntPlusDevicesSubscriptionMethod = getFoundAntPlusDevicesSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.antplus.AntPlusDeviceList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "FoundAntPlusDevicesSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.antplus.AntPlusDeviceList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getFoundAntPlusDevicesSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static AntPlusServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AntPlusServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AntPlusServiceStub>() {
        @java.lang.Override
        public AntPlusServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AntPlusServiceStub(channel, callOptions);
        }
      };
    return AntPlusServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static AntPlusServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AntPlusServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AntPlusServiceBlockingStub>() {
        @java.lang.Override
        public AntPlusServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AntPlusServiceBlockingStub(channel, callOptions);
        }
      };
    return AntPlusServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static AntPlusServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AntPlusServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AntPlusServiceFutureStub>() {
        @java.lang.Override
        public AntPlusServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AntPlusServiceFutureStub(channel, callOptions);
        }
      };
    return AntPlusServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void scanForDuration(com.ifit.glassos.antplus.AntPlusScanDurationMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getScanForDurationMethod(), responseObserver);
    }

    /**
     */
    default void foundAntPlusDevicesSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.antplus.AntPlusDeviceList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getFoundAntPlusDevicesSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service AntPlusService.
   */
  public static abstract class AntPlusServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return AntPlusServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service AntPlusService.
   */
  public static final class AntPlusServiceStub
      extends io.grpc.stub.AbstractAsyncStub<AntPlusServiceStub> {
    private AntPlusServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AntPlusServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AntPlusServiceStub(channel, callOptions);
    }

    /**
     */
    public void scanForDuration(com.ifit.glassos.antplus.AntPlusScanDurationMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getScanForDurationMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void foundAntPlusDevicesSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.antplus.AntPlusDeviceList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getFoundAntPlusDevicesSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service AntPlusService.
   */
  public static final class AntPlusServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<AntPlusServiceBlockingStub> {
    private AntPlusServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AntPlusServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AntPlusServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.Empty scanForDuration(com.ifit.glassos.antplus.AntPlusScanDurationMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getScanForDurationMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.antplus.AntPlusDeviceList> foundAntPlusDevicesSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getFoundAntPlusDevicesSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service AntPlusService.
   */
  public static final class AntPlusServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<AntPlusServiceFutureStub> {
    private AntPlusServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AntPlusServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AntPlusServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> scanForDuration(
        com.ifit.glassos.antplus.AntPlusScanDurationMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getScanForDurationMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_SCAN_FOR_DURATION = 0;
  private static final int METHODID_FOUND_ANT_PLUS_DEVICES_SUBSCRIPTION = 1;

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
        case METHODID_SCAN_FOR_DURATION:
          serviceImpl.scanForDuration((com.ifit.glassos.antplus.AntPlusScanDurationMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_FOUND_ANT_PLUS_DEVICES_SUBSCRIPTION:
          serviceImpl.foundAntPlusDevicesSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.antplus.AntPlusDeviceList>) responseObserver);
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
          getScanForDurationMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.antplus.AntPlusScanDurationMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SCAN_FOR_DURATION)))
        .addMethod(
          getFoundAntPlusDevicesSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.antplus.AntPlusDeviceList>(
                service, METHODID_FOUND_ANT_PLUS_DEVICES_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (AntPlusServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getScanForDurationMethod())
              .addMethod(getFoundAntPlusDevicesSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

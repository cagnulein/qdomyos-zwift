package com.ifit.glassos.console.virtualdmk;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/virtualdmk/VirtualDMKService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class VirtualDMKServiceGrpc {

  private VirtualDMKServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.VirtualDMKService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetDMKOverrideMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetDMKOverride",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getGetDMKOverrideMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getGetDMKOverrideMethod;
    if ((getGetDMKOverrideMethod = VirtualDMKServiceGrpc.getGetDMKOverrideMethod) == null) {
      synchronized (VirtualDMKServiceGrpc.class) {
        if ((getGetDMKOverrideMethod = VirtualDMKServiceGrpc.getGetDMKOverrideMethod) == null) {
          VirtualDMKServiceGrpc.getGetDMKOverrideMethod = getGetDMKOverrideMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetDMKOverride"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetDMKOverrideMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.IFitError> getSetDMKOverrideMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetDMKOverride",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.util.IFitError.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.IFitError> getSetDMKOverrideMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.IFitError> getSetDMKOverrideMethod;
    if ((getSetDMKOverrideMethod = VirtualDMKServiceGrpc.getSetDMKOverrideMethod) == null) {
      synchronized (VirtualDMKServiceGrpc.class) {
        if ((getSetDMKOverrideMethod = VirtualDMKServiceGrpc.getSetDMKOverrideMethod) == null) {
          VirtualDMKServiceGrpc.getSetDMKOverrideMethod = getSetDMKOverrideMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.IFitError>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetDMKOverride"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.IFitError.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetDMKOverrideMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static VirtualDMKServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VirtualDMKServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VirtualDMKServiceStub>() {
        @java.lang.Override
        public VirtualDMKServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VirtualDMKServiceStub(channel, callOptions);
        }
      };
    return VirtualDMKServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static VirtualDMKServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VirtualDMKServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VirtualDMKServiceBlockingStub>() {
        @java.lang.Override
        public VirtualDMKServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VirtualDMKServiceBlockingStub(channel, callOptions);
        }
      };
    return VirtualDMKServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static VirtualDMKServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VirtualDMKServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VirtualDMKServiceFutureStub>() {
        @java.lang.Override
        public VirtualDMKServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VirtualDMKServiceFutureStub(channel, callOptions);
        }
      };
    return VirtualDMKServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void getDMKOverride(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetDMKOverrideMethod(), responseObserver);
    }

    /**
     */
    default void setDMKOverride(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.IFitError> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetDMKOverrideMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service VirtualDMKService.
   */
  public static abstract class VirtualDMKServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return VirtualDMKServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service VirtualDMKService.
   */
  public static final class VirtualDMKServiceStub
      extends io.grpc.stub.AbstractAsyncStub<VirtualDMKServiceStub> {
    private VirtualDMKServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VirtualDMKServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VirtualDMKServiceStub(channel, callOptions);
    }

    /**
     */
    public void getDMKOverride(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetDMKOverrideMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setDMKOverride(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.IFitError> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetDMKOverrideMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service VirtualDMKService.
   */
  public static final class VirtualDMKServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<VirtualDMKServiceBlockingStub> {
    private VirtualDMKServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VirtualDMKServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VirtualDMKServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse getDMKOverride(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetDMKOverrideMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.IFitError setDMKOverride(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetDMKOverrideMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service VirtualDMKService.
   */
  public static final class VirtualDMKServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<VirtualDMKServiceFutureStub> {
    private VirtualDMKServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VirtualDMKServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VirtualDMKServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> getDMKOverride(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetDMKOverrideMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.IFitError> setDMKOverride(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetDMKOverrideMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_DMKOVERRIDE = 0;
  private static final int METHODID_SET_DMKOVERRIDE = 1;

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
        case METHODID_GET_DMKOVERRIDE:
          serviceImpl.getDMKOverride((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_SET_DMKOVERRIDE:
          serviceImpl.setDMKOverride((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.IFitError>) responseObserver);
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
          getGetDMKOverrideMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_GET_DMKOVERRIDE)))
        .addMethod(
          getSetDMKOverrideMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.util.IFitError>(
                service, METHODID_SET_DMKOVERRIDE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (VirtualDMKServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetDMKOverrideMethod())
              .addMethod(getSetDMKOverrideMethod())
              .build();
        }
      }
    }
    return result;
  }
}

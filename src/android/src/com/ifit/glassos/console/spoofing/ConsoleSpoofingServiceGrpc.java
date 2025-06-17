package com.ifit.glassos.console.spoofing;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/spoofing/ConsoleSpoofingService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class ConsoleSpoofingServiceGrpc {

  private ConsoleSpoofingServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.ConsoleSpoofingService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.IntRequest,
      com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getSetSpoofedPartNumberMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetSpoofedPartNumber",
      requestType = com.ifit.glassos.util.IntRequest.class,
      responseType = com.ifit.glassos.console.spoofing.SpoofPartNumberResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.IntRequest,
      com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getSetSpoofedPartNumberMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.IntRequest, com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getSetSpoofedPartNumberMethod;
    if ((getSetSpoofedPartNumberMethod = ConsoleSpoofingServiceGrpc.getSetSpoofedPartNumberMethod) == null) {
      synchronized (ConsoleSpoofingServiceGrpc.class) {
        if ((getSetSpoofedPartNumberMethod = ConsoleSpoofingServiceGrpc.getSetSpoofedPartNumberMethod) == null) {
          ConsoleSpoofingServiceGrpc.getSetSpoofedPartNumberMethod = getSetSpoofedPartNumberMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.IntRequest, com.ifit.glassos.console.spoofing.SpoofPartNumberResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetSpoofedPartNumber"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.IntRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.spoofing.SpoofPartNumberResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetSpoofedPartNumberMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getGetSpoofedPartNumberMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSpoofedPartNumber",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.spoofing.SpoofPartNumberResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getGetSpoofedPartNumberMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getGetSpoofedPartNumberMethod;
    if ((getGetSpoofedPartNumberMethod = ConsoleSpoofingServiceGrpc.getGetSpoofedPartNumberMethod) == null) {
      synchronized (ConsoleSpoofingServiceGrpc.class) {
        if ((getGetSpoofedPartNumberMethod = ConsoleSpoofingServiceGrpc.getGetSpoofedPartNumberMethod) == null) {
          ConsoleSpoofingServiceGrpc.getGetSpoofedPartNumberMethod = getGetSpoofedPartNumberMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.spoofing.SpoofPartNumberResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSpoofedPartNumber"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.spoofing.SpoofPartNumberResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSpoofedPartNumberMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getClearSpoofedPartNumberMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ClearSpoofedPartNumber",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.spoofing.SpoofPartNumberResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getClearSpoofedPartNumberMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getClearSpoofedPartNumberMethod;
    if ((getClearSpoofedPartNumberMethod = ConsoleSpoofingServiceGrpc.getClearSpoofedPartNumberMethod) == null) {
      synchronized (ConsoleSpoofingServiceGrpc.class) {
        if ((getClearSpoofedPartNumberMethod = ConsoleSpoofingServiceGrpc.getClearSpoofedPartNumberMethod) == null) {
          ConsoleSpoofingServiceGrpc.getClearSpoofedPartNumberMethod = getClearSpoofedPartNumberMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.spoofing.SpoofPartNumberResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ClearSpoofedPartNumber"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.spoofing.SpoofPartNumberResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getClearSpoofedPartNumberMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static ConsoleSpoofingServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ConsoleSpoofingServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ConsoleSpoofingServiceStub>() {
        @java.lang.Override
        public ConsoleSpoofingServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ConsoleSpoofingServiceStub(channel, callOptions);
        }
      };
    return ConsoleSpoofingServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static ConsoleSpoofingServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ConsoleSpoofingServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ConsoleSpoofingServiceBlockingStub>() {
        @java.lang.Override
        public ConsoleSpoofingServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ConsoleSpoofingServiceBlockingStub(channel, callOptions);
        }
      };
    return ConsoleSpoofingServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static ConsoleSpoofingServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<ConsoleSpoofingServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<ConsoleSpoofingServiceFutureStub>() {
        @java.lang.Override
        public ConsoleSpoofingServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new ConsoleSpoofingServiceFutureStub(channel, callOptions);
        }
      };
    return ConsoleSpoofingServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void setSpoofedPartNumber(com.ifit.glassos.util.IntRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetSpoofedPartNumberMethod(), responseObserver);
    }

    /**
     */
    default void getSpoofedPartNumber(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSpoofedPartNumberMethod(), responseObserver);
    }

    /**
     */
    default void clearSpoofedPartNumber(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getClearSpoofedPartNumberMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service ConsoleSpoofingService.
   */
  public static abstract class ConsoleSpoofingServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return ConsoleSpoofingServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service ConsoleSpoofingService.
   */
  public static final class ConsoleSpoofingServiceStub
      extends io.grpc.stub.AbstractAsyncStub<ConsoleSpoofingServiceStub> {
    private ConsoleSpoofingServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ConsoleSpoofingServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ConsoleSpoofingServiceStub(channel, callOptions);
    }

    /**
     */
    public void setSpoofedPartNumber(com.ifit.glassos.util.IntRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetSpoofedPartNumberMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getSpoofedPartNumber(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSpoofedPartNumberMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void clearSpoofedPartNumber(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getClearSpoofedPartNumberMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service ConsoleSpoofingService.
   */
  public static final class ConsoleSpoofingServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<ConsoleSpoofingServiceBlockingStub> {
    private ConsoleSpoofingServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ConsoleSpoofingServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ConsoleSpoofingServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.console.spoofing.SpoofPartNumberResult setSpoofedPartNumber(com.ifit.glassos.util.IntRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetSpoofedPartNumberMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.spoofing.SpoofPartNumberResult getSpoofedPartNumber(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSpoofedPartNumberMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.spoofing.SpoofPartNumberResult clearSpoofedPartNumber(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getClearSpoofedPartNumberMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service ConsoleSpoofingService.
   */
  public static final class ConsoleSpoofingServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<ConsoleSpoofingServiceFutureStub> {
    private ConsoleSpoofingServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected ConsoleSpoofingServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new ConsoleSpoofingServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> setSpoofedPartNumber(
        com.ifit.glassos.util.IntRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetSpoofedPartNumberMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> getSpoofedPartNumber(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSpoofedPartNumberMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.spoofing.SpoofPartNumberResult> clearSpoofedPartNumber(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getClearSpoofedPartNumberMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_SET_SPOOFED_PART_NUMBER = 0;
  private static final int METHODID_GET_SPOOFED_PART_NUMBER = 1;
  private static final int METHODID_CLEAR_SPOOFED_PART_NUMBER = 2;

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
        case METHODID_SET_SPOOFED_PART_NUMBER:
          serviceImpl.setSpoofedPartNumber((com.ifit.glassos.util.IntRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult>) responseObserver);
          break;
        case METHODID_GET_SPOOFED_PART_NUMBER:
          serviceImpl.getSpoofedPartNumber((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult>) responseObserver);
          break;
        case METHODID_CLEAR_SPOOFED_PART_NUMBER:
          serviceImpl.clearSpoofedPartNumber((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.spoofing.SpoofPartNumberResult>) responseObserver);
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
          getSetSpoofedPartNumberMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.IntRequest,
              com.ifit.glassos.console.spoofing.SpoofPartNumberResult>(
                service, METHODID_SET_SPOOFED_PART_NUMBER)))
        .addMethod(
          getGetSpoofedPartNumberMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.spoofing.SpoofPartNumberResult>(
                service, METHODID_GET_SPOOFED_PART_NUMBER)))
        .addMethod(
          getClearSpoofedPartNumberMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.spoofing.SpoofPartNumberResult>(
                service, METHODID_CLEAR_SPOOFED_PART_NUMBER)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (ConsoleSpoofingServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getSetSpoofedPartNumberMethod())
              .addMethod(getGetSpoofedPartNumberMethod())
              .addMethod(getClearSpoofedPartNumberMethod())
              .build();
        }
      }
    }
    return result;
  }
}

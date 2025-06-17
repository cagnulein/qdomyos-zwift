package com.ifit.glassos.featuregates;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: featuregates/FeatureGateService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class FeatureGateServiceGrpc {

  private FeatureGateServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.FeatureGateService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.featuregates.GatedFeatureMessage,
      com.ifit.glassos.featuregates.FeatureGateResult> getCheckFeatureGateStatusMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CheckFeatureGateStatus",
      requestType = com.ifit.glassos.featuregates.GatedFeatureMessage.class,
      responseType = com.ifit.glassos.featuregates.FeatureGateResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.featuregates.GatedFeatureMessage,
      com.ifit.glassos.featuregates.FeatureGateResult> getCheckFeatureGateStatusMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.featuregates.GatedFeatureMessage, com.ifit.glassos.featuregates.FeatureGateResult> getCheckFeatureGateStatusMethod;
    if ((getCheckFeatureGateStatusMethod = FeatureGateServiceGrpc.getCheckFeatureGateStatusMethod) == null) {
      synchronized (FeatureGateServiceGrpc.class) {
        if ((getCheckFeatureGateStatusMethod = FeatureGateServiceGrpc.getCheckFeatureGateStatusMethod) == null) {
          FeatureGateServiceGrpc.getCheckFeatureGateStatusMethod = getCheckFeatureGateStatusMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.featuregates.GatedFeatureMessage, com.ifit.glassos.featuregates.FeatureGateResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CheckFeatureGateStatus"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.featuregates.GatedFeatureMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.featuregates.FeatureGateResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCheckFeatureGateStatusMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.featuregates.GatedFeatureMessage,
      com.ifit.glassos.featuregates.FeatureGateRule> getGetFeatureGateRuleMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetFeatureGateRule",
      requestType = com.ifit.glassos.featuregates.GatedFeatureMessage.class,
      responseType = com.ifit.glassos.featuregates.FeatureGateRule.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.featuregates.GatedFeatureMessage,
      com.ifit.glassos.featuregates.FeatureGateRule> getGetFeatureGateRuleMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.featuregates.GatedFeatureMessage, com.ifit.glassos.featuregates.FeatureGateRule> getGetFeatureGateRuleMethod;
    if ((getGetFeatureGateRuleMethod = FeatureGateServiceGrpc.getGetFeatureGateRuleMethod) == null) {
      synchronized (FeatureGateServiceGrpc.class) {
        if ((getGetFeatureGateRuleMethod = FeatureGateServiceGrpc.getGetFeatureGateRuleMethod) == null) {
          FeatureGateServiceGrpc.getGetFeatureGateRuleMethod = getGetFeatureGateRuleMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.featuregates.GatedFeatureMessage, com.ifit.glassos.featuregates.FeatureGateRule>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetFeatureGateRule"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.featuregates.GatedFeatureMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.featuregates.FeatureGateRule.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetFeatureGateRuleMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static FeatureGateServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FeatureGateServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FeatureGateServiceStub>() {
        @java.lang.Override
        public FeatureGateServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FeatureGateServiceStub(channel, callOptions);
        }
      };
    return FeatureGateServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static FeatureGateServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FeatureGateServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FeatureGateServiceBlockingStub>() {
        @java.lang.Override
        public FeatureGateServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FeatureGateServiceBlockingStub(channel, callOptions);
        }
      };
    return FeatureGateServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static FeatureGateServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<FeatureGateServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<FeatureGateServiceFutureStub>() {
        @java.lang.Override
        public FeatureGateServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new FeatureGateServiceFutureStub(channel, callOptions);
        }
      };
    return FeatureGateServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void checkFeatureGateStatus(com.ifit.glassos.featuregates.GatedFeatureMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.featuregates.FeatureGateResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCheckFeatureGateStatusMethod(), responseObserver);
    }

    /**
     */
    default void getFeatureGateRule(com.ifit.glassos.featuregates.GatedFeatureMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.featuregates.FeatureGateRule> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetFeatureGateRuleMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service FeatureGateService.
   */
  public static abstract class FeatureGateServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return FeatureGateServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service FeatureGateService.
   */
  public static final class FeatureGateServiceStub
      extends io.grpc.stub.AbstractAsyncStub<FeatureGateServiceStub> {
    private FeatureGateServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FeatureGateServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FeatureGateServiceStub(channel, callOptions);
    }

    /**
     */
    public void checkFeatureGateStatus(com.ifit.glassos.featuregates.GatedFeatureMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.featuregates.FeatureGateResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getCheckFeatureGateStatusMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getFeatureGateRule(com.ifit.glassos.featuregates.GatedFeatureMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.featuregates.FeatureGateRule> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetFeatureGateRuleMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service FeatureGateService.
   */
  public static final class FeatureGateServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<FeatureGateServiceBlockingStub> {
    private FeatureGateServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FeatureGateServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FeatureGateServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.featuregates.FeatureGateResult checkFeatureGateStatus(com.ifit.glassos.featuregates.GatedFeatureMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getCheckFeatureGateStatusMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.featuregates.FeatureGateRule getFeatureGateRule(com.ifit.glassos.featuregates.GatedFeatureMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetFeatureGateRuleMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service FeatureGateService.
   */
  public static final class FeatureGateServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<FeatureGateServiceFutureStub> {
    private FeatureGateServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected FeatureGateServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new FeatureGateServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.featuregates.FeatureGateResult> checkFeatureGateStatus(
        com.ifit.glassos.featuregates.GatedFeatureMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getCheckFeatureGateStatusMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.featuregates.FeatureGateRule> getFeatureGateRule(
        com.ifit.glassos.featuregates.GatedFeatureMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetFeatureGateRuleMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CHECK_FEATURE_GATE_STATUS = 0;
  private static final int METHODID_GET_FEATURE_GATE_RULE = 1;

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
        case METHODID_CHECK_FEATURE_GATE_STATUS:
          serviceImpl.checkFeatureGateStatus((com.ifit.glassos.featuregates.GatedFeatureMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.featuregates.FeatureGateResult>) responseObserver);
          break;
        case METHODID_GET_FEATURE_GATE_RULE:
          serviceImpl.getFeatureGateRule((com.ifit.glassos.featuregates.GatedFeatureMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.featuregates.FeatureGateRule>) responseObserver);
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
          getCheckFeatureGateStatusMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.featuregates.GatedFeatureMessage,
              com.ifit.glassos.featuregates.FeatureGateResult>(
                service, METHODID_CHECK_FEATURE_GATE_STATUS)))
        .addMethod(
          getGetFeatureGateRuleMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.featuregates.GatedFeatureMessage,
              com.ifit.glassos.featuregates.FeatureGateRule>(
                service, METHODID_GET_FEATURE_GATE_RULE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (FeatureGateServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCheckFeatureGateStatusMethod())
              .addMethod(getGetFeatureGateRuleMethod())
              .build();
        }
      }
    }
    return result;
  }
}

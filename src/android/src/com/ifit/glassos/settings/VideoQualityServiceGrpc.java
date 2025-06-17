package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/VideoQualityService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class VideoQualityServiceGrpc {

  private VideoQualityServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.VideoQualityService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.VideoQuality,
      com.ifit.glassos.settings.VideoQualityResult> getSetVideoQualityMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetVideoQuality",
      requestType = com.ifit.glassos.settings.VideoQuality.class,
      responseType = com.ifit.glassos.settings.VideoQualityResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.VideoQuality,
      com.ifit.glassos.settings.VideoQualityResult> getSetVideoQualityMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.VideoQuality, com.ifit.glassos.settings.VideoQualityResult> getSetVideoQualityMethod;
    if ((getSetVideoQualityMethod = VideoQualityServiceGrpc.getSetVideoQualityMethod) == null) {
      synchronized (VideoQualityServiceGrpc.class) {
        if ((getSetVideoQualityMethod = VideoQualityServiceGrpc.getSetVideoQualityMethod) == null) {
          VideoQualityServiceGrpc.getSetVideoQualityMethod = getSetVideoQualityMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.VideoQuality, com.ifit.glassos.settings.VideoQualityResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetVideoQuality"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VideoQuality.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VideoQualityResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetVideoQualityMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VideoQualityResult> getGetVideoQualityMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetVideoQuality",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.VideoQualityResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VideoQualityResult> getGetVideoQualityMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VideoQualityResult> getGetVideoQualityMethod;
    if ((getGetVideoQualityMethod = VideoQualityServiceGrpc.getGetVideoQualityMethod) == null) {
      synchronized (VideoQualityServiceGrpc.class) {
        if ((getGetVideoQualityMethod = VideoQualityServiceGrpc.getGetVideoQualityMethod) == null) {
          VideoQualityServiceGrpc.getGetVideoQualityMethod = getGetVideoQualityMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VideoQualityResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetVideoQuality"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VideoQualityResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetVideoQualityMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VideoQuality> getVideoQualityChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "VideoQualityChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.VideoQuality.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VideoQuality> getVideoQualityChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VideoQuality> getVideoQualityChangedMethod;
    if ((getVideoQualityChangedMethod = VideoQualityServiceGrpc.getVideoQualityChangedMethod) == null) {
      synchronized (VideoQualityServiceGrpc.class) {
        if ((getVideoQualityChangedMethod = VideoQualityServiceGrpc.getVideoQualityChangedMethod) == null) {
          VideoQualityServiceGrpc.getVideoQualityChangedMethod = getVideoQualityChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VideoQuality>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "VideoQualityChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VideoQuality.getDefaultInstance()))
              .build();
        }
      }
    }
    return getVideoQualityChangedMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static VideoQualityServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VideoQualityServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VideoQualityServiceStub>() {
        @java.lang.Override
        public VideoQualityServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VideoQualityServiceStub(channel, callOptions);
        }
      };
    return VideoQualityServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static VideoQualityServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VideoQualityServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VideoQualityServiceBlockingStub>() {
        @java.lang.Override
        public VideoQualityServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VideoQualityServiceBlockingStub(channel, callOptions);
        }
      };
    return VideoQualityServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static VideoQualityServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VideoQualityServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VideoQualityServiceFutureStub>() {
        @java.lang.Override
        public VideoQualityServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VideoQualityServiceFutureStub(channel, callOptions);
        }
      };
    return VideoQualityServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void setVideoQuality(com.ifit.glassos.settings.VideoQuality request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQualityResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetVideoQualityMethod(), responseObserver);
    }

    /**
     */
    default void getVideoQuality(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQualityResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetVideoQualityMethod(), responseObserver);
    }

    /**
     */
    default void videoQualityChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQuality> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getVideoQualityChangedMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service VideoQualityService.
   */
  public static abstract class VideoQualityServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return VideoQualityServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service VideoQualityService.
   */
  public static final class VideoQualityServiceStub
      extends io.grpc.stub.AbstractAsyncStub<VideoQualityServiceStub> {
    private VideoQualityServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VideoQualityServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VideoQualityServiceStub(channel, callOptions);
    }

    /**
     */
    public void setVideoQuality(com.ifit.glassos.settings.VideoQuality request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQualityResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetVideoQualityMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getVideoQuality(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQualityResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetVideoQualityMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void videoQualityChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQuality> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getVideoQualityChangedMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service VideoQualityService.
   */
  public static final class VideoQualityServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<VideoQualityServiceBlockingStub> {
    private VideoQualityServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VideoQualityServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VideoQualityServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.settings.VideoQualityResult setVideoQuality(com.ifit.glassos.settings.VideoQuality request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetVideoQualityMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.VideoQualityResult getVideoQuality(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetVideoQualityMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.VideoQuality> videoQualityChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getVideoQualityChangedMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service VideoQualityService.
   */
  public static final class VideoQualityServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<VideoQualityServiceFutureStub> {
    private VideoQualityServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VideoQualityServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VideoQualityServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.VideoQualityResult> setVideoQuality(
        com.ifit.glassos.settings.VideoQuality request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetVideoQualityMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.VideoQualityResult> getVideoQuality(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetVideoQualityMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_SET_VIDEO_QUALITY = 0;
  private static final int METHODID_GET_VIDEO_QUALITY = 1;
  private static final int METHODID_VIDEO_QUALITY_CHANGED = 2;

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
        case METHODID_SET_VIDEO_QUALITY:
          serviceImpl.setVideoQuality((com.ifit.glassos.settings.VideoQuality) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQualityResult>) responseObserver);
          break;
        case METHODID_GET_VIDEO_QUALITY:
          serviceImpl.getVideoQuality((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQualityResult>) responseObserver);
          break;
        case METHODID_VIDEO_QUALITY_CHANGED:
          serviceImpl.videoQualityChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VideoQuality>) responseObserver);
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
          getSetVideoQualityMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.VideoQuality,
              com.ifit.glassos.settings.VideoQualityResult>(
                service, METHODID_SET_VIDEO_QUALITY)))
        .addMethod(
          getGetVideoQualityMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.VideoQualityResult>(
                service, METHODID_GET_VIDEO_QUALITY)))
        .addMethod(
          getVideoQualityChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.VideoQuality>(
                service, METHODID_VIDEO_QUALITY_CHANGED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (VideoQualityServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getSetVideoQualityMethod())
              .addMethod(getGetVideoQualityMethod())
              .addMethod(getVideoQualityChangedMethod())
              .build();
        }
      }
    }
    return result;
  }
}

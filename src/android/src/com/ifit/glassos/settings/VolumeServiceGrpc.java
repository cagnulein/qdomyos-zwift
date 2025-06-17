package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/VolumeService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class VolumeServiceGrpc {

  private VolumeServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.VolumeService";

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
    if ((getCanReadMethod = VolumeServiceGrpc.getCanReadMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getCanReadMethod = VolumeServiceGrpc.getCanReadMethod) == null) {
          VolumeServiceGrpc.getCanReadMethod = getCanReadMethod =
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
    if ((getCanWriteMethod = VolumeServiceGrpc.getCanWriteMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getCanWriteMethod = VolumeServiceGrpc.getCanWriteMethod) == null) {
          VolumeServiceGrpc.getCanWriteMethod = getCanWriteMethod =
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
      com.ifit.glassos.settings.VolumeMessage> getGetVolumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetVolume",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.VolumeMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VolumeMessage> getGetVolumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VolumeMessage> getGetVolumeMethod;
    if ((getGetVolumeMethod = VolumeServiceGrpc.getGetVolumeMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getGetVolumeMethod = VolumeServiceGrpc.getGetVolumeMethod) == null) {
          VolumeServiceGrpc.getGetVolumeMethod = getGetVolumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VolumeMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetVolume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VolumeMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetVolumeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getIncreaseVolumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IncreaseVolume",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getIncreaseVolumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getIncreaseVolumeMethod;
    if ((getIncreaseVolumeMethod = VolumeServiceGrpc.getIncreaseVolumeMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getIncreaseVolumeMethod = VolumeServiceGrpc.getIncreaseVolumeMethod) == null) {
          VolumeServiceGrpc.getIncreaseVolumeMethod = getIncreaseVolumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IncreaseVolume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIncreaseVolumeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getDecreaseVolumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "DecreaseVolume",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getDecreaseVolumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getDecreaseVolumeMethod;
    if ((getDecreaseVolumeMethod = VolumeServiceGrpc.getDecreaseVolumeMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getDecreaseVolumeMethod = VolumeServiceGrpc.getDecreaseVolumeMethod) == null) {
          VolumeServiceGrpc.getDecreaseVolumeMethod = getDecreaseVolumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "DecreaseVolume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDecreaseVolumeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VolumeMessage> getVolumeChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "VolumeChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.VolumeMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.VolumeMessage> getVolumeChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VolumeMessage> getVolumeChangedMethod;
    if ((getVolumeChangedMethod = VolumeServiceGrpc.getVolumeChangedMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getVolumeChangedMethod = VolumeServiceGrpc.getVolumeChangedMethod) == null) {
          VolumeServiceGrpc.getVolumeChangedMethod = getVolumeChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.VolumeMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "VolumeChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VolumeMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getVolumeChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.VolumeMessage,
      com.ifit.glassos.util.Empty> getSetVolumeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetVolume",
      requestType = com.ifit.glassos.settings.VolumeMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.VolumeMessage,
      com.ifit.glassos.util.Empty> getSetVolumeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.VolumeMessage, com.ifit.glassos.util.Empty> getSetVolumeMethod;
    if ((getSetVolumeMethod = VolumeServiceGrpc.getSetVolumeMethod) == null) {
      synchronized (VolumeServiceGrpc.class) {
        if ((getSetVolumeMethod = VolumeServiceGrpc.getSetVolumeMethod) == null) {
          VolumeServiceGrpc.getSetVolumeMethod = getSetVolumeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.VolumeMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetVolume"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.VolumeMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetVolumeMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static VolumeServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VolumeServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VolumeServiceStub>() {
        @java.lang.Override
        public VolumeServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VolumeServiceStub(channel, callOptions);
        }
      };
    return VolumeServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static VolumeServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VolumeServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VolumeServiceBlockingStub>() {
        @java.lang.Override
        public VolumeServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VolumeServiceBlockingStub(channel, callOptions);
        }
      };
    return VolumeServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static VolumeServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<VolumeServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<VolumeServiceFutureStub>() {
        @java.lang.Override
        public VolumeServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new VolumeServiceFutureStub(channel, callOptions);
        }
      };
    return VolumeServiceFutureStub.newStub(factory, channel);
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
    default void getVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VolumeMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetVolumeMethod(), responseObserver);
    }

    /**
     */
    default void increaseVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIncreaseVolumeMethod(), responseObserver);
    }

    /**
     */
    default void decreaseVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDecreaseVolumeMethod(), responseObserver);
    }

    /**
     */
    default void volumeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VolumeMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getVolumeChangedMethod(), responseObserver);
    }

    /**
     */
    default void setVolume(com.ifit.glassos.settings.VolumeMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetVolumeMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service VolumeService.
   */
  public static abstract class VolumeServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return VolumeServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service VolumeService.
   */
  public static final class VolumeServiceStub
      extends io.grpc.stub.AbstractAsyncStub<VolumeServiceStub> {
    private VolumeServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VolumeServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VolumeServiceStub(channel, callOptions);
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
    public void getVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VolumeMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetVolumeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void increaseVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIncreaseVolumeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void decreaseVolume(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getDecreaseVolumeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void volumeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VolumeMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getVolumeChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setVolume(com.ifit.glassos.settings.VolumeMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetVolumeMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service VolumeService.
   */
  public static final class VolumeServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<VolumeServiceBlockingStub> {
    private VolumeServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VolumeServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VolumeServiceBlockingStub(channel, callOptions);
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
    public com.ifit.glassos.settings.VolumeMessage getVolume(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetVolumeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty increaseVolume(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIncreaseVolumeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty decreaseVolume(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getDecreaseVolumeMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.VolumeMessage> volumeChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getVolumeChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setVolume(com.ifit.glassos.settings.VolumeMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetVolumeMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service VolumeService.
   */
  public static final class VolumeServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<VolumeServiceFutureStub> {
    private VolumeServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected VolumeServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new VolumeServiceFutureStub(channel, callOptions);
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
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.VolumeMessage> getVolume(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetVolumeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> increaseVolume(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIncreaseVolumeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> decreaseVolume(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getDecreaseVolumeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setVolume(
        com.ifit.glassos.settings.VolumeMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetVolumeMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CAN_READ = 0;
  private static final int METHODID_CAN_WRITE = 1;
  private static final int METHODID_GET_VOLUME = 2;
  private static final int METHODID_INCREASE_VOLUME = 3;
  private static final int METHODID_DECREASE_VOLUME = 4;
  private static final int METHODID_VOLUME_CHANGED = 5;
  private static final int METHODID_SET_VOLUME = 6;

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
        case METHODID_GET_VOLUME:
          serviceImpl.getVolume((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VolumeMessage>) responseObserver);
          break;
        case METHODID_INCREASE_VOLUME:
          serviceImpl.increaseVolume((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_DECREASE_VOLUME:
          serviceImpl.decreaseVolume((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_VOLUME_CHANGED:
          serviceImpl.volumeChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.VolumeMessage>) responseObserver);
          break;
        case METHODID_SET_VOLUME:
          serviceImpl.setVolume((com.ifit.glassos.settings.VolumeMessage) request,
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
          getGetVolumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.VolumeMessage>(
                service, METHODID_GET_VOLUME)))
        .addMethod(
          getIncreaseVolumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_INCREASE_VOLUME)))
        .addMethod(
          getDecreaseVolumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_DECREASE_VOLUME)))
        .addMethod(
          getVolumeChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.VolumeMessage>(
                service, METHODID_VOLUME_CHANGED)))
        .addMethod(
          getSetVolumeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.VolumeMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_VOLUME)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (VolumeServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getCanReadMethod())
              .addMethod(getCanWriteMethod())
              .addMethod(getGetVolumeMethod())
              .addMethod(getIncreaseVolumeMethod())
              .addMethod(getDecreaseVolumeMethod())
              .addMethod(getVolumeChangedMethod())
              .addMethod(getSetVolumeMethod())
              .build();
        }
      }
    }
    return result;
  }
}

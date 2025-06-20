package com.ifit.glassos.settings.hdmi;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/hdmi/HdmiSoundService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class HdmiSoundServiceGrpc {

  private HdmiSoundServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.HdmiSoundService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getIsEnabledMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsEnabled",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getIsEnabledMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getIsEnabledMethod;
    if ((getIsEnabledMethod = HdmiSoundServiceGrpc.getIsEnabledMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getIsEnabledMethod = HdmiSoundServiceGrpc.getIsEnabledMethod) == null) {
          HdmiSoundServiceGrpc.getIsEnabledMethod = getIsEnabledMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsEnabled"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsEnabledMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getIsEnabledFlowMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsEnabledFlow",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.AvailabilityResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.AvailabilityResponse> getIsEnabledFlowMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse> getIsEnabledFlowMethod;
    if ((getIsEnabledFlowMethod = HdmiSoundServiceGrpc.getIsEnabledFlowMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getIsEnabledFlowMethod = HdmiSoundServiceGrpc.getIsEnabledFlowMethod) == null) {
          HdmiSoundServiceGrpc.getIsEnabledFlowMethod = getIsEnabledFlowMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.AvailabilityResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsEnabledFlow"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.AvailabilityResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsEnabledFlowMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getSetMuteMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetMute",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getSetMuteMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty> getSetMuteMethod;
    if ((getSetMuteMethod = HdmiSoundServiceGrpc.getSetMuteMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getSetMuteMethod = HdmiSoundServiceGrpc.getSetMuteMethod) == null) {
          HdmiSoundServiceGrpc.getSetMuteMethod = getSetMuteMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetMute"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetMuteMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.hdmi.AqProfileMessage,
      com.ifit.glassos.util.Empty> getSetAqProfileMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetAqProfile",
      requestType = com.ifit.glassos.settings.hdmi.AqProfileMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.hdmi.AqProfileMessage,
      com.ifit.glassos.util.Empty> getSetAqProfileMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.hdmi.AqProfileMessage, com.ifit.glassos.util.Empty> getSetAqProfileMethod;
    if ((getSetAqProfileMethod = HdmiSoundServiceGrpc.getSetAqProfileMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getSetAqProfileMethod = HdmiSoundServiceGrpc.getSetAqProfileMethod) == null) {
          HdmiSoundServiceGrpc.getSetAqProfileMethod = getSetAqProfileMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.hdmi.AqProfileMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetAqProfile"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.hdmi.AqProfileMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetAqProfileMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.hdmi.AqLevelsMessage,
      com.ifit.glassos.util.Empty> getSetAqLevelsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetAqLevels",
      requestType = com.ifit.glassos.settings.hdmi.AqLevelsMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.hdmi.AqLevelsMessage,
      com.ifit.glassos.util.Empty> getSetAqLevelsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.hdmi.AqLevelsMessage, com.ifit.glassos.util.Empty> getSetAqLevelsMethod;
    if ((getSetAqLevelsMethod = HdmiSoundServiceGrpc.getSetAqLevelsMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getSetAqLevelsMethod = HdmiSoundServiceGrpc.getSetAqLevelsMethod) == null) {
          HdmiSoundServiceGrpc.getSetAqLevelsMethod = getSetAqLevelsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.hdmi.AqLevelsMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetAqLevels"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.hdmi.AqLevelsMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetAqLevelsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getGetHdmiSoundStateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetHdmiSoundState",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getGetHdmiSoundStateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getGetHdmiSoundStateMethod;
    if ((getGetHdmiSoundStateMethod = HdmiSoundServiceGrpc.getGetHdmiSoundStateMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getGetHdmiSoundStateMethod = HdmiSoundServiceGrpc.getGetHdmiSoundStateMethod) == null) {
          HdmiSoundServiceGrpc.getGetHdmiSoundStateMethod = getGetHdmiSoundStateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetHdmiSoundState"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetHdmiSoundStateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getHdmiSoundStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "HdmiSoundStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getHdmiSoundStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getHdmiSoundStateChangedMethod;
    if ((getHdmiSoundStateChangedMethod = HdmiSoundServiceGrpc.getHdmiSoundStateChangedMethod) == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        if ((getHdmiSoundStateChangedMethod = HdmiSoundServiceGrpc.getHdmiSoundStateChangedMethod) == null) {
          HdmiSoundServiceGrpc.getHdmiSoundStateChangedMethod = getHdmiSoundStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "HdmiSoundStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getHdmiSoundStateChangedMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static HdmiSoundServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HdmiSoundServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HdmiSoundServiceStub>() {
        @java.lang.Override
        public HdmiSoundServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HdmiSoundServiceStub(channel, callOptions);
        }
      };
    return HdmiSoundServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static HdmiSoundServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HdmiSoundServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HdmiSoundServiceBlockingStub>() {
        @java.lang.Override
        public HdmiSoundServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HdmiSoundServiceBlockingStub(channel, callOptions);
        }
      };
    return HdmiSoundServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static HdmiSoundServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<HdmiSoundServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<HdmiSoundServiceFutureStub>() {
        @java.lang.Override
        public HdmiSoundServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new HdmiSoundServiceFutureStub(channel, callOptions);
        }
      };
    return HdmiSoundServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void isEnabled(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsEnabledMethod(), responseObserver);
    }

    /**
     */
    default void isEnabledFlow(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsEnabledFlowMethod(), responseObserver);
    }

    /**
     */
    default void setMute(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetMuteMethod(), responseObserver);
    }

    /**
     */
    default void setAqProfile(com.ifit.glassos.settings.hdmi.AqProfileMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetAqProfileMethod(), responseObserver);
    }

    /**
     */
    default void setAqLevels(com.ifit.glassos.settings.hdmi.AqLevelsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetAqLevelsMethod(), responseObserver);
    }

    /**
     */
    default void getHdmiSoundState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetHdmiSoundStateMethod(), responseObserver);
    }

    /**
     */
    default void hdmiSoundStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getHdmiSoundStateChangedMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service HdmiSoundService.
   */
  public static abstract class HdmiSoundServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return HdmiSoundServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service HdmiSoundService.
   */
  public static final class HdmiSoundServiceStub
      extends io.grpc.stub.AbstractAsyncStub<HdmiSoundServiceStub> {
    private HdmiSoundServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HdmiSoundServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HdmiSoundServiceStub(channel, callOptions);
    }

    /**
     */
    public void isEnabled(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsEnabledMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isEnabledFlow(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getIsEnabledFlowMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setMute(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetMuteMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setAqProfile(com.ifit.glassos.settings.hdmi.AqProfileMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetAqProfileMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setAqLevels(com.ifit.glassos.settings.hdmi.AqLevelsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetAqLevelsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getHdmiSoundState(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetHdmiSoundStateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void hdmiSoundStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getHdmiSoundStateChangedMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service HdmiSoundService.
   */
  public static final class HdmiSoundServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<HdmiSoundServiceBlockingStub> {
    private HdmiSoundServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HdmiSoundServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HdmiSoundServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.AvailabilityResponse isEnabled(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsEnabledMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.AvailabilityResponse> isEnabledFlow(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getIsEnabledFlowMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setMute(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetMuteMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setAqProfile(com.ifit.glassos.settings.hdmi.AqProfileMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetAqProfileMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setAqLevels(com.ifit.glassos.settings.hdmi.AqLevelsMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetAqLevelsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage getHdmiSoundState(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetHdmiSoundStateMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> hdmiSoundStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getHdmiSoundStateChangedMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service HdmiSoundService.
   */
  public static final class HdmiSoundServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<HdmiSoundServiceFutureStub> {
    private HdmiSoundServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected HdmiSoundServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new HdmiSoundServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.AvailabilityResponse> isEnabled(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsEnabledMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setMute(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetMuteMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setAqProfile(
        com.ifit.glassos.settings.hdmi.AqProfileMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetAqProfileMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setAqLevels(
        com.ifit.glassos.settings.hdmi.AqLevelsMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetAqLevelsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage> getHdmiSoundState(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetHdmiSoundStateMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_IS_ENABLED = 0;
  private static final int METHODID_IS_ENABLED_FLOW = 1;
  private static final int METHODID_SET_MUTE = 2;
  private static final int METHODID_SET_AQ_PROFILE = 3;
  private static final int METHODID_SET_AQ_LEVELS = 4;
  private static final int METHODID_GET_HDMI_SOUND_STATE = 5;
  private static final int METHODID_HDMI_SOUND_STATE_CHANGED = 6;

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
        case METHODID_IS_ENABLED:
          serviceImpl.isEnabled((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_IS_ENABLED_FLOW:
          serviceImpl.isEnabledFlow((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.AvailabilityResponse>) responseObserver);
          break;
        case METHODID_SET_MUTE:
          serviceImpl.setMute((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_SET_AQ_PROFILE:
          serviceImpl.setAqProfile((com.ifit.glassos.settings.hdmi.AqProfileMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_SET_AQ_LEVELS:
          serviceImpl.setAqLevels((com.ifit.glassos.settings.hdmi.AqLevelsMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_HDMI_SOUND_STATE:
          serviceImpl.getHdmiSoundState((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage>) responseObserver);
          break;
        case METHODID_HDMI_SOUND_STATE_CHANGED:
          serviceImpl.hdmiSoundStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage>) responseObserver);
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
          getIsEnabledMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_IS_ENABLED)))
        .addMethod(
          getIsEnabledFlowMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.AvailabilityResponse>(
                service, METHODID_IS_ENABLED_FLOW)))
        .addMethod(
          getSetMuteMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_MUTE)))
        .addMethod(
          getSetAqProfileMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.hdmi.AqProfileMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_AQ_PROFILE)))
        .addMethod(
          getSetAqLevelsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.hdmi.AqLevelsMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_AQ_LEVELS)))
        .addMethod(
          getGetHdmiSoundStateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage>(
                service, METHODID_GET_HDMI_SOUND_STATE)))
        .addMethod(
          getHdmiSoundStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.hdmi.HdmiSoundStateMessage>(
                service, METHODID_HDMI_SOUND_STATE_CHANGED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (HdmiSoundServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getIsEnabledMethod())
              .addMethod(getIsEnabledFlowMethod())
              .addMethod(getSetMuteMethod())
              .addMethod(getSetAqProfileMethod())
              .addMethod(getSetAqLevelsMethod())
              .addMethod(getGetHdmiSoundStateMethod())
              .addMethod(getHdmiSoundStateChangedMethod())
              .build();
        }
      }
    }
    return result;
  }
}

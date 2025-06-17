package com.ifit.glassos.settings;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: settings/TimeZoneService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class TimeZoneServiceGrpc {

  private TimeZoneServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.TimeZoneService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.TimeZoneList> getGetAllTimeZonesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetAllTimeZones",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.TimeZoneList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.TimeZoneList> getGetAllTimeZonesMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.TimeZoneList> getGetAllTimeZonesMethod;
    if ((getGetAllTimeZonesMethod = TimeZoneServiceGrpc.getGetAllTimeZonesMethod) == null) {
      synchronized (TimeZoneServiceGrpc.class) {
        if ((getGetAllTimeZonesMethod = TimeZoneServiceGrpc.getGetAllTimeZonesMethod) == null) {
          TimeZoneServiceGrpc.getGetAllTimeZonesMethod = getGetAllTimeZonesMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.TimeZoneList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetAllTimeZones"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.TimeZoneList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetAllTimeZonesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.TimeZone,
      com.ifit.glassos.settings.TimeZoneResult> getSetTimeZoneMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetTimeZone",
      requestType = com.ifit.glassos.settings.TimeZone.class,
      responseType = com.ifit.glassos.settings.TimeZoneResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.TimeZone,
      com.ifit.glassos.settings.TimeZoneResult> getSetTimeZoneMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.TimeZone, com.ifit.glassos.settings.TimeZoneResult> getSetTimeZoneMethod;
    if ((getSetTimeZoneMethod = TimeZoneServiceGrpc.getSetTimeZoneMethod) == null) {
      synchronized (TimeZoneServiceGrpc.class) {
        if ((getSetTimeZoneMethod = TimeZoneServiceGrpc.getSetTimeZoneMethod) == null) {
          TimeZoneServiceGrpc.getSetTimeZoneMethod = getSetTimeZoneMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.TimeZone, com.ifit.glassos.settings.TimeZoneResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetTimeZone"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.TimeZone.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.TimeZoneResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetTimeZoneMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.TimeZoneResult> getGetCurrentTimeZoneMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentTimeZone",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.TimeZoneResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.TimeZoneResult> getGetCurrentTimeZoneMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.TimeZoneResult> getGetCurrentTimeZoneMethod;
    if ((getGetCurrentTimeZoneMethod = TimeZoneServiceGrpc.getGetCurrentTimeZoneMethod) == null) {
      synchronized (TimeZoneServiceGrpc.class) {
        if ((getGetCurrentTimeZoneMethod = TimeZoneServiceGrpc.getGetCurrentTimeZoneMethod) == null) {
          TimeZoneServiceGrpc.getGetCurrentTimeZoneMethod = getGetCurrentTimeZoneMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.TimeZoneResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentTimeZone"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.TimeZoneResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentTimeZoneMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.TimeZone> getTimeZoneChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "TimeZoneChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.settings.TimeZone.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.settings.TimeZone> getTimeZoneChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.TimeZone> getTimeZoneChangedMethod;
    if ((getTimeZoneChangedMethod = TimeZoneServiceGrpc.getTimeZoneChangedMethod) == null) {
      synchronized (TimeZoneServiceGrpc.class) {
        if ((getTimeZoneChangedMethod = TimeZoneServiceGrpc.getTimeZoneChangedMethod) == null) {
          TimeZoneServiceGrpc.getTimeZoneChangedMethod = getTimeZoneChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.settings.TimeZone>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "TimeZoneChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.TimeZone.getDefaultInstance()))
              .build();
        }
      }
    }
    return getTimeZoneChangedMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static TimeZoneServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<TimeZoneServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<TimeZoneServiceStub>() {
        @java.lang.Override
        public TimeZoneServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new TimeZoneServiceStub(channel, callOptions);
        }
      };
    return TimeZoneServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static TimeZoneServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<TimeZoneServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<TimeZoneServiceBlockingStub>() {
        @java.lang.Override
        public TimeZoneServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new TimeZoneServiceBlockingStub(channel, callOptions);
        }
      };
    return TimeZoneServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static TimeZoneServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<TimeZoneServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<TimeZoneServiceFutureStub>() {
        @java.lang.Override
        public TimeZoneServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new TimeZoneServiceFutureStub(channel, callOptions);
        }
      };
    return TimeZoneServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void getAllTimeZones(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetAllTimeZonesMethod(), responseObserver);
    }

    /**
     */
    default void setTimeZone(com.ifit.glassos.settings.TimeZone request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetTimeZoneMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentTimeZone(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentTimeZoneMethod(), responseObserver);
    }

    /**
     */
    default void timeZoneChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZone> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getTimeZoneChangedMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service TimeZoneService.
   */
  public static abstract class TimeZoneServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return TimeZoneServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service TimeZoneService.
   */
  public static final class TimeZoneServiceStub
      extends io.grpc.stub.AbstractAsyncStub<TimeZoneServiceStub> {
    private TimeZoneServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected TimeZoneServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new TimeZoneServiceStub(channel, callOptions);
    }

    /**
     */
    public void getAllTimeZones(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetAllTimeZonesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setTimeZone(com.ifit.glassos.settings.TimeZone request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetTimeZoneMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentTimeZone(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentTimeZoneMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void timeZoneChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZone> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getTimeZoneChangedMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service TimeZoneService.
   */
  public static final class TimeZoneServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<TimeZoneServiceBlockingStub> {
    private TimeZoneServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected TimeZoneServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new TimeZoneServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.settings.TimeZoneList getAllTimeZones(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetAllTimeZonesMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.TimeZoneResult setTimeZone(com.ifit.glassos.settings.TimeZone request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetTimeZoneMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.TimeZoneResult getCurrentTimeZone(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentTimeZoneMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.settings.TimeZone> timeZoneChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getTimeZoneChangedMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service TimeZoneService.
   */
  public static final class TimeZoneServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<TimeZoneServiceFutureStub> {
    private TimeZoneServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected TimeZoneServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new TimeZoneServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.TimeZoneList> getAllTimeZones(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetAllTimeZonesMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.TimeZoneResult> setTimeZone(
        com.ifit.glassos.settings.TimeZone request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetTimeZoneMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.TimeZoneResult> getCurrentTimeZone(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentTimeZoneMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_ALL_TIME_ZONES = 0;
  private static final int METHODID_SET_TIME_ZONE = 1;
  private static final int METHODID_GET_CURRENT_TIME_ZONE = 2;
  private static final int METHODID_TIME_ZONE_CHANGED = 3;

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
        case METHODID_GET_ALL_TIME_ZONES:
          serviceImpl.getAllTimeZones((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneList>) responseObserver);
          break;
        case METHODID_SET_TIME_ZONE:
          serviceImpl.setTimeZone((com.ifit.glassos.settings.TimeZone) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneResult>) responseObserver);
          break;
        case METHODID_GET_CURRENT_TIME_ZONE:
          serviceImpl.getCurrentTimeZone((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZoneResult>) responseObserver);
          break;
        case METHODID_TIME_ZONE_CHANGED:
          serviceImpl.timeZoneChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.TimeZone>) responseObserver);
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
          getGetAllTimeZonesMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.TimeZoneList>(
                service, METHODID_GET_ALL_TIME_ZONES)))
        .addMethod(
          getSetTimeZoneMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.TimeZone,
              com.ifit.glassos.settings.TimeZoneResult>(
                service, METHODID_SET_TIME_ZONE)))
        .addMethod(
          getGetCurrentTimeZoneMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.TimeZoneResult>(
                service, METHODID_GET_CURRENT_TIME_ZONE)))
        .addMethod(
          getTimeZoneChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.settings.TimeZone>(
                service, METHODID_TIME_ZONE_CHANGED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (TimeZoneServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetAllTimeZonesMethod())
              .addMethod(getSetTimeZoneMethod())
              .addMethod(getGetCurrentTimeZoneMethod())
              .addMethod(getTimeZoneChangedMethod())
              .build();
        }
      }
    }
    return result;
  }
}

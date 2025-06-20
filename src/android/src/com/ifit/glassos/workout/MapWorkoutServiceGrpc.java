package com.ifit.glassos.workout;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: workout/MapWorkoutService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class MapWorkoutServiceGrpc {

  private MapWorkoutServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.MapWorkoutService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getGetLocationMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetLocation",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.map.MapCoordinate.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getGetLocationMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate> getGetLocationMethod;
    if ((getGetLocationMethod = MapWorkoutServiceGrpc.getGetLocationMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getGetLocationMethod = MapWorkoutServiceGrpc.getGetLocationMethod) == null) {
          MapWorkoutServiceGrpc.getGetLocationMethod = getGetLocationMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetLocation"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.map.MapCoordinate.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetLocationMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getLocationChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "LocationChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.map.MapCoordinate.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getLocationChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate> getLocationChangedMethod;
    if ((getLocationChangedMethod = MapWorkoutServiceGrpc.getLocationChangedMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getLocationChangedMethod = MapWorkoutServiceGrpc.getLocationChangedMethod) == null) {
          MapWorkoutServiceGrpc.getLocationChangedMethod = getLocationChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "LocationChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.map.MapCoordinate.getDefaultInstance()))
              .build();
        }
      }
    }
    return getLocationChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getGetSelectedMapTypeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSelectedMapType",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.MapViewTypeMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getGetSelectedMapTypeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage> getGetSelectedMapTypeMethod;
    if ((getGetSelectedMapTypeMethod = MapWorkoutServiceGrpc.getGetSelectedMapTypeMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getGetSelectedMapTypeMethod = MapWorkoutServiceGrpc.getGetSelectedMapTypeMethod) == null) {
          MapWorkoutServiceGrpc.getGetSelectedMapTypeMethod = getGetSelectedMapTypeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSelectedMapType"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.MapViewTypeMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSelectedMapTypeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getSelectedMapTypeChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SelectedMapTypeChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.MapViewTypeMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getSelectedMapTypeChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage> getSelectedMapTypeChangedMethod;
    if ((getSelectedMapTypeChangedMethod = MapWorkoutServiceGrpc.getSelectedMapTypeChangedMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getSelectedMapTypeChangedMethod = MapWorkoutServiceGrpc.getSelectedMapTypeChangedMethod) == null) {
          MapWorkoutServiceGrpc.getSelectedMapTypeChangedMethod = getSelectedMapTypeChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SelectedMapTypeChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.MapViewTypeMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSelectedMapTypeChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getGetCurrentMapTypeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentMapType",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.MapViewTypeMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getGetCurrentMapTypeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage> getGetCurrentMapTypeMethod;
    if ((getGetCurrentMapTypeMethod = MapWorkoutServiceGrpc.getGetCurrentMapTypeMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getGetCurrentMapTypeMethod = MapWorkoutServiceGrpc.getGetCurrentMapTypeMethod) == null) {
          MapWorkoutServiceGrpc.getGetCurrentMapTypeMethod = getGetCurrentMapTypeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentMapType"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.MapViewTypeMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentMapTypeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getCurrentMapTypeChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CurrentMapTypeChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.MapViewTypeMessage.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.MapViewTypeMessage> getCurrentMapTypeChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage> getCurrentMapTypeChangedMethod;
    if ((getCurrentMapTypeChangedMethod = MapWorkoutServiceGrpc.getCurrentMapTypeChangedMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getCurrentMapTypeChangedMethod = MapWorkoutServiceGrpc.getCurrentMapTypeChangedMethod) == null) {
          MapWorkoutServiceGrpc.getCurrentMapTypeChangedMethod = getCurrentMapTypeChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.MapViewTypeMessage>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CurrentMapTypeChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.MapViewTypeMessage.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCurrentMapTypeChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getGetStreetViewPointMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStreetViewPoint",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.map.MapCoordinate.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getGetStreetViewPointMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate> getGetStreetViewPointMethod;
    if ((getGetStreetViewPointMethod = MapWorkoutServiceGrpc.getGetStreetViewPointMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getGetStreetViewPointMethod = MapWorkoutServiceGrpc.getGetStreetViewPointMethod) == null) {
          MapWorkoutServiceGrpc.getGetStreetViewPointMethod = getGetStreetViewPointMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStreetViewPoint"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.map.MapCoordinate.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStreetViewPointMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getStreetViewPointChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StreetViewPointChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.workout.data.map.MapCoordinate.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.workout.data.map.MapCoordinate> getStreetViewPointChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate> getStreetViewPointChangedMethod;
    if ((getStreetViewPointChangedMethod = MapWorkoutServiceGrpc.getStreetViewPointChangedMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getStreetViewPointChangedMethod = MapWorkoutServiceGrpc.getStreetViewPointChangedMethod) == null) {
          MapWorkoutServiceGrpc.getStreetViewPointChangedMethod = getStreetViewPointChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.workout.data.map.MapCoordinate>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StreetViewPointChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.data.map.MapCoordinate.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStreetViewPointChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.workout.MapViewTypeMessage,
      com.ifit.glassos.util.Empty> getSetMapTypeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetMapType",
      requestType = com.ifit.glassos.workout.MapViewTypeMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.workout.MapViewTypeMessage,
      com.ifit.glassos.util.Empty> getSetMapTypeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.workout.MapViewTypeMessage, com.ifit.glassos.util.Empty> getSetMapTypeMethod;
    if ((getSetMapTypeMethod = MapWorkoutServiceGrpc.getSetMapTypeMethod) == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        if ((getSetMapTypeMethod = MapWorkoutServiceGrpc.getSetMapTypeMethod) == null) {
          MapWorkoutServiceGrpc.getSetMapTypeMethod = getSetMapTypeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.workout.MapViewTypeMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetMapType"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.workout.MapViewTypeMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetMapTypeMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static MapWorkoutServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<MapWorkoutServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<MapWorkoutServiceStub>() {
        @java.lang.Override
        public MapWorkoutServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new MapWorkoutServiceStub(channel, callOptions);
        }
      };
    return MapWorkoutServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static MapWorkoutServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<MapWorkoutServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<MapWorkoutServiceBlockingStub>() {
        @java.lang.Override
        public MapWorkoutServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new MapWorkoutServiceBlockingStub(channel, callOptions);
        }
      };
    return MapWorkoutServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static MapWorkoutServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<MapWorkoutServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<MapWorkoutServiceFutureStub>() {
        @java.lang.Override
        public MapWorkoutServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new MapWorkoutServiceFutureStub(channel, callOptions);
        }
      };
    return MapWorkoutServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void getLocation(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetLocationMethod(), responseObserver);
    }

    /**
     */
    default void locationChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getLocationChangedMethod(), responseObserver);
    }

    /**
     */
    default void getSelectedMapType(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSelectedMapTypeMethod(), responseObserver);
    }

    /**
     */
    default void selectedMapTypeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSelectedMapTypeChangedMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentMapType(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentMapTypeMethod(), responseObserver);
    }

    /**
     */
    default void currentMapTypeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCurrentMapTypeChangedMethod(), responseObserver);
    }

    /**
     */
    default void getStreetViewPoint(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStreetViewPointMethod(), responseObserver);
    }

    /**
     */
    default void streetViewPointChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStreetViewPointChangedMethod(), responseObserver);
    }

    /**
     */
    default void setMapType(com.ifit.glassos.workout.MapViewTypeMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetMapTypeMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service MapWorkoutService.
   */
  public static abstract class MapWorkoutServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return MapWorkoutServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service MapWorkoutService.
   */
  public static final class MapWorkoutServiceStub
      extends io.grpc.stub.AbstractAsyncStub<MapWorkoutServiceStub> {
    private MapWorkoutServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected MapWorkoutServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new MapWorkoutServiceStub(channel, callOptions);
    }

    /**
     */
    public void getLocation(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetLocationMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void locationChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getLocationChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getSelectedMapType(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSelectedMapTypeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void selectedMapTypeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getSelectedMapTypeChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentMapType(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentMapTypeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void currentMapTypeChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getCurrentMapTypeChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getStreetViewPoint(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStreetViewPointMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void streetViewPointChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getStreetViewPointChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setMapType(com.ifit.glassos.workout.MapViewTypeMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetMapTypeMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service MapWorkoutService.
   */
  public static final class MapWorkoutServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<MapWorkoutServiceBlockingStub> {
    private MapWorkoutServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected MapWorkoutServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new MapWorkoutServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.workout.data.map.MapCoordinate getLocation(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetLocationMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.map.MapCoordinate> locationChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getLocationChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.MapViewTypeMessage getSelectedMapType(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSelectedMapTypeMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.MapViewTypeMessage> selectedMapTypeChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getSelectedMapTypeChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.MapViewTypeMessage getCurrentMapType(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentMapTypeMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.MapViewTypeMessage> currentMapTypeChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getCurrentMapTypeChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.workout.data.map.MapCoordinate getStreetViewPoint(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStreetViewPointMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.workout.data.map.MapCoordinate> streetViewPointChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getStreetViewPointChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setMapType(com.ifit.glassos.workout.MapViewTypeMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetMapTypeMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service MapWorkoutService.
   */
  public static final class MapWorkoutServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<MapWorkoutServiceFutureStub> {
    private MapWorkoutServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected MapWorkoutServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new MapWorkoutServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.data.map.MapCoordinate> getLocation(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetLocationMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.MapViewTypeMessage> getSelectedMapType(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSelectedMapTypeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.MapViewTypeMessage> getCurrentMapType(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentMapTypeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.workout.data.map.MapCoordinate> getStreetViewPoint(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStreetViewPointMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setMapType(
        com.ifit.glassos.workout.MapViewTypeMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetMapTypeMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_LOCATION = 0;
  private static final int METHODID_LOCATION_CHANGED = 1;
  private static final int METHODID_GET_SELECTED_MAP_TYPE = 2;
  private static final int METHODID_SELECTED_MAP_TYPE_CHANGED = 3;
  private static final int METHODID_GET_CURRENT_MAP_TYPE = 4;
  private static final int METHODID_CURRENT_MAP_TYPE_CHANGED = 5;
  private static final int METHODID_GET_STREET_VIEW_POINT = 6;
  private static final int METHODID_STREET_VIEW_POINT_CHANGED = 7;
  private static final int METHODID_SET_MAP_TYPE = 8;

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
        case METHODID_GET_LOCATION:
          serviceImpl.getLocation((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate>) responseObserver);
          break;
        case METHODID_LOCATION_CHANGED:
          serviceImpl.locationChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate>) responseObserver);
          break;
        case METHODID_GET_SELECTED_MAP_TYPE:
          serviceImpl.getSelectedMapType((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage>) responseObserver);
          break;
        case METHODID_SELECTED_MAP_TYPE_CHANGED:
          serviceImpl.selectedMapTypeChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage>) responseObserver);
          break;
        case METHODID_GET_CURRENT_MAP_TYPE:
          serviceImpl.getCurrentMapType((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage>) responseObserver);
          break;
        case METHODID_CURRENT_MAP_TYPE_CHANGED:
          serviceImpl.currentMapTypeChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.MapViewTypeMessage>) responseObserver);
          break;
        case METHODID_GET_STREET_VIEW_POINT:
          serviceImpl.getStreetViewPoint((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate>) responseObserver);
          break;
        case METHODID_STREET_VIEW_POINT_CHANGED:
          serviceImpl.streetViewPointChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.workout.data.map.MapCoordinate>) responseObserver);
          break;
        case METHODID_SET_MAP_TYPE:
          serviceImpl.setMapType((com.ifit.glassos.workout.MapViewTypeMessage) request,
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
          getGetLocationMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.map.MapCoordinate>(
                service, METHODID_GET_LOCATION)))
        .addMethod(
          getLocationChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.map.MapCoordinate>(
                service, METHODID_LOCATION_CHANGED)))
        .addMethod(
          getGetSelectedMapTypeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.MapViewTypeMessage>(
                service, METHODID_GET_SELECTED_MAP_TYPE)))
        .addMethod(
          getSelectedMapTypeChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.MapViewTypeMessage>(
                service, METHODID_SELECTED_MAP_TYPE_CHANGED)))
        .addMethod(
          getGetCurrentMapTypeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.MapViewTypeMessage>(
                service, METHODID_GET_CURRENT_MAP_TYPE)))
        .addMethod(
          getCurrentMapTypeChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.MapViewTypeMessage>(
                service, METHODID_CURRENT_MAP_TYPE_CHANGED)))
        .addMethod(
          getGetStreetViewPointMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.map.MapCoordinate>(
                service, METHODID_GET_STREET_VIEW_POINT)))
        .addMethod(
          getStreetViewPointChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.workout.data.map.MapCoordinate>(
                service, METHODID_STREET_VIEW_POINT_CHANGED)))
        .addMethod(
          getSetMapTypeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.workout.MapViewTypeMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_MAP_TYPE)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (MapWorkoutServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetLocationMethod())
              .addMethod(getLocationChangedMethod())
              .addMethod(getGetSelectedMapTypeMethod())
              .addMethod(getSelectedMapTypeChangedMethod())
              .addMethod(getGetCurrentMapTypeMethod())
              .addMethod(getCurrentMapTypeChangedMethod())
              .addMethod(getGetStreetViewPointMethod())
              .addMethod(getStreetViewPointChangedMethod())
              .addMethod(getSetMapTypeMethod())
              .build();
        }
      }
    }
    return result;
  }
}

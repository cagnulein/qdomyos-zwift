package com.ifit.glassos.appstore;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: appstore/AppStoreService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class AppStoreServiceGrpc {

  private AppStoreServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.AppStoreService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appstore.AppStoreState> getAppStoreStateFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AppStoreStateFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appstore.AppStoreState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appstore.AppStoreState> getAppStoreStateFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appstore.AppStoreState> getAppStoreStateFlowSubscriptionMethod;
    if ((getAppStoreStateFlowSubscriptionMethod = AppStoreServiceGrpc.getAppStoreStateFlowSubscriptionMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getAppStoreStateFlowSubscriptionMethod = AppStoreServiceGrpc.getAppStoreStateFlowSubscriptionMethod) == null) {
          AppStoreServiceGrpc.getAppStoreStateFlowSubscriptionMethod = getAppStoreStateFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appstore.AppStoreState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AppStoreStateFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appstore.AppStoreState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAppStoreStateFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appstore.AppStoreAppList> getAppsFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "AppsFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appstore.AppStoreAppList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appstore.AppStoreAppList> getAppsFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appstore.AppStoreAppList> getAppsFlowSubscriptionMethod;
    if ((getAppsFlowSubscriptionMethod = AppStoreServiceGrpc.getAppsFlowSubscriptionMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getAppsFlowSubscriptionMethod = AppStoreServiceGrpc.getAppsFlowSubscriptionMethod) == null) {
          AppStoreServiceGrpc.getAppsFlowSubscriptionMethod = getAppsFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appstore.AppStoreAppList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "AppsFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appstore.AppStoreAppList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getAppsFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appstore.AppStoreActionRequest,
      com.ifit.glassos.util.Empty> getRequestAppInstallMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RequestAppInstall",
      requestType = com.ifit.glassos.appstore.AppStoreActionRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appstore.AppStoreActionRequest,
      com.ifit.glassos.util.Empty> getRequestAppInstallMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appstore.AppStoreActionRequest, com.ifit.glassos.util.Empty> getRequestAppInstallMethod;
    if ((getRequestAppInstallMethod = AppStoreServiceGrpc.getRequestAppInstallMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getRequestAppInstallMethod = AppStoreServiceGrpc.getRequestAppInstallMethod) == null) {
          AppStoreServiceGrpc.getRequestAppInstallMethod = getRequestAppInstallMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appstore.AppStoreActionRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RequestAppInstall"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appstore.AppStoreActionRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRequestAppInstallMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appstore.AppStoreActionRequest,
      com.ifit.glassos.util.Empty> getRequestAppUninstallMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RequestAppUninstall",
      requestType = com.ifit.glassos.appstore.AppStoreActionRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appstore.AppStoreActionRequest,
      com.ifit.glassos.util.Empty> getRequestAppUninstallMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appstore.AppStoreActionRequest, com.ifit.glassos.util.Empty> getRequestAppUninstallMethod;
    if ((getRequestAppUninstallMethod = AppStoreServiceGrpc.getRequestAppUninstallMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getRequestAppUninstallMethod = AppStoreServiceGrpc.getRequestAppUninstallMethod) == null) {
          AppStoreServiceGrpc.getRequestAppUninstallMethod = getRequestAppUninstallMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appstore.AppStoreActionRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RequestAppUninstall"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appstore.AppStoreActionRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRequestAppUninstallMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getGetAppsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetApps",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getGetAppsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty> getGetAppsMethod;
    if ((getGetAppsMethod = AppStoreServiceGrpc.getGetAppsMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getGetAppsMethod = AppStoreServiceGrpc.getGetAppsMethod) == null) {
          AppStoreServiceGrpc.getGetAppsMethod = getGetAppsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetApps"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetAppsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getGoIdleMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GoIdle",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getGoIdleMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getGoIdleMethod;
    if ((getGoIdleMethod = AppStoreServiceGrpc.getGoIdleMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getGoIdleMethod = AppStoreServiceGrpc.getGoIdleMethod) == null) {
          AppStoreServiceGrpc.getGoIdleMethod = getGoIdleMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GoIdle"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGoIdleMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appstore.StorageStats> getGetStorageStatsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetStorageStats",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appstore.StorageStats.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appstore.StorageStats> getGetStorageStatsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appstore.StorageStats> getGetStorageStatsMethod;
    if ((getGetStorageStatsMethod = AppStoreServiceGrpc.getGetStorageStatsMethod) == null) {
      synchronized (AppStoreServiceGrpc.class) {
        if ((getGetStorageStatsMethod = AppStoreServiceGrpc.getGetStorageStatsMethod) == null) {
          AppStoreServiceGrpc.getGetStorageStatsMethod = getGetStorageStatsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appstore.StorageStats>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetStorageStats"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appstore.StorageStats.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetStorageStatsMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static AppStoreServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AppStoreServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AppStoreServiceStub>() {
        @java.lang.Override
        public AppStoreServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AppStoreServiceStub(channel, callOptions);
        }
      };
    return AppStoreServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static AppStoreServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AppStoreServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AppStoreServiceBlockingStub>() {
        @java.lang.Override
        public AppStoreServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AppStoreServiceBlockingStub(channel, callOptions);
        }
      };
    return AppStoreServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static AppStoreServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AppStoreServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AppStoreServiceFutureStub>() {
        @java.lang.Override
        public AppStoreServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AppStoreServiceFutureStub(channel, callOptions);
        }
      };
    return AppStoreServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void appStoreStateFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.AppStoreState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAppStoreStateFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void appsFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.AppStoreAppList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getAppsFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void requestAppInstall(com.ifit.glassos.appstore.AppStoreActionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRequestAppInstallMethod(), responseObserver);
    }

    /**
     */
    default void requestAppUninstall(com.ifit.glassos.appstore.AppStoreActionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRequestAppUninstallMethod(), responseObserver);
    }

    /**
     */
    default void getApps(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetAppsMethod(), responseObserver);
    }

    /**
     */
    default void goIdle(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGoIdleMethod(), responseObserver);
    }

    /**
     */
    default void getStorageStats(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.StorageStats> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetStorageStatsMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service AppStoreService.
   */
  public static abstract class AppStoreServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return AppStoreServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service AppStoreService.
   */
  public static final class AppStoreServiceStub
      extends io.grpc.stub.AbstractAsyncStub<AppStoreServiceStub> {
    private AppStoreServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AppStoreServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AppStoreServiceStub(channel, callOptions);
    }

    /**
     */
    public void appStoreStateFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.AppStoreState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getAppStoreStateFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void appsFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.AppStoreAppList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getAppsFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void requestAppInstall(com.ifit.glassos.appstore.AppStoreActionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRequestAppInstallMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void requestAppUninstall(com.ifit.glassos.appstore.AppStoreActionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRequestAppUninstallMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getApps(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetAppsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void goIdle(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGoIdleMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getStorageStats(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.StorageStats> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetStorageStatsMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service AppStoreService.
   */
  public static final class AppStoreServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<AppStoreServiceBlockingStub> {
    private AppStoreServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AppStoreServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AppStoreServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.appstore.AppStoreState> appStoreStateFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getAppStoreStateFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.appstore.AppStoreAppList> appsFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getAppsFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty requestAppInstall(com.ifit.glassos.appstore.AppStoreActionRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRequestAppInstallMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty requestAppUninstall(com.ifit.glassos.appstore.AppStoreActionRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRequestAppUninstallMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty getApps(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetAppsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty goIdle(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGoIdleMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.appstore.StorageStats getStorageStats(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetStorageStatsMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service AppStoreService.
   */
  public static final class AppStoreServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<AppStoreServiceFutureStub> {
    private AppStoreServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AppStoreServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AppStoreServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> requestAppInstall(
        com.ifit.glassos.appstore.AppStoreActionRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRequestAppInstallMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> requestAppUninstall(
        com.ifit.glassos.appstore.AppStoreActionRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRequestAppUninstallMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> getApps(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetAppsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> goIdle(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGoIdleMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.appstore.StorageStats> getStorageStats(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetStorageStatsMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_APP_STORE_STATE_FLOW_SUBSCRIPTION = 0;
  private static final int METHODID_APPS_FLOW_SUBSCRIPTION = 1;
  private static final int METHODID_REQUEST_APP_INSTALL = 2;
  private static final int METHODID_REQUEST_APP_UNINSTALL = 3;
  private static final int METHODID_GET_APPS = 4;
  private static final int METHODID_GO_IDLE = 5;
  private static final int METHODID_GET_STORAGE_STATS = 6;

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
        case METHODID_APP_STORE_STATE_FLOW_SUBSCRIPTION:
          serviceImpl.appStoreStateFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.AppStoreState>) responseObserver);
          break;
        case METHODID_APPS_FLOW_SUBSCRIPTION:
          serviceImpl.appsFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.AppStoreAppList>) responseObserver);
          break;
        case METHODID_REQUEST_APP_INSTALL:
          serviceImpl.requestAppInstall((com.ifit.glassos.appstore.AppStoreActionRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_REQUEST_APP_UNINSTALL:
          serviceImpl.requestAppUninstall((com.ifit.glassos.appstore.AppStoreActionRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_APPS:
          serviceImpl.getApps((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GO_IDLE:
          serviceImpl.goIdle((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_STORAGE_STATS:
          serviceImpl.getStorageStats((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appstore.StorageStats>) responseObserver);
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
          getAppStoreStateFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appstore.AppStoreState>(
                service, METHODID_APP_STORE_STATE_FLOW_SUBSCRIPTION)))
        .addMethod(
          getAppsFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appstore.AppStoreAppList>(
                service, METHODID_APPS_FLOW_SUBSCRIPTION)))
        .addMethod(
          getRequestAppInstallMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appstore.AppStoreActionRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_REQUEST_APP_INSTALL)))
        .addMethod(
          getRequestAppUninstallMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appstore.AppStoreActionRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_REQUEST_APP_UNINSTALL)))
        .addMethod(
          getGetAppsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_GET_APPS)))
        .addMethod(
          getGoIdleMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_GO_IDLE)))
        .addMethod(
          getGetStorageStatsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appstore.StorageStats>(
                service, METHODID_GET_STORAGE_STATS)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (AppStoreServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getAppStoreStateFlowSubscriptionMethod())
              .addMethod(getAppsFlowSubscriptionMethod())
              .addMethod(getRequestAppInstallMethod())
              .addMethod(getRequestAppUninstallMethod())
              .addMethod(getGetAppsMethod())
              .addMethod(getGoIdleMethod())
              .addMethod(getGetStorageStatsMethod())
              .build();
        }
      }
    }
    return result;
  }
}

package com.ifit.glassos.appnavigation;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: appnavigation/AppNavigationService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class AppNavigationServiceGrpc {

  private AppNavigationServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.AppNavigationService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.ForegroundFqns> getForegroundFQNsFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ForegroundFQNsFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appnavigation.ForegroundFqns.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.ForegroundFqns> getForegroundFQNsFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.ForegroundFqns> getForegroundFQNsFlowSubscriptionMethod;
    if ((getForegroundFQNsFlowSubscriptionMethod = AppNavigationServiceGrpc.getForegroundFQNsFlowSubscriptionMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getForegroundFQNsFlowSubscriptionMethod = AppNavigationServiceGrpc.getForegroundFQNsFlowSubscriptionMethod) == null) {
          AppNavigationServiceGrpc.getForegroundFQNsFlowSubscriptionMethod = getForegroundFQNsFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.ForegroundFqns>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ForegroundFQNsFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundFqns.getDefaultInstance()))
              .build();
        }
      }
    }
    return getForegroundFQNsFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.TouchEvent> getTouchFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "TouchFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appnavigation.TouchEvent.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.TouchEvent> getTouchFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.TouchEvent> getTouchFlowSubscriptionMethod;
    if ((getTouchFlowSubscriptionMethod = AppNavigationServiceGrpc.getTouchFlowSubscriptionMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getTouchFlowSubscriptionMethod = AppNavigationServiceGrpc.getTouchFlowSubscriptionMethod) == null) {
          AppNavigationServiceGrpc.getTouchFlowSubscriptionMethod = getTouchFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.TouchEvent>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "TouchFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.TouchEvent.getDefaultInstance()))
              .build();
        }
      }
    }
    return getTouchFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getEnabledSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "EnabledSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getEnabledSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getEnabledSubscriptionMethod;
    if ((getEnabledSubscriptionMethod = AppNavigationServiceGrpc.getEnabledSubscriptionMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getEnabledSubscriptionMethod = AppNavigationServiceGrpc.getEnabledSubscriptionMethod) == null) {
          AppNavigationServiceGrpc.getEnabledSubscriptionMethod = getEnabledSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "EnabledSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getEnabledSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getKeyboardVisibleFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "KeyboardVisibleFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getKeyboardVisibleFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getKeyboardVisibleFlowSubscriptionMethod;
    if ((getKeyboardVisibleFlowSubscriptionMethod = AppNavigationServiceGrpc.getKeyboardVisibleFlowSubscriptionMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getKeyboardVisibleFlowSubscriptionMethod = AppNavigationServiceGrpc.getKeyboardVisibleFlowSubscriptionMethod) == null) {
          AppNavigationServiceGrpc.getKeyboardVisibleFlowSubscriptionMethod = getKeyboardVisibleFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "KeyboardVisibleFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getKeyboardVisibleFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.ListStringResponse> getForegroundClassNameFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ForegroundClassNameFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.ListStringResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.ListStringResponse> getForegroundClassNameFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.ListStringResponse> getForegroundClassNameFlowSubscriptionMethod;
    if ((getForegroundClassNameFlowSubscriptionMethod = AppNavigationServiceGrpc.getForegroundClassNameFlowSubscriptionMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getForegroundClassNameFlowSubscriptionMethod = AppNavigationServiceGrpc.getForegroundClassNameFlowSubscriptionMethod) == null) {
          AppNavigationServiceGrpc.getForegroundClassNameFlowSubscriptionMethod = getForegroundClassNameFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.ListStringResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ForegroundClassNameFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.ListStringResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getForegroundClassNameFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.ForegroundClasses> getForegroundClassesFlowSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ForegroundClassesFlowSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appnavigation.ForegroundClasses.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.ForegroundClasses> getForegroundClassesFlowSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.ForegroundClasses> getForegroundClassesFlowSubscriptionMethod;
    if ((getForegroundClassesFlowSubscriptionMethod = AppNavigationServiceGrpc.getForegroundClassesFlowSubscriptionMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getForegroundClassesFlowSubscriptionMethod = AppNavigationServiceGrpc.getForegroundClassesFlowSubscriptionMethod) == null) {
          AppNavigationServiceGrpc.getForegroundClassesFlowSubscriptionMethod = getForegroundClassesFlowSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.ForegroundClasses>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ForegroundClassesFlowSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundClasses.getDefaultInstance()))
              .build();
        }
      }
    }
    return getForegroundClassesFlowSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getPerformBackButtonMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "PerformBackButton",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getPerformBackButtonMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getPerformBackButtonMethod;
    if ((getPerformBackButtonMethod = AppNavigationServiceGrpc.getPerformBackButtonMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getPerformBackButtonMethod = AppNavigationServiceGrpc.getPerformBackButtonMethod) == null) {
          AppNavigationServiceGrpc.getPerformBackButtonMethod = getPerformBackButtonMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "PerformBackButton"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getPerformBackButtonMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.ForegroundFqns> getGetForegroundFqnsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetForegroundFqns",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.appnavigation.ForegroundFqns.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.appnavigation.ForegroundFqns> getGetForegroundFqnsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.ForegroundFqns> getGetForegroundFqnsMethod;
    if ((getGetForegroundFqnsMethod = AppNavigationServiceGrpc.getGetForegroundFqnsMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getGetForegroundFqnsMethod = AppNavigationServiceGrpc.getGetForegroundFqnsMethod) == null) {
          AppNavigationServiceGrpc.getGetForegroundFqnsMethod = getGetForegroundFqnsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.appnavigation.ForegroundFqns>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetForegroundFqns"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundFqns.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetForegroundFqnsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getSetCurrentForegroundFQNMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetCurrentForegroundFQN",
      requestType = com.ifit.glassos.appnavigation.ForegroundFqnRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getSetCurrentForegroundFQNMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty> getSetCurrentForegroundFQNMethod;
    if ((getSetCurrentForegroundFQNMethod = AppNavigationServiceGrpc.getSetCurrentForegroundFQNMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getSetCurrentForegroundFQNMethod = AppNavigationServiceGrpc.getSetCurrentForegroundFQNMethod) == null) {
          AppNavigationServiceGrpc.getSetCurrentForegroundFQNMethod = getSetCurrentForegroundFQNMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetCurrentForegroundFQN"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundFqnRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetCurrentForegroundFQNMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getRemoveCurrentForegroundFQNMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RemoveCurrentForegroundFQN",
      requestType = com.ifit.glassos.appnavigation.ForegroundFqnRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getRemoveCurrentForegroundFQNMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty> getRemoveCurrentForegroundFQNMethod;
    if ((getRemoveCurrentForegroundFQNMethod = AppNavigationServiceGrpc.getRemoveCurrentForegroundFQNMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getRemoveCurrentForegroundFQNMethod = AppNavigationServiceGrpc.getRemoveCurrentForegroundFQNMethod) == null) {
          AppNavigationServiceGrpc.getRemoveCurrentForegroundFQNMethod = getRemoveCurrentForegroundFQNMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RemoveCurrentForegroundFQN"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundFqnRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRemoveCurrentForegroundFQNMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getRemoveForegroundFQNFromHistoryMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RemoveForegroundFQNFromHistory",
      requestType = com.ifit.glassos.appnavigation.ForegroundFqnRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getRemoveForegroundFQNFromHistoryMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty> getRemoveForegroundFQNFromHistoryMethod;
    if ((getRemoveForegroundFQNFromHistoryMethod = AppNavigationServiceGrpc.getRemoveForegroundFQNFromHistoryMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getRemoveForegroundFQNFromHistoryMethod = AppNavigationServiceGrpc.getRemoveForegroundFQNFromHistoryMethod) == null) {
          AppNavigationServiceGrpc.getRemoveForegroundFQNFromHistoryMethod = getRemoveForegroundFQNFromHistoryMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RemoveForegroundFQNFromHistory"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundFqnRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRemoveForegroundFQNFromHistoryMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundClassNameRequest,
      com.ifit.glassos.util.Empty> getSetCurrentForegroundClassMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetCurrentForegroundClass",
      requestType = com.ifit.glassos.appnavigation.ForegroundClassNameRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundClassNameRequest,
      com.ifit.glassos.util.Empty> getSetCurrentForegroundClassMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundClassNameRequest, com.ifit.glassos.util.Empty> getSetCurrentForegroundClassMethod;
    if ((getSetCurrentForegroundClassMethod = AppNavigationServiceGrpc.getSetCurrentForegroundClassMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getSetCurrentForegroundClassMethod = AppNavigationServiceGrpc.getSetCurrentForegroundClassMethod) == null) {
          AppNavigationServiceGrpc.getSetCurrentForegroundClassMethod = getSetCurrentForegroundClassMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appnavigation.ForegroundClassNameRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetCurrentForegroundClass"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundClassNameRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetCurrentForegroundClassMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getNavigatedToThirdPartyMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "NavigatedToThirdParty",
      requestType = com.ifit.glassos.appnavigation.ForegroundFqnRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest,
      com.ifit.glassos.util.Empty> getNavigatedToThirdPartyMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty> getNavigatedToThirdPartyMethod;
    if ((getNavigatedToThirdPartyMethod = AppNavigationServiceGrpc.getNavigatedToThirdPartyMethod) == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        if ((getNavigatedToThirdPartyMethod = AppNavigationServiceGrpc.getNavigatedToThirdPartyMethod) == null) {
          AppNavigationServiceGrpc.getNavigatedToThirdPartyMethod = getNavigatedToThirdPartyMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.appnavigation.ForegroundFqnRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "NavigatedToThirdParty"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.appnavigation.ForegroundFqnRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getNavigatedToThirdPartyMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static AppNavigationServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AppNavigationServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AppNavigationServiceStub>() {
        @java.lang.Override
        public AppNavigationServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AppNavigationServiceStub(channel, callOptions);
        }
      };
    return AppNavigationServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static AppNavigationServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AppNavigationServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AppNavigationServiceBlockingStub>() {
        @java.lang.Override
        public AppNavigationServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AppNavigationServiceBlockingStub(channel, callOptions);
        }
      };
    return AppNavigationServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static AppNavigationServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AppNavigationServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AppNavigationServiceFutureStub>() {
        @java.lang.Override
        public AppNavigationServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AppNavigationServiceFutureStub(channel, callOptions);
        }
      };
    return AppNavigationServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void foregroundFQNsFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundFqns> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getForegroundFQNsFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void touchFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.TouchEvent> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getTouchFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void enabledSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getEnabledSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void keyboardVisibleFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getKeyboardVisibleFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void foregroundClassNameFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.ListStringResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getForegroundClassNameFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void foregroundClassesFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundClasses> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getForegroundClassesFlowSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void performBackButton(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getPerformBackButtonMethod(), responseObserver);
    }

    /**
     */
    default void getForegroundFqns(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundFqns> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetForegroundFqnsMethod(), responseObserver);
    }

    /**
     */
    default void setCurrentForegroundFQN(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetCurrentForegroundFQNMethod(), responseObserver);
    }

    /**
     */
    default void removeCurrentForegroundFQN(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRemoveCurrentForegroundFQNMethod(), responseObserver);
    }

    /**
     */
    default void removeForegroundFQNFromHistory(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRemoveForegroundFQNFromHistoryMethod(), responseObserver);
    }

    /**
     */
    default void setCurrentForegroundClass(com.ifit.glassos.appnavigation.ForegroundClassNameRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetCurrentForegroundClassMethod(), responseObserver);
    }

    /**
     */
    default void navigatedToThirdParty(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getNavigatedToThirdPartyMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service AppNavigationService.
   */
  public static abstract class AppNavigationServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return AppNavigationServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service AppNavigationService.
   */
  public static final class AppNavigationServiceStub
      extends io.grpc.stub.AbstractAsyncStub<AppNavigationServiceStub> {
    private AppNavigationServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AppNavigationServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AppNavigationServiceStub(channel, callOptions);
    }

    /**
     */
    public void foregroundFQNsFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundFqns> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getForegroundFQNsFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void touchFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.TouchEvent> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getTouchFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void enabledSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getEnabledSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void keyboardVisibleFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getKeyboardVisibleFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void foregroundClassNameFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.ListStringResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getForegroundClassNameFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void foregroundClassesFlowSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundClasses> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getForegroundClassesFlowSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void performBackButton(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getPerformBackButtonMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getForegroundFqns(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundFqns> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetForegroundFqnsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setCurrentForegroundFQN(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetCurrentForegroundFQNMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void removeCurrentForegroundFQN(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRemoveCurrentForegroundFQNMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void removeForegroundFQNFromHistory(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRemoveForegroundFQNFromHistoryMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setCurrentForegroundClass(com.ifit.glassos.appnavigation.ForegroundClassNameRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetCurrentForegroundClassMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void navigatedToThirdParty(com.ifit.glassos.appnavigation.ForegroundFqnRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getNavigatedToThirdPartyMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service AppNavigationService.
   */
  public static final class AppNavigationServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<AppNavigationServiceBlockingStub> {
    private AppNavigationServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AppNavigationServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AppNavigationServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.appnavigation.ForegroundFqns> foregroundFQNsFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getForegroundFQNsFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.appnavigation.TouchEvent> touchFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getTouchFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> enabledSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getEnabledSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.BooleanResponse> keyboardVisibleFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getKeyboardVisibleFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.ListStringResponse> foregroundClassNameFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getForegroundClassNameFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.appnavigation.ForegroundClasses> foregroundClassesFlowSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getForegroundClassesFlowSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty performBackButton(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getPerformBackButtonMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.appnavigation.ForegroundFqns getForegroundFqns(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetForegroundFqnsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setCurrentForegroundFQN(com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetCurrentForegroundFQNMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty removeCurrentForegroundFQN(com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRemoveCurrentForegroundFQNMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty removeForegroundFQNFromHistory(com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRemoveForegroundFQNFromHistoryMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty setCurrentForegroundClass(com.ifit.glassos.appnavigation.ForegroundClassNameRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetCurrentForegroundClassMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty navigatedToThirdParty(com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getNavigatedToThirdPartyMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service AppNavigationService.
   */
  public static final class AppNavigationServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<AppNavigationServiceFutureStub> {
    private AppNavigationServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AppNavigationServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AppNavigationServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> performBackButton(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getPerformBackButtonMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.appnavigation.ForegroundFqns> getForegroundFqns(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetForegroundFqnsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setCurrentForegroundFQN(
        com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetCurrentForegroundFQNMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> removeCurrentForegroundFQN(
        com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRemoveCurrentForegroundFQNMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> removeForegroundFQNFromHistory(
        com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRemoveForegroundFQNFromHistoryMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> setCurrentForegroundClass(
        com.ifit.glassos.appnavigation.ForegroundClassNameRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetCurrentForegroundClassMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> navigatedToThirdParty(
        com.ifit.glassos.appnavigation.ForegroundFqnRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getNavigatedToThirdPartyMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_FOREGROUND_FQNS_FLOW_SUBSCRIPTION = 0;
  private static final int METHODID_TOUCH_FLOW_SUBSCRIPTION = 1;
  private static final int METHODID_ENABLED_SUBSCRIPTION = 2;
  private static final int METHODID_KEYBOARD_VISIBLE_FLOW_SUBSCRIPTION = 3;
  private static final int METHODID_FOREGROUND_CLASS_NAME_FLOW_SUBSCRIPTION = 4;
  private static final int METHODID_FOREGROUND_CLASSES_FLOW_SUBSCRIPTION = 5;
  private static final int METHODID_PERFORM_BACK_BUTTON = 6;
  private static final int METHODID_GET_FOREGROUND_FQNS = 7;
  private static final int METHODID_SET_CURRENT_FOREGROUND_FQN = 8;
  private static final int METHODID_REMOVE_CURRENT_FOREGROUND_FQN = 9;
  private static final int METHODID_REMOVE_FOREGROUND_FQNFROM_HISTORY = 10;
  private static final int METHODID_SET_CURRENT_FOREGROUND_CLASS = 11;
  private static final int METHODID_NAVIGATED_TO_THIRD_PARTY = 12;

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
        case METHODID_FOREGROUND_FQNS_FLOW_SUBSCRIPTION:
          serviceImpl.foregroundFQNsFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundFqns>) responseObserver);
          break;
        case METHODID_TOUCH_FLOW_SUBSCRIPTION:
          serviceImpl.touchFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.TouchEvent>) responseObserver);
          break;
        case METHODID_ENABLED_SUBSCRIPTION:
          serviceImpl.enabledSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_KEYBOARD_VISIBLE_FLOW_SUBSCRIPTION:
          serviceImpl.keyboardVisibleFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_FOREGROUND_CLASS_NAME_FLOW_SUBSCRIPTION:
          serviceImpl.foregroundClassNameFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.ListStringResponse>) responseObserver);
          break;
        case METHODID_FOREGROUND_CLASSES_FLOW_SUBSCRIPTION:
          serviceImpl.foregroundClassesFlowSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundClasses>) responseObserver);
          break;
        case METHODID_PERFORM_BACK_BUTTON:
          serviceImpl.performBackButton((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_FOREGROUND_FQNS:
          serviceImpl.getForegroundFqns((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.appnavigation.ForegroundFqns>) responseObserver);
          break;
        case METHODID_SET_CURRENT_FOREGROUND_FQN:
          serviceImpl.setCurrentForegroundFQN((com.ifit.glassos.appnavigation.ForegroundFqnRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_REMOVE_CURRENT_FOREGROUND_FQN:
          serviceImpl.removeCurrentForegroundFQN((com.ifit.glassos.appnavigation.ForegroundFqnRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_REMOVE_FOREGROUND_FQNFROM_HISTORY:
          serviceImpl.removeForegroundFQNFromHistory((com.ifit.glassos.appnavigation.ForegroundFqnRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_SET_CURRENT_FOREGROUND_CLASS:
          serviceImpl.setCurrentForegroundClass((com.ifit.glassos.appnavigation.ForegroundClassNameRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_NAVIGATED_TO_THIRD_PARTY:
          serviceImpl.navigatedToThirdParty((com.ifit.glassos.appnavigation.ForegroundFqnRequest) request,
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
          getForegroundFQNsFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appnavigation.ForegroundFqns>(
                service, METHODID_FOREGROUND_FQNS_FLOW_SUBSCRIPTION)))
        .addMethod(
          getTouchFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appnavigation.TouchEvent>(
                service, METHODID_TOUCH_FLOW_SUBSCRIPTION)))
        .addMethod(
          getEnabledSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_ENABLED_SUBSCRIPTION)))
        .addMethod(
          getKeyboardVisibleFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_KEYBOARD_VISIBLE_FLOW_SUBSCRIPTION)))
        .addMethod(
          getForegroundClassNameFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.ListStringResponse>(
                service, METHODID_FOREGROUND_CLASS_NAME_FLOW_SUBSCRIPTION)))
        .addMethod(
          getForegroundClassesFlowSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appnavigation.ForegroundClasses>(
                service, METHODID_FOREGROUND_CLASSES_FLOW_SUBSCRIPTION)))
        .addMethod(
          getPerformBackButtonMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_PERFORM_BACK_BUTTON)))
        .addMethod(
          getGetForegroundFqnsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.appnavigation.ForegroundFqns>(
                service, METHODID_GET_FOREGROUND_FQNS)))
        .addMethod(
          getSetCurrentForegroundFQNMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appnavigation.ForegroundFqnRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_CURRENT_FOREGROUND_FQN)))
        .addMethod(
          getRemoveCurrentForegroundFQNMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appnavigation.ForegroundFqnRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_REMOVE_CURRENT_FOREGROUND_FQN)))
        .addMethod(
          getRemoveForegroundFQNFromHistoryMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appnavigation.ForegroundFqnRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_REMOVE_FOREGROUND_FQNFROM_HISTORY)))
        .addMethod(
          getSetCurrentForegroundClassMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appnavigation.ForegroundClassNameRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SET_CURRENT_FOREGROUND_CLASS)))
        .addMethod(
          getNavigatedToThirdPartyMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.appnavigation.ForegroundFqnRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_NAVIGATED_TO_THIRD_PARTY)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (AppNavigationServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getForegroundFQNsFlowSubscriptionMethod())
              .addMethod(getTouchFlowSubscriptionMethod())
              .addMethod(getEnabledSubscriptionMethod())
              .addMethod(getKeyboardVisibleFlowSubscriptionMethod())
              .addMethod(getForegroundClassNameFlowSubscriptionMethod())
              .addMethod(getForegroundClassesFlowSubscriptionMethod())
              .addMethod(getPerformBackButtonMethod())
              .addMethod(getGetForegroundFqnsMethod())
              .addMethod(getSetCurrentForegroundFQNMethod())
              .addMethod(getRemoveCurrentForegroundFQNMethod())
              .addMethod(getRemoveForegroundFQNFromHistoryMethod())
              .addMethod(getSetCurrentForegroundClassMethod())
              .addMethod(getNavigatedToThirdPartyMethod())
              .build();
        }
      }
    }
    return result;
  }
}

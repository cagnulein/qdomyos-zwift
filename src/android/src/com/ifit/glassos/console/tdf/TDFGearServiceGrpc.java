package com.ifit.glassos.console.tdf;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: console/tdf/TDFGearService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class TDFGearServiceGrpc {

  private TDFGearServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.TDFGearService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsSupportedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsSupported",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsSupportedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsSupportedMethod;
    if ((getIsSupportedMethod = TDFGearServiceGrpc.getIsSupportedMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getIsSupportedMethod = TDFGearServiceGrpc.getIsSupportedMethod) == null) {
          TDFGearServiceGrpc.getIsSupportedMethod = getIsSupportedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsSupported"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsSupportedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearConfigResult> getResetGearConfigMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ResetGearConfig",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFGearConfigResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearConfigResult> getResetGearConfigMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearConfigResult> getResetGearConfigMethod;
    if ((getResetGearConfigMethod = TDFGearServiceGrpc.getResetGearConfigMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getResetGearConfigMethod = TDFGearServiceGrpc.getResetGearConfigMethod) == null) {
          TDFGearServiceGrpc.getResetGearConfigMethod = getResetGearConfigMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearConfigResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ResetGearConfig"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearConfigResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getResetGearConfigMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getResetGearAndGearConfigMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ResetGearAndGearConfig",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getResetGearAndGearConfigMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getResetGearAndGearConfigMethod;
    if ((getResetGearAndGearConfigMethod = TDFGearServiceGrpc.getResetGearAndGearConfigMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getResetGearAndGearConfigMethod = TDFGearServiceGrpc.getResetGearAndGearConfigMethod) == null) {
          TDFGearServiceGrpc.getResetGearAndGearConfigMethod = getResetGearAndGearConfigMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ResetGearAndGearConfig"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getResetGearAndGearConfigMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.tdf.TDFGearConfig,
      com.ifit.glassos.console.tdf.TDFGearConfigResult> getSetGearConfigMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetGearConfig",
      requestType = com.ifit.glassos.console.tdf.TDFGearConfig.class,
      responseType = com.ifit.glassos.console.tdf.TDFGearConfigResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.tdf.TDFGearConfig,
      com.ifit.glassos.console.tdf.TDFGearConfigResult> getSetGearConfigMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.tdf.TDFGearConfig, com.ifit.glassos.console.tdf.TDFGearConfigResult> getSetGearConfigMethod;
    if ((getSetGearConfigMethod = TDFGearServiceGrpc.getSetGearConfigMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getSetGearConfigMethod = TDFGearServiceGrpc.getSetGearConfigMethod) == null) {
          TDFGearServiceGrpc.getSetGearConfigMethod = getSetGearConfigMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.tdf.TDFGearConfig, com.ifit.glassos.console.tdf.TDFGearConfigResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetGearConfig"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearConfig.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearConfigResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetGearConfigMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearConfigResult> getGetGearConfigMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetGearConfig",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFGearConfigResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearConfigResult> getGetGearConfigMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearConfigResult> getGetGearConfigMethod;
    if ((getGetGearConfigMethod = TDFGearServiceGrpc.getGetGearConfigMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getGetGearConfigMethod = TDFGearServiceGrpc.getGetGearConfigMethod) == null) {
          TDFGearServiceGrpc.getGetGearConfigMethod = getGetGearConfigMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearConfigResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetGearConfig"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearConfigResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetGearConfigMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFChainRingConfigsResult> getListFrontGearConfigsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ListFrontGearConfigs",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFChainRingConfigsResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFChainRingConfigsResult> getListFrontGearConfigsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFChainRingConfigsResult> getListFrontGearConfigsMethod;
    if ((getListFrontGearConfigsMethod = TDFGearServiceGrpc.getListFrontGearConfigsMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getListFrontGearConfigsMethod = TDFGearServiceGrpc.getListFrontGearConfigsMethod) == null) {
          TDFGearServiceGrpc.getListFrontGearConfigsMethod = getListFrontGearConfigsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFChainRingConfigsResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ListFrontGearConfigs"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFChainRingConfigsResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getListFrontGearConfigsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult> getListRearGearConfigsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ListRearGearConfigs",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult> getListRearGearConfigsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult> getListRearGearConfigsMethod;
    if ((getListRearGearConfigsMethod = TDFGearServiceGrpc.getListRearGearConfigsMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getListRearGearConfigsMethod = TDFGearServiceGrpc.getListRearGearConfigsMethod) == null) {
          TDFGearServiceGrpc.getListRearGearConfigsMethod = getListRearGearConfigsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ListRearGearConfigs"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getListRearGearConfigsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearConfig> getGearConfigChangedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GearConfigChangedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFGearConfig.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearConfig> getGearConfigChangedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearConfig> getGearConfigChangedSubscriptionMethod;
    if ((getGearConfigChangedSubscriptionMethod = TDFGearServiceGrpc.getGearConfigChangedSubscriptionMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getGearConfigChangedSubscriptionMethod = TDFGearServiceGrpc.getGearConfigChangedSubscriptionMethod) == null) {
          TDFGearServiceGrpc.getGearConfigChangedSubscriptionMethod = getGearConfigChangedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearConfig>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GearConfigChangedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearConfig.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGearConfigChangedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.console.tdf.TDFGear,
      com.ifit.glassos.console.tdf.TDFGearResult> getSetGearMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetGear",
      requestType = com.ifit.glassos.console.tdf.TDFGear.class,
      responseType = com.ifit.glassos.console.tdf.TDFGearResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.console.tdf.TDFGear,
      com.ifit.glassos.console.tdf.TDFGearResult> getSetGearMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.console.tdf.TDFGear, com.ifit.glassos.console.tdf.TDFGearResult> getSetGearMethod;
    if ((getSetGearMethod = TDFGearServiceGrpc.getSetGearMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getSetGearMethod = TDFGearServiceGrpc.getSetGearMethod) == null) {
          TDFGearServiceGrpc.getSetGearMethod = getSetGearMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.console.tdf.TDFGear, com.ifit.glassos.console.tdf.TDFGearResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetGear"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGear.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetGearMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearResult> getGetCurrentGearMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentGear",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFGearResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGearResult> getGetCurrentGearMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearResult> getGetCurrentGearMethod;
    if ((getGetCurrentGearMethod = TDFGearServiceGrpc.getGetCurrentGearMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getGetCurrentGearMethod = TDFGearServiceGrpc.getGetCurrentGearMethod) == null) {
          TDFGearServiceGrpc.getGetCurrentGearMethod = getGetCurrentGearMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGearResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentGear"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGearResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentGearMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGear> getGearChangedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GearChangedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.console.tdf.TDFGear.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.console.tdf.TDFGear> getGearChangedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGear> getGearChangedSubscriptionMethod;
    if ((getGearChangedSubscriptionMethod = TDFGearServiceGrpc.getGearChangedSubscriptionMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getGearChangedSubscriptionMethod = TDFGearServiceGrpc.getGearChangedSubscriptionMethod) == null) {
          TDFGearServiceGrpc.getGearChangedSubscriptionMethod = getGearChangedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.console.tdf.TDFGear>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GearChangedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.tdf.TDFGear.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGearChangedSubscriptionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getGetGearRatioMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetGearRatio",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.FloatResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getGetGearRatioMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse> getGetGearRatioMethod;
    if ((getGetGearRatioMethod = TDFGearServiceGrpc.getGetGearRatioMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getGetGearRatioMethod = TDFGearServiceGrpc.getGetGearRatioMethod) == null) {
          TDFGearServiceGrpc.getGetGearRatioMethod = getGetGearRatioMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetGearRatio"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.FloatResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetGearRatioMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getGearRatioChangedSubscriptionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GearRatioChangedSubscription",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.FloatResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.FloatResponse> getGearRatioChangedSubscriptionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse> getGearRatioChangedSubscriptionMethod;
    if ((getGearRatioChangedSubscriptionMethod = TDFGearServiceGrpc.getGearRatioChangedSubscriptionMethod) == null) {
      synchronized (TDFGearServiceGrpc.class) {
        if ((getGearRatioChangedSubscriptionMethod = TDFGearServiceGrpc.getGearRatioChangedSubscriptionMethod) == null) {
          TDFGearServiceGrpc.getGearRatioChangedSubscriptionMethod = getGearRatioChangedSubscriptionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.FloatResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GearRatioChangedSubscription"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.FloatResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGearRatioChangedSubscriptionMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static TDFGearServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<TDFGearServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<TDFGearServiceStub>() {
        @java.lang.Override
        public TDFGearServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new TDFGearServiceStub(channel, callOptions);
        }
      };
    return TDFGearServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static TDFGearServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<TDFGearServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<TDFGearServiceBlockingStub>() {
        @java.lang.Override
        public TDFGearServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new TDFGearServiceBlockingStub(channel, callOptions);
        }
      };
    return TDFGearServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static TDFGearServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<TDFGearServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<TDFGearServiceFutureStub>() {
        @java.lang.Override
        public TDFGearServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new TDFGearServiceFutureStub(channel, callOptions);
        }
      };
    return TDFGearServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void isSupported(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsSupportedMethod(), responseObserver);
    }

    /**
     */
    default void resetGearConfig(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getResetGearConfigMethod(), responseObserver);
    }

    /**
     */
    default void resetGearAndGearConfig(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getResetGearAndGearConfigMethod(), responseObserver);
    }

    /**
     */
    default void setGearConfig(com.ifit.glassos.console.tdf.TDFGearConfig request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetGearConfigMethod(), responseObserver);
    }

    /**
     */
    default void getGearConfig(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetGearConfigMethod(), responseObserver);
    }

    /**
     */
    default void listFrontGearConfigs(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFChainRingConfigsResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getListFrontGearConfigsMethod(), responseObserver);
    }

    /**
     */
    default void listRearGearConfigs(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getListRearGearConfigsMethod(), responseObserver);
    }

    /**
     */
    default void gearConfigChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfig> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGearConfigChangedSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void setGear(com.ifit.glassos.console.tdf.TDFGear request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetGearMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentGear(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentGearMethod(), responseObserver);
    }

    /**
     */
    default void gearChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGear> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGearChangedSubscriptionMethod(), responseObserver);
    }

    /**
     */
    default void getGearRatio(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetGearRatioMethod(), responseObserver);
    }

    /**
     */
    default void gearRatioChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGearRatioChangedSubscriptionMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service TDFGearService.
   */
  public static abstract class TDFGearServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return TDFGearServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service TDFGearService.
   */
  public static final class TDFGearServiceStub
      extends io.grpc.stub.AbstractAsyncStub<TDFGearServiceStub> {
    private TDFGearServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected TDFGearServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new TDFGearServiceStub(channel, callOptions);
    }

    /**
     */
    public void isSupported(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsSupportedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void resetGearConfig(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getResetGearConfigMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void resetGearAndGearConfig(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getResetGearAndGearConfigMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setGearConfig(com.ifit.glassos.console.tdf.TDFGearConfig request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetGearConfigMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getGearConfig(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetGearConfigMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void listFrontGearConfigs(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFChainRingConfigsResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getListFrontGearConfigsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void listRearGearConfigs(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getListRearGearConfigsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void gearConfigChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfig> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getGearConfigChangedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setGear(com.ifit.glassos.console.tdf.TDFGear request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetGearMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentGear(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentGearMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void gearChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGear> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getGearChangedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getGearRatio(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetGearRatioMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void gearRatioChangedSubscription(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getGearRatioChangedSubscriptionMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service TDFGearService.
   */
  public static final class TDFGearServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<TDFGearServiceBlockingStub> {
    private TDFGearServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected TDFGearServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new TDFGearServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isSupported(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsSupportedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFGearConfigResult resetGearConfig(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getResetGearConfigMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse resetGearAndGearConfig(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getResetGearAndGearConfigMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFGearConfigResult setGearConfig(com.ifit.glassos.console.tdf.TDFGearConfig request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetGearConfigMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFGearConfigResult getGearConfig(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetGearConfigMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFChainRingConfigsResult listFrontGearConfigs(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getListFrontGearConfigsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult listRearGearConfigs(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getListRearGearConfigsMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.tdf.TDFGearConfig> gearConfigChangedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getGearConfigChangedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFGearResult setGear(com.ifit.glassos.console.tdf.TDFGear request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetGearMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.tdf.TDFGearResult getCurrentGear(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentGearMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.console.tdf.TDFGear> gearChangedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getGearChangedSubscriptionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.FloatResponse getGearRatio(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetGearRatioMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.util.FloatResponse> gearRatioChangedSubscription(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getGearRatioChangedSubscriptionMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service TDFGearService.
   */
  public static final class TDFGearServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<TDFGearServiceFutureStub> {
    private TDFGearServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected TDFGearServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new TDFGearServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isSupported(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsSupportedMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFGearConfigResult> resetGearConfig(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getResetGearConfigMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> resetGearAndGearConfig(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getResetGearAndGearConfigMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFGearConfigResult> setGearConfig(
        com.ifit.glassos.console.tdf.TDFGearConfig request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetGearConfigMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFGearConfigResult> getGearConfig(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetGearConfigMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFChainRingConfigsResult> listFrontGearConfigs(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getListFrontGearConfigsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult> listRearGearConfigs(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getListRearGearConfigsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFGearResult> setGear(
        com.ifit.glassos.console.tdf.TDFGear request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetGearMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.tdf.TDFGearResult> getCurrentGear(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentGearMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.FloatResponse> getGearRatio(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetGearRatioMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_IS_SUPPORTED = 0;
  private static final int METHODID_RESET_GEAR_CONFIG = 1;
  private static final int METHODID_RESET_GEAR_AND_GEAR_CONFIG = 2;
  private static final int METHODID_SET_GEAR_CONFIG = 3;
  private static final int METHODID_GET_GEAR_CONFIG = 4;
  private static final int METHODID_LIST_FRONT_GEAR_CONFIGS = 5;
  private static final int METHODID_LIST_REAR_GEAR_CONFIGS = 6;
  private static final int METHODID_GEAR_CONFIG_CHANGED_SUBSCRIPTION = 7;
  private static final int METHODID_SET_GEAR = 8;
  private static final int METHODID_GET_CURRENT_GEAR = 9;
  private static final int METHODID_GEAR_CHANGED_SUBSCRIPTION = 10;
  private static final int METHODID_GET_GEAR_RATIO = 11;
  private static final int METHODID_GEAR_RATIO_CHANGED_SUBSCRIPTION = 12;

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
        case METHODID_IS_SUPPORTED:
          serviceImpl.isSupported((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_RESET_GEAR_CONFIG:
          serviceImpl.resetGearConfig((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult>) responseObserver);
          break;
        case METHODID_RESET_GEAR_AND_GEAR_CONFIG:
          serviceImpl.resetGearAndGearConfig((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_SET_GEAR_CONFIG:
          serviceImpl.setGearConfig((com.ifit.glassos.console.tdf.TDFGearConfig) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult>) responseObserver);
          break;
        case METHODID_GET_GEAR_CONFIG:
          serviceImpl.getGearConfig((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfigResult>) responseObserver);
          break;
        case METHODID_LIST_FRONT_GEAR_CONFIGS:
          serviceImpl.listFrontGearConfigs((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFChainRingConfigsResult>) responseObserver);
          break;
        case METHODID_LIST_REAR_GEAR_CONFIGS:
          serviceImpl.listRearGearConfigs((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult>) responseObserver);
          break;
        case METHODID_GEAR_CONFIG_CHANGED_SUBSCRIPTION:
          serviceImpl.gearConfigChangedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearConfig>) responseObserver);
          break;
        case METHODID_SET_GEAR:
          serviceImpl.setGear((com.ifit.glassos.console.tdf.TDFGear) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearResult>) responseObserver);
          break;
        case METHODID_GET_CURRENT_GEAR:
          serviceImpl.getCurrentGear((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGearResult>) responseObserver);
          break;
        case METHODID_GEAR_CHANGED_SUBSCRIPTION:
          serviceImpl.gearChangedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.tdf.TDFGear>) responseObserver);
          break;
        case METHODID_GET_GEAR_RATIO:
          serviceImpl.getGearRatio((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse>) responseObserver);
          break;
        case METHODID_GEAR_RATIO_CHANGED_SUBSCRIPTION:
          serviceImpl.gearRatioChangedSubscription((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.FloatResponse>) responseObserver);
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
          getIsSupportedMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_SUPPORTED)))
        .addMethod(
          getResetGearConfigMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFGearConfigResult>(
                service, METHODID_RESET_GEAR_CONFIG)))
        .addMethod(
          getResetGearAndGearConfigMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_RESET_GEAR_AND_GEAR_CONFIG)))
        .addMethod(
          getSetGearConfigMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.tdf.TDFGearConfig,
              com.ifit.glassos.console.tdf.TDFGearConfigResult>(
                service, METHODID_SET_GEAR_CONFIG)))
        .addMethod(
          getGetGearConfigMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFGearConfigResult>(
                service, METHODID_GET_GEAR_CONFIG)))
        .addMethod(
          getListFrontGearConfigsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFChainRingConfigsResult>(
                service, METHODID_LIST_FRONT_GEAR_CONFIGS)))
        .addMethod(
          getListRearGearConfigsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFRearCassetteConfigsResult>(
                service, METHODID_LIST_REAR_GEAR_CONFIGS)))
        .addMethod(
          getGearConfigChangedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFGearConfig>(
                service, METHODID_GEAR_CONFIG_CHANGED_SUBSCRIPTION)))
        .addMethod(
          getSetGearMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.console.tdf.TDFGear,
              com.ifit.glassos.console.tdf.TDFGearResult>(
                service, METHODID_SET_GEAR)))
        .addMethod(
          getGetCurrentGearMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFGearResult>(
                service, METHODID_GET_CURRENT_GEAR)))
        .addMethod(
          getGearChangedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.console.tdf.TDFGear>(
                service, METHODID_GEAR_CHANGED_SUBSCRIPTION)))
        .addMethod(
          getGetGearRatioMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.FloatResponse>(
                service, METHODID_GET_GEAR_RATIO)))
        .addMethod(
          getGearRatioChangedSubscriptionMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.FloatResponse>(
                service, METHODID_GEAR_RATIO_CHANGED_SUBSCRIPTION)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (TDFGearServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getIsSupportedMethod())
              .addMethod(getResetGearConfigMethod())
              .addMethod(getResetGearAndGearConfigMethod())
              .addMethod(getSetGearConfigMethod())
              .addMethod(getGetGearConfigMethod())
              .addMethod(getListFrontGearConfigsMethod())
              .addMethod(getListRearGearConfigsMethod())
              .addMethod(getGearConfigChangedSubscriptionMethod())
              .addMethod(getSetGearMethod())
              .addMethod(getGetCurrentGearMethod())
              .addMethod(getGearChangedSubscriptionMethod())
              .addMethod(getGetGearRatioMethod())
              .addMethod(getGearRatioChangedSubscriptionMethod())
              .build();
        }
      }
    }
    return result;
  }
}

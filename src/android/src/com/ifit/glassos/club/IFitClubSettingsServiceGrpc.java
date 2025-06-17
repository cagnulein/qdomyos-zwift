package com.ifit.glassos.club;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 * <pre>
 * Service definition for IFitClubSettingsService
 * </pre>
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: club/ClubSettingsService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class IFitClubSettingsServiceGrpc {

  private IFitClubSettingsServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.club.IFitClubSettingsService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.club.ChangeUserRoleRequest,
      com.ifit.glassos.util.Empty> getChangeUserRoleMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ChangeUserRole",
      requestType = com.ifit.glassos.club.ChangeUserRoleRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.club.ChangeUserRoleRequest,
      com.ifit.glassos.util.Empty> getChangeUserRoleMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.club.ChangeUserRoleRequest, com.ifit.glassos.util.Empty> getChangeUserRoleMethod;
    if ((getChangeUserRoleMethod = IFitClubSettingsServiceGrpc.getChangeUserRoleMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getChangeUserRoleMethod = IFitClubSettingsServiceGrpc.getChangeUserRoleMethod) == null) {
          IFitClubSettingsServiceGrpc.getChangeUserRoleMethod = getChangeUserRoleMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.club.ChangeUserRoleRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ChangeUserRole"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.ChangeUserRoleRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getChangeUserRoleMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.UserRoleResponse> getCurrentUserRoleMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CurrentUserRole",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.club.UserRoleResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.UserRoleResponse> getCurrentUserRoleMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.club.UserRoleResponse> getCurrentUserRoleMethod;
    if ((getCurrentUserRoleMethod = IFitClubSettingsServiceGrpc.getCurrentUserRoleMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getCurrentUserRoleMethod = IFitClubSettingsServiceGrpc.getCurrentUserRoleMethod) == null) {
          IFitClubSettingsServiceGrpc.getCurrentUserRoleMethod = getCurrentUserRoleMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.club.UserRoleResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CurrentUserRole"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.UserRoleResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCurrentUserRoleMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getRestoreClubOwnerDefaultSettingsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "RestoreClubOwnerDefaultSettings",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getRestoreClubOwnerDefaultSettingsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getRestoreClubOwnerDefaultSettingsMethod;
    if ((getRestoreClubOwnerDefaultSettingsMethod = IFitClubSettingsServiceGrpc.getRestoreClubOwnerDefaultSettingsMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getRestoreClubOwnerDefaultSettingsMethod = IFitClubSettingsServiceGrpc.getRestoreClubOwnerDefaultSettingsMethod) == null) {
          IFitClubSettingsServiceGrpc.getRestoreClubOwnerDefaultSettingsMethod = getRestoreClubOwnerDefaultSettingsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "RestoreClubOwnerDefaultSettings"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getRestoreClubOwnerDefaultSettingsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.GetClubCodeResponse> getGetClubCodeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetClubCode",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.club.GetClubCodeResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.GetClubCodeResponse> getGetClubCodeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.club.GetClubCodeResponse> getGetClubCodeMethod;
    if ((getGetClubCodeMethod = IFitClubSettingsServiceGrpc.getGetClubCodeMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getGetClubCodeMethod = IFitClubSettingsServiceGrpc.getGetClubCodeMethod) == null) {
          IFitClubSettingsServiceGrpc.getGetClubCodeMethod = getGetClubCodeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.club.GetClubCodeResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetClubCode"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.GetClubCodeResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetClubCodeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveClubCodeRequest,
      com.ifit.glassos.util.Empty> getSaveClubCodeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SaveClubCode",
      requestType = com.ifit.glassos.club.SaveClubCodeRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveClubCodeRequest,
      com.ifit.glassos.util.Empty> getSaveClubCodeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveClubCodeRequest, com.ifit.glassos.util.Empty> getSaveClubCodeMethod;
    if ((getSaveClubCodeMethod = IFitClubSettingsServiceGrpc.getSaveClubCodeMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getSaveClubCodeMethod = IFitClubSettingsServiceGrpc.getSaveClubCodeMethod) == null) {
          IFitClubSettingsServiceGrpc.getSaveClubCodeMethod = getSaveClubCodeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.club.SaveClubCodeRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SaveClubCode"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.SaveClubCodeRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSaveClubCodeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.GetUseVideoScreensaverResponse> getGetUseVideoScreensaverMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetUseVideoScreensaver",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.club.GetUseVideoScreensaverResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.GetUseVideoScreensaverResponse> getGetUseVideoScreensaverMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.club.GetUseVideoScreensaverResponse> getGetUseVideoScreensaverMethod;
    if ((getGetUseVideoScreensaverMethod = IFitClubSettingsServiceGrpc.getGetUseVideoScreensaverMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getGetUseVideoScreensaverMethod = IFitClubSettingsServiceGrpc.getGetUseVideoScreensaverMethod) == null) {
          IFitClubSettingsServiceGrpc.getGetUseVideoScreensaverMethod = getGetUseVideoScreensaverMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.club.GetUseVideoScreensaverResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetUseVideoScreensaver"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.GetUseVideoScreensaverResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetUseVideoScreensaverMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveUseVideoScreensaverRequest,
      com.ifit.glassos.util.Empty> getSaveUseVideoScreensaverMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SaveUseVideoScreensaver",
      requestType = com.ifit.glassos.club.SaveUseVideoScreensaverRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveUseVideoScreensaverRequest,
      com.ifit.glassos.util.Empty> getSaveUseVideoScreensaverMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveUseVideoScreensaverRequest, com.ifit.glassos.util.Empty> getSaveUseVideoScreensaverMethod;
    if ((getSaveUseVideoScreensaverMethod = IFitClubSettingsServiceGrpc.getSaveUseVideoScreensaverMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getSaveUseVideoScreensaverMethod = IFitClubSettingsServiceGrpc.getSaveUseVideoScreensaverMethod) == null) {
          IFitClubSettingsServiceGrpc.getSaveUseVideoScreensaverMethod = getSaveUseVideoScreensaverMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.club.SaveUseVideoScreensaverRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SaveUseVideoScreensaver"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.SaveUseVideoScreensaverRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSaveUseVideoScreensaverMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage,
      com.ifit.glassos.util.Empty> getSaveDefaultSystemUnitsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SaveDefaultSystemUnits",
      requestType = com.ifit.glassos.settings.SystemUnitsMessage.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage,
      com.ifit.glassos.util.Empty> getSaveDefaultSystemUnitsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage, com.ifit.glassos.util.Empty> getSaveDefaultSystemUnitsMethod;
    if ((getSaveDefaultSystemUnitsMethod = IFitClubSettingsServiceGrpc.getSaveDefaultSystemUnitsMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getSaveDefaultSystemUnitsMethod = IFitClubSettingsServiceGrpc.getSaveDefaultSystemUnitsMethod) == null) {
          IFitClubSettingsServiceGrpc.getSaveDefaultSystemUnitsMethod = getSaveDefaultSystemUnitsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.SystemUnitsMessage, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SaveDefaultSystemUnits"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.SystemUnitsMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSaveDefaultSystemUnitsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveDefaultLanguageRequest,
      com.ifit.glassos.util.Empty> getSaveDefaultLanguageMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SaveDefaultLanguage",
      requestType = com.ifit.glassos.club.SaveDefaultLanguageRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveDefaultLanguageRequest,
      com.ifit.glassos.util.Empty> getSaveDefaultLanguageMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.club.SaveDefaultLanguageRequest, com.ifit.glassos.util.Empty> getSaveDefaultLanguageMethod;
    if ((getSaveDefaultLanguageMethod = IFitClubSettingsServiceGrpc.getSaveDefaultLanguageMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getSaveDefaultLanguageMethod = IFitClubSettingsServiceGrpc.getSaveDefaultLanguageMethod) == null) {
          IFitClubSettingsServiceGrpc.getSaveDefaultLanguageMethod = getSaveDefaultLanguageMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.club.SaveDefaultLanguageRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SaveDefaultLanguage"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.SaveDefaultLanguageRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSaveDefaultLanguageMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.UserRoleResponse> getGetCurrentUserRoleMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentUserRole",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.club.UserRoleResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.club.UserRoleResponse> getGetCurrentUserRoleMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.club.UserRoleResponse> getGetCurrentUserRoleMethod;
    if ((getGetCurrentUserRoleMethod = IFitClubSettingsServiceGrpc.getGetCurrentUserRoleMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getGetCurrentUserRoleMethod = IFitClubSettingsServiceGrpc.getGetCurrentUserRoleMethod) == null) {
          IFitClubSettingsServiceGrpc.getGetCurrentUserRoleMethod = getGetCurrentUserRoleMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.club.UserRoleResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentUserRole"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.UserRoleResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentUserRoleMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getSaveAdminEgymEnabledStateMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SaveAdminEgymEnabledState",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.util.Empty> getSaveAdminEgymEnabledStateMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty> getSaveAdminEgymEnabledStateMethod;
    if ((getSaveAdminEgymEnabledStateMethod = IFitClubSettingsServiceGrpc.getSaveAdminEgymEnabledStateMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getSaveAdminEgymEnabledStateMethod = IFitClubSettingsServiceGrpc.getSaveAdminEgymEnabledStateMethod) == null) {
          IFitClubSettingsServiceGrpc.getSaveAdminEgymEnabledStateMethod = getSaveAdminEgymEnabledStateMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SaveAdminEgymEnabledState"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSaveAdminEgymEnabledStateMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsClub",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsClubMethod;
    if ((getIsClubMethod = IFitClubSettingsServiceGrpc.getIsClubMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getIsClubMethod = IFitClubSettingsServiceGrpc.getIsClubMethod) == null) {
          IFitClubSettingsServiceGrpc.getIsClubMethod = getIsClubMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsClub"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsClubMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsClubUser",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsClubUserMethod;
    if ((getIsClubUserMethod = IFitClubSettingsServiceGrpc.getIsClubUserMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getIsClubUserMethod = IFitClubSettingsServiceGrpc.getIsClubUserMethod) == null) {
          IFitClubSettingsServiceGrpc.getIsClubUserMethod = getIsClubUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsClubUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsClubUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubFreeUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsClubFreeUser",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubFreeUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsClubFreeUserMethod;
    if ((getIsClubFreeUserMethod = IFitClubSettingsServiceGrpc.getIsClubFreeUserMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getIsClubFreeUserMethod = IFitClubSettingsServiceGrpc.getIsClubFreeUserMethod) == null) {
          IFitClubSettingsServiceGrpc.getIsClubFreeUserMethod = getIsClubFreeUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsClubFreeUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsClubFreeUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubGuestMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsClubGuest",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubGuestMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsClubGuestMethod;
    if ((getIsClubGuestMethod = IFitClubSettingsServiceGrpc.getIsClubGuestMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getIsClubGuestMethod = IFitClubSettingsServiceGrpc.getIsClubGuestMethod) == null) {
          IFitClubSettingsServiceGrpc.getIsClubGuestMethod = getIsClubGuestMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsClubGuest"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsClubGuestMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubPremiumUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsClubPremiumUser",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.BooleanResponse> getIsClubPremiumUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse> getIsClubPremiumUserMethod;
    if ((getIsClubPremiumUserMethod = IFitClubSettingsServiceGrpc.getIsClubPremiumUserMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getIsClubPremiumUserMethod = IFitClubSettingsServiceGrpc.getIsClubPremiumUserMethod) == null) {
          IFitClubSettingsServiceGrpc.getIsClubPremiumUserMethod = getIsClubPremiumUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsClubPremiumUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsClubPremiumUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.club.IsEgymEnabledRequest,
      com.ifit.glassos.util.BooleanResponse> getIsEgymEnabledMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "IsEgymEnabled",
      requestType = com.ifit.glassos.club.IsEgymEnabledRequest.class,
      responseType = com.ifit.glassos.util.BooleanResponse.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.club.IsEgymEnabledRequest,
      com.ifit.glassos.util.BooleanResponse> getIsEgymEnabledMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.club.IsEgymEnabledRequest, com.ifit.glassos.util.BooleanResponse> getIsEgymEnabledMethod;
    if ((getIsEgymEnabledMethod = IFitClubSettingsServiceGrpc.getIsEgymEnabledMethod) == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        if ((getIsEgymEnabledMethod = IFitClubSettingsServiceGrpc.getIsEgymEnabledMethod) == null) {
          IFitClubSettingsServiceGrpc.getIsEgymEnabledMethod = getIsEgymEnabledMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.club.IsEgymEnabledRequest, com.ifit.glassos.util.BooleanResponse>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "IsEgymEnabled"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.club.IsEgymEnabledRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanResponse.getDefaultInstance()))
              .build();
        }
      }
    }
    return getIsEgymEnabledMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static IFitClubSettingsServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<IFitClubSettingsServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<IFitClubSettingsServiceStub>() {
        @java.lang.Override
        public IFitClubSettingsServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new IFitClubSettingsServiceStub(channel, callOptions);
        }
      };
    return IFitClubSettingsServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static IFitClubSettingsServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<IFitClubSettingsServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<IFitClubSettingsServiceBlockingStub>() {
        @java.lang.Override
        public IFitClubSettingsServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new IFitClubSettingsServiceBlockingStub(channel, callOptions);
        }
      };
    return IFitClubSettingsServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static IFitClubSettingsServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<IFitClubSettingsServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<IFitClubSettingsServiceFutureStub>() {
        @java.lang.Override
        public IFitClubSettingsServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new IFitClubSettingsServiceFutureStub(channel, callOptions);
        }
      };
    return IFitClubSettingsServiceFutureStub.newStub(factory, channel);
  }

  /**
   * <pre>
   * Service definition for IFitClubSettingsService
   * </pre>
   */
  public interface AsyncService {

    /**
     */
    default void changeUserRole(com.ifit.glassos.club.ChangeUserRoleRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getChangeUserRoleMethod(), responseObserver);
    }

    /**
     */
    default void currentUserRole(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.UserRoleResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCurrentUserRoleMethod(), responseObserver);
    }

    /**
     */
    default void restoreClubOwnerDefaultSettings(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getRestoreClubOwnerDefaultSettingsMethod(), responseObserver);
    }

    /**
     */
    default void getClubCode(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.GetClubCodeResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetClubCodeMethod(), responseObserver);
    }

    /**
     */
    default void saveClubCode(com.ifit.glassos.club.SaveClubCodeRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSaveClubCodeMethod(), responseObserver);
    }

    /**
     */
    default void getUseVideoScreensaver(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.GetUseVideoScreensaverResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetUseVideoScreensaverMethod(), responseObserver);
    }

    /**
     */
    default void saveUseVideoScreensaver(com.ifit.glassos.club.SaveUseVideoScreensaverRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSaveUseVideoScreensaverMethod(), responseObserver);
    }

    /**
     */
    default void saveDefaultSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSaveDefaultSystemUnitsMethod(), responseObserver);
    }

    /**
     */
    default void saveDefaultLanguage(com.ifit.glassos.club.SaveDefaultLanguageRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSaveDefaultLanguageMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentUserRole(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.UserRoleResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentUserRoleMethod(), responseObserver);
    }

    /**
     */
    default void saveAdminEgymEnabledState(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSaveAdminEgymEnabledStateMethod(), responseObserver);
    }

    /**
     */
    default void isClub(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsClubMethod(), responseObserver);
    }

    /**
     */
    default void isClubUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsClubUserMethod(), responseObserver);
    }

    /**
     */
    default void isClubFreeUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsClubFreeUserMethod(), responseObserver);
    }

    /**
     */
    default void isClubGuest(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsClubGuestMethod(), responseObserver);
    }

    /**
     */
    default void isClubPremiumUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsClubPremiumUserMethod(), responseObserver);
    }

    /**
     */
    default void isEgymEnabled(com.ifit.glassos.club.IsEgymEnabledRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getIsEgymEnabledMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service IFitClubSettingsService.
   * <pre>
   * Service definition for IFitClubSettingsService
   * </pre>
   */
  public static abstract class IFitClubSettingsServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return IFitClubSettingsServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service IFitClubSettingsService.
   * <pre>
   * Service definition for IFitClubSettingsService
   * </pre>
   */
  public static final class IFitClubSettingsServiceStub
      extends io.grpc.stub.AbstractAsyncStub<IFitClubSettingsServiceStub> {
    private IFitClubSettingsServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected IFitClubSettingsServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new IFitClubSettingsServiceStub(channel, callOptions);
    }

    /**
     */
    public void changeUserRole(com.ifit.glassos.club.ChangeUserRoleRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getChangeUserRoleMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void currentUserRole(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.UserRoleResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getCurrentUserRoleMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void restoreClubOwnerDefaultSettings(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getRestoreClubOwnerDefaultSettingsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getClubCode(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.GetClubCodeResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetClubCodeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void saveClubCode(com.ifit.glassos.club.SaveClubCodeRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSaveClubCodeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getUseVideoScreensaver(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.GetUseVideoScreensaverResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetUseVideoScreensaverMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void saveUseVideoScreensaver(com.ifit.glassos.club.SaveUseVideoScreensaverRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSaveUseVideoScreensaverMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void saveDefaultSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSaveDefaultSystemUnitsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void saveDefaultLanguage(com.ifit.glassos.club.SaveDefaultLanguageRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSaveDefaultLanguageMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentUserRole(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.club.UserRoleResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentUserRoleMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void saveAdminEgymEnabledState(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSaveAdminEgymEnabledStateMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isClub(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsClubMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isClubUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsClubUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isClubFreeUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsClubFreeUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isClubGuest(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsClubGuestMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isClubPremiumUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsClubPremiumUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void isEgymEnabled(com.ifit.glassos.club.IsEgymEnabledRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getIsEgymEnabledMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service IFitClubSettingsService.
   * <pre>
   * Service definition for IFitClubSettingsService
   * </pre>
   */
  public static final class IFitClubSettingsServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<IFitClubSettingsServiceBlockingStub> {
    private IFitClubSettingsServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected IFitClubSettingsServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new IFitClubSettingsServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.util.Empty changeUserRole(com.ifit.glassos.club.ChangeUserRoleRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getChangeUserRoleMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.club.UserRoleResponse> currentUserRole(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getCurrentUserRoleMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty restoreClubOwnerDefaultSettings(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getRestoreClubOwnerDefaultSettingsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.club.GetClubCodeResponse getClubCode(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetClubCodeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty saveClubCode(com.ifit.glassos.club.SaveClubCodeRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSaveClubCodeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.club.GetUseVideoScreensaverResponse getUseVideoScreensaver(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetUseVideoScreensaverMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty saveUseVideoScreensaver(com.ifit.glassos.club.SaveUseVideoScreensaverRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSaveUseVideoScreensaverMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty saveDefaultSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSaveDefaultSystemUnitsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty saveDefaultLanguage(com.ifit.glassos.club.SaveDefaultLanguageRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSaveDefaultLanguageMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.club.UserRoleResponse getCurrentUserRole(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentUserRoleMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty saveAdminEgymEnabledState(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSaveAdminEgymEnabledStateMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isClub(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsClubMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isClubUser(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsClubUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isClubFreeUser(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsClubFreeUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isClubGuest(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsClubGuestMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isClubPremiumUser(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsClubPremiumUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.BooleanResponse isEgymEnabled(com.ifit.glassos.club.IsEgymEnabledRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getIsEgymEnabledMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service IFitClubSettingsService.
   * <pre>
   * Service definition for IFitClubSettingsService
   * </pre>
   */
  public static final class IFitClubSettingsServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<IFitClubSettingsServiceFutureStub> {
    private IFitClubSettingsServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected IFitClubSettingsServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new IFitClubSettingsServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> changeUserRole(
        com.ifit.glassos.club.ChangeUserRoleRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getChangeUserRoleMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> restoreClubOwnerDefaultSettings(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getRestoreClubOwnerDefaultSettingsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.club.GetClubCodeResponse> getClubCode(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetClubCodeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> saveClubCode(
        com.ifit.glassos.club.SaveClubCodeRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSaveClubCodeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.club.GetUseVideoScreensaverResponse> getUseVideoScreensaver(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetUseVideoScreensaverMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> saveUseVideoScreensaver(
        com.ifit.glassos.club.SaveUseVideoScreensaverRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSaveUseVideoScreensaverMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> saveDefaultSystemUnits(
        com.ifit.glassos.settings.SystemUnitsMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSaveDefaultSystemUnitsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> saveDefaultLanguage(
        com.ifit.glassos.club.SaveDefaultLanguageRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSaveDefaultLanguageMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.club.UserRoleResponse> getCurrentUserRole(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentUserRoleMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> saveAdminEgymEnabledState(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSaveAdminEgymEnabledStateMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isClub(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsClubMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isClubUser(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsClubUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isClubFreeUser(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsClubFreeUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isClubGuest(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsClubGuestMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isClubPremiumUser(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsClubPremiumUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.BooleanResponse> isEgymEnabled(
        com.ifit.glassos.club.IsEgymEnabledRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getIsEgymEnabledMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_CHANGE_USER_ROLE = 0;
  private static final int METHODID_CURRENT_USER_ROLE = 1;
  private static final int METHODID_RESTORE_CLUB_OWNER_DEFAULT_SETTINGS = 2;
  private static final int METHODID_GET_CLUB_CODE = 3;
  private static final int METHODID_SAVE_CLUB_CODE = 4;
  private static final int METHODID_GET_USE_VIDEO_SCREENSAVER = 5;
  private static final int METHODID_SAVE_USE_VIDEO_SCREENSAVER = 6;
  private static final int METHODID_SAVE_DEFAULT_SYSTEM_UNITS = 7;
  private static final int METHODID_SAVE_DEFAULT_LANGUAGE = 8;
  private static final int METHODID_GET_CURRENT_USER_ROLE = 9;
  private static final int METHODID_SAVE_ADMIN_EGYM_ENABLED_STATE = 10;
  private static final int METHODID_IS_CLUB = 11;
  private static final int METHODID_IS_CLUB_USER = 12;
  private static final int METHODID_IS_CLUB_FREE_USER = 13;
  private static final int METHODID_IS_CLUB_GUEST = 14;
  private static final int METHODID_IS_CLUB_PREMIUM_USER = 15;
  private static final int METHODID_IS_EGYM_ENABLED = 16;

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
        case METHODID_CHANGE_USER_ROLE:
          serviceImpl.changeUserRole((com.ifit.glassos.club.ChangeUserRoleRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_CURRENT_USER_ROLE:
          serviceImpl.currentUserRole((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.club.UserRoleResponse>) responseObserver);
          break;
        case METHODID_RESTORE_CLUB_OWNER_DEFAULT_SETTINGS:
          serviceImpl.restoreClubOwnerDefaultSettings((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_CLUB_CODE:
          serviceImpl.getClubCode((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.club.GetClubCodeResponse>) responseObserver);
          break;
        case METHODID_SAVE_CLUB_CODE:
          serviceImpl.saveClubCode((com.ifit.glassos.club.SaveClubCodeRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_USE_VIDEO_SCREENSAVER:
          serviceImpl.getUseVideoScreensaver((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.club.GetUseVideoScreensaverResponse>) responseObserver);
          break;
        case METHODID_SAVE_USE_VIDEO_SCREENSAVER:
          serviceImpl.saveUseVideoScreensaver((com.ifit.glassos.club.SaveUseVideoScreensaverRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_SAVE_DEFAULT_SYSTEM_UNITS:
          serviceImpl.saveDefaultSystemUnits((com.ifit.glassos.settings.SystemUnitsMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_SAVE_DEFAULT_LANGUAGE:
          serviceImpl.saveDefaultLanguage((com.ifit.glassos.club.SaveDefaultLanguageRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_CURRENT_USER_ROLE:
          serviceImpl.getCurrentUserRole((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.club.UserRoleResponse>) responseObserver);
          break;
        case METHODID_SAVE_ADMIN_EGYM_ENABLED_STATE:
          serviceImpl.saveAdminEgymEnabledState((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_IS_CLUB:
          serviceImpl.isClub((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_IS_CLUB_USER:
          serviceImpl.isClubUser((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_IS_CLUB_FREE_USER:
          serviceImpl.isClubFreeUser((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_IS_CLUB_GUEST:
          serviceImpl.isClubGuest((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_IS_CLUB_PREMIUM_USER:
          serviceImpl.isClubPremiumUser((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
          break;
        case METHODID_IS_EGYM_ENABLED:
          serviceImpl.isEgymEnabled((com.ifit.glassos.club.IsEgymEnabledRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.BooleanResponse>) responseObserver);
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
          getChangeUserRoleMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.club.ChangeUserRoleRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_CHANGE_USER_ROLE)))
        .addMethod(
          getCurrentUserRoleMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.club.UserRoleResponse>(
                service, METHODID_CURRENT_USER_ROLE)))
        .addMethod(
          getRestoreClubOwnerDefaultSettingsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_RESTORE_CLUB_OWNER_DEFAULT_SETTINGS)))
        .addMethod(
          getGetClubCodeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.club.GetClubCodeResponse>(
                service, METHODID_GET_CLUB_CODE)))
        .addMethod(
          getSaveClubCodeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.club.SaveClubCodeRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SAVE_CLUB_CODE)))
        .addMethod(
          getGetUseVideoScreensaverMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.club.GetUseVideoScreensaverResponse>(
                service, METHODID_GET_USE_VIDEO_SCREENSAVER)))
        .addMethod(
          getSaveUseVideoScreensaverMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.club.SaveUseVideoScreensaverRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SAVE_USE_VIDEO_SCREENSAVER)))
        .addMethod(
          getSaveDefaultSystemUnitsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.SystemUnitsMessage,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SAVE_DEFAULT_SYSTEM_UNITS)))
        .addMethod(
          getSaveDefaultLanguageMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.club.SaveDefaultLanguageRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SAVE_DEFAULT_LANGUAGE)))
        .addMethod(
          getGetCurrentUserRoleMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.club.UserRoleResponse>(
                service, METHODID_GET_CURRENT_USER_ROLE)))
        .addMethod(
          getSaveAdminEgymEnabledStateMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.util.Empty>(
                service, METHODID_SAVE_ADMIN_EGYM_ENABLED_STATE)))
        .addMethod(
          getIsClubMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_CLUB)))
        .addMethod(
          getIsClubUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_CLUB_USER)))
        .addMethod(
          getIsClubFreeUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_CLUB_FREE_USER)))
        .addMethod(
          getIsClubGuestMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_CLUB_GUEST)))
        .addMethod(
          getIsClubPremiumUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_CLUB_PREMIUM_USER)))
        .addMethod(
          getIsEgymEnabledMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.club.IsEgymEnabledRequest,
              com.ifit.glassos.util.BooleanResponse>(
                service, METHODID_IS_EGYM_ENABLED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (IFitClubSettingsServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getChangeUserRoleMethod())
              .addMethod(getCurrentUserRoleMethod())
              .addMethod(getRestoreClubOwnerDefaultSettingsMethod())
              .addMethod(getGetClubCodeMethod())
              .addMethod(getSaveClubCodeMethod())
              .addMethod(getGetUseVideoScreensaverMethod())
              .addMethod(getSaveUseVideoScreensaverMethod())
              .addMethod(getSaveDefaultSystemUnitsMethod())
              .addMethod(getSaveDefaultLanguageMethod())
              .addMethod(getGetCurrentUserRoleMethod())
              .addMethod(getSaveAdminEgymEnabledStateMethod())
              .addMethod(getIsClubMethod())
              .addMethod(getIsClubUserMethod())
              .addMethod(getIsClubFreeUserMethod())
              .addMethod(getIsClubGuestMethod())
              .addMethod(getIsClubPremiumUserMethod())
              .addMethod(getIsEgymEnabledMethod())
              .build();
        }
      }
    }
    return result;
  }
}

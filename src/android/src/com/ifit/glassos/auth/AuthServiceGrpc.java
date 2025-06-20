package com.ifit.glassos.auth;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: auth/AuthService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class AuthServiceGrpc {

  private AuthServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.AuthService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.auth.UserCredentials,
      com.ifit.glassos.auth.AuthResult> getLoginMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Login",
      requestType = com.ifit.glassos.auth.UserCredentials.class,
      responseType = com.ifit.glassos.auth.AuthResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.auth.UserCredentials,
      com.ifit.glassos.auth.AuthResult> getLoginMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.auth.UserCredentials, com.ifit.glassos.auth.AuthResult> getLoginMethod;
    if ((getLoginMethod = AuthServiceGrpc.getLoginMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getLoginMethod = AuthServiceGrpc.getLoginMethod) == null) {
          AuthServiceGrpc.getLoginMethod = getLoginMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.auth.UserCredentials, com.ifit.glassos.auth.AuthResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Login"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.UserCredentials.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getLoginMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.auth.Username,
      com.ifit.glassos.auth.AuthResult> getSwitchUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SwitchUser",
      requestType = com.ifit.glassos.auth.Username.class,
      responseType = com.ifit.glassos.auth.AuthResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.auth.Username,
      com.ifit.glassos.auth.AuthResult> getSwitchUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.auth.Username, com.ifit.glassos.auth.AuthResult> getSwitchUserMethod;
    if ((getSwitchUserMethod = AuthServiceGrpc.getSwitchUserMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getSwitchUserMethod = AuthServiceGrpc.getSwitchUserMethod) == null) {
          AuthServiceGrpc.getSwitchUserMethod = getSwitchUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.auth.Username, com.ifit.glassos.auth.AuthResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SwitchUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.Username.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSwitchUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.auth.AuthCredentials,
      com.ifit.glassos.auth.AuthResult> getSetCredentialsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetCredentials",
      requestType = com.ifit.glassos.auth.AuthCredentials.class,
      responseType = com.ifit.glassos.auth.AuthResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.auth.AuthCredentials,
      com.ifit.glassos.auth.AuthResult> getSetCredentialsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.auth.AuthCredentials, com.ifit.glassos.auth.AuthResult> getSetCredentialsMethod;
    if ((getSetCredentialsMethod = AuthServiceGrpc.getSetCredentialsMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getSetCredentialsMethod = AuthServiceGrpc.getSetCredentialsMethod) == null) {
          AuthServiceGrpc.getSetCredentialsMethod = getSetCredentialsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.auth.AuthCredentials, com.ifit.glassos.auth.AuthResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetCredentials"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthCredentials.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetCredentialsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getLogoutMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "Logout",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getLogoutMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getLogoutMethod;
    if ((getLogoutMethod = AuthServiceGrpc.getLogoutMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getLogoutMethod = AuthServiceGrpc.getLogoutMethod) == null) {
          AuthServiceGrpc.getLogoutMethod = getLogoutMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "Logout"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getLogoutMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthQRCodeResult> getGetQRCodeDataMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetQRCodeData",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.auth.AuthQRCodeResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthQRCodeResult> getGetQRCodeDataMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthQRCodeResult> getGetQRCodeDataMethod;
    if ((getGetQRCodeDataMethod = AuthServiceGrpc.getGetQRCodeDataMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getGetQRCodeDataMethod = AuthServiceGrpc.getGetQRCodeDataMethod) == null) {
          AuthServiceGrpc.getGetQRCodeDataMethod = getGetQRCodeDataMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthQRCodeResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetQRCodeData"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthQRCodeResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetQRCodeDataMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getStopPollingForQRAuthTokenMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StopPollingForQRAuthToken",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.util.Empty.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.util.Empty> getStopPollingForQRAuthTokenMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty> getStopPollingForQRAuthTokenMethod;
    if ((getStopPollingForQRAuthTokenMethod = AuthServiceGrpc.getStopPollingForQRAuthTokenMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getStopPollingForQRAuthTokenMethod = AuthServiceGrpc.getStopPollingForQRAuthTokenMethod) == null) {
          AuthServiceGrpc.getStopPollingForQRAuthTokenMethod = getStopPollingForQRAuthTokenMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.util.Empty>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StopPollingForQRAuthToken"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStopPollingForQRAuthTokenMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthQRCodePollingState> getQrCodePollingStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "QrCodePollingStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.auth.AuthQRCodePollingState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthQRCodePollingState> getQrCodePollingStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthQRCodePollingState> getQrCodePollingStateChangedMethod;
    if ((getQrCodePollingStateChangedMethod = AuthServiceGrpc.getQrCodePollingStateChangedMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getQrCodePollingStateChangedMethod = AuthServiceGrpc.getQrCodePollingStateChangedMethod) == null) {
          AuthServiceGrpc.getQrCodePollingStateChangedMethod = getQrCodePollingStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthQRCodePollingState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "QrCodePollingStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthQRCodePollingState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getQrCodePollingStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthToken> getTokenChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "TokenChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.auth.AuthToken.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthToken> getTokenChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthToken> getTokenChangedMethod;
    if ((getTokenChangedMethod = AuthServiceGrpc.getTokenChangedMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getTokenChangedMethod = AuthServiceGrpc.getTokenChangedMethod) == null) {
          AuthServiceGrpc.getTokenChangedMethod = getTokenChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthToken>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "TokenChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthToken.getDefaultInstance()))
              .build();
        }
      }
    }
    return getTokenChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.auth.GetCurrentTokenRequest,
      com.ifit.glassos.auth.AuthResult> getGetCurrentTokenMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentToken",
      requestType = com.ifit.glassos.auth.GetCurrentTokenRequest.class,
      responseType = com.ifit.glassos.auth.AuthResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.auth.GetCurrentTokenRequest,
      com.ifit.glassos.auth.AuthResult> getGetCurrentTokenMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.auth.GetCurrentTokenRequest, com.ifit.glassos.auth.AuthResult> getGetCurrentTokenMethod;
    if ((getGetCurrentTokenMethod = AuthServiceGrpc.getGetCurrentTokenMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getGetCurrentTokenMethod = AuthServiceGrpc.getGetCurrentTokenMethod) == null) {
          AuthServiceGrpc.getGetCurrentTokenMethod = getGetCurrentTokenMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.auth.GetCurrentTokenRequest, com.ifit.glassos.auth.AuthResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentToken"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.GetCurrentTokenRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentTokenMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthTokenList> getGetAllTokensMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetAllTokens",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.auth.AuthTokenList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.AuthTokenList> getGetAllTokensMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthTokenList> getGetAllTokensMethod;
    if ((getGetAllTokensMethod = AuthServiceGrpc.getGetAllTokensMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getGetAllTokensMethod = AuthServiceGrpc.getGetAllTokensMethod) == null) {
          AuthServiceGrpc.getGetAllTokensMethod = getGetAllTokensMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.AuthTokenList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetAllTokens"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.AuthTokenList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetAllTokensMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.MachineToken> getMachineTokenChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "MachineTokenChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.auth.MachineToken.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.MachineToken> getMachineTokenChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.MachineToken> getMachineTokenChangedMethod;
    if ((getMachineTokenChangedMethod = AuthServiceGrpc.getMachineTokenChangedMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getMachineTokenChangedMethod = AuthServiceGrpc.getMachineTokenChangedMethod) == null) {
          AuthServiceGrpc.getMachineTokenChangedMethod = getMachineTokenChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.MachineToken>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "MachineTokenChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.MachineToken.getDefaultInstance()))
              .build();
        }
      }
    }
    return getMachineTokenChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.MachineTokenResult> getGetMachineTokenMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetMachineToken",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.auth.MachineTokenResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.auth.MachineTokenResult> getGetMachineTokenMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.MachineTokenResult> getGetMachineTokenMethod;
    if ((getGetMachineTokenMethod = AuthServiceGrpc.getGetMachineTokenMethod) == null) {
      synchronized (AuthServiceGrpc.class) {
        if ((getGetMachineTokenMethod = AuthServiceGrpc.getGetMachineTokenMethod) == null) {
          AuthServiceGrpc.getGetMachineTokenMethod = getGetMachineTokenMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.auth.MachineTokenResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetMachineToken"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.auth.MachineTokenResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetMachineTokenMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static AuthServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AuthServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AuthServiceStub>() {
        @java.lang.Override
        public AuthServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AuthServiceStub(channel, callOptions);
        }
      };
    return AuthServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static AuthServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AuthServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AuthServiceBlockingStub>() {
        @java.lang.Override
        public AuthServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AuthServiceBlockingStub(channel, callOptions);
        }
      };
    return AuthServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static AuthServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<AuthServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<AuthServiceFutureStub>() {
        @java.lang.Override
        public AuthServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new AuthServiceFutureStub(channel, callOptions);
        }
      };
    return AuthServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void login(com.ifit.glassos.auth.UserCredentials request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getLoginMethod(), responseObserver);
    }

    /**
     */
    default void switchUser(com.ifit.glassos.auth.Username request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSwitchUserMethod(), responseObserver);
    }

    /**
     */
    default void setCredentials(com.ifit.glassos.auth.AuthCredentials request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetCredentialsMethod(), responseObserver);
    }

    /**
     */
    default void logout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getLogoutMethod(), responseObserver);
    }

    /**
     */
    default void getQRCodeData(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthQRCodeResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetQRCodeDataMethod(), responseObserver);
    }

    /**
     */
    default void stopPollingForQRAuthToken(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStopPollingForQRAuthTokenMethod(), responseObserver);
    }

    /**
     */
    default void qrCodePollingStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthQRCodePollingState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getQrCodePollingStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void tokenChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthToken> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getTokenChangedMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentToken(com.ifit.glassos.auth.GetCurrentTokenRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentTokenMethod(), responseObserver);
    }

    /**
     */
    default void getAllTokens(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthTokenList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetAllTokensMethod(), responseObserver);
    }

    /**
     */
    default void machineTokenChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.MachineToken> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getMachineTokenChangedMethod(), responseObserver);
    }

    /**
     */
    default void getMachineToken(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.MachineTokenResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetMachineTokenMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service AuthService.
   */
  public static abstract class AuthServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return AuthServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service AuthService.
   */
  public static final class AuthServiceStub
      extends io.grpc.stub.AbstractAsyncStub<AuthServiceStub> {
    private AuthServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AuthServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AuthServiceStub(channel, callOptions);
    }

    /**
     */
    public void login(com.ifit.glassos.auth.UserCredentials request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getLoginMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void switchUser(com.ifit.glassos.auth.Username request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSwitchUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setCredentials(com.ifit.glassos.auth.AuthCredentials request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetCredentialsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void logout(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getLogoutMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getQRCodeData(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthQRCodeResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetQRCodeDataMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void stopPollingForQRAuthToken(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStopPollingForQRAuthTokenMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void qrCodePollingStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthQRCodePollingState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getQrCodePollingStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void tokenChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthToken> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getTokenChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentToken(com.ifit.glassos.auth.GetCurrentTokenRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentTokenMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getAllTokens(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthTokenList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetAllTokensMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void machineTokenChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.MachineToken> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getMachineTokenChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getMachineToken(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.auth.MachineTokenResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetMachineTokenMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service AuthService.
   */
  public static final class AuthServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<AuthServiceBlockingStub> {
    private AuthServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AuthServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AuthServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.auth.AuthResult login(com.ifit.glassos.auth.UserCredentials request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getLoginMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.auth.AuthResult switchUser(com.ifit.glassos.auth.Username request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSwitchUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.auth.AuthResult setCredentials(com.ifit.glassos.auth.AuthCredentials request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetCredentialsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty logout(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getLogoutMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.auth.AuthQRCodeResult getQRCodeData(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetQRCodeDataMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.util.Empty stopPollingForQRAuthToken(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStopPollingForQRAuthTokenMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.auth.AuthQRCodePollingState> qrCodePollingStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getQrCodePollingStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.auth.AuthToken> tokenChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getTokenChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.auth.AuthResult getCurrentToken(com.ifit.glassos.auth.GetCurrentTokenRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentTokenMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.auth.AuthTokenList getAllTokens(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetAllTokensMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.auth.MachineToken> machineTokenChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getMachineTokenChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.auth.MachineTokenResult getMachineToken(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetMachineTokenMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service AuthService.
   */
  public static final class AuthServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<AuthServiceFutureStub> {
    private AuthServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected AuthServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new AuthServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.AuthResult> login(
        com.ifit.glassos.auth.UserCredentials request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getLoginMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.AuthResult> switchUser(
        com.ifit.glassos.auth.Username request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSwitchUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.AuthResult> setCredentials(
        com.ifit.glassos.auth.AuthCredentials request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetCredentialsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> logout(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getLogoutMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.AuthQRCodeResult> getQRCodeData(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetQRCodeDataMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.util.Empty> stopPollingForQRAuthToken(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStopPollingForQRAuthTokenMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.AuthResult> getCurrentToken(
        com.ifit.glassos.auth.GetCurrentTokenRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentTokenMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.AuthTokenList> getAllTokens(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetAllTokensMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.auth.MachineTokenResult> getMachineToken(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetMachineTokenMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_LOGIN = 0;
  private static final int METHODID_SWITCH_USER = 1;
  private static final int METHODID_SET_CREDENTIALS = 2;
  private static final int METHODID_LOGOUT = 3;
  private static final int METHODID_GET_QRCODE_DATA = 4;
  private static final int METHODID_STOP_POLLING_FOR_QRAUTH_TOKEN = 5;
  private static final int METHODID_QR_CODE_POLLING_STATE_CHANGED = 6;
  private static final int METHODID_TOKEN_CHANGED = 7;
  private static final int METHODID_GET_CURRENT_TOKEN = 8;
  private static final int METHODID_GET_ALL_TOKENS = 9;
  private static final int METHODID_MACHINE_TOKEN_CHANGED = 10;
  private static final int METHODID_GET_MACHINE_TOKEN = 11;

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
        case METHODID_LOGIN:
          serviceImpl.login((com.ifit.glassos.auth.UserCredentials) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult>) responseObserver);
          break;
        case METHODID_SWITCH_USER:
          serviceImpl.switchUser((com.ifit.glassos.auth.Username) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult>) responseObserver);
          break;
        case METHODID_SET_CREDENTIALS:
          serviceImpl.setCredentials((com.ifit.glassos.auth.AuthCredentials) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult>) responseObserver);
          break;
        case METHODID_LOGOUT:
          serviceImpl.logout((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_GET_QRCODE_DATA:
          serviceImpl.getQRCodeData((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthQRCodeResult>) responseObserver);
          break;
        case METHODID_STOP_POLLING_FOR_QRAUTH_TOKEN:
          serviceImpl.stopPollingForQRAuthToken((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.util.Empty>) responseObserver);
          break;
        case METHODID_QR_CODE_POLLING_STATE_CHANGED:
          serviceImpl.qrCodePollingStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthQRCodePollingState>) responseObserver);
          break;
        case METHODID_TOKEN_CHANGED:
          serviceImpl.tokenChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthToken>) responseObserver);
          break;
        case METHODID_GET_CURRENT_TOKEN:
          serviceImpl.getCurrentToken((com.ifit.glassos.auth.GetCurrentTokenRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthResult>) responseObserver);
          break;
        case METHODID_GET_ALL_TOKENS:
          serviceImpl.getAllTokens((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.AuthTokenList>) responseObserver);
          break;
        case METHODID_MACHINE_TOKEN_CHANGED:
          serviceImpl.machineTokenChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.MachineToken>) responseObserver);
          break;
        case METHODID_GET_MACHINE_TOKEN:
          serviceImpl.getMachineToken((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.auth.MachineTokenResult>) responseObserver);
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
          getLoginMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.auth.UserCredentials,
              com.ifit.glassos.auth.AuthResult>(
                service, METHODID_LOGIN)))
        .addMethod(
          getSwitchUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.auth.Username,
              com.ifit.glassos.auth.AuthResult>(
                service, METHODID_SWITCH_USER)))
        .addMethod(
          getSetCredentialsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.auth.AuthCredentials,
              com.ifit.glassos.auth.AuthResult>(
                service, METHODID_SET_CREDENTIALS)))
        .addMethod(
          getLogoutMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_LOGOUT)))
        .addMethod(
          getGetQRCodeDataMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.auth.AuthQRCodeResult>(
                service, METHODID_GET_QRCODE_DATA)))
        .addMethod(
          getStopPollingForQRAuthTokenMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.util.Empty>(
                service, METHODID_STOP_POLLING_FOR_QRAUTH_TOKEN)))
        .addMethod(
          getQrCodePollingStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.auth.AuthQRCodePollingState>(
                service, METHODID_QR_CODE_POLLING_STATE_CHANGED)))
        .addMethod(
          getTokenChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.auth.AuthToken>(
                service, METHODID_TOKEN_CHANGED)))
        .addMethod(
          getGetCurrentTokenMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.auth.GetCurrentTokenRequest,
              com.ifit.glassos.auth.AuthResult>(
                service, METHODID_GET_CURRENT_TOKEN)))
        .addMethod(
          getGetAllTokensMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.auth.AuthTokenList>(
                service, METHODID_GET_ALL_TOKENS)))
        .addMethod(
          getMachineTokenChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.auth.MachineToken>(
                service, METHODID_MACHINE_TOKEN_CHANGED)))
        .addMethod(
          getGetMachineTokenMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.auth.MachineTokenResult>(
                service, METHODID_GET_MACHINE_TOKEN)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (AuthServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getLoginMethod())
              .addMethod(getSwitchUserMethod())
              .addMethod(getSetCredentialsMethod())
              .addMethod(getLogoutMethod())
              .addMethod(getGetQRCodeDataMethod())
              .addMethod(getStopPollingForQRAuthTokenMethod())
              .addMethod(getQrCodePollingStateChangedMethod())
              .addMethod(getTokenChangedMethod())
              .addMethod(getGetCurrentTokenMethod())
              .addMethod(getGetAllTokensMethod())
              .addMethod(getMachineTokenChangedMethod())
              .addMethod(getGetMachineTokenMethod())
              .build();
        }
      }
    }
    return result;
  }
}

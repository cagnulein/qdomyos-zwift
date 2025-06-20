package com.ifit.glassos.user;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: user/UserService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class UserServiceGrpc {

  private UserServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.UserService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.GetUserRequest,
      com.ifit.glassos.user.UserResult> getGetCurrentUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentUser",
      requestType = com.ifit.glassos.user.GetUserRequest.class,
      responseType = com.ifit.glassos.user.UserResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.GetUserRequest,
      com.ifit.glassos.user.UserResult> getGetCurrentUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.GetUserRequest, com.ifit.glassos.user.UserResult> getGetCurrentUserMethod;
    if ((getGetCurrentUserMethod = UserServiceGrpc.getGetCurrentUserMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetCurrentUserMethod = UserServiceGrpc.getGetCurrentUserMethod) == null) {
          UserServiceGrpc.getGetCurrentUserMethod = getGetCurrentUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.GetUserRequest, com.ifit.glassos.user.UserResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.GetUserRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.UserResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.GetUserRequest,
      com.ifit.glassos.user.UserResult> getGetCurrentUserWithResultMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetCurrentUserWithResult",
      requestType = com.ifit.glassos.user.GetUserRequest.class,
      responseType = com.ifit.glassos.user.UserResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.GetUserRequest,
      com.ifit.glassos.user.UserResult> getGetCurrentUserWithResultMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.GetUserRequest, com.ifit.glassos.user.UserResult> getGetCurrentUserWithResultMethod;
    if ((getGetCurrentUserWithResultMethod = UserServiceGrpc.getGetCurrentUserWithResultMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetCurrentUserWithResultMethod = UserServiceGrpc.getGetCurrentUserWithResultMethod) == null) {
          UserServiceGrpc.getGetCurrentUserWithResultMethod = getGetCurrentUserWithResultMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.GetUserRequest, com.ifit.glassos.user.UserResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetCurrentUserWithResult"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.GetUserRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.UserResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetCurrentUserWithResultMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.User> getCurrentUserChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "CurrentUserChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.user.User.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.User> getCurrentUserChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.user.User> getCurrentUserChangedMethod;
    if ((getCurrentUserChangedMethod = UserServiceGrpc.getCurrentUserChangedMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getCurrentUserChangedMethod = UserServiceGrpc.getCurrentUserChangedMethod) == null) {
          UserServiceGrpc.getCurrentUserChangedMethod = getCurrentUserChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.user.User>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "CurrentUserChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.User.getDefaultInstance()))
              .build();
        }
      }
    }
    return getCurrentUserChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserListResult> getGetSecondaryUsersMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSecondaryUsers",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.user.UserListResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserListResult> getGetSecondaryUsersMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserListResult> getGetSecondaryUsersMethod;
    if ((getGetSecondaryUsersMethod = UserServiceGrpc.getGetSecondaryUsersMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetSecondaryUsersMethod = UserServiceGrpc.getGetSecondaryUsersMethod) == null) {
          UserServiceGrpc.getGetSecondaryUsersMethod = getGetSecondaryUsersMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserListResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSecondaryUsers"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.UserListResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSecondaryUsersMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserResult> getGetPrimaryUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetPrimaryUser",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.user.UserResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserResult> getGetPrimaryUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserResult> getGetPrimaryUserMethod;
    if ((getGetPrimaryUserMethod = UserServiceGrpc.getGetPrimaryUserMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetPrimaryUserMethod = UserServiceGrpc.getGetPrimaryUserMethod) == null) {
          UserServiceGrpc.getGetPrimaryUserMethod = getGetPrimaryUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetPrimaryUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.UserResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetPrimaryUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.EmailAddress,
      com.ifit.glassos.user.EmailAddressResult> getInviteSecondaryUserMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "InviteSecondaryUser",
      requestType = com.ifit.glassos.user.EmailAddress.class,
      responseType = com.ifit.glassos.user.EmailAddressResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.EmailAddress,
      com.ifit.glassos.user.EmailAddressResult> getInviteSecondaryUserMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.EmailAddress, com.ifit.glassos.user.EmailAddressResult> getInviteSecondaryUserMethod;
    if ((getInviteSecondaryUserMethod = UserServiceGrpc.getInviteSecondaryUserMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getInviteSecondaryUserMethod = UserServiceGrpc.getInviteSecondaryUserMethod) == null) {
          UserServiceGrpc.getInviteSecondaryUserMethod = getInviteSecondaryUserMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.EmailAddress, com.ifit.glassos.user.EmailAddressResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "InviteSecondaryUser"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.EmailAddress.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.EmailAddressResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getInviteSecondaryUserMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserInvitedListResult> getGetSecondaryInvitedUsersMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetSecondaryInvitedUsers",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.user.UserInvitedListResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserInvitedListResult> getGetSecondaryInvitedUsersMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserInvitedListResult> getGetSecondaryInvitedUsersMethod;
    if ((getGetSecondaryInvitedUsersMethod = UserServiceGrpc.getGetSecondaryInvitedUsersMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetSecondaryInvitedUsersMethod = UserServiceGrpc.getGetSecondaryInvitedUsersMethod) == null) {
          UserServiceGrpc.getGetSecondaryInvitedUsersMethod = getGetSecondaryInvitedUsersMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserInvitedListResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetSecondaryInvitedUsers"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.UserInvitedListResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetSecondaryInvitedUsersMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserListResult> getGetAllLinkedUsersMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetAllLinkedUsers",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.user.UserListResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.UserListResult> getGetAllLinkedUsersMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserListResult> getGetAllLinkedUsersMethod;
    if ((getGetAllLinkedUsersMethod = UserServiceGrpc.getGetAllLinkedUsersMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetAllLinkedUsersMethod = UserServiceGrpc.getGetAllLinkedUsersMethod) == null) {
          UserServiceGrpc.getGetAllLinkedUsersMethod = getGetAllLinkedUsersMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.user.UserListResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetAllLinkedUsers"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.UserListResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetAllLinkedUsersMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.RequiredTermsVersionResult> getGetRequiredTermsVersionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetRequiredTermsVersion",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.user.RequiredTermsVersionResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.user.RequiredTermsVersionResult> getGetRequiredTermsVersionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.user.RequiredTermsVersionResult> getGetRequiredTermsVersionMethod;
    if ((getGetRequiredTermsVersionMethod = UserServiceGrpc.getGetRequiredTermsVersionMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getGetRequiredTermsVersionMethod = UserServiceGrpc.getGetRequiredTermsVersionMethod) == null) {
          UserServiceGrpc.getGetRequiredTermsVersionMethod = getGetRequiredTermsVersionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.user.RequiredTermsVersionResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetRequiredTermsVersion"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.RequiredTermsVersionResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetRequiredTermsVersionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.Name,
      com.ifit.glassos.user.NameResult> getSetNameMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetName",
      requestType = com.ifit.glassos.user.Name.class,
      responseType = com.ifit.glassos.user.NameResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.Name,
      com.ifit.glassos.user.NameResult> getSetNameMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.Name, com.ifit.glassos.user.NameResult> getSetNameMethod;
    if ((getSetNameMethod = UserServiceGrpc.getSetNameMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetNameMethod = UserServiceGrpc.getSetNameMethod) == null) {
          UserServiceGrpc.getSetNameMethod = getSetNameMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.Name, com.ifit.glassos.user.NameResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetName"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.Name.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.NameResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetNameMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.Gender,
      com.ifit.glassos.user.GenderResult> getSetGenderMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetGender",
      requestType = com.ifit.glassos.user.Gender.class,
      responseType = com.ifit.glassos.user.GenderResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.Gender,
      com.ifit.glassos.user.GenderResult> getSetGenderMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.Gender, com.ifit.glassos.user.GenderResult> getSetGenderMethod;
    if ((getSetGenderMethod = UserServiceGrpc.getSetGenderMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetGenderMethod = UserServiceGrpc.getSetGenderMethod) == null) {
          UserServiceGrpc.getSetGenderMethod = getSetGenderMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.Gender, com.ifit.glassos.user.GenderResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetGender"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.Gender.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.GenderResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetGenderMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.Birthday,
      com.ifit.glassos.user.BirthdayResult> getSetBirthdayMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetBirthday",
      requestType = com.ifit.glassos.user.Birthday.class,
      responseType = com.ifit.glassos.user.BirthdayResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.Birthday,
      com.ifit.glassos.user.BirthdayResult> getSetBirthdayMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.Birthday, com.ifit.glassos.user.BirthdayResult> getSetBirthdayMethod;
    if ((getSetBirthdayMethod = UserServiceGrpc.getSetBirthdayMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetBirthdayMethod = UserServiceGrpc.getSetBirthdayMethod) == null) {
          UserServiceGrpc.getSetBirthdayMethod = getSetBirthdayMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.Birthday, com.ifit.glassos.user.BirthdayResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetBirthday"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.Birthday.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.BirthdayResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetBirthdayMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.Weight,
      com.ifit.glassos.user.WeightResult> getSetWeightMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetWeight",
      requestType = com.ifit.glassos.user.Weight.class,
      responseType = com.ifit.glassos.user.WeightResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.Weight,
      com.ifit.glassos.user.WeightResult> getSetWeightMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.Weight, com.ifit.glassos.user.WeightResult> getSetWeightMethod;
    if ((getSetWeightMethod = UserServiceGrpc.getSetWeightMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetWeightMethod = UserServiceGrpc.getSetWeightMethod) == null) {
          UserServiceGrpc.getSetWeightMethod = getSetWeightMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.Weight, com.ifit.glassos.user.WeightResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetWeight"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.Weight.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.WeightResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetWeightMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.Height,
      com.ifit.glassos.user.HeightResult> getSetHeightMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetHeight",
      requestType = com.ifit.glassos.user.Height.class,
      responseType = com.ifit.glassos.user.HeightResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.Height,
      com.ifit.glassos.user.HeightResult> getSetHeightMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.Height, com.ifit.glassos.user.HeightResult> getSetHeightMethod;
    if ((getSetHeightMethod = UserServiceGrpc.getSetHeightMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetHeightMethod = UserServiceGrpc.getSetHeightMethod) == null) {
          UserServiceGrpc.getSetHeightMethod = getSetHeightMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.Height, com.ifit.glassos.user.HeightResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetHeight"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.Height.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.HeightResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetHeightMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage,
      com.ifit.glassos.settings.SystemUnitsResult> getSetSystemUnitsMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetSystemUnits",
      requestType = com.ifit.glassos.settings.SystemUnitsMessage.class,
      responseType = com.ifit.glassos.settings.SystemUnitsResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage,
      com.ifit.glassos.settings.SystemUnitsResult> getSetSystemUnitsMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.settings.SystemUnitsMessage, com.ifit.glassos.settings.SystemUnitsResult> getSetSystemUnitsMethod;
    if ((getSetSystemUnitsMethod = UserServiceGrpc.getSetSystemUnitsMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetSystemUnitsMethod = UserServiceGrpc.getSetSystemUnitsMethod) == null) {
          UserServiceGrpc.getSetSystemUnitsMethod = getSetSystemUnitsMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.settings.SystemUnitsMessage, com.ifit.glassos.settings.SystemUnitsResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetSystemUnits"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.SystemUnitsMessage.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.settings.SystemUnitsResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetSystemUnitsMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.user.WarningAcceptance,
      com.ifit.glassos.user.WarningAcceptanceResult> getSetWarningAcceptanceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetWarningAcceptance",
      requestType = com.ifit.glassos.user.WarningAcceptance.class,
      responseType = com.ifit.glassos.user.WarningAcceptanceResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.user.WarningAcceptance,
      com.ifit.glassos.user.WarningAcceptanceResult> getSetWarningAcceptanceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.user.WarningAcceptance, com.ifit.glassos.user.WarningAcceptanceResult> getSetWarningAcceptanceMethod;
    if ((getSetWarningAcceptanceMethod = UserServiceGrpc.getSetWarningAcceptanceMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetWarningAcceptanceMethod = UserServiceGrpc.getSetWarningAcceptanceMethod) == null) {
          UserServiceGrpc.getSetWarningAcceptanceMethod = getSetWarningAcceptanceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.user.WarningAcceptance, com.ifit.glassos.user.WarningAcceptanceResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetWarningAcceptance"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.WarningAcceptance.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.user.WarningAcceptanceResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetWarningAcceptanceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetPrivateModeMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetPrivateMode",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.console.useractivity.UserActivityServiceResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetPrivateModeMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetPrivateModeMethod;
    if ((getSetPrivateModeMethod = UserServiceGrpc.getSetPrivateModeMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetPrivateModeMethod = UserServiceGrpc.getSetPrivateModeMethod) == null) {
          UserServiceGrpc.getSetPrivateModeMethod = getSetPrivateModeMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetPrivateMode"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityServiceResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetPrivateModeMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.IntRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetLatestAcceptedTermsVersionMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetLatestAcceptedTermsVersion",
      requestType = com.ifit.glassos.util.IntRequest.class,
      responseType = com.ifit.glassos.console.useractivity.UserActivityServiceResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.IntRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetLatestAcceptedTermsVersionMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.IntRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetLatestAcceptedTermsVersionMethod;
    if ((getSetLatestAcceptedTermsVersionMethod = UserServiceGrpc.getSetLatestAcceptedTermsVersionMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetLatestAcceptedTermsVersionMethod = UserServiceGrpc.getSetLatestAcceptedTermsVersionMethod) == null) {
          UserServiceGrpc.getSetLatestAcceptedTermsVersionMethod = getSetLatestAcceptedTermsVersionMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.IntRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetLatestAcceptedTermsVersion"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.IntRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityServiceResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetLatestAcceptedTermsVersionMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetSkipFSBWarningMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "SetSkipFSBWarning",
      requestType = com.ifit.glassos.util.BooleanRequest.class,
      responseType = com.ifit.glassos.console.useractivity.UserActivityServiceResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest,
      com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetSkipFSBWarningMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult> getSetSkipFSBWarningMethod;
    if ((getSetSkipFSBWarningMethod = UserServiceGrpc.getSetSkipFSBWarningMethod) == null) {
      synchronized (UserServiceGrpc.class) {
        if ((getSetSkipFSBWarningMethod = UserServiceGrpc.getSetSkipFSBWarningMethod) == null) {
          UserServiceGrpc.getSetSkipFSBWarningMethod = getSetSkipFSBWarningMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.BooleanRequest, com.ifit.glassos.console.useractivity.UserActivityServiceResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "SetSkipFSBWarning"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.BooleanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.console.useractivity.UserActivityServiceResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getSetSkipFSBWarningMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static UserServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<UserServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<UserServiceStub>() {
        @java.lang.Override
        public UserServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new UserServiceStub(channel, callOptions);
        }
      };
    return UserServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static UserServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<UserServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<UserServiceBlockingStub>() {
        @java.lang.Override
        public UserServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new UserServiceBlockingStub(channel, callOptions);
        }
      };
    return UserServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static UserServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<UserServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<UserServiceFutureStub>() {
        @java.lang.Override
        public UserServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new UserServiceFutureStub(channel, callOptions);
        }
      };
    return UserServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void getCurrentUser(com.ifit.glassos.user.GetUserRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentUserMethod(), responseObserver);
    }

    /**
     */
    default void getCurrentUserWithResult(com.ifit.glassos.user.GetUserRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetCurrentUserWithResultMethod(), responseObserver);
    }

    /**
     */
    default void currentUserChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.User> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getCurrentUserChangedMethod(), responseObserver);
    }

    /**
     */
    default void getSecondaryUsers(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserListResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSecondaryUsersMethod(), responseObserver);
    }

    /**
     */
    default void getPrimaryUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetPrimaryUserMethod(), responseObserver);
    }

    /**
     */
    default void inviteSecondaryUser(com.ifit.glassos.user.EmailAddress request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.EmailAddressResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getInviteSecondaryUserMethod(), responseObserver);
    }

    /**
     */
    default void getSecondaryInvitedUsers(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserInvitedListResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetSecondaryInvitedUsersMethod(), responseObserver);
    }

    /**
     */
    default void getAllLinkedUsers(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserListResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetAllLinkedUsersMethod(), responseObserver);
    }

    /**
     */
    default void getRequiredTermsVersion(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.RequiredTermsVersionResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetRequiredTermsVersionMethod(), responseObserver);
    }

    /**
     */
    default void setName(com.ifit.glassos.user.Name request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.NameResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetNameMethod(), responseObserver);
    }

    /**
     */
    default void setGender(com.ifit.glassos.user.Gender request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.GenderResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetGenderMethod(), responseObserver);
    }

    /**
     */
    default void setBirthday(com.ifit.glassos.user.Birthday request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.BirthdayResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetBirthdayMethod(), responseObserver);
    }

    /**
     */
    default void setWeight(com.ifit.glassos.user.Weight request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.WeightResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetWeightMethod(), responseObserver);
    }

    /**
     */
    default void setHeight(com.ifit.glassos.user.Height request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.HeightResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetHeightMethod(), responseObserver);
    }

    /**
     */
    default void setSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetSystemUnitsMethod(), responseObserver);
    }

    /**
     */
    default void setWarningAcceptance(com.ifit.glassos.user.WarningAcceptance request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.WarningAcceptanceResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetWarningAcceptanceMethod(), responseObserver);
    }

    /**
     */
    default void setPrivateMode(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetPrivateModeMethod(), responseObserver);
    }

    /**
     */
    default void setLatestAcceptedTermsVersion(com.ifit.glassos.util.IntRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetLatestAcceptedTermsVersionMethod(), responseObserver);
    }

    /**
     */
    default void setSkipFSBWarning(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getSetSkipFSBWarningMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service UserService.
   */
  public static abstract class UserServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return UserServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service UserService.
   */
  public static final class UserServiceStub
      extends io.grpc.stub.AbstractAsyncStub<UserServiceStub> {
    private UserServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected UserServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new UserServiceStub(channel, callOptions);
    }

    /**
     */
    public void getCurrentUser(com.ifit.glassos.user.GetUserRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getCurrentUserWithResult(com.ifit.glassos.user.GetUserRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetCurrentUserWithResultMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void currentUserChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.User> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getCurrentUserChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getSecondaryUsers(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserListResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSecondaryUsersMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getPrimaryUser(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetPrimaryUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void inviteSecondaryUser(com.ifit.glassos.user.EmailAddress request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.EmailAddressResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getInviteSecondaryUserMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getSecondaryInvitedUsers(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserInvitedListResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetSecondaryInvitedUsersMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getAllLinkedUsers(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserListResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetAllLinkedUsersMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getRequiredTermsVersion(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.RequiredTermsVersionResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetRequiredTermsVersionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setName(com.ifit.glassos.user.Name request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.NameResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetNameMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setGender(com.ifit.glassos.user.Gender request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.GenderResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetGenderMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setBirthday(com.ifit.glassos.user.Birthday request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.BirthdayResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetBirthdayMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setWeight(com.ifit.glassos.user.Weight request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.WeightResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetWeightMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setHeight(com.ifit.glassos.user.Height request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.HeightResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetHeightMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetSystemUnitsMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setWarningAcceptance(com.ifit.glassos.user.WarningAcceptance request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.user.WarningAcceptanceResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetWarningAcceptanceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setPrivateMode(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetPrivateModeMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setLatestAcceptedTermsVersion(com.ifit.glassos.util.IntRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetLatestAcceptedTermsVersionMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void setSkipFSBWarning(com.ifit.glassos.util.BooleanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getSetSkipFSBWarningMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service UserService.
   */
  public static final class UserServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<UserServiceBlockingStub> {
    private UserServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected UserServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new UserServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public com.ifit.glassos.user.UserResult getCurrentUser(com.ifit.glassos.user.GetUserRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.UserResult getCurrentUserWithResult(com.ifit.glassos.user.GetUserRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetCurrentUserWithResultMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.user.User> currentUserChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getCurrentUserChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.UserListResult getSecondaryUsers(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSecondaryUsersMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.UserResult getPrimaryUser(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetPrimaryUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.EmailAddressResult inviteSecondaryUser(com.ifit.glassos.user.EmailAddress request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getInviteSecondaryUserMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.UserInvitedListResult getSecondaryInvitedUsers(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetSecondaryInvitedUsersMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.UserListResult getAllLinkedUsers(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetAllLinkedUsersMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.RequiredTermsVersionResult getRequiredTermsVersion(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetRequiredTermsVersionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.NameResult setName(com.ifit.glassos.user.Name request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetNameMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.GenderResult setGender(com.ifit.glassos.user.Gender request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetGenderMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.BirthdayResult setBirthday(com.ifit.glassos.user.Birthday request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetBirthdayMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.WeightResult setWeight(com.ifit.glassos.user.Weight request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetWeightMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.HeightResult setHeight(com.ifit.glassos.user.Height request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetHeightMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.settings.SystemUnitsResult setSystemUnits(com.ifit.glassos.settings.SystemUnitsMessage request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetSystemUnitsMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.user.WarningAcceptanceResult setWarningAcceptance(com.ifit.glassos.user.WarningAcceptance request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetWarningAcceptanceMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.useractivity.UserActivityServiceResult setPrivateMode(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetPrivateModeMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.useractivity.UserActivityServiceResult setLatestAcceptedTermsVersion(com.ifit.glassos.util.IntRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetLatestAcceptedTermsVersionMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.console.useractivity.UserActivityServiceResult setSkipFSBWarning(com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getSetSkipFSBWarningMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service UserService.
   */
  public static final class UserServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<UserServiceFutureStub> {
    private UserServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected UserServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new UserServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.UserResult> getCurrentUser(
        com.ifit.glassos.user.GetUserRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.UserResult> getCurrentUserWithResult(
        com.ifit.glassos.user.GetUserRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetCurrentUserWithResultMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.UserListResult> getSecondaryUsers(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSecondaryUsersMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.UserResult> getPrimaryUser(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetPrimaryUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.EmailAddressResult> inviteSecondaryUser(
        com.ifit.glassos.user.EmailAddress request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getInviteSecondaryUserMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.UserInvitedListResult> getSecondaryInvitedUsers(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetSecondaryInvitedUsersMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.UserListResult> getAllLinkedUsers(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetAllLinkedUsersMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.RequiredTermsVersionResult> getRequiredTermsVersion(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetRequiredTermsVersionMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.NameResult> setName(
        com.ifit.glassos.user.Name request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetNameMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.GenderResult> setGender(
        com.ifit.glassos.user.Gender request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetGenderMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.BirthdayResult> setBirthday(
        com.ifit.glassos.user.Birthday request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetBirthdayMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.WeightResult> setWeight(
        com.ifit.glassos.user.Weight request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetWeightMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.HeightResult> setHeight(
        com.ifit.glassos.user.Height request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetHeightMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.settings.SystemUnitsResult> setSystemUnits(
        com.ifit.glassos.settings.SystemUnitsMessage request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetSystemUnitsMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.user.WarningAcceptanceResult> setWarningAcceptance(
        com.ifit.glassos.user.WarningAcceptance request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetWarningAcceptanceMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.useractivity.UserActivityServiceResult> setPrivateMode(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetPrivateModeMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.useractivity.UserActivityServiceResult> setLatestAcceptedTermsVersion(
        com.ifit.glassos.util.IntRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetLatestAcceptedTermsVersionMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.console.useractivity.UserActivityServiceResult> setSkipFSBWarning(
        com.ifit.glassos.util.BooleanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getSetSkipFSBWarningMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_GET_CURRENT_USER = 0;
  private static final int METHODID_GET_CURRENT_USER_WITH_RESULT = 1;
  private static final int METHODID_CURRENT_USER_CHANGED = 2;
  private static final int METHODID_GET_SECONDARY_USERS = 3;
  private static final int METHODID_GET_PRIMARY_USER = 4;
  private static final int METHODID_INVITE_SECONDARY_USER = 5;
  private static final int METHODID_GET_SECONDARY_INVITED_USERS = 6;
  private static final int METHODID_GET_ALL_LINKED_USERS = 7;
  private static final int METHODID_GET_REQUIRED_TERMS_VERSION = 8;
  private static final int METHODID_SET_NAME = 9;
  private static final int METHODID_SET_GENDER = 10;
  private static final int METHODID_SET_BIRTHDAY = 11;
  private static final int METHODID_SET_WEIGHT = 12;
  private static final int METHODID_SET_HEIGHT = 13;
  private static final int METHODID_SET_SYSTEM_UNITS = 14;
  private static final int METHODID_SET_WARNING_ACCEPTANCE = 15;
  private static final int METHODID_SET_PRIVATE_MODE = 16;
  private static final int METHODID_SET_LATEST_ACCEPTED_TERMS_VERSION = 17;
  private static final int METHODID_SET_SKIP_FSBWARNING = 18;

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
        case METHODID_GET_CURRENT_USER:
          serviceImpl.getCurrentUser((com.ifit.glassos.user.GetUserRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult>) responseObserver);
          break;
        case METHODID_GET_CURRENT_USER_WITH_RESULT:
          serviceImpl.getCurrentUserWithResult((com.ifit.glassos.user.GetUserRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult>) responseObserver);
          break;
        case METHODID_CURRENT_USER_CHANGED:
          serviceImpl.currentUserChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.User>) responseObserver);
          break;
        case METHODID_GET_SECONDARY_USERS:
          serviceImpl.getSecondaryUsers((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserListResult>) responseObserver);
          break;
        case METHODID_GET_PRIMARY_USER:
          serviceImpl.getPrimaryUser((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserResult>) responseObserver);
          break;
        case METHODID_INVITE_SECONDARY_USER:
          serviceImpl.inviteSecondaryUser((com.ifit.glassos.user.EmailAddress) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.EmailAddressResult>) responseObserver);
          break;
        case METHODID_GET_SECONDARY_INVITED_USERS:
          serviceImpl.getSecondaryInvitedUsers((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserInvitedListResult>) responseObserver);
          break;
        case METHODID_GET_ALL_LINKED_USERS:
          serviceImpl.getAllLinkedUsers((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.UserListResult>) responseObserver);
          break;
        case METHODID_GET_REQUIRED_TERMS_VERSION:
          serviceImpl.getRequiredTermsVersion((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.RequiredTermsVersionResult>) responseObserver);
          break;
        case METHODID_SET_NAME:
          serviceImpl.setName((com.ifit.glassos.user.Name) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.NameResult>) responseObserver);
          break;
        case METHODID_SET_GENDER:
          serviceImpl.setGender((com.ifit.glassos.user.Gender) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.GenderResult>) responseObserver);
          break;
        case METHODID_SET_BIRTHDAY:
          serviceImpl.setBirthday((com.ifit.glassos.user.Birthday) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.BirthdayResult>) responseObserver);
          break;
        case METHODID_SET_WEIGHT:
          serviceImpl.setWeight((com.ifit.glassos.user.Weight) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.WeightResult>) responseObserver);
          break;
        case METHODID_SET_HEIGHT:
          serviceImpl.setHeight((com.ifit.glassos.user.Height) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.HeightResult>) responseObserver);
          break;
        case METHODID_SET_SYSTEM_UNITS:
          serviceImpl.setSystemUnits((com.ifit.glassos.settings.SystemUnitsMessage) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.settings.SystemUnitsResult>) responseObserver);
          break;
        case METHODID_SET_WARNING_ACCEPTANCE:
          serviceImpl.setWarningAcceptance((com.ifit.glassos.user.WarningAcceptance) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.user.WarningAcceptanceResult>) responseObserver);
          break;
        case METHODID_SET_PRIVATE_MODE:
          serviceImpl.setPrivateMode((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult>) responseObserver);
          break;
        case METHODID_SET_LATEST_ACCEPTED_TERMS_VERSION:
          serviceImpl.setLatestAcceptedTermsVersion((com.ifit.glassos.util.IntRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult>) responseObserver);
          break;
        case METHODID_SET_SKIP_FSBWARNING:
          serviceImpl.setSkipFSBWarning((com.ifit.glassos.util.BooleanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.console.useractivity.UserActivityServiceResult>) responseObserver);
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
          getGetCurrentUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.GetUserRequest,
              com.ifit.glassos.user.UserResult>(
                service, METHODID_GET_CURRENT_USER)))
        .addMethod(
          getGetCurrentUserWithResultMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.GetUserRequest,
              com.ifit.glassos.user.UserResult>(
                service, METHODID_GET_CURRENT_USER_WITH_RESULT)))
        .addMethod(
          getCurrentUserChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.user.User>(
                service, METHODID_CURRENT_USER_CHANGED)))
        .addMethod(
          getGetSecondaryUsersMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.user.UserListResult>(
                service, METHODID_GET_SECONDARY_USERS)))
        .addMethod(
          getGetPrimaryUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.user.UserResult>(
                service, METHODID_GET_PRIMARY_USER)))
        .addMethod(
          getInviteSecondaryUserMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.EmailAddress,
              com.ifit.glassos.user.EmailAddressResult>(
                service, METHODID_INVITE_SECONDARY_USER)))
        .addMethod(
          getGetSecondaryInvitedUsersMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.user.UserInvitedListResult>(
                service, METHODID_GET_SECONDARY_INVITED_USERS)))
        .addMethod(
          getGetAllLinkedUsersMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.user.UserListResult>(
                service, METHODID_GET_ALL_LINKED_USERS)))
        .addMethod(
          getGetRequiredTermsVersionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.user.RequiredTermsVersionResult>(
                service, METHODID_GET_REQUIRED_TERMS_VERSION)))
        .addMethod(
          getSetNameMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.Name,
              com.ifit.glassos.user.NameResult>(
                service, METHODID_SET_NAME)))
        .addMethod(
          getSetGenderMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.Gender,
              com.ifit.glassos.user.GenderResult>(
                service, METHODID_SET_GENDER)))
        .addMethod(
          getSetBirthdayMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.Birthday,
              com.ifit.glassos.user.BirthdayResult>(
                service, METHODID_SET_BIRTHDAY)))
        .addMethod(
          getSetWeightMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.Weight,
              com.ifit.glassos.user.WeightResult>(
                service, METHODID_SET_WEIGHT)))
        .addMethod(
          getSetHeightMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.Height,
              com.ifit.glassos.user.HeightResult>(
                service, METHODID_SET_HEIGHT)))
        .addMethod(
          getSetSystemUnitsMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.settings.SystemUnitsMessage,
              com.ifit.glassos.settings.SystemUnitsResult>(
                service, METHODID_SET_SYSTEM_UNITS)))
        .addMethod(
          getSetWarningAcceptanceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.user.WarningAcceptance,
              com.ifit.glassos.user.WarningAcceptanceResult>(
                service, METHODID_SET_WARNING_ACCEPTANCE)))
        .addMethod(
          getSetPrivateModeMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.console.useractivity.UserActivityServiceResult>(
                service, METHODID_SET_PRIVATE_MODE)))
        .addMethod(
          getSetLatestAcceptedTermsVersionMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.IntRequest,
              com.ifit.glassos.console.useractivity.UserActivityServiceResult>(
                service, METHODID_SET_LATEST_ACCEPTED_TERMS_VERSION)))
        .addMethod(
          getSetSkipFSBWarningMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.BooleanRequest,
              com.ifit.glassos.console.useractivity.UserActivityServiceResult>(
                service, METHODID_SET_SKIP_FSBWARNING)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (UserServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getGetCurrentUserMethod())
              .addMethod(getGetCurrentUserWithResultMethod())
              .addMethod(getCurrentUserChangedMethod())
              .addMethod(getGetSecondaryUsersMethod())
              .addMethod(getGetPrimaryUserMethod())
              .addMethod(getInviteSecondaryUserMethod())
              .addMethod(getGetSecondaryInvitedUsersMethod())
              .addMethod(getGetAllLinkedUsersMethod())
              .addMethod(getGetRequiredTermsVersionMethod())
              .addMethod(getSetNameMethod())
              .addMethod(getSetGenderMethod())
              .addMethod(getSetBirthdayMethod())
              .addMethod(getSetWeightMethod())
              .addMethod(getSetHeightMethod())
              .addMethod(getSetSystemUnitsMethod())
              .addMethod(getSetWarningAcceptanceMethod())
              .addMethod(getSetPrivateModeMethod())
              .addMethod(getSetLatestAcceptedTermsVersionMethod())
              .addMethod(getSetSkipFSBWarningMethod())
              .build();
        }
      }
    }
    return result;
  }
}

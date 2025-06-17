package com.ifit.glassos.bluetooth;

import static io.grpc.MethodDescriptor.generateFullMethodName;

/**
 */
@javax.annotation.Generated(
    value = "by gRPC proto compiler (version 1.63.0)",
    comments = "Source: bluetooth/BluetoothService.proto")
@io.grpc.stub.annotations.GrpcGenerated
public final class BluetoothServiceGrpc {

  private BluetoothServiceGrpc() {}

  public static final java.lang.String SERVICE_NAME = "com.ifit.glassos.BluetoothService";

  // Static method descriptors that strictly reflect the proto.
  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothScanState> getScanStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ScanStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothScanState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothScanState> getScanStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothScanState> getScanStateChangedMethod;
    if ((getScanStateChangedMethod = BluetoothServiceGrpc.getScanStateChangedMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getScanStateChangedMethod = BluetoothServiceGrpc.getScanStateChangedMethod) == null) {
          BluetoothServiceGrpc.getScanStateChangedMethod = getScanStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothScanState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ScanStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothScanState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getScanStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothServiceState> getBluetoothServiceStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "BluetoothServiceStateChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothServiceState.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothServiceState> getBluetoothServiceStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothServiceState> getBluetoothServiceStateChangedMethod;
    if ((getBluetoothServiceStateChangedMethod = BluetoothServiceGrpc.getBluetoothServiceStateChangedMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getBluetoothServiceStateChangedMethod = BluetoothServiceGrpc.getBluetoothServiceStateChangedMethod) == null) {
          BluetoothServiceGrpc.getBluetoothServiceStateChangedMethod = getBluetoothServiceStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothServiceState>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "BluetoothServiceStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothServiceState.getDefaultInstance()))
              .build();
        }
      }
    }
    return getBluetoothServiceStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothDevice> getFoundDevicesChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "FoundDevicesChanged",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothDevice.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothDevice> getFoundDevicesChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothDevice> getFoundDevicesChangedMethod;
    if ((getFoundDevicesChangedMethod = BluetoothServiceGrpc.getFoundDevicesChangedMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getFoundDevicesChangedMethod = BluetoothServiceGrpc.getFoundDevicesChangedMethod) == null) {
          BluetoothServiceGrpc.getFoundDevicesChangedMethod = getFoundDevicesChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothDevice>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "FoundDevicesChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothDevice.getDefaultInstance()))
              .build();
        }
      }
    }
    return getFoundDevicesChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.StartScanRequest,
      com.ifit.glassos.bluetooth.BluetoothResult> getStartScanMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StartScan",
      requestType = com.ifit.glassos.bluetooth.StartScanRequest.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.StartScanRequest,
      com.ifit.glassos.bluetooth.BluetoothResult> getStartScanMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.StartScanRequest, com.ifit.glassos.bluetooth.BluetoothResult> getStartScanMethod;
    if ((getStartScanMethod = BluetoothServiceGrpc.getStartScanMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getStartScanMethod = BluetoothServiceGrpc.getStartScanMethod) == null) {
          BluetoothServiceGrpc.getStartScanMethod = getStartScanMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.StartScanRequest, com.ifit.glassos.bluetooth.BluetoothResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StartScan"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.StartScanRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStartScanMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothResult> getStopScanMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "StopScan",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothResult> getStopScanMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothResult> getStopScanMethod;
    if ((getStopScanMethod = BluetoothServiceGrpc.getStopScanMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getStopScanMethod = BluetoothServiceGrpc.getStopScanMethod) == null) {
          BluetoothServiceGrpc.getStopScanMethod = getStopScanMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "StopScan"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getStopScanMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.BluetoothDevice,
      com.ifit.glassos.bluetooth.BluetoothResult> getConnectDeviceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConnectDevice",
      requestType = com.ifit.glassos.bluetooth.BluetoothDevice.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.BluetoothDevice,
      com.ifit.glassos.bluetooth.BluetoothResult> getConnectDeviceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.BluetoothDevice, com.ifit.glassos.bluetooth.BluetoothResult> getConnectDeviceMethod;
    if ((getConnectDeviceMethod = BluetoothServiceGrpc.getConnectDeviceMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getConnectDeviceMethod = BluetoothServiceGrpc.getConnectDeviceMethod) == null) {
          BluetoothServiceGrpc.getConnectDeviceMethod = getConnectDeviceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.BluetoothDevice, com.ifit.glassos.bluetooth.BluetoothResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConnectDevice"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothDevice.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConnectDeviceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.MACAddressConnectionRequest,
      com.ifit.glassos.bluetooth.MACAddressConnectionResult> getConnectWithMACAddressMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConnectWithMACAddress",
      requestType = com.ifit.glassos.bluetooth.MACAddressConnectionRequest.class,
      responseType = com.ifit.glassos.bluetooth.MACAddressConnectionResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.MACAddressConnectionRequest,
      com.ifit.glassos.bluetooth.MACAddressConnectionResult> getConnectWithMACAddressMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.MACAddressConnectionRequest, com.ifit.glassos.bluetooth.MACAddressConnectionResult> getConnectWithMACAddressMethod;
    if ((getConnectWithMACAddressMethod = BluetoothServiceGrpc.getConnectWithMACAddressMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getConnectWithMACAddressMethod = BluetoothServiceGrpc.getConnectWithMACAddressMethod) == null) {
          BluetoothServiceGrpc.getConnectWithMACAddressMethod = getConnectWithMACAddressMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.MACAddressConnectionRequest, com.ifit.glassos.bluetooth.MACAddressConnectionResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConnectWithMACAddress"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.MACAddressConnectionRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.MACAddressConnectionResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConnectWithMACAddressMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.BluetoothDevice,
      com.ifit.glassos.bluetooth.BluetoothResult> getDisconnectDeviceMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "DisconnectDevice",
      requestType = com.ifit.glassos.bluetooth.BluetoothDevice.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.BluetoothDevice,
      com.ifit.glassos.bluetooth.BluetoothResult> getDisconnectDeviceMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.BluetoothDevice, com.ifit.glassos.bluetooth.BluetoothResult> getDisconnectDeviceMethod;
    if ((getDisconnectDeviceMethod = BluetoothServiceGrpc.getDisconnectDeviceMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getDisconnectDeviceMethod = BluetoothServiceGrpc.getDisconnectDeviceMethod) == null) {
          BluetoothServiceGrpc.getDisconnectDeviceMethod = getDisconnectDeviceMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.BluetoothDevice, com.ifit.glassos.bluetooth.BluetoothResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "DisconnectDevice"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothDevice.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getDisconnectDeviceMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceIdentifierRequest,
      com.ifit.glassos.bluetooth.BluetoothResult> getConnectToHRMMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConnectToHRM",
      requestType = com.ifit.glassos.bluetooth.DeviceIdentifierRequest.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceIdentifierRequest,
      com.ifit.glassos.bluetooth.BluetoothResult> getConnectToHRMMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceIdentifierRequest, com.ifit.glassos.bluetooth.BluetoothResult> getConnectToHRMMethod;
    if ((getConnectToHRMMethod = BluetoothServiceGrpc.getConnectToHRMMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getConnectToHRMMethod = BluetoothServiceGrpc.getConnectToHRMMethod) == null) {
          BluetoothServiceGrpc.getConnectToHRMMethod = getConnectToHRMMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.DeviceIdentifierRequest, com.ifit.glassos.bluetooth.BluetoothResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConnectToHRM"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceIdentifierRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConnectToHRMMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceIdentifierRequest,
      com.ifit.glassos.bluetooth.BluetoothResult> getConnectToRingMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "ConnectToRing",
      requestType = com.ifit.glassos.bluetooth.DeviceIdentifierRequest.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceIdentifierRequest,
      com.ifit.glassos.bluetooth.BluetoothResult> getConnectToRingMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceIdentifierRequest, com.ifit.glassos.bluetooth.BluetoothResult> getConnectToRingMethod;
    if ((getConnectToRingMethod = BluetoothServiceGrpc.getConnectToRingMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getConnectToRingMethod = BluetoothServiceGrpc.getConnectToRingMethod) == null) {
          BluetoothServiceGrpc.getConnectToRingMethod = getConnectToRingMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.DeviceIdentifierRequest, com.ifit.glassos.bluetooth.BluetoothResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "ConnectToRing"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceIdentifierRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getConnectToRingMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothDeviceList> getGetPairedDevicesMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "GetPairedDevices",
      requestType = com.ifit.glassos.util.Empty.class,
      responseType = com.ifit.glassos.bluetooth.BluetoothDeviceList.class,
      methodType = io.grpc.MethodDescriptor.MethodType.UNARY)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty,
      com.ifit.glassos.bluetooth.BluetoothDeviceList> getGetPairedDevicesMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothDeviceList> getGetPairedDevicesMethod;
    if ((getGetPairedDevicesMethod = BluetoothServiceGrpc.getGetPairedDevicesMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getGetPairedDevicesMethod = BluetoothServiceGrpc.getGetPairedDevicesMethod) == null) {
          BluetoothServiceGrpc.getGetPairedDevicesMethod = getGetPairedDevicesMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.util.Empty, com.ifit.glassos.bluetooth.BluetoothDeviceList>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.UNARY)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "GetPairedDevices"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.util.Empty.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.BluetoothDeviceList.getDefaultInstance()))
              .build();
        }
      }
    }
    return getGetPairedDevicesMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest,
      com.ifit.glassos.bluetooth.DeviceBatteryLevelResult> getBluetoothDeviceBatteryLevelChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "BluetoothDeviceBatteryLevelChanged",
      requestType = com.ifit.glassos.bluetooth.DeviceStreamRequest.class,
      responseType = com.ifit.glassos.bluetooth.DeviceBatteryLevelResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest,
      com.ifit.glassos.bluetooth.DeviceBatteryLevelResult> getBluetoothDeviceBatteryLevelChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest, com.ifit.glassos.bluetooth.DeviceBatteryLevelResult> getBluetoothDeviceBatteryLevelChangedMethod;
    if ((getBluetoothDeviceBatteryLevelChangedMethod = BluetoothServiceGrpc.getBluetoothDeviceBatteryLevelChangedMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getBluetoothDeviceBatteryLevelChangedMethod = BluetoothServiceGrpc.getBluetoothDeviceBatteryLevelChangedMethod) == null) {
          BluetoothServiceGrpc.getBluetoothDeviceBatteryLevelChangedMethod = getBluetoothDeviceBatteryLevelChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.DeviceStreamRequest, com.ifit.glassos.bluetooth.DeviceBatteryLevelResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "BluetoothDeviceBatteryLevelChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceStreamRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceBatteryLevelResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getBluetoothDeviceBatteryLevelChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest,
      com.ifit.glassos.bluetooth.DeviceConnectionStateResult> getBluetoothDeviceConnectionStateChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "BluetoothDeviceConnectionStateChanged",
      requestType = com.ifit.glassos.bluetooth.DeviceStreamRequest.class,
      responseType = com.ifit.glassos.bluetooth.DeviceConnectionStateResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest,
      com.ifit.glassos.bluetooth.DeviceConnectionStateResult> getBluetoothDeviceConnectionStateChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest, com.ifit.glassos.bluetooth.DeviceConnectionStateResult> getBluetoothDeviceConnectionStateChangedMethod;
    if ((getBluetoothDeviceConnectionStateChangedMethod = BluetoothServiceGrpc.getBluetoothDeviceConnectionStateChangedMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getBluetoothDeviceConnectionStateChangedMethod = BluetoothServiceGrpc.getBluetoothDeviceConnectionStateChangedMethod) == null) {
          BluetoothServiceGrpc.getBluetoothDeviceConnectionStateChangedMethod = getBluetoothDeviceConnectionStateChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.DeviceStreamRequest, com.ifit.glassos.bluetooth.DeviceConnectionStateResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "BluetoothDeviceConnectionStateChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceStreamRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceConnectionStateResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getBluetoothDeviceConnectionStateChangedMethod;
  }

  private static volatile io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest,
      com.ifit.glassos.bluetooth.DeviceRssiResult> getBluetoothDeviceRSSIChangedMethod;

  @io.grpc.stub.annotations.RpcMethod(
      fullMethodName = SERVICE_NAME + '/' + "BluetoothDeviceRSSIChanged",
      requestType = com.ifit.glassos.bluetooth.DeviceStreamRequest.class,
      responseType = com.ifit.glassos.bluetooth.DeviceRssiResult.class,
      methodType = io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
  public static io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest,
      com.ifit.glassos.bluetooth.DeviceRssiResult> getBluetoothDeviceRSSIChangedMethod() {
    io.grpc.MethodDescriptor<com.ifit.glassos.bluetooth.DeviceStreamRequest, com.ifit.glassos.bluetooth.DeviceRssiResult> getBluetoothDeviceRSSIChangedMethod;
    if ((getBluetoothDeviceRSSIChangedMethod = BluetoothServiceGrpc.getBluetoothDeviceRSSIChangedMethod) == null) {
      synchronized (BluetoothServiceGrpc.class) {
        if ((getBluetoothDeviceRSSIChangedMethod = BluetoothServiceGrpc.getBluetoothDeviceRSSIChangedMethod) == null) {
          BluetoothServiceGrpc.getBluetoothDeviceRSSIChangedMethod = getBluetoothDeviceRSSIChangedMethod =
              io.grpc.MethodDescriptor.<com.ifit.glassos.bluetooth.DeviceStreamRequest, com.ifit.glassos.bluetooth.DeviceRssiResult>newBuilder()
              .setType(io.grpc.MethodDescriptor.MethodType.SERVER_STREAMING)
              .setFullMethodName(generateFullMethodName(SERVICE_NAME, "BluetoothDeviceRSSIChanged"))
              .setSampledToLocalTracing(true)
              .setRequestMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceStreamRequest.getDefaultInstance()))
              .setResponseMarshaller(io.grpc.protobuf.lite.ProtoLiteUtils.marshaller(
                  com.ifit.glassos.bluetooth.DeviceRssiResult.getDefaultInstance()))
              .build();
        }
      }
    }
    return getBluetoothDeviceRSSIChangedMethod;
  }

  /**
   * Creates a new async stub that supports all call types for the service
   */
  public static BluetoothServiceStub newStub(io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<BluetoothServiceStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<BluetoothServiceStub>() {
        @java.lang.Override
        public BluetoothServiceStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new BluetoothServiceStub(channel, callOptions);
        }
      };
    return BluetoothServiceStub.newStub(factory, channel);
  }

  /**
   * Creates a new blocking-style stub that supports unary and streaming output calls on the service
   */
  public static BluetoothServiceBlockingStub newBlockingStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<BluetoothServiceBlockingStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<BluetoothServiceBlockingStub>() {
        @java.lang.Override
        public BluetoothServiceBlockingStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new BluetoothServiceBlockingStub(channel, callOptions);
        }
      };
    return BluetoothServiceBlockingStub.newStub(factory, channel);
  }

  /**
   * Creates a new ListenableFuture-style stub that supports unary calls on the service
   */
  public static BluetoothServiceFutureStub newFutureStub(
      io.grpc.Channel channel) {
    io.grpc.stub.AbstractStub.StubFactory<BluetoothServiceFutureStub> factory =
      new io.grpc.stub.AbstractStub.StubFactory<BluetoothServiceFutureStub>() {
        @java.lang.Override
        public BluetoothServiceFutureStub newStub(io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
          return new BluetoothServiceFutureStub(channel, callOptions);
        }
      };
    return BluetoothServiceFutureStub.newStub(factory, channel);
  }

  /**
   */
  public interface AsyncService {

    /**
     */
    default void scanStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothScanState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getScanStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void bluetoothServiceStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothServiceState> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getBluetoothServiceStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void foundDevicesChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothDevice> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getFoundDevicesChangedMethod(), responseObserver);
    }

    /**
     */
    default void startScan(com.ifit.glassos.bluetooth.StartScanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStartScanMethod(), responseObserver);
    }

    /**
     */
    default void stopScan(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getStopScanMethod(), responseObserver);
    }

    /**
     */
    default void connectDevice(com.ifit.glassos.bluetooth.BluetoothDevice request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConnectDeviceMethod(), responseObserver);
    }

    /**
     */
    default void connectWithMACAddress(com.ifit.glassos.bluetooth.MACAddressConnectionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.MACAddressConnectionResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConnectWithMACAddressMethod(), responseObserver);
    }

    /**
     */
    default void disconnectDevice(com.ifit.glassos.bluetooth.BluetoothDevice request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getDisconnectDeviceMethod(), responseObserver);
    }

    /**
     */
    default void connectToHRM(com.ifit.glassos.bluetooth.DeviceIdentifierRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConnectToHRMMethod(), responseObserver);
    }

    /**
     */
    default void connectToRing(com.ifit.glassos.bluetooth.DeviceIdentifierRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getConnectToRingMethod(), responseObserver);
    }

    /**
     */
    default void getPairedDevices(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothDeviceList> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getGetPairedDevicesMethod(), responseObserver);
    }

    /**
     */
    default void bluetoothDeviceBatteryLevelChanged(com.ifit.glassos.bluetooth.DeviceStreamRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceBatteryLevelResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getBluetoothDeviceBatteryLevelChangedMethod(), responseObserver);
    }

    /**
     */
    default void bluetoothDeviceConnectionStateChanged(com.ifit.glassos.bluetooth.DeviceStreamRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceConnectionStateResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getBluetoothDeviceConnectionStateChangedMethod(), responseObserver);
    }

    /**
     */
    default void bluetoothDeviceRSSIChanged(com.ifit.glassos.bluetooth.DeviceStreamRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceRssiResult> responseObserver) {
      io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall(getBluetoothDeviceRSSIChangedMethod(), responseObserver);
    }
  }

  /**
   * Base class for the server implementation of the service BluetoothService.
   */
  public static abstract class BluetoothServiceImplBase
      implements io.grpc.BindableService, AsyncService {

    @java.lang.Override public final io.grpc.ServerServiceDefinition bindService() {
      return BluetoothServiceGrpc.bindService(this);
    }
  }

  /**
   * A stub to allow clients to do asynchronous rpc calls to service BluetoothService.
   */
  public static final class BluetoothServiceStub
      extends io.grpc.stub.AbstractAsyncStub<BluetoothServiceStub> {
    private BluetoothServiceStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected BluetoothServiceStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new BluetoothServiceStub(channel, callOptions);
    }

    /**
     */
    public void scanStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothScanState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getScanStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void bluetoothServiceStateChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothServiceState> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getBluetoothServiceStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void foundDevicesChanged(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothDevice> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getFoundDevicesChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void startScan(com.ifit.glassos.bluetooth.StartScanRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStartScanMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void stopScan(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getStopScanMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void connectDevice(com.ifit.glassos.bluetooth.BluetoothDevice request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getConnectDeviceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void connectWithMACAddress(com.ifit.glassos.bluetooth.MACAddressConnectionRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.MACAddressConnectionResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getConnectWithMACAddressMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void disconnectDevice(com.ifit.glassos.bluetooth.BluetoothDevice request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getDisconnectDeviceMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void connectToHRM(com.ifit.glassos.bluetooth.DeviceIdentifierRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getConnectToHRMMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void connectToRing(com.ifit.glassos.bluetooth.DeviceIdentifierRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getConnectToRingMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void getPairedDevices(com.ifit.glassos.util.Empty request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothDeviceList> responseObserver) {
      io.grpc.stub.ClientCalls.asyncUnaryCall(
          getChannel().newCall(getGetPairedDevicesMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void bluetoothDeviceBatteryLevelChanged(com.ifit.glassos.bluetooth.DeviceStreamRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceBatteryLevelResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getBluetoothDeviceBatteryLevelChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void bluetoothDeviceConnectionStateChanged(com.ifit.glassos.bluetooth.DeviceStreamRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceConnectionStateResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getBluetoothDeviceConnectionStateChangedMethod(), getCallOptions()), request, responseObserver);
    }

    /**
     */
    public void bluetoothDeviceRSSIChanged(com.ifit.glassos.bluetooth.DeviceStreamRequest request,
        io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceRssiResult> responseObserver) {
      io.grpc.stub.ClientCalls.asyncServerStreamingCall(
          getChannel().newCall(getBluetoothDeviceRSSIChangedMethod(), getCallOptions()), request, responseObserver);
    }
  }

  /**
   * A stub to allow clients to do synchronous rpc calls to service BluetoothService.
   */
  public static final class BluetoothServiceBlockingStub
      extends io.grpc.stub.AbstractBlockingStub<BluetoothServiceBlockingStub> {
    private BluetoothServiceBlockingStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected BluetoothServiceBlockingStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new BluetoothServiceBlockingStub(channel, callOptions);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.bluetooth.BluetoothScanState> scanStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getScanStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.bluetooth.BluetoothServiceState> bluetoothServiceStateChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getBluetoothServiceStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.bluetooth.BluetoothDevice> foundDevicesChanged(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getFoundDevicesChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothResult startScan(com.ifit.glassos.bluetooth.StartScanRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStartScanMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothResult stopScan(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getStopScanMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothResult connectDevice(com.ifit.glassos.bluetooth.BluetoothDevice request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getConnectDeviceMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.MACAddressConnectionResult connectWithMACAddress(com.ifit.glassos.bluetooth.MACAddressConnectionRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getConnectWithMACAddressMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothResult disconnectDevice(com.ifit.glassos.bluetooth.BluetoothDevice request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getDisconnectDeviceMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothResult connectToHRM(com.ifit.glassos.bluetooth.DeviceIdentifierRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getConnectToHRMMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothResult connectToRing(com.ifit.glassos.bluetooth.DeviceIdentifierRequest request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getConnectToRingMethod(), getCallOptions(), request);
    }

    /**
     */
    public com.ifit.glassos.bluetooth.BluetoothDeviceList getPairedDevices(com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.blockingUnaryCall(
          getChannel(), getGetPairedDevicesMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.bluetooth.DeviceBatteryLevelResult> bluetoothDeviceBatteryLevelChanged(
        com.ifit.glassos.bluetooth.DeviceStreamRequest request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getBluetoothDeviceBatteryLevelChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.bluetooth.DeviceConnectionStateResult> bluetoothDeviceConnectionStateChanged(
        com.ifit.glassos.bluetooth.DeviceStreamRequest request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getBluetoothDeviceConnectionStateChangedMethod(), getCallOptions(), request);
    }

    /**
     */
    public java.util.Iterator<com.ifit.glassos.bluetooth.DeviceRssiResult> bluetoothDeviceRSSIChanged(
        com.ifit.glassos.bluetooth.DeviceStreamRequest request) {
      return io.grpc.stub.ClientCalls.blockingServerStreamingCall(
          getChannel(), getBluetoothDeviceRSSIChangedMethod(), getCallOptions(), request);
    }
  }

  /**
   * A stub to allow clients to do ListenableFuture-style rpc calls to service BluetoothService.
   */
  public static final class BluetoothServiceFutureStub
      extends io.grpc.stub.AbstractFutureStub<BluetoothServiceFutureStub> {
    private BluetoothServiceFutureStub(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      super(channel, callOptions);
    }

    @java.lang.Override
    protected BluetoothServiceFutureStub build(
        io.grpc.Channel channel, io.grpc.CallOptions callOptions) {
      return new BluetoothServiceFutureStub(channel, callOptions);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothResult> startScan(
        com.ifit.glassos.bluetooth.StartScanRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStartScanMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothResult> stopScan(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getStopScanMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothResult> connectDevice(
        com.ifit.glassos.bluetooth.BluetoothDevice request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getConnectDeviceMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.MACAddressConnectionResult> connectWithMACAddress(
        com.ifit.glassos.bluetooth.MACAddressConnectionRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getConnectWithMACAddressMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothResult> disconnectDevice(
        com.ifit.glassos.bluetooth.BluetoothDevice request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getDisconnectDeviceMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothResult> connectToHRM(
        com.ifit.glassos.bluetooth.DeviceIdentifierRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getConnectToHRMMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothResult> connectToRing(
        com.ifit.glassos.bluetooth.DeviceIdentifierRequest request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getConnectToRingMethod(), getCallOptions()), request);
    }

    /**
     */
    public com.google.common.util.concurrent.ListenableFuture<com.ifit.glassos.bluetooth.BluetoothDeviceList> getPairedDevices(
        com.ifit.glassos.util.Empty request) {
      return io.grpc.stub.ClientCalls.futureUnaryCall(
          getChannel().newCall(getGetPairedDevicesMethod(), getCallOptions()), request);
    }
  }

  private static final int METHODID_SCAN_STATE_CHANGED = 0;
  private static final int METHODID_BLUETOOTH_SERVICE_STATE_CHANGED = 1;
  private static final int METHODID_FOUND_DEVICES_CHANGED = 2;
  private static final int METHODID_START_SCAN = 3;
  private static final int METHODID_STOP_SCAN = 4;
  private static final int METHODID_CONNECT_DEVICE = 5;
  private static final int METHODID_CONNECT_WITH_MACADDRESS = 6;
  private static final int METHODID_DISCONNECT_DEVICE = 7;
  private static final int METHODID_CONNECT_TO_HRM = 8;
  private static final int METHODID_CONNECT_TO_RING = 9;
  private static final int METHODID_GET_PAIRED_DEVICES = 10;
  private static final int METHODID_BLUETOOTH_DEVICE_BATTERY_LEVEL_CHANGED = 11;
  private static final int METHODID_BLUETOOTH_DEVICE_CONNECTION_STATE_CHANGED = 12;
  private static final int METHODID_BLUETOOTH_DEVICE_RSSICHANGED = 13;

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
        case METHODID_SCAN_STATE_CHANGED:
          serviceImpl.scanStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothScanState>) responseObserver);
          break;
        case METHODID_BLUETOOTH_SERVICE_STATE_CHANGED:
          serviceImpl.bluetoothServiceStateChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothServiceState>) responseObserver);
          break;
        case METHODID_FOUND_DEVICES_CHANGED:
          serviceImpl.foundDevicesChanged((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothDevice>) responseObserver);
          break;
        case METHODID_START_SCAN:
          serviceImpl.startScan((com.ifit.glassos.bluetooth.StartScanRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult>) responseObserver);
          break;
        case METHODID_STOP_SCAN:
          serviceImpl.stopScan((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult>) responseObserver);
          break;
        case METHODID_CONNECT_DEVICE:
          serviceImpl.connectDevice((com.ifit.glassos.bluetooth.BluetoothDevice) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult>) responseObserver);
          break;
        case METHODID_CONNECT_WITH_MACADDRESS:
          serviceImpl.connectWithMACAddress((com.ifit.glassos.bluetooth.MACAddressConnectionRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.MACAddressConnectionResult>) responseObserver);
          break;
        case METHODID_DISCONNECT_DEVICE:
          serviceImpl.disconnectDevice((com.ifit.glassos.bluetooth.BluetoothDevice) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult>) responseObserver);
          break;
        case METHODID_CONNECT_TO_HRM:
          serviceImpl.connectToHRM((com.ifit.glassos.bluetooth.DeviceIdentifierRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult>) responseObserver);
          break;
        case METHODID_CONNECT_TO_RING:
          serviceImpl.connectToRing((com.ifit.glassos.bluetooth.DeviceIdentifierRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothResult>) responseObserver);
          break;
        case METHODID_GET_PAIRED_DEVICES:
          serviceImpl.getPairedDevices((com.ifit.glassos.util.Empty) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.BluetoothDeviceList>) responseObserver);
          break;
        case METHODID_BLUETOOTH_DEVICE_BATTERY_LEVEL_CHANGED:
          serviceImpl.bluetoothDeviceBatteryLevelChanged((com.ifit.glassos.bluetooth.DeviceStreamRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceBatteryLevelResult>) responseObserver);
          break;
        case METHODID_BLUETOOTH_DEVICE_CONNECTION_STATE_CHANGED:
          serviceImpl.bluetoothDeviceConnectionStateChanged((com.ifit.glassos.bluetooth.DeviceStreamRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceConnectionStateResult>) responseObserver);
          break;
        case METHODID_BLUETOOTH_DEVICE_RSSICHANGED:
          serviceImpl.bluetoothDeviceRSSIChanged((com.ifit.glassos.bluetooth.DeviceStreamRequest) request,
              (io.grpc.stub.StreamObserver<com.ifit.glassos.bluetooth.DeviceRssiResult>) responseObserver);
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
          getScanStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.bluetooth.BluetoothScanState>(
                service, METHODID_SCAN_STATE_CHANGED)))
        .addMethod(
          getBluetoothServiceStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.bluetooth.BluetoothServiceState>(
                service, METHODID_BLUETOOTH_SERVICE_STATE_CHANGED)))
        .addMethod(
          getFoundDevicesChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.bluetooth.BluetoothDevice>(
                service, METHODID_FOUND_DEVICES_CHANGED)))
        .addMethod(
          getStartScanMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.StartScanRequest,
              com.ifit.glassos.bluetooth.BluetoothResult>(
                service, METHODID_START_SCAN)))
        .addMethod(
          getStopScanMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.bluetooth.BluetoothResult>(
                service, METHODID_STOP_SCAN)))
        .addMethod(
          getConnectDeviceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.BluetoothDevice,
              com.ifit.glassos.bluetooth.BluetoothResult>(
                service, METHODID_CONNECT_DEVICE)))
        .addMethod(
          getConnectWithMACAddressMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.MACAddressConnectionRequest,
              com.ifit.glassos.bluetooth.MACAddressConnectionResult>(
                service, METHODID_CONNECT_WITH_MACADDRESS)))
        .addMethod(
          getDisconnectDeviceMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.BluetoothDevice,
              com.ifit.glassos.bluetooth.BluetoothResult>(
                service, METHODID_DISCONNECT_DEVICE)))
        .addMethod(
          getConnectToHRMMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.DeviceIdentifierRequest,
              com.ifit.glassos.bluetooth.BluetoothResult>(
                service, METHODID_CONNECT_TO_HRM)))
        .addMethod(
          getConnectToRingMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.DeviceIdentifierRequest,
              com.ifit.glassos.bluetooth.BluetoothResult>(
                service, METHODID_CONNECT_TO_RING)))
        .addMethod(
          getGetPairedDevicesMethod(),
          io.grpc.stub.ServerCalls.asyncUnaryCall(
            new MethodHandlers<
              com.ifit.glassos.util.Empty,
              com.ifit.glassos.bluetooth.BluetoothDeviceList>(
                service, METHODID_GET_PAIRED_DEVICES)))
        .addMethod(
          getBluetoothDeviceBatteryLevelChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.DeviceStreamRequest,
              com.ifit.glassos.bluetooth.DeviceBatteryLevelResult>(
                service, METHODID_BLUETOOTH_DEVICE_BATTERY_LEVEL_CHANGED)))
        .addMethod(
          getBluetoothDeviceConnectionStateChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.DeviceStreamRequest,
              com.ifit.glassos.bluetooth.DeviceConnectionStateResult>(
                service, METHODID_BLUETOOTH_DEVICE_CONNECTION_STATE_CHANGED)))
        .addMethod(
          getBluetoothDeviceRSSIChangedMethod(),
          io.grpc.stub.ServerCalls.asyncServerStreamingCall(
            new MethodHandlers<
              com.ifit.glassos.bluetooth.DeviceStreamRequest,
              com.ifit.glassos.bluetooth.DeviceRssiResult>(
                service, METHODID_BLUETOOTH_DEVICE_RSSICHANGED)))
        .build();
  }

  private static volatile io.grpc.ServiceDescriptor serviceDescriptor;

  public static io.grpc.ServiceDescriptor getServiceDescriptor() {
    io.grpc.ServiceDescriptor result = serviceDescriptor;
    if (result == null) {
      synchronized (BluetoothServiceGrpc.class) {
        result = serviceDescriptor;
        if (result == null) {
          serviceDescriptor = result = io.grpc.ServiceDescriptor.newBuilder(SERVICE_NAME)
              .addMethod(getScanStateChangedMethod())
              .addMethod(getBluetoothServiceStateChangedMethod())
              .addMethod(getFoundDevicesChangedMethod())
              .addMethod(getStartScanMethod())
              .addMethod(getStopScanMethod())
              .addMethod(getConnectDeviceMethod())
              .addMethod(getConnectWithMACAddressMethod())
              .addMethod(getDisconnectDeviceMethod())
              .addMethod(getConnectToHRMMethod())
              .addMethod(getConnectToRingMethod())
              .addMethod(getGetPairedDevicesMethod())
              .addMethod(getBluetoothDeviceBatteryLevelChangedMethod())
              .addMethod(getBluetoothDeviceConnectionStateChangedMethod())
              .addMethod(getBluetoothDeviceRSSIChangedMethod())
              .build();
        }
      }
    }
    return result;
  }
}

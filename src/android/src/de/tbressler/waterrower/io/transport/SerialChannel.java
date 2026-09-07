/*
 * Copyright 2017 The Netty Project
 *
 * The Netty Project licenses this file to you under the Apache License,
 * version 2.0 (the "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package de.tbressler.waterrower.io.transport;

import com.hoho.android.usbserial.driver.UsbSerialPort;
import io.netty.channel.ChannelFuture;
import io.netty.channel.ChannelPromise;
import io.netty.channel.oio.OioByteStreamChannel;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.SocketAddress;
import java.util.concurrent.TimeUnit;

import static de.tbressler.waterrower.io.transport.SerialChannelOption.*;


/**
 * A channel to a serial device.
 *
 * On Android, USB CDC-ACM devices (like the WaterRower S4/S5 USB monitor) are not exposed as
 * kernel tty nodes to unprivileged apps, so jSerialComm can not open them directly. Instead, the
 * caller (see WaterRowerBridge) opens the device with the usb-serial-for-android library and
 * hands the already-open {@link UsbSerialPort} to this channel via {@link #setUsbSerialPort}.
 */
public class SerialChannel extends OioByteStreamChannel {

    private static final SerialDeviceAddress LOCAL_ADDRESS = new SerialDeviceAddress("localhost");

    /* The maximum number of bytes attempted per single read, matches typical USB full-speed
       bulk endpoint packet size. */
    private static final int READ_CHUNK_SIZE = 64;

    /* Milliseconds to wait for data on each poll of the USB port before retrying. */
    private static final int READ_POLL_TIMEOUT_MS = 500;

    /* Milliseconds to wait for a write to complete. */
    private static final int WRITE_TIMEOUT_MS = 2000;

    /* The USB serial port that was opened by the caller before connect() was invoked. */
    private static volatile UsbSerialPort pendingUsbSerialPort;

    /**
     * Sets the already-open USB serial port that will be used by the next {@link #doConnect}
     * call. Must be called right before triggering the connection.
     *
     * @param port The opened USB serial port, or null to clear it.
     */
    public static void setUsbSerialPort(UsbSerialPort port) {
        pendingUsbSerialPort = port;
    }

    private final SerialChannelConfig config;

    private boolean open = true;
    private SerialDeviceAddress deviceAddress;
    private UsbSerialPort usbSerialPort;
    private UsbSerialInputStream usbSerialInputStream;

    public SerialChannel() {
        super(null);

        config = new DefaultSerialChannelConfig(this);
        config.setReadTimeout(3000);
        config.setAutoClose(true);
    }

    @Override
    public SerialChannelConfig config() {
        return config;
    }

    @Override
    public boolean isOpen() {
        return open;
    }

    @Override
    protected AbstractUnsafe newUnsafe() {
        return new JSCUnsafe();
    }

    @Override
    protected void doConnect(SocketAddress remoteAddress, SocketAddress localAddress) throws Exception {
        SerialDeviceAddress remote = (SerialDeviceAddress) remoteAddress;

        UsbSerialPort port = pendingUsbSerialPort;
        pendingUsbSerialPort = null;
        if (port == null || !port.isOpen()) {
            throw new IOException("Could not open port: " + remote.value());
        }

        deviceAddress = remote;
        usbSerialPort = port;
    }

    protected void doInit() throws Exception {
        usbSerialInputStream = new UsbSerialInputStream(usbSerialPort);
        activate(usbSerialInputStream, new UsbSerialOutputStream(usbSerialPort));
    }

    @Override
    public SerialDeviceAddress localAddress() {
        return (SerialDeviceAddress) super.localAddress();
    }

    @Override
    public SerialDeviceAddress remoteAddress() {
        return (SerialDeviceAddress) super.remoteAddress();
    }

    @Override
    protected SerialDeviceAddress localAddress0() {
        return LOCAL_ADDRESS;
    }

    @Override
    protected SerialDeviceAddress remoteAddress0() {
        return deviceAddress;
    }

    @Override
    protected void doBind(SocketAddress localAddress) throws Exception {
        throw new UnsupportedOperationException();
    }

    @Override
    protected void doDisconnect() throws Exception {
        doClose();
    }

    @Override
    protected void doClose() throws Exception {
        open = false;
        if (usbSerialInputStream != null) {
            usbSerialInputStream.close();
        }
        try {
           super.doClose();
        } finally {
            if (usbSerialPort != null) {
                try {
                    usbSerialPort.close();
                } catch (IOException ignored) {
                    // Already closed/detached.
                }
                usbSerialPort = null;
            }
        }
    }

    @Override
    protected boolean isInputShutdown() {
        return !open;
    }

    @Override
    protected ChannelFuture shutdownInput() {
        return newFailedFuture(new UnsupportedOperationException("shutdownInput"));
    }


    private final class JSCUnsafe extends AbstractUnsafe {
        @Override
        public void connect(
                final SocketAddress remoteAddress,
                final SocketAddress localAddress, final ChannelPromise promise) {
            if (!promise.setUncancellable() || !isOpen()) {
                return;
            }

            try {
                final boolean wasActive = isActive();
                doConnect(remoteAddress, localAddress);

                int waitTime = config().getOption(WAIT_TIME);
                if (waitTime > 0) {
                    eventLoop().schedule(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                doInit();
                                safeSetSuccess(promise);
                                if (!wasActive && isActive()) {
                                    pipeline().fireChannelActive();
                                }
                            } catch (Throwable t) {
                                safeSetFailure(promise, t);
                                closeIfClosed();
                            }
                        }
                   }, waitTime, TimeUnit.MILLISECONDS);
                } else {
                    doInit();
                    safeSetSuccess(promise);
                    if (!wasActive && isActive()) {
                        pipeline().fireChannelActive();
                    }
                }
            } catch (Throwable t) {
                safeSetFailure(promise, t);
                closeIfClosed();
            }
        }
    }


    /**
     * Bridges the blocking {@link UsbSerialPort#read(byte[], int)} API to an {@link InputStream}.
     * Since {@link OioByteStreamChannel} only calls {@link #read(byte[], int, int)} when
     * {@link #available()} reports a positive value, this stream always reports a fixed chunk
     * size as available, and performs the actual blocking wait inside {@link #read(byte[], int, int)}.
     */
    private static final class UsbSerialInputStream extends InputStream {
        private final UsbSerialPort port;
        private volatile boolean closed = false;

        UsbSerialInputStream(UsbSerialPort port) {
            this.port = port;
        }

        @Override
        public int available() {
            return closed ? 0 : READ_CHUNK_SIZE;
        }

        @Override
        public int read() throws IOException {
            byte[] single = new byte[1];
            int n = read(single, 0, 1);
            return n <= 0 ? -1 : (single[0] & 0xFF);
        }

        @Override
        public int read(byte[] b, int off, int len) throws IOException {
            if (len == 0) {
                return 0;
            }

            byte[] tmp = new byte[len];
            while (!closed && port.isOpen()) {
                int n = port.read(tmp, READ_POLL_TIMEOUT_MS);
                if (n > 0) {
                    System.arraycopy(tmp, 0, b, off, n);
                    return n;
                }
            }
            return -1;
        }

        @Override
        public void close() {
            closed = true;
        }
    }

    private static final class UsbSerialOutputStream extends OutputStream {
        private final UsbSerialPort port;

        UsbSerialOutputStream(UsbSerialPort port) {
            this.port = port;
        }

        @Override
        public void write(int b) throws IOException {
            write(new byte[]{(byte) b}, 0, 1);
        }

        @Override
        public void write(byte[] b, int off, int len) throws IOException {
            byte[] tmp = new byte[len];
            System.arraycopy(b, off, tmp, 0, len);
            port.write(tmp, WRITE_TIMEOUT_MS);
        }
    }
}

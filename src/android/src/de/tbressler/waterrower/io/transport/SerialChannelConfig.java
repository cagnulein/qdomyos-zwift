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

import com.fazecast.jSerialComm.SerialPort;
import io.netty.buffer.ByteBufAllocator;
import io.netty.channel.ChannelConfig;
import io.netty.channel.MessageSizeEstimator;
import io.netty.channel.RecvByteBufAllocator;

/**
 * A configuration class for JSerialComm device connections.
 *
 * <h3>Available options</h3>
 *
 * In addition to the options provided by {@link ChannelConfig},
 * {@link DefaultSerialChannelConfig} allows the following options in the option map:
 *
 * <table border="1" cellspacing="0" cellpadding="6">
 * <tr>
 * <th>Name</th><th>Associated setter method</th>
 * </tr><tr>
 * <td>{@link SerialChannelOption#BAUD_RATE}</td><td>{@link #setBaudrate(int)}</td>
 * </tr><tr>
 * <td>{@link SerialChannelOption#STOP_BITS}</td><td>{@link #setStopbits(Stopbits)}</td>
 * </tr><tr>
 * <td>{@link SerialChannelOption#DATA_BITS}</td><td>{@link #setDatabits(int)}</td>
 * </tr><tr>
 * <td>{@link SerialChannelOption#PARITY_BIT}</td><td>{@link #setParitybit(Paritybit)}</td>
 * </tr><tr>
 * <td>{@link SerialChannelOption#WAIT_TIME}</td><td>{@link #setWaitTimeMillis(int)}</td>
 * </tr>
 * </table>
 */
public interface SerialChannelConfig extends ChannelConfig {
    enum Stopbits {
        /**
         * 1 stop bit will be sent at the end of every character
         */
        STOPBITS_1(SerialPort.ONE_STOP_BIT),
        /**
         * 2 stop bits will be sent at the end of every character
         */
        STOPBITS_2(SerialPort.TWO_STOP_BITS),
        /**
         * 1.5 stop bits will be sent at the end of every character
         */
        STOPBITS_1_5(SerialPort.ONE_POINT_FIVE_STOP_BITS);

        private final int value;

        Stopbits(int value) {
            this.value = value;
        }

        public int value() {
            return value;
        }

        public static Stopbits valueOf(int value) {
            for (Stopbits stopbit : Stopbits.values()) {
                if (stopbit.value == value) {
                    return stopbit;
                }
            }
            throw new IllegalArgumentException("unknown " + Stopbits.class.getSimpleName() + " value: " + value);
        }
    }

    enum Paritybit {
        /**
         * No parity bit will be sent with each data character at all
         */
        NONE(SerialPort.NO_PARITY),
        /**
         * An odd parity bit will be sent with each data character, ie. will be set
         * to 1 if the data character contains an even number of bits set to 1.
         */
        ODD(SerialPort.ODD_PARITY),
        /**
         * An even parity bit will be sent with each data character, ie. will be set
         * to 1 if the data character contains an odd number of bits set to 1.
         */
        EVEN(SerialPort.EVEN_PARITY),
        /**
         * A mark parity bit (ie. always 1) will be sent with each data character
         */
        MARK(SerialPort.MARK_PARITY),
        /**
         * A space parity bit (ie. always 0) will be sent with each data character
         */
        SPACE(SerialPort.SPACE_PARITY);

        private final int value;

        Paritybit(int value) {
            this.value = value;
        }

        public int value() {
            return value;
        }

        public static Paritybit valueOf(int value) {
            for (Paritybit paritybit : Paritybit.values()) {
                if (paritybit.value == value) {
                    return paritybit;
                }
            }
            throw new IllegalArgumentException("unknown " + Paritybit.class.getSimpleName() + " value: " + value);
        }
    }

    /**
     * Sets the baud rate (ie. bits per second) for communication with the serial device.
     * The baud rate will include bits for framing (in the form of stop bits and parity),
     * such that the effective data rate will be lower than this value.
     *
     * @param baudrate The baud rate (in bits per second)
     */
    SerialChannelConfig setBaudrate(int baudrate);

    /**
     * Sets the number of stop bits to include at the end of every character to aid the
     * serial device in synchronising with the data.
     *
     * @param stopbits The number of stop bits to use
     */
    SerialChannelConfig setStopbits(Stopbits stopbits);

    /**
     * Sets the number of data bits to use to make up each character sent to the serial
     * device.
     *
     * @param databits The number of data bits to use
     */
    SerialChannelConfig setDatabits(int databits);

    /**
     * Sets the type of parity bit to be used when communicating with the serial device.
     *
     * @param paritybit The type of parity bit to be used
     */
    SerialChannelConfig setParitybit(Paritybit paritybit);

    /**
     * @return The configured baud rate, defaulting to 115200 if unset
     */
    int getBaudrate();

    /**
     * @return The configured stop bits, defaulting to {@link Stopbits#STOPBITS_1} if unset
     */
    Stopbits getStopbits();

    /**
     * @return The configured data bits, defaulting to 8 if unset
     */
    int getDatabits();

    /**
     * @return The configured parity bit, defaulting to {@link Paritybit#NONE} if unset
     */
    Paritybit getParitybit();

    /**
     * @return The number of milliseconds to wait between opening the serial port and
     *     initialising.
     */
    int getWaitTimeMillis();

    /**
     * Sets the time to wait after opening the serial port and before sending it any
     * configuration information or data. A value of 0 indicates that no waiting should
     * occur.
     *
     * @param waitTimeMillis The number of milliseconds to wait, defaulting to 0 (no
     *     wait) if unset
     * @throws IllegalArgumentException if the supplied value is &lt; 0
     */
    SerialChannelConfig setWaitTimeMillis(int waitTimeMillis);

    /**
     * Sets the maximal time (in ms) to block while try to read from the serial port. Default is 1000ms
     */
    SerialChannelConfig setReadTimeout(int readTimeout);

    /**
     * Return the maximal time (in ms) to block and wait for something to be ready to read.
     */
    int getReadTimeout();

    @Override
    SerialChannelConfig setConnectTimeoutMillis(int connectTimeoutMillis);

    @Override
    SerialChannelConfig setWriteSpinCount(int writeSpinCount);

    @Override
    SerialChannelConfig setAllocator(ByteBufAllocator allocator);

    @Override
    SerialChannelConfig setRecvByteBufAllocator(RecvByteBufAllocator allocator);

    @Override
    SerialChannelConfig setAutoRead(boolean autoRead);

    @Override
    SerialChannelConfig setWriteBufferHighWaterMark(int writeBufferHighWaterMark);

    @Override
    SerialChannelConfig setWriteBufferLowWaterMark(int writeBufferLowWaterMark);

    @Override
    SerialChannelConfig setMessageSizeEstimator(MessageSizeEstimator estimator);
}
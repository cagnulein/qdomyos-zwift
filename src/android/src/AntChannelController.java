/*
 * Copyright 2012 Dynastream Innovations Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
package org.cagnulen.qdomyoszwift;

import android.os.RemoteException;

import android.util.Log;

import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.AcknowledgedDataMessage;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;

// the common bits that are shared between all of the _ChannelController classes
public class AntChannelController {

    private AntChannel mAntChannel;

    private int msensor_id;

    // The device type and transmission type to be part of the channel ID message
    private int mdevice_type;
    private int mtransmission_type;

    // The period and frequency values
    private int mperiod; // 8192 -> 1 Hz
    private int mfrequency;

    private String mtag;

    private IAntChannelEventHandler mChannelEventCallback;
    private ChannelType mchannel_type;

    private boolean mIsOpen;

    public AntChannelController(AntChannel antChannel, int sensor_id, int device_type, int transmission_type, int period, int frequency, ChannelType channel_type, String tag, IAntChannelEventHandler callback) {
        mAntChannel = antChannel;
        msensor_id = sensor_id;
        mdevice_type = device_type;
        mtransmission_type = transmission_type;
        mperiod = period; // 8192 -> 1 Hz
        mfrequency = frequency;
        mchannel_type = channel_type;
        mtag = tag;
        mChannelEventCallback = callback;
        openChannel();
    }

    public boolean openChannel() {
        if (null != mAntChannel) {
            if (mIsOpen) {
                Log.w(TAG, "Channel was already open");
            } else {
                // Channel ID message contains device number, type and transmission type. In
                // order for master (TX) channels and slave (RX) channels to connect, they
                // must have the same channel ID, or wildcard (0) is used.
                ChannelId channelId = new ChannelId(msensor_id & 0xFFFF, mdevice_type, mtransmission_type);

                try {
                    // Setting the channel event handler so that we can receive messages from ANT
                    mAntChannel.setChannelEventHandler(mChannelEventCallback);

                    // Performs channel assignment by assigning the type to the channel. Additional
                    // features (such as, background scanning and frequency agility) can be enabled
                    // by passing an ExtendedAssignment object to assign(ChannelType, ExtendedAssignment).
                    mAntChannel.assign(mchannel_type);

                    /*
                     * Configures the channel ID, messaging period and rf frequency after assigning,
                     * then opening the channel.
                     *
                     * For any additional ANT features such as proximity search or background scanning, refer to
                     * the ANT Protocol Doc found at:
                     * http://www.thisisant.com/resources/ant-message-protocol-and-usage/
                     */
                    mAntChannel.setChannelId(channelId);
                    mAntChannel.setPeriod(mperiod);
                    mAntChannel.setRfFrequency(mfrequency);
                    mAntChannel.open();
                    mIsOpen = true;

                    Log.d(mtag, "Opened channel with device number: " + msensor_id);

                } catch (RemoteException e) {
                    channelError(e);
                } catch (AntCommandFailedException e) {
                    // This will release, and therefore unassign if required
                    channelError("Open failed", e);
                }
            }
        } else {
            Log.w(mtag, "No channel available");
        }

        return mIsOpen;
    }

    public boolean isOpen() {
        return mIsOpen;
    }

    private void channelError(RemoteException e) {
        String logString = "Remote service communication failed.";

        Log.e(mtag, logString);
    }

    private void channelError(String error, AntCommandFailedException e) {
        StringBuilder logString;

        if (e.getResponseMessage() != null) {
            String initiatingMessageId = "0x" + Integer.toHexString(
                    e.getResponseMessage().getInitiatingMessageId());
            String rawResponseCode = "0x" + Integer.toHexString(
                    e.getResponseMessage().getRawResponseCode());

            logString = new StringBuilder(error)
                    .append(". Command ")
                    .append(initiatingMessageId)
                    .append(" failed with code ")
                    .append(rawResponseCode);
        } else {
            String attemptedMessageId = "0x" + Integer.toHexString(
                    e.getAttemptedMessageType().getMessageId());
            String failureReason = e.getFailureReason().toString();

            logString = new StringBuilder(error)
                    .append(". Command ")
                    .append(attemptedMessageId)
                    .append(" failed with reason ")
                    .append(failureReason);
        }

        Log.e(mtag, logString.toString());

        close();
    }

    public void close() {
        // TODO kill all our resources
        if (null != mAntChannel) {
            mIsOpen = false;

            // Releasing the channel to make it available for others.
            // After releasing, the AntChannel instance cannot be reused.
            mAntChannel.release();
            mAntChannel = null;
        }

        Log.e(mtag, "Channel Closed");
    }

}

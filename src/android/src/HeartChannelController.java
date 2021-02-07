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
import com.dsi.ant.message.fromant.BroadcastDataMessage;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

import java.util.Random;

public class HeartChannelController {
	 // The device type and transmission type to be part of the channel ID message
	 private static final int CHANNEL_HEART_DEVICE_TYPE = 0x78;
	 private static final int CHANNEL_HEART_TRANSMISSION_TYPE = 1;

	 // The period and frequency values the channel will be configured to
	 private static final int CHANNEL_HEART_PERIOD = 8118; // 1 Hz
	 private static final int CHANNEL_HEART_FREQUENCY = 57;

	 private static final String TAG = HeartChannelController.class.getSimpleName();

	 private static Random randGen = new Random();

	 private AntChannel mAntChannel;

	 private ChannelEventCallback mChannelEventCallback = new ChannelEventCallback();


	 private boolean mIsOpen;
	 int heart = 0;

	 public HeartChannelController(AntChannel antChannel) {
		  mAntChannel = antChannel;
		  openChannel();
		}

	 boolean openChannel() {
		  if (null != mAntChannel) {
			   if (mIsOpen) {
					 Log.w(TAG, "Channel was already open");
					} else {
					 // Channel ID message contains device number, type and transmission type. In
					 // order for master (TX) channels and slave (RX) channels to connect, they
					 // must have the same channel ID, or wildcard (0) is used.
					 ChannelId channelId = new ChannelId(0,
					         CHANNEL_HEART_DEVICE_TYPE, CHANNEL_HEART_TRANSMISSION_TYPE);

								try {
						  // Setting the channel event handler so that we can receive messages from ANT
						  mAntChannel.setChannelEventHandler(mChannelEventCallback);

						  // Performs channel assignment by assigning the type to the channel. Additional
						  // features (such as, background scanning and frequency agility) can be enabled
						  // by passing an ExtendedAssignment object to assign(ChannelType, ExtendedAssignment).
						  mAntChannel.assign(ChannelType.SLAVE_RECEIVE_ONLY);

						  /*
						   * Configures the channel ID, messaging period and rf frequency after assigning,
							* then opening the channel.
							*
							* For any additional ANT features such as proximity search or background scanning, refer to
							* the ANT Protocol Doc found at:
							* http://www.thisisant.com/resources/ant-message-protocol-and-usage/
							*/
							mAntChannel.setChannelId(channelId);
						  mAntChannel.setPeriod(CHANNEL_HEART_PERIOD);
						  mAntChannel.setRfFrequency(CHANNEL_HEART_FREQUENCY);
						  mAntChannel.open();
						  mIsOpen = true;

						  Log.d(TAG, "Opened channel with device number");
						} catch (RemoteException e) {
						  channelError(e);
						} catch (AntCommandFailedException e) {
						  // This will release, and therefore unassign if required
						  channelError("Open failed", e);
						}
				}
			} else {
			   Log.w(TAG, "No channel available");
				}

			return mIsOpen;
		}

	 void channelError(RemoteException e) {
		  String logString = "Remote service communication failed.";

		  Log.e(TAG, logString);
		  }

	 void channelError(String error, AntCommandFailedException e) {
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

					Log.e(TAG, logString.toString());

		  mAntChannel.release();

		  Log.e(TAG, "ANT Command Failed");
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

			Log.e(TAG, "Channel Closed");
		}

	 /**
	  * Implements the Channel Event Handler Interface so that messages can be
	  * received and channel death events can be handled.
	  */
	 public class ChannelEventCallback implements IAntChannelEventHandler {
		  int revCounts = 0;
		  int ucMessageCount = 0;
		  byte ucPageChange = 0;
		  byte ucExtMesgType = 1;
		  long lastTime = 0;
		  double way;
		  int rev;
		  double remWay;
		  double wheel = 0.1;

		  @Override
		  public void onChannelDeath() {
			   // Display channel death message when channel dies
				Log.e(TAG, "Channel Death");
				}

			@Override
		  public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
			   Log.d(TAG, "Rx: " + antParcel);
				Log.d(TAG, "Message Type: " + messageType);

				// Switching on message type to handle different types of messages
				switch (messageType) {
					 // If data message, construct from parcel and update channel data
					 case BROADCAST_DATA:
					     // Rx Data
						  //updateData(new BroadcastDataMessage(antParcel).getPayload());
						  BroadcastDataMessage m = new BroadcastDataMessage(antParcel);
						  Log.d(TAG, "BROADCAST_DATA: " + m.getPayload());
						  heart = m.getPayload()[7];
						  Log.d(TAG, "BROADCAST_DATA: " + heart);
						  break;
						case ACKNOWLEDGED_DATA:
						  // Rx Data
						  //updateData(new AcknowledgedDataMessage(antParcel).getPayload());
						  Log.d(TAG, "ACKNOWLEDGED_DATA: " + new AcknowledgedDataMessage(antParcel).getPayload());
						  break;
						case CHANNEL_EVENT:
						  // Constructing channel event message from parcel
						  ChannelEventMessage eventMessage = new ChannelEventMessage(antParcel);
						  EventCode code = eventMessage.getEventCode();
						  Log.d(TAG, "Event Code: " + code);

						  // Switching on event code to handle the different types of channel events
						  switch (code) {
							   case TX:
								    break;
									case CHANNEL_COLLISION:
									 ucPageChange += 0x20;
									 ucPageChange &= 0xF0;
									 ucMessageCount += 1;
									 break;
									case RX_SEARCH_TIMEOUT:
									 // TODO May want to keep searching
									 Log.e(TAG, "No Device Found");
									 break;
									case CHANNEL_CLOSED:
								case RX_FAIL:
								case RX_FAIL_GO_TO_SEARCH:
								case TRANSFER_RX_FAILED:
								case TRANSFER_TX_COMPLETED:
								case TRANSFER_TX_FAILED:
								case TRANSFER_TX_START:
								case UNKNOWN:
								    // TODO More complex communication will need to handle these events
									 break;
									}
								break;
						case ANT_VERSION:
					 case BURST_TRANSFER_DATA:
					 case CAPABILITIES:
					 case CHANNEL_ID:
					 case CHANNEL_RESPONSE:
					 case CHANNEL_STATUS:
					 case SERIAL_NUMBER:
					 case OTHER:
					     // TODO More complex communication will need to handle these message types
						  break;
						}
			}
	 }
}

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





public class HeartChannelController extends AntChannelController{
     public static final int HEART_SENSOR_ID = 0;

	 // The device type and transmission type to be part of the channel ID message
	 private static final int CHANNEL_HEART_DEVICE_TYPE = 0x78;
	 private static final int CHANNEL_HEART_TRANSMISSION_TYPE = 1;

	 // The period and frequency values the channel will be configured to
	 private static final int CHANNEL_HEART_PERIOD = 8118; // 1 Hz
	 private static final int CHANNEL_HEART_FREQUENCY = 57;

	 private static final String TAG = HeartChannelController.class.getSimpleName();

	 int heart = 0;

	 public HeartChannelController(AntChannel antChannel) {
		 super(antChannel, HEART_SENSOR_ID, CHANNEL_HEART_DEVICE_TYPE, CHANNEL_HEART_TRANSMISSION_TYPE, CHANNEL_HEART_PERIOD, CHANNEL_HEART_FREQUENCY, ChannelType.SLAVE_RECEIVE_ONLY, TAG, new ChannelEventCallback());
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

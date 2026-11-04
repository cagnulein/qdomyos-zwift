#pragma once

struct LcdDashboardData {
  bool wifiConnected;
  bool bikeConnected;
  bool telemetryFresh;
  bool xiaoOnline;
  bool antChannelReady;
  bool fecPeerActive;
  float watts;
  float cadenceRpm;
  float requestedWatts;
};

bool lcdDashboardBegin();
void lcdDashboardUpdate(const LcdDashboardData &data);
int lcdDashboardReadErgAdjustment();

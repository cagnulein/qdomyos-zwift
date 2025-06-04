package com.rvalerio.fgchecker.detectors;

import android.annotation.TargetApi;
import android.app.Service;
import android.app.usage.UsageEvents;
import android.app.usage.UsageStatsManager;
import android.content.Context;
import android.os.Build;

import com.rvalerio.fgchecker.Utils;

public class LollipopDetector implements Detector {
    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public String getForegroundApp(final Context context) {
        if(!Utils.hasUsageStatsPermission(context))
            return null;

        String foregroundApp = null;

        UsageStatsManager mUsageStatsManager = (UsageStatsManager) context.getSystemService(Service.USAGE_STATS_SERVICE);
        long time = System.currentTimeMillis();

        UsageEvents usageEvents = mUsageStatsManager.queryEvents(time - 1000 * 3600, time);
        UsageEvents.Event event = new UsageEvents.Event();
        while (usageEvents.hasNextEvent()) {
            usageEvents.getNextEvent(event);
            if(event.getEventType() == UsageEvents.Event.MOVE_TO_FOREGROUND) {
                foregroundApp = event.getPackageName();
            }
        }

        return foregroundApp ;
    }
}

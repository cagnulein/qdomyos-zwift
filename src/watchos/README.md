# watchOS SwiftUI Redesign

This directory contains the new SwiftUI-based Apple Watch app for qdomyos-zwift.

## Overview

The watchOS app has been completely redesigned using SwiftUI with an Apple Workout-like interface. The app now displays configurable metric tiles based on the settings configured in the main iOS app.

## Architecture

### Key Components

1. **WorkoutMetrics.swift** - Observable model containing all workout metrics (heart rate, speed, cadence, power, calories, distance, etc.)

2. **TileConfiguration.swift** - Manages which tiles to display based on settings synced from the iOS app. Reads `tile_*_enabled` and `tile_*_order` settings.

3. **MetricTileView.swift** - SwiftUI view for individual metric tiles with Apple Workout-inspired design (colored icons, large values, small units)

4. **WorkoutContentView.swift** - Main workout view with:
   - Sport type selector (Bike, Run, Walk, Elliptical, Rowing)
   - Configurable grid of metric tiles (2 columns)
   - Start/Stop workout button

5. **WatchKitConnection.swift** - Enhanced to receive tile settings from iOS app via `WCSession.applicationContext`

6. **qdomyoszwiftApp.swift** - SwiftUI App entry point using `WorkoutContentView`

## Settings Synchronization

The watchOS app automatically syncs tile configuration from the iOS app:

- When the iOS app activates the Watch Connectivity session, it sends all `tile_*_enabled` and `tile_*_order` settings
- The watch receives these settings and updates `TileConfiguration`
- Only **enabled tiles** are displayed, sorted by their **order** value
- Maximum of **6 tiles** are shown on the watch (top 6 by order)

## Supported Metrics

The following metrics can be displayed (matching iOS app tile settings):

- Speed
- Inclination
- Cadence
- Elevation
- Calories
- Distance (Odometer)
- Pace
- Resistance
- Power (Watt)
- Average Power (Avg Watt)
- Heart Rate
- Elapsed Time
- Target Resistance

## Integration Instructions

### Adding Files to Xcode Project

1. Open the qdomyos-zwift Xcode project
2. Navigate to the **watchkit Extension** target
3. Add all `.swift` files from this directory to the target:
   - WorkoutMetrics.swift
   - TileConfiguration.swift
   - MetricTileView.swift
   - WorkoutContentView.swift
   - WatchKitConnection.swift (replace existing)
   - qdomyoszwiftApp.swift (replace existing)

4. Ensure the deployment target is set to **watchOS 7.0 or later** (for SwiftUI support)

### Updating iOS WatchKitConnection

The iOS-side `WatchKitConnection.swift` in `src/ios/` has been updated to sync tile settings. No manual changes needed.

## Design Philosophy

The UI follows Apple Workout app design principles:

- **Dark background** with semi-transparent tile backgrounds
- **Colored icons** for each metric type (heart = red, speed = green, power = yellow, etc.)
- **Large, bold values** in rounded font
- **Small, gray units** and labels
- **Grid layout** (2 columns) for optimal watch screen usage
- **Minimal, clean** interface focused on glanceable metrics

## Tile Configuration Example

If the iOS app has these settings:
```
tile_heart_enabled = true, tile_heart_order = 0
tile_speed_enabled = true, tile_speed_order = 1
tile_cadence_enabled = true, tile_cadence_order = 2
tile_power_enabled = true, tile_power_order = 3
tile_calories_enabled = true, tile_calories_order = 4
tile_distance_enabled = false
```

The watch will display (in order):
1. Heart Rate (top-left)
2. Speed (top-right)
3. Cadence (middle-left)
4. Power (middle-right)
5. Calories (bottom-left)

## Future Enhancements

Potential improvements:
- Swipeable pages for more than 6 metrics
- Custom tile colors per user preference
- Complications for watch face integration
- Haptic feedback for zone changes
- Always-On Display optimization

## Credits

Redesigned using SwiftUI with Apple Workout-inspired design.
Original WatchKit implementation by NhatHM.
SwiftUI redesign by Claude Code (2024).

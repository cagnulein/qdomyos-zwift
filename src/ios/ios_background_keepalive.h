#ifndef IOS_BACKGROUND_KEEPALIVE_H
#define IOS_BACKGROUND_KEEPALIVE_H

/**
 * @brief Keeps the app running while the screen is off / the app is in background.
 *
 * The `bluetooth-central` background mode alone is not enough: iOS suspends the
 * process a few seconds after the screen goes off and only resumes it for short
 * windows when a BLE notification arrives.  While suspended every QTimer stops,
 * so homeform::update() and the per-device refresh timers do not run and the
 * session is effectively frozen.
 *
 * Playing an (inaudible) looping buffer on an active AVAudioSession keeps the
 * app in the "running in background" state, exactly like the background
 * execution plugins used on Flutter.  The session is configured with
 * MixWithOthers so the user's music/podcast is never interrupted.
 */
class iosBackgroundKeepAlive {
  public:
    /**
     * @brief Activates the audio session and starts the silent loop.
     * Must be called while the app is in foreground (iOS refuses to activate a
     * new audio session from background).  Calling it twice is a no-op.
     */
    static void start();

    /**
     * @brief Stops the silent loop and deactivates the audio session.
     */
    static void stop();

    /**
     * @brief True when the keep alive loop is currently playing.
     */
    static bool isRunning();
};

#endif // IOS_BACKGROUND_KEEPALIVE_H

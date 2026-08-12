#ifndef RTSSOSD_H
#define RTSSOSD_H

#include <QString>
#include <QtGlobal>

/**
 * @brief Publishes a few lines of text into RivaTuner Statistics Server's OSD.
 *
 * RTSS draws inside the game's own frame through its D3D hook, so this is the
 * only way to put QZ's gear on screen over a training app running exclusive
 * fullscreen. Third-party text goes through a named shared memory block that
 * RTSS creates: an app claims an OSD slot by writing its name into the slot's
 * owner field, writes its text, and bumps a frame counter to ask for a redraw.
 *
 * Everything is best-effort and silent. With RTSS not installed or not running
 * the shared memory simply is not there, publish() does nothing, and QZ carries
 * on - this is an overlay, not a feature anything depends on.
 *
 * Windows only. Elsewhere the methods compile to no-ops so callers need no
 * platform checks.
 *
 * The layout is not hardcoded: the offset and size of the slot array are read
 * out of the header, so RTSS versions that grew the entry still work, and
 * whether the extended 4096-byte text field exists is decided from the entry
 * size rather than from a version number.
 */
class RtssOsd {
  public:
    RtssOsd() = default;
    ~RtssOsd();

    RtssOsd(const RtssOsd &) = delete;
    RtssOsd &operator=(const RtssOsd &) = delete;

    /**
     * @brief Show @p text, attaching to RTSS on first use.
     *
     * Safe to call on a timer: when RTSS is absent this retries only
     * occasionally rather than on every tick.
     */
    void publish(const QString &text);

    /**
     * @brief Clear the text and give the slot back.
     *
     * Without this RTSS keeps drawing the last thing written, so a stale gear
     * number stays frozen on screen after QZ exits.
     */
    void release();

  private:
#ifdef Q_OS_WIN
    bool attach();
    void detach();
    bool headerLooksValid() const;

    void *handle = nullptr;   // HANDLE from OpenFileMapping
    unsigned char *view = nullptr;

    quint32 osdEntrySize = 0;
    quint32 osdArrOffset = 0;
    quint32 osdArrSize = 0;
    bool hasOsdEx = false;

    int slot = -1;
    qint64 lastAttachAttemptMs = 0;
    QString lastPublished;
#endif
};

#endif // RTSSOSD_H

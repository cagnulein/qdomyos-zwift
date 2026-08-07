#ifndef INSPIREIC15DSERIALREADER_H
#define INSPIREIC15DSERIALREADER_H

#include <QByteArray>
#include <QMutex>
#include <QString>
#include <QThread>

class inspireic15dserialreader : public QThread {
  public:
    explicit inspireic15dserialreader(QObject *parent, const QString &deviceFilename, bool metricPollingEnabled);
    ~inspireic15dserialreader() override;

    void snapshot(bool &portOpen, qint64 &totalBytes, QByteArray &lastChunk, QString &error, int &cadence,
                  int &power, int &resistance, qint64 &validFrames) const;

    // Pure, I/O-free decoding of the IC15D "f1.../f3..." reply protocol, factored out of run()
    // so the real, hardware-validated framing/checksum/decode logic is directly unit-testable.
    struct ParsedFrame {
        bool valid = false;
        bool negativeAcknowledgement = false;
        char type = 0;
        int value = 0;
    };

    // Scans buffer for the next frame marker (0xf1 ack / 0xf3 nak). Returns its index, or -1 if
    // no marker is present anywhere in buffer (caller should discard the whole buffer).
    static int findFrameMarker(const QByteArray &buffer);

    // Given buffer.at(start) is a frame marker, returns the total frame size in bytes: 5 for a
    // NAK, or payloadLength + 5 for an ACK (payloadLength read from buffer.at(start + 2)).
    // Returns 0 if buffer doesn't yet hold enough bytes (start + 3) to know the size, or -1 if
    // the declared ACK payload length is implausible (> 32) and the marker byte should be
    // dropped to resynchronize.
    static int frameSizeAt(const QByteArray &buffer, int start);

    // Parses one complete frame (frame.size() == frameSizeAt(frame, 0)). A NAK frame
    // (marker 0xf3) is reported via negativeAcknowledgement without further validation, matching
    // the stock protocol's NAK framing. An ACK frame (marker 0xf1) is validated for terminator
    // (0xf6), checksum (sum of bytes before the checksum byte, mod 256), and all-ASCII-digit
    // payload, then decoded least-significant-digit-first, as observed from the hardware.
    static ParsedFrame parseFrame(const QByteArray &frame);

  protected:
    void run() override;

  private:
    const QString deviceFilename;
    const bool metricPollingEnabled;
    mutable QMutex stateMutex;
    bool portOpen = false;
    qint64 totalBytes = 0;
    QByteArray lastChunk;
    QString error;
    int cadence = -1;
    int power = -1;
    int resistance = -1;
    qint64 validFrames = 0;
};

#endif // INSPIREIC15DSERIALREADER_H

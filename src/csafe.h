#ifndef CSAFE_H
#define CSAFE_H

#include <QVariant>

#include <QByteArray>

#include <QList>
#include <QObject>
#include <QString>
#include <stdexcept>

#include <QBuffer>
#include <QMap>

class csafe {
  public:
    csafe();

    QMap<QString, uint8_t> flags;
    QMap<QString, QMap<QString, QVariant>> commands;

  private:
    void initializeFlags();
    void initializeCommands();
};

class csafeFrame {
  public:
    QByteArray buffer;

    class InvalidFrameError : public std::runtime_error {
      public:
        InvalidFrameError(const std::string &message) : std::runtime_error(message) {}
    };

    csafeFrame(const QByteArray &buffer) : buffer(buffer) {
        if (isStuffed()) {
            unstuff();
        }

        if (!validateChecksum()) {
            throw InvalidFrameError("Checksum does not match.");
        }
    }

    static quint8 stuffByte(quint8 byte) { return byte & 0xF; }

    static quint8 unstuffByte(quint8 byte) { return byte + 0xF0; }

    static quint8 checksumFromBytes(const QByteArray &bytes) {
        quint8 checksum = 0x00;

        for (int i = 0; i < bytes.length(); ++i) {
            quint8 byte = bytes[i];
            checksum = checksum ^ byte;
        }

        return checksum;
    }

    QString type() const {
        quint8 startFlag = buffer[0];

        if (startFlag == csafe().flags.value("ExtendedStartFlag"))
            return "extended";
        else if (startFlag == csafe().flags.value("StandardStartFlag"))
            return "standard";

        throw std::runtime_error("Invalid or unknown startFlag for Frame.");
    }

    QByteArray contents() const { return buffer.mid(1, buffer.length() - 2); }

    quint8 checksum() const { return buffer.at(buffer.length() - 2); }

    bool validateChecksum() const {
        int startIdx = 1;
        int endIdx = buffer.length() - 2;
        QByteArray checkBytes = buffer.mid(startIdx, endIdx - startIdx);
        quint8 validate = checksumFromBytes(checkBytes);

        return (checksum() == validate);
    }

    bool isStuffed() const { return (buffer.indexOf(csafe().flags.value("StuffFlag")) > -1); }

    void stuff() {
        int startIdx = 1;
        int endIdx = buffer.length() - 1;

        for (int i = startIdx; i <= endIdx; ++i) {
            quint8 byte = buffer.at(i);

            if (byte == csafe().flags.value("ExtendedStartFlag") || byte == csafe().flags.value("StandardStartFlag") ||
                byte == csafe().flags.value("StopFlag") || byte == csafe().flags.value("StuffFlag")) {
                QByteArray bufferList;
                bufferList.append(buffer.left(i));
                bufferList.append(csafe().flags.value("StuffFlag"));
                bufferList.append(stuffByte(byte));
                bufferList.append(buffer.mid(i + 1, buffer.length() - i));

                // Update with the new buffer, which now has a length of n + 1.
                buffer = bufferList;
                break;
            }
        }
    }

    void unstuff() {
        int startIdx = 1;
        int endIdx = buffer.length() - 1;

        for (int i = startIdx; i <= endIdx; ++i) {
            if (buffer.at(i) == csafe().flags.value("StuffFlag")) {
                QByteArray bufferList;
                bufferList.append(buffer.left(i));
                bufferList.append(unstuffByte(buffer.at(i + 1)));
                bufferList.append(buffer.mid(i + 2, buffer.length() - i));

                // Update with the new buffer, which now has a length of n - 1.
                buffer = bufferList;
            }
        }
    }
};

class csafeCommand : public csafeFrame {
  public:
    csafeCommand(const QString &commandName, const QByteArray &data = QByteArray())
        : csafeFrame(createBuffer(commandName, data)) {}

    static QByteArray createBuffer(const QString &commandName, const QByteArray &data) {
        quint8 startFlag = csafe().flags.value("StandardStartFlag");
        quint8 stopFlag = csafe().flags.value("StopFlag");
        QMap<QString, QVariant> command = csafe().commands.value(commandName);
        QByteArray bufferArray;
        bufferArray.append(command.value("id").toUInt());

        if (command.value("type").toString() == "long") {
            bufferArray.append(data.length());
            bufferArray.append(data);
        }

        quint8 checksum = csafeFrame::checksumFromBytes(bufferArray);
        bufferArray.append(checksum);

        bufferArray.prepend(startFlag);
        bufferArray.append(stopFlag);

        return QByteArray(reinterpret_cast<const char *>(bufferArray.data()), bufferArray.length());
    }
};

class csafeResponse : public csafeFrame {
  public:
    csafeResponse(const QByteArray &buffer) : csafeFrame(buffer) {}

    struct DataStructure {
        quint8 id;
        quint8 length;
        QByteArray data;
    };

    struct PrevStatus {
        quint8 value;
        QString label;
    };

    struct State {
        quint8 value;
        QString label;
    };

    PrevStatus prevStatus() const {
        const QStringList StatusLabel = {"prevOk", "prevReject", "prevBad", "prevNotReady"};

        quint8 byte = contents()[0];
        return {(quint8)((byte >> 4) & 0x03), StatusLabel.value((byte >> 4) & 0x03)};
    }

    State state() const {
        const QStringList StateLabel = {"Error", "Ready",  "Idle",     "HaveId", nullptr,
                                        "InUse", "Paused", "Finished", "Manual", "Offline"};
        quint8 byte = contents()[0];
        return {(quint8)(byte & 0x0F), StateLabel.value(byte & 0x0F)};
    }

    QList<DataStructure> data() const {
        QList<DataStructure> dataObjects;
        int startIdx = 1;
        int i = startIdx;

        while (i < contents().length()) {
            DataStructure d;
            d.id = contents()[i++];
            d.length = contents()[i++];
            d.data = contents().mid(i, d.length);
            i += d.length;

            dataObjects.append(d);
        }

        return dataObjects;
    }
};

class csafeFrameReader {

  public:
    explicit csafeFrameReader() : frameBuffer(nullptr) {}

    void read(const QByteArray &buffer) {
        if (!buffer.isDetached()) {
            throw std::runtime_error("Invalid data passed to FrameReader, buffer expected.");
        }

        // Create a new frame buffer if start flag is detected.
        // TODO: Handle extended frames.
        int frameStartIdx = buffer.indexOf(csafe().flags.value("StandardStartFlag"));
        if (frameStartIdx > -1) {
            frameBuffer = buffer.mid(frameStartIdx);
        }
        // Append the new buffer to the existing frame buffer.
        else if (!frameBuffer.isEmpty()) {
            frameBuffer.append(buffer);
        }

        emitFrameIfComplete();
    }

  signals:
    void frame(csafeResponse frame);

  private:
    void emitFrameIfComplete() {
        int stopFlagIdx = frameBuffer.indexOf(csafe().flags.value("StopFlag"));
        if (stopFlagIdx > -1) {
            frameBuffer = frameBuffer.left(stopFlagIdx + 1);

            csafeResponse frame(frameBuffer);
            // emit frame(frame);
            frameBuffer.clear();
        }
    }

    QByteArray frameBuffer;
};

#endif // CSAFE_H

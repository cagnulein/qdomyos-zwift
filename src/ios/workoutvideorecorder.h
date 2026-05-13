#ifndef IOS_WORKOUT_VIDEO_RECORDER_H
#define IOS_WORKOUT_VIDEO_RECORDER_H

#include <QObject>
#include <QString>
#include <QStringList>

class IOSWorkoutVideoRecorderPrivate;

class IOSWorkoutVideoRecorder : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

  public:
    explicit IOSWorkoutVideoRecorder(QObject *parent = nullptr);
    ~IOSWorkoutVideoRecorder() override;

    bool available() const;
    bool recording() const;
    bool paused() const;

    Q_INVOKABLE bool startRecording(const QString &outputLocation, const QString &cameraPosition);
    Q_INVOKABLE void updateMetrics(const QStringList &metrics);
    Q_INVOKABLE void pauseRecording();
    Q_INVOKABLE void stopRecording();

  signals:
    void recordingChanged();
    void pausedChanged();
    void errorOccurred(const QString &message);
    void saved(const QString &outputLocation);

  private:
    IOSWorkoutVideoRecorderPrivate *d;
};

#endif // IOS_WORKOUT_VIDEO_RECORDER_H

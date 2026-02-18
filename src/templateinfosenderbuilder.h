#ifndef TEMPLATEINFOSENDERBUILDER_H
#define TEMPLATEINFOSENDERBUILDER_H
#include "fit_profile.hpp"
#include "devices/bluetoothdevice.h"
#include "templateinfosender.h"
#include <QHash>
#include <QJSEngine>
#include <QJsonArray>
#include <QSettings>

#define TEMPLATE_TYPE_TCPCLIENT QStringLiteral("TcpClient")
#define TEMPLATE_TYPE_WEBSERVER QStringLiteral("WebServer")
#define TEMPLATE_PRIVATE_WEBSERVER_ID "QZWS"

class TemplateInfoSenderBuilder : public QObject {
    Q_OBJECT
  public:
    static TemplateInfoSenderBuilder *getInstance(const QString &idInfo, const QStringList &folders,
                                                  QObject *parent = nullptr);
    void reinit();
    void start(bluetoothdevice *device);
    void stop();
    QStringList templateIdList() const;
    ~TemplateInfoSenderBuilder();

    void previewSessionOnChart(QList<SessionLine> *session, FIT_SPORT sport);
    void previewSessionOnChart(QList<SessionLine> *session, FIT_SPORT sport, const QString &workoutName);
  signals:
    void activityDescriptionChanged(QString newDescription);
    void chartSaved(QString filename);
    void lap();
    void floatingClose();
    void pelotonOffset_Plus();
    void pelotonOffset_Minus();
    void gears_Plus();
    void gears_Minus();
    void speed_Plus();
    void speed_Minus();
    void inclination_Plus();
    void inclination_Minus();
    int pelotonOffset();
    bool pelotonAskStart();
    void peloton_start_workout();
    void peloton_abort_workout();
    void Start();
    void Pause();
    void Stop();
    void autoResistance();

  private:
    bool validFileTemplateType(const QString &tp) const;
    void buildContext(bool forceReinit = false);
    QString activityDescription;
    void createTemplatesFromFolder(const QString &idInfo, const QString &folder, QStringList &dirTemplates);
    void clearSessionArray();
    void clearPreviewSessionArray();
    bluetoothdevice *device = nullptr;
    QTimer updateTimer;
    QString masterId;
    QStringList foldersToLook;
    QJsonArray sessionArray;
    QJsonArray previewSessionArray;
    QHash<QString, QVariant> context;
    QJSEngine *engine = nullptr;
    TemplateInfoSenderBuilder(QObject *parent);
    void load(const QString &idInfo, const QStringList &folders);
    static QHash<QString, TemplateInfoSenderBuilder *> instanceMap;
    QHash<QString, TemplateInfoSender *> templateInfoMap;
    TemplateInfoSender *newTemplate(const QString &id, const QString &tp, const QString &dataTempl);
    QHash<QString, QString> templateFilesList;
    void onSetSettings(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGetSettings(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSetResistance(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSetFanSpeed(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSetPower(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSetCadence(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSetSpeed(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSetDifficult(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSaveChart(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGetPelotonImage(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onLap(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onPelotonOffsetPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onPelotonOffsetMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGearsPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGearsMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSpeedPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSpeedMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onInclinationPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onInclinationMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onPelotonStartWorkout(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onPelotonAbortWorkout(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onFloatingClose(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onAutoresistance(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onSaveTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onDeleteTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onLoadTrainingPrograms(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGetTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onTrainingProgramPreview(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGetWorkoutPreview(TemplateInfoSender *tempSender);
    void onTrainingProgramOpen(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onTrainingProgramAutostart(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onWorkoutEditorEnv(TemplateInfoSender *tempSender);
    void onWorkoutEditorStart(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onAppendActivityDescription(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGetSessionArray(TemplateInfoSender *tempSender);
    void onGetPreviewSessionArray(TemplateInfoSender *tempSender);
    void onGetLatLon(TemplateInfoSender *tempSender);
    void onNextInclination300Meters(TemplateInfoSender *tempSender);
    void onGetGPXBase64(TemplateInfoSender *tempSender);
    void onStart(TemplateInfoSender *tempSender);
    void onPause(TemplateInfoSender *tempSender);
    void onStop(TemplateInfoSender *tempSender);
    QString workoutName = QStringLiteral("");
    QString workoutStartDate = QStringLiteral("");
    QString instructorName = QStringLiteral("");
  private slots:
    void onUpdateTimeout();
    void onDataReceived(const QByteArray &data);
  public slots:
    void onWorkoutNameChanged(QString name) { workoutName = name; }
    void onWorkoutStartDate(QString name) { workoutStartDate = name; }
    void onInstructorName(QString name) { instructorName = name; }
    void workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state);
};

#endif // TEMPLATEINFOSENDERBUILDER_H

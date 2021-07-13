#ifndef TEMPLATEINFOSENDERBUILDER_H
#define TEMPLATEINFOSENDERBUILDER_H
#include "bluetoothdevice.h"
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
    static TemplateInfoSenderBuilder *getInstance(QObject *parent = nullptr);
    void reinit();
    void start(bluetoothdevice *device);
    void stop();
    QStringList templateIdList() const;
    ~TemplateInfoSenderBuilder();
  signals:
    void activityDescriptionChanged(QString newDescription);

  private:
    bool validFileTemplateType(const QString &tp) const;
    void buildContext(bool forceReinit = false);
    QString activityDescription;
    void createTemplatesFromFolder(const QString &folder, QStringList &dirTemplates);
    bluetoothdevice *device = nullptr;
    QTimer updateTimer;
    QJsonArray sessionArray;
    QHash<QString, QVariant> context;
    QJSEngine *engine = nullptr;
    TemplateInfoSenderBuilder(QObject *parent);
    void load();
    static TemplateInfoSenderBuilder *instance;
    QSettings settings;
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
    void onSaveTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onLoadTrainingPrograms(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onAppendActivityDescription(const QJsonValue &msgContent, TemplateInfoSender *tempSender);
    void onGetSessionArray(TemplateInfoSender *tempSender);
  private slots:
    void onUpdateTimeout();
    void onDataReceived(const QByteArray &data);
};

#endif // TEMPLATEINFOSENDERBUILDER_H

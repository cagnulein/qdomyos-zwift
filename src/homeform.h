#ifndef HOMEFORM_H
#define HOMEFORM_H

#include <QQuickItem>
#include <QQmlApplicationEngine>
#include <QOAuth2AuthorizationCodeFlow>
#include <QNetworkReply>
#include "bluetooth.h"
#include "sessionline.h"
#include "trainprogram.h"

class DataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString secondLine READ secondLine WRITE setSecondLine NOTIFY secondLineChanged)
    Q_PROPERTY(int valueFontSize READ valueFontSize WRITE setValueFontSize NOTIFY valueFontSizeChanged)
    Q_PROPERTY(QString valueFontColor READ valueFontColor WRITE setValueFontColor NOTIFY valueFontColorChanged)
    Q_PROPERTY(int labelFontSize READ labelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(bool writable READ writable NOTIFY writableChanged)
    Q_PROPERTY(bool visibleItem READ visibleItem NOTIFY visibleChanged)
    Q_PROPERTY(QString plusName READ plusName NOTIFY plusNameChanged)
    Q_PROPERTY(QString minusName READ minusName NOTIFY minusNameChanged)

public:
    DataObject(QString name, QString icon, QString value, bool writable, QString id, int valueFontSize, int labelFontSize, QString valueFontColor = "white", QString secondLine = "");
    void setValue(QString value);
    void setSecondLine(QString value);
    void setValueFontSize(int value);
    void setValueFontColor(QString value);
    void setLabelFontSize(int value);
    void setVisible(bool visible);
    QString name() {return m_name;}
    QString icon() {return m_icon;}
    QString value() {return m_value;}
    QString secondLine() {return m_secondLine;}
    int valueFontSize() {return m_valueFontSize;}
    QString valueFontColor() {return m_valueFontColor;}
    int labelFontSize() {return m_labelFontSize;}
    bool writable() {return m_writable;}
    bool visibleItem() {return m_visible;}
    QString plusName() {return m_id + "_plus";}
    QString minusName() {return m_id + "_minus";}

    QString m_id;
    QString m_name;
    QString m_icon;
    QString m_value;
    QString m_secondLine = "";
    int m_valueFontSize;
    QString m_valueFontColor = "white";
    int m_labelFontSize;
    bool m_writable;
    bool m_visible = true;

signals:
    void valueChanged(QString value);
    void secondLineChanged(QString value);
    void valueFontSizeChanged(int value);
    void valueFontColorChanged(QString value);
    void labelFontSizeChanged(int value);
    void nameChanged(QString value);
    void iconChanged(QString value);
    void writableChanged(bool value);
    void visibleChanged(bool value);
    void plusNameChanged(QString value);
    void minusNameChanged(QString value);
};

class homeform: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool labelHelp READ labelHelp NOTIFY changeLabelHelp)
    Q_PROPERTY( bool device READ getDevice NOTIFY changeOfdevice)
    Q_PROPERTY( bool zwift READ getZwift NOTIFY changeOfzwift)
    Q_PROPERTY(int topBarHeight READ topBarHeight NOTIFY topBarHeightChanged)
    Q_PROPERTY(QString info READ info NOTIFY infoChanged)    
    Q_PROPERTY(QString signal READ signal NOTIFY signalChanged)
    Q_PROPERTY(QString startText READ startText NOTIFY startTextChanged)
    Q_PROPERTY(QString startIcon READ startIcon NOTIFY startIconChanged)
    Q_PROPERTY(QString startColor READ startColor NOTIFY startColorChanged)
    Q_PROPERTY(QString stopText READ stopText NOTIFY stopTextChanged)
    Q_PROPERTY(QString stopIcon READ stopIcon NOTIFY stopIconChanged)
    Q_PROPERTY(QString stopColor READ stopColor NOTIFY stopColorChanged)
    Q_PROPERTY(QStringList bluetoothDevices READ bluetoothDevices NOTIFY bluetoothDevicesChanged)
    Q_PROPERTY(QStringList tile_order READ tile_order NOTIFY tile_orderChanged)
    Q_PROPERTY(bool generalPopupVisible READ generalPopupVisible NOTIFY generalPopupVisibleChanged WRITE setGeneralPopupVisible)

public:
    homeform(QQmlApplicationEngine* engine, bluetooth* bl);
    ~homeform();
    int topBarHeight() {return m_topBarHeight;}
    QString info() {return m_info;}
    QString signal();
    QString startText();
    QString startIcon();
    QString startColor();
    QString stopText();
    QString stopIcon();
    QString stopColor();
    bool generalPopupVisible();
    bool labelHelp();
    QStringList bluetoothDevices();
    QStringList tile_order();
    void setGeneralPopupVisible(bool value);

private:
    QList<QObject *> dataList;
    QList<SessionLine> Session;
    bluetooth* bluetoothManager = 0;
    QQmlApplicationEngine* engine;
    trainprogram* trainProgram = 0;
    QString backupFitFileName = "QZ-backup-" + QDateTime::currentDateTime().toString().replace(":", "_") + ".fit";

    int m_topBarHeight = 120;
    QString m_info = "Connecting...";
    bool m_labelHelp = true;
    bool m_generalPopupVisible = false;
    QOAuth2AuthorizationCodeFlow* strava;
    QNetworkAccessManager* manager = 0;

    bool paused = false;
    bool stopped = false;

    DataObject* speed;
    DataObject* inclination;
    DataObject* cadence;
    DataObject* elevation;
    DataObject* calories;
    DataObject* odometer;
    DataObject* pace;
    DataObject* datetime;
    DataObject* resistance;
    DataObject* watt;
    DataObject* avgWatt;
    DataObject* heart;
    DataObject* fan;
    DataObject* jouls;
    DataObject* elapsed;
    DataObject* peloton_resistance;
    DataObject* ftp;

    QTimer* timer;
    QTimer* backupTimer;

    QString strava_code;
    QOAuth2AuthorizationCodeFlow* strava_connect();
    void strava_refreshtoken();
    QNetworkReply *replyStrava;
    QAbstractOAuth::ModifyParametersFunction buildModifyParametersFunction(QUrl clientIdentifier,QUrl clientIdentifierSharedKey);
    bool strava_upload_file(QByteArray &data, QString remotename);

    void update();
    void backup();
    bool getDevice();
    bool getZwift();

private slots:
    void Start();
    void Stop();
    void Minus(QString);
    void Plus(QString);
    void deviceFound(QString name);
    void deviceConnected();
    void trainprogram_open_clicked(QUrl fileName);
    void gpx_open_clicked(QUrl fileName);
    void gpx_save_clicked();
    void fit_save_clicked();
    void strava_connect_clicked();
    void trainProgramSignals();
    void refresh_bluetooth_devices_clicked();
    void onStravaGranted();
    void onStravaAuthorizeWithBrowser(const QUrl &url);
    void replyDataReceived(QByteArray v);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError>& error);
    void networkRequestFinished(QNetworkReply *reply);
    void callbackReceived(const QVariantMap &values);
    void writeFileCompleted();
    void errorOccurredUploadStrava(QNetworkReply::NetworkError code);

signals:

 void changeOfdevice();
 void changeOfzwift();
 void signalChanged(QString value);
 void startTextChanged(QString value);
 void startIconChanged(QString value);
 void startColorChanged(QString value);
 void stopTextChanged(QString value);
 void stopIconChanged(QString value);
 void stopColorChanged(QString value);
 void infoChanged(QString value);
 void topBarHeightChanged(int value);
 void bluetoothDevicesChanged(QStringList value);
 void tile_orderChanged(QStringList value);
 void changeLabelHelp(bool value);
 void generalPopupVisibleChanged(bool value);
};

#endif // HOMEFORM_H

#include "templateinfosenderbuilder.h"
#include "treadmill.h"
#include "bike.h"
#include <QStandardPaths>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QNetworkInterface>
#ifdef Q_HTTPSERVER
#include "webserverinfosender.h"
#endif
#include "tcpclientinfosender.h"

TemplateInfoSenderBuilder * TemplateInfoSenderBuilder::instance = 0;
TemplateInfoSenderBuilder::TemplateInfoSenderBuilder(QObject * parent):QObject(parent)
{
    engine = new QJSEngine(this);
    engine->installExtensions(QJSEngine::AllExtensions);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeout()));
    updateTimer.setSingleShot(false);
}

TemplateInfoSenderBuilder::~TemplateInfoSenderBuilder() {
    stop();
}

void TemplateInfoSenderBuilder::onUpdateTimeout() {
    buildContext();
    QHash<QString,TemplateInfoSender *>::Iterator it;
    bool rv;
    for(it = templateInfoMap.begin(); it != templateInfoMap.end(); it++) {
        rv = it.value()->update(engine);
        if (!rv)
            qDebug() << "Error updating" << it.key() << "template";
    }
}

void TemplateInfoSenderBuilder::stop() {
    updateTimer.stop();
    QHash<QString,TemplateInfoSender *>::Iterator it;
    for(it = templateInfoMap.begin(); it != templateInfoMap.end(); it++) {
        it.value()->stop();
    }
}

TemplateInfoSenderBuilder * TemplateInfoSenderBuilder::getInstance(QObject * parent) {
    if (instance)
        return instance;
    else {
        instance = new TemplateInfoSenderBuilder(parent);
        instance->load();
        return instance;
    }
}

bool TemplateInfoSenderBuilder::validFileTemplateType(const QString& tp) const {
    return tp == TEMPLATE_TYPE_TCPCLIENT;
}

void TemplateInfoSenderBuilder::createTemplatesFromFolder(const QString& folder, QStringList& dirTemplates) {
    QDirIterator it(folder);
    QString content, templateId;
    QString tempType;
    QString fileName, filePath;
    QFileInfo fileInfo;
    while (it.hasNext()) {
        filePath = it.next();
        fileInfo = it.fileInfo();
        if (fileInfo.isFile() && fileInfo.completeSuffix() == "qzt" && (fileName = it.fileName()).length() > 4) {
            qDebug() << "Template File Found"<<filePath;
            QFile f(filePath);
            if (!f.open(QFile::ReadOnly | QFile::Text)) continue;
            QTextStream in(&f);
            if (f.size() && !(content = in.readAll()).isEmpty()) {
                templateId = fileName.left(fileName.length()-4);
                int idx = templateId.lastIndexOf("-");
                if (idx > 0) {
                    QString tempType = templateId.mid(idx + 1);
                    templateId = templateId.mid(0, idx);
                    qDebug() << "Template type"<<tempType<<" id"<<templateId;
                    templateFilesList.insert(templateId, filePath);
                    QString savedType = settings.value("template_" + templateId + "_type", QString()).toString();
                    if (savedType != tempType && validFileTemplateType(tempType)) {
                        settings.setValue("template_" + templateId + "_enabled", false);
                        settings.setValue("template_" + templateId + "_type", tempType);
                    }
                    else if (settings.value("template_" + templateId + "_enabled", false).toBool()) {
                        newTemplate(templateId, tempType, content);
                    }
                    else
                        qDebug() << "Template"<<templateId<<" is disabled: not created";
                }
            }
        }
        else if (fileInfo.isDir()) {
            int idx = filePath.lastIndexOf('/');
            QString pathEl = idx<0?filePath:filePath.mid(idx + 1);
            qDebug() << "Template Dir Found"<<filePath;
            dirTemplates += pathEl;
        }
    }
}

void TemplateInfoSenderBuilder::load() {
    QString path = "";
    QStringList dirTemplatesQrc, dirTemplatesLocal;
    stop();
#if defined(Q_OS_ANDROID) || defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif
    templateInfoMap.clear();
    templateFilesList.clear();
    qDebug() << "Load start from qrc";
    createTemplatesFromFolder(":/templates", dirTemplatesQrc);
    qDebug() << "Load start from local"<<path+"QZTemplates";
    createTemplatesFromFolder(path + "QZTemplates", dirTemplatesLocal);
    for (auto& tdir: dirTemplatesLocal) {
        if (dirTemplatesQrc.contains(tdir))
            dirTemplatesQrc.removeAll(tdir);
    }
    QMutableListIterator<QString> it(dirTemplatesLocal);
    while (it.hasNext()) {
        it.setValue(path + "QZTemplates/" + it.next());
    }
    for (auto& fld: dirTemplatesQrc) {
        dirTemplatesLocal.append(":/templates/" + fld);
    }
    if (!dirTemplatesLocal.isEmpty()) {
        QStringList addressList;
        qDebug()<< "Folder List"<<dirTemplatesLocal;
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (auto &address: QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
                 addressList += address.toString();
        }
        qDebug() << "addressList "<<addressList;
        settings.setValue("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_ips", addressList);
        templateFilesList.insert(TEMPLATE_PRIVATE_WEBSERVER_ID, TEMPLATE_TYPE_WEBSERVER);
        QString temptype = settings.value("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_type", QString()).toString();
        settings.setValue("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_folders", dirTemplatesLocal);
        settings.setValue("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_ips", addressList);
        if (temptype != TEMPLATE_TYPE_WEBSERVER) {
            settings.setValue("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_type", QString(TEMPLATE_TYPE_WEBSERVER));
            settings.setValue("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_enabled", false);
        }
        else if (settings.value("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_enabled", false).toBool()) {
            newTemplate(TEMPLATE_PRIVATE_WEBSERVER_ID, TEMPLATE_TYPE_WEBSERVER, "JSON.stringify({msg: \"workout\", content: this.workout})");
        }
        else
            qDebug() << "Template"<<TEMPLATE_PRIVATE_WEBSERVER_ID<<" is disabled: not created";
    }
    qDebug()<< "Setting template_ids"<<templateFilesList.keys();
    settings.setValue("template_ids", QStringList(templateFilesList.keys()));
}

TemplateInfoSender * TemplateInfoSenderBuilder::newTemplate(const QString& id, const QString& tp, const QString& dataTempl) {
    TemplateInfoSender * tempInfo = 0;
#ifdef Q_HTTPSERVER
    if (tp == TEMPLATE_TYPE_WEBSERVER) {
        tempInfo = new WebServerInfoSender(id, this);
    }
    else
#endif
    if (tp == TEMPLATE_TYPE_TCPCLIENT) {
        tempInfo = new TcpClientInfoSender(id, this);
    }
    if (tempInfo) {
        TemplateInfoSender * old;
        if ((old = templateInfoMap.value(id, 0))) {
            delete old;
        }
        qDebug() << "Template Registered"<<id <<" type"<<tp<<" Template"<<dataTempl;
        templateInfoMap.insert(id, tempInfo);
        tempInfo->init(dataTempl);
        connect(tempInfo, SIGNAL(onDataReceived(QByteArray)), this, SLOT(onDataReceived(QByteArray)));
    }
    return tempInfo;
}

void TemplateInfoSenderBuilder::reinit() {
    load();
}

void TemplateInfoSenderBuilder::start(bluetoothdevice * dev) {
    device = dev;

    updateTimer.start(1000);
}

QStringList TemplateInfoSenderBuilder::templateIdList() const {
    return templateFilesList.keys();
}

void TemplateInfoSenderBuilder::onGetSettings(const QJsonValue& val, TemplateInfoSender * tempSender) {
    QJsonDocument out = QJsonDocument::fromJson("{msg: \"R_getsettings\", content: null}");
    QJsonObject outObj;
    QStringList keys = settings.childKeys();
    QJsonValue keys_req;
    QJsonArray keys_arr;
    QVariantList keys_to_retrieve;
    if (val.isObject() && (keys_req = val.toObject()["keys"]).isArray() && (keys_arr = keys_req.toArray()).size()) {
        keys_to_retrieve = keys_arr.toVariantList();
        QString key;
        for (auto& kk: keys_to_retrieve) {
            key = kk.toString();
            if (settings.contains(key)) {
                outObj.insert(key, QJsonValue::fromVariant(settings.value(key)));
            }
        }
    }
    else {
        for (auto& key: settings.childKeys()) {
            outObj.insert(key, QJsonValue::fromVariant(settings.value(key)));
        }
    }
    out.object()["content"] = outObj;
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetSettings(const QJsonObject& obj, TemplateInfoSender * tempSender) {

    QStringList keys = obj.keys();
    QJsonValue val;
    QVariant valConv;
    QVariant settingVal;
    QJsonDocument out = QJsonDocument::fromJson("{msg: \"R_setsettings\", content: null}");
    QJsonObject outObj;
    foreach (const QString key, keys) {
        if (settings.contains(key)) {
            val = obj[key];
            valConv = val.toVariant();
            settingVal = settings.value(key);
            if (valConv.type() == settingVal.type()) {
                settings.setValue(key, valConv);
                outObj.insert(key, val);
            }
            else {
                outObj.insert(key, QJsonValue::fromVariant(settingVal));
            }
        }
        else {
            outObj.insert(key, QJsonValue());
        }
    }
    out.object()["content"] = outObj;
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onDataReceived(QByteArray data) {
    TemplateInfoSender* sender = qobject_cast<TemplateInfoSender*>(this->sender());
    if (!sender)
        return;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    if (jsonResponse.isObject()) {
        QJsonObject jsonObject = jsonResponse.object();
        if (jsonObject.contains("msg")) {
            QJsonValue msgType = jsonObject["msg"];
            if (msgType.isString()) {
                QString msg = msgType.toString();
                if (msg == "getsettings") {
                    onGetSettings(jsonObject["content"], sender);
                    return;
                }
                else if (msg == "setsettings") {
                    if (jsonObject.contains("content")) {
                        QJsonValue msgContent = jsonObject["content"];
                        if (msgContent.isObject()) {
                            onSetSettings(msgContent.toObject(), sender);
                            return;
                        }
                    }
                }
            }
        }
    }
    qDebug() << "Unrecognized message" <<data;
}

void TemplateInfoSenderBuilder::buildContext()  {
    QJSValue glob = engine->globalObject();
    QJSValue obj;
    if (!glob.hasOwnProperty("workout")) {
        obj = engine->newObject();
        glob.setProperty("workout", obj);
    }
    else
        obj = glob.property("workout");

    if (!glob.hasOwnProperty("settings")) {
        QJSValue sett = engine->newObject();
        glob.setProperty("settings", sett);
        QVariant::Type typesett;
        QVariant valsett;
        int i = 0;
        for (auto& key: settings.childKeys()) {
            valsett = settings.value(key);
            typesett = valsett.type();
            if (typesett == QVariant::Int)
                sett.setProperty(key, valsett.toInt());
            else if (typesett == QVariant::Double)
                sett.setProperty(key, valsett.toDouble());
            else if (typesett == QVariant::String)
                sett.setProperty(key, valsett.toString());
            else if (typesett == QVariant::Bool)
                sett.setProperty(key, valsett.toBool());
            else if (typesett == QVariant::UInt)
                sett.setProperty(key, valsett.toUInt());
            else if (typesett == QVariant::StringList) {
                QStringList settL = valsett.toStringList();
                QJSValue settLJ = engine->newArray(settL.size());
                i = 0;
                for (auto& settLK: settL)
                    settLJ.setProperty(i++, settLK);
                sett.setProperty(key, settLJ);
            }
        }
    }
    if (!device)
        obj.setProperty("deviceId", QJSValue());
    else {
        QTime el = device->elapsedTime();
        QString name;
        bluetoothdevice::BLUETOOTH_TYPE tp = device->deviceType();

        metric dep;
#ifdef Q_OS_IOS
        obj.setProperty("deviceId", device->bluetoothDevice.deviceUuid().toString());
#else
        obj.setProperty("deviceId", device->bluetoothDevice.address().toString());
#endif
        obj.setProperty("deviceName", (name = device->bluetoothDevice.name()).isEmpty()?QString("N/A"):name);
        obj.setProperty("deviceRSSI", device->bluetoothDevice.rssi());
        obj.setProperty("deviceType", (int)device->deviceType());
        obj.setProperty("deviceConnected", (bool)device->connected());
        obj.setProperty("BIKE_TYPE", (int)bluetoothdevice::BIKE);
        obj.setProperty("ELLIPTICAL_TYPE", (int)bluetoothdevice::ELLIPTICAL);
        obj.setProperty("ROWING_TYPE", (int)bluetoothdevice::ROWING);
        obj.setProperty("TREADMILL_TYPE", (int)bluetoothdevice::TREADMILL);
        obj.setProperty("UNKNOWN_TYPE", (int)bluetoothdevice::UNKNOWN);
        obj.setProperty("elapsed_s", el.second());
        obj.setProperty("elapsed_m", el.minute());
        obj.setProperty("elapsed_h", el.hour());
        el = device->currentPace();
        obj.setProperty("pace_s", el.second());
        obj.setProperty("pace_m", el.minute());
        obj.setProperty("pace_h", el.hour());
        obj.setProperty("speed", (dep = device->currentSpeed()).value());
        obj.setProperty("speed_avg", dep.average());
        obj.setProperty("calories", device->calories());
        obj.setProperty("distance", device->odometer());
        obj.setProperty("heart", (dep = device->currentHeart()).value());
        obj.setProperty("heart_avg", dep.average());
        obj.setProperty("jouls", device->jouls().value());
        obj.setProperty("elevation", device->elevationGain());
        obj.setProperty("watts", (dep = device->wattsMetric()).value());
        obj.setProperty("watts_avg", dep.average());
        if (tp == bluetoothdevice::BIKE) {
            obj.setProperty("peloton_resistance", (dep = ((bike *)device)->pelotonResistance()).value());
            obj.setProperty("peloton_resistance_avg", dep.average());
            obj.setProperty("cadence", (dep = ((bike *)device)->currentCadence()).value());
            obj.setProperty("cadence_avg", dep.average());
            obj.setProperty("resistance", (dep = ((bike *)device)->currentResistance()).value());
            obj.setProperty("resistance_avg", dep.average());
            obj.setProperty("cranks", ((bike *)device)->currentCrankRevolutions());
            obj.setProperty("cranktime", ((bike *)device)->lastCrankEventTime());
        }
        else {
            obj.setProperty("resistance", (dep = ((treadmill *)device)->currentInclination()).value());
            obj.setProperty("resistance_avg", dep.average());
        }
    }
}

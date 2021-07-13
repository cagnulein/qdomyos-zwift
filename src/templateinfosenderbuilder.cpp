#include "templateinfosenderbuilder.h"
#include "bike.h"
#include "treadmill.h"
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QTime>
#ifdef Q_HTTPSERVER
#include "webserverinfosender.h"
#endif
#include "homeform.h"
#include "tcpclientinfosender.h"
#include "trainprogram.h"
#include <chrono>

using namespace std::chrono_literals;

TemplateInfoSenderBuilder *TemplateInfoSenderBuilder::instance = nullptr;
TemplateInfoSenderBuilder::TemplateInfoSenderBuilder(QObject *parent) : QObject(parent) {
    engine = new QJSEngine(this);
    engine->installExtensions(QJSEngine::AllExtensions);
    connect(&updateTimer, &QTimer::timeout, this, &TemplateInfoSenderBuilder::onUpdateTimeout);
    updateTimer.setSingleShot(false);
}

TemplateInfoSenderBuilder::~TemplateInfoSenderBuilder() { stop(); }

void TemplateInfoSenderBuilder::onUpdateTimeout() {
    buildContext();
    QHash<QString, TemplateInfoSender *>::Iterator it;
    bool rv;
    for (it = templateInfoMap.begin(); it != templateInfoMap.end(); it++) {
        rv = it.value()->update(engine);
        if (!rv) {
            qDebug() << QStringLiteral("Error updating") << it.key() << QStringLiteral("template");
        }
    }
}

void TemplateInfoSenderBuilder::stop() {
    updateTimer.stop();
    QHash<QString, TemplateInfoSender *>::Iterator it;
    for (it = templateInfoMap.begin(); it != templateInfoMap.end(); it++) {
        it.value()->stop();
    }
}

TemplateInfoSenderBuilder *TemplateInfoSenderBuilder::getInstance(QObject *parent) {
    if (instance) {
        return instance;
    } else {
        instance = new TemplateInfoSenderBuilder(parent);
        instance->load();
        return instance;
    }
}

bool TemplateInfoSenderBuilder::validFileTemplateType(const QString &tp) const { return tp == TEMPLATE_TYPE_TCPCLIENT; }

void TemplateInfoSenderBuilder::createTemplatesFromFolder(const QString &folder, QStringList &dirTemplates) {
    QDirIterator it(folder);
    QString content, templateId;
    // QString tempType; // NOTE: clazy-unused-non-triviak-variable
    QString fileName, filePath;
    QFileInfo fileInfo;
    while (it.hasNext()) {
        filePath = it.next();
        fileInfo = it.fileInfo();
        if (fileInfo.isFile() && fileInfo.completeSuffix() == QStringLiteral("qzt") &&
            (fileName = it.fileName()).length() > 4) {
            qDebug() << QStringLiteral("Template File Found") << filePath;
            QFile f(filePath);
            if (!f.open(QFile::ReadOnly | QFile::Text)) {
                continue;
            }
            QTextStream in(&f);
            if (f.size() && !(content = in.readAll()).isEmpty()) {
                templateId = fileName.left(fileName.length() - 4);
                int idx = templateId.lastIndexOf(QStringLiteral("-"));
                if (idx > 0) {
                    QString tempType = templateId.mid(idx + 1);
                    templateId = templateId.mid(0, idx);
                    qDebug() << QStringLiteral("Template type") << tempType << QStringLiteral(" id") << templateId;
                    templateFilesList.insert(templateId, filePath);
                    QString savedType =
                        settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_type"), QString())
                            .toString();
                    if (savedType != tempType && validFileTemplateType(tempType)) {
                        settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_enabled"), false);
                        settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_type"), tempType);
                    } else if (settings
                                   .value(QStringLiteral("template_") + templateId + QStringLiteral("_enabled"), false)
                                   .toBool()) {
                        newTemplate(templateId, tempType, content);
                    } else {
                        qDebug() << QStringLiteral("Template") << templateId
                                 << QStringLiteral(" is disabled: not created");
                    }
                }
            }
        } else if (fileInfo.isDir()) {
            int idx = filePath.lastIndexOf('/');
            QString pathEl = idx < 0 ? filePath : filePath.mid(idx + 1);
            if (pathEl != QStringLiteral(".") && pathEl != QStringLiteral("..")) {
                qDebug() << QStringLiteral("Template Dir Found") << filePath;
                dirTemplates += pathEl;
            }
        }
    }
}

void TemplateInfoSenderBuilder::load() {
    QString path = QLatin1String("");
    QStringList dirTemplatesQrc, dirTemplatesLocal;
    stop();
#if defined(Q_OS_ANDROID) || defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif
    templateInfoMap.clear();
    templateFilesList.clear();
    qDebug() << QStringLiteral("Load start from qrc");
    createTemplatesFromFolder(QStringLiteral(":/templates"), dirTemplatesQrc);
    qDebug() << QStringLiteral("Load start from local") << path + QStringLiteral("QZTemplates");
    createTemplatesFromFolder(path + QStringLiteral("QZTemplates"), dirTemplatesLocal);
    for (auto &tdir : dirTemplatesLocal) {
        if (dirTemplatesQrc.contains(tdir)) {
            dirTemplatesQrc.removeAll(tdir);
        }
    }
    QMutableListIterator<QString> it(dirTemplatesLocal);
    while (it.hasNext()) {
        it.setValue(path + QStringLiteral("QZTemplates/") + it.next());
    }
    dirTemplatesLocal.reserve(dirTemplatesQrc.size() + 1);
    for (auto &fld : dirTemplatesQrc) {
        dirTemplatesLocal.append(QStringLiteral(":/templates/") + fld);
    }
    if (!dirTemplatesLocal.isEmpty()) {
        QStringList addressList;
        qDebug() << QStringLiteral("Folder List") << dirTemplatesLocal;
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (auto &address : QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
                addressList += address.toString();
            }
        }
        qDebug() << QStringLiteral("addressList ") << addressList;
        settings.setValue(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_ips"), addressList);
        templateFilesList.insert(QStringLiteral(TEMPLATE_PRIVATE_WEBSERVER_ID), TEMPLATE_TYPE_WEBSERVER);
        QString temptype =
            settings.value(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_type"), QString()).toString();
        settings.setValue(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_folders"), dirTemplatesLocal);
        settings.setValue(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_ips"), addressList);
        if (temptype != TEMPLATE_TYPE_WEBSERVER) {
            settings.setValue(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_type"),
                              QString(TEMPLATE_TYPE_WEBSERVER));
            settings.setValue(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_enabled"), false);
        } else if (settings.value(QStringLiteral("template_" TEMPLATE_PRIVATE_WEBSERVER_ID "_enabled"), false)
                       .toBool()) {
            newTemplate(QStringLiteral(TEMPLATE_PRIVATE_WEBSERVER_ID), TEMPLATE_TYPE_WEBSERVER,
                        QStringLiteral("JSON.stringify({msg: \"workout\", content: this.workout})"));
        } else {
            qDebug() << QStringLiteral("Template") << QStringLiteral(TEMPLATE_PRIVATE_WEBSERVER_ID)
                     << QStringLiteral(" is disabled: not created");
        }
    }
    qDebug() << QStringLiteral("Setting template_ids") << templateFilesList.keys();
    settings.setValue(QStringLiteral("template_ids"), QStringList(templateFilesList.keys()));
}

TemplateInfoSender *TemplateInfoSenderBuilder::newTemplate(const QString &id, const QString &tp,
                                                           const QString &dataTempl) {
    TemplateInfoSender *tempInfo = nullptr;
#ifdef Q_HTTPSERVER
    if (tp == TEMPLATE_TYPE_WEBSERVER) {
        tempInfo = new WebServerInfoSender(id, this);
    } else
#endif
        if (tp == TEMPLATE_TYPE_TCPCLIENT) {
        tempInfo = new TcpClientInfoSender(id, this);
    }
    if (tempInfo) {
        TemplateInfoSender *old;
        if ((old = templateInfoMap.value(id, 0))) {
            delete old;
        }
        qDebug() << QStringLiteral("Template Registered") << id << QStringLiteral(" type") << tp
                 << QStringLiteral(" Template") << dataTempl;
        templateInfoMap.insert(id, tempInfo);
        tempInfo->init(dataTempl);
        connect(tempInfo, &TemplateInfoSender::onDataReceived, this, &TemplateInfoSenderBuilder::onDataReceived);
    }
    return tempInfo;
}

void TemplateInfoSenderBuilder::reinit() { load(); }

void TemplateInfoSenderBuilder::start(bluetoothdevice *dev) {
    device = nullptr;
    for (int i = 0; i < sessionArray.count(); i++) {
        sessionArray.removeAt(0);
    }
    buildContext(true);
    device = dev;
    activityDescription = QLatin1String("");
    updateTimer.start(1s);
}

QStringList TemplateInfoSenderBuilder::templateIdList() const { return templateFilesList.keys(); }

void TemplateInfoSenderBuilder::onGetSettings(const QJsonValue &val, TemplateInfoSender *tempSender) {
    QJsonObject outObj;
    QStringList keys = settings.allKeys();
    QJsonValue keys_req;
    QJsonArray keys_arr;
    QVariantList keys_to_retrieve;
    if (val.isObject() && (keys_req = val.toObject()[QStringLiteral("keys")]).isArray() &&
        !(keys_arr = keys_req.toArray()).isEmpty()) {
        keys_to_retrieve = keys_arr.toVariantList();
        QString key;
        for (auto &kk : keys_to_retrieve) {
            key = kk.toString();
            if (key.startsWith(QStringLiteral("$"))) {
                outObj.insert(key, 1);
                QRegExp regex(key.mid(1));
                for (auto &keypresent : settings.allKeys()) {
                    if (regex.indexIn(keypresent) >= 0) {
                        outObj.insert(keypresent, QJsonValue::fromVariant(settings.value(keypresent)));
                    }
                }
            } else if (settings.contains(key)) {
                outObj.insert(key, QJsonValue::fromVariant(settings.value(key)));
            } else {
                outObj.insert(key, QJsonValue());
            }
        }
    } else {
        for (auto &key : settings.allKeys()) {
            outObj.insert(key, QJsonValue::fromVariant(settings.value(key)));
        }
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_getsettings");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetResistance(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject obj, outObj;
    QJsonValue resVal;
    outObj[QStringLiteral("value")] = QJsonValue(QJsonValue::Null);
    if (device && msgContent.isObject() && (obj = msgContent.toObject()).contains(QStringLiteral("value")) &&
        (resVal = msgContent[QStringLiteral("value")]).isDouble()) {
        bluetoothdevice::BLUETOOTH_TYPE tp = device->deviceType();
        if (tp == bluetoothdevice::BIKE || tp == bluetoothdevice::ROWING) {
            int res;
            if ((res = resVal.toInt()) >= 0 && res < 255) {
                ((bike *)device)->changeResistance((uint8_t)res);
                outObj[QStringLiteral("value")] = res;
            }
        } else {
            double resd;
            ((treadmill *)device)->changeInclination(resd = resVal.toDouble());
            outObj[QStringLiteral("value")] = resd;
        }
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setresistance");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetFanSpeed(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject obj, outObj;
    QJsonValue resVal;
    int res;
    outObj[QStringLiteral("value")] = QJsonValue(QJsonValue::Null);
    if (device && msgContent.isObject() && (obj = msgContent.toObject()).contains(QStringLiteral("value")) &&
        (resVal = msgContent[QStringLiteral("value")]).isDouble() && (res = resVal.toInt()) >= 0 && res < 255) {
        outObj[QStringLiteral("value")] = res;
        ((bike *)device)->changeFanSpeed((uint8_t)res);
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setfanspeed");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetPower(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject obj, outObj;
    QJsonValue resVal;
    outObj[QStringLiteral("value")] = QJsonValue(QJsonValue::Null);
    if (device && msgContent.isObject() && (obj = msgContent.toObject()).contains(QStringLiteral("value")) &&
        (resVal = msgContent[QStringLiteral("value")]).isDouble() &&
        (device->deviceType() == bluetoothdevice::BIKE || device->deviceType() == bluetoothdevice::ROWING)) {
        int val;
        if ((val = resVal.toInt()) > 0) {
            ((bike *)device)->changePower((uint32_t)val);
            outObj[QStringLiteral("value")] = val;
        }
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setpower");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetCadence(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject obj, outObj;
    QJsonValue resVal;
    outObj[QStringLiteral("value")] = QJsonValue(QJsonValue::Null);
    if (device && msgContent.isObject() && (obj = msgContent.toObject()).contains(QStringLiteral("value")) &&
        (resVal = msgContent[QStringLiteral("value")]).isDouble() &&
        (device->deviceType() == bluetoothdevice::BIKE || device->deviceType() == bluetoothdevice::ROWING)) {
        int val;
        if ((val = resVal.toInt()) > 0) {
            ((bike *)device)->changeCadence((uint16_t)val);
            outObj[QStringLiteral("value")] = val;
        }
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setcadence");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetSpeed(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject obj, outObj;
    QJsonValue resVal;
    double vald;
    outObj[QStringLiteral("value")] = QJsonValue(QJsonValue::Null);
    if (device && msgContent.isObject() && (obj = msgContent.toObject()).contains(QStringLiteral("value")) &&
        (resVal = msgContent[QStringLiteral("value")]).isDouble() &&
        device->deviceType() == bluetoothdevice::TREADMILL && (vald = resVal.toDouble()) >= 0) {
        ((treadmill *)device)->changeSpeed(vald);
        outObj[QStringLiteral("value")] = vald;
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setspeed");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetDifficult(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject obj, outObj;
    QJsonValue resVal;
    outObj[QStringLiteral("value")] = QJsonValue(QJsonValue::Null);
    double vald;
    if (device && msgContent.isObject() && (obj = msgContent.toObject()).contains(QStringLiteral("value")) &&
        (resVal = msgContent[QStringLiteral("value")]).isDouble() && (vald = resVal.toDouble()) >= 0) {
        device->setDifficult(vald);
        outObj[QStringLiteral("value")] = vald;
    }
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setdifficult");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSetSettings(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    if (!msgContent.isObject()) {
        return;
    }
    QJsonObject obj = msgContent.toObject();
    QStringList keys = obj.keys();
    QJsonValue val;
    QVariant valConv;
    QVariant settingVal;
    QJsonObject outObj;
    for (auto &key : keys) {
        if (settings.contains(key)) {
            val = obj[key];
            valConv.setValue(val);
            settingVal.setValue(settings.value(key));
            if (valConv.type() == settingVal.type()) {
                settings.setValue(key, valConv);
                outObj.insert(key, val);
            } else {
                outObj.insert(key, QJsonValue::fromVariant(settingVal));
            }
        } else {
            val = obj[key];
            settings.setValue(key, val.toVariant());
            outObj.insert(key, val);
        }
    }
    settings.sync();
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_setsettings");
    main[QStringLiteral("content")] = outObj;
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onLoadTrainingPrograms(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject main;
    QJsonArray outArr;
    QJsonObject outObj;
    QString fileXml;
    if ((fileXml = msgContent.toString()).isEmpty()) {
        QDirIterator it(homeform::getWritableAppDir() + QStringLiteral("training"));
        QString fileName, filePath;
        QFileInfo fileInfo;
        while (it.hasNext()) {
            filePath = it.next();
            fileInfo = it.fileInfo();
            if (fileInfo.isFile() && fileInfo.completeSuffix() == QStringLiteral("xml") &&
                (fileName = it.fileName()).length() > 4) {
                outArr.append(fileName.mid(0, fileName.length() - 4));
            }
        }
    } else {
        QList<trainrow> lst = trainprogram::loadXML(homeform::getWritableAppDir() + QStringLiteral("training/") +
                                                    fileXml + QStringLiteral(".xml"));
        for (auto &row : lst) {
            QJsonObject item;
            item[QStringLiteral("duration")] = row.duration.toString();
            item[QStringLiteral("speed")] = row.speed;
            item[QStringLiteral("fanspeed")] = row.fanspeed;
            item[QStringLiteral("inclination")] = row.inclination;
            item[QStringLiteral("resistance")] = row.resistance;
            item[QStringLiteral("requested_peloton_resistance")] = row.requested_peloton_resistance;
            item[QStringLiteral("cadence")] = row.cadence;
            item[QStringLiteral("forcespeed")] = row.forcespeed;
            item[QStringLiteral("loopTimeHR")] = row.loopTimeHR;
            item[QStringLiteral("zoneHR")] = row.zoneHR;
            item[QStringLiteral("maxSpeed")] = row.maxSpeed;
            outArr.append(item);
        }
    }
    outObj[QStringLiteral("list")] = outArr;
    outObj[QStringLiteral("name")] = fileXml;
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_loadtrainingprograms");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onAppendActivityDescription(const QJsonValue &msgContent,
                                                            TemplateInfoSender *tempSender) {
    QJsonObject content;
    QJsonValue descV;
    if (!device || (content = msgContent.toObject()).isEmpty() || !content.contains(QStringLiteral("desc")) ||
        !(descV = content.value(QStringLiteral("desc"))).isString())
        return;
    QString desc = descV.toString();
    if (content.contains(QStringLiteral("append")) && content.value(QStringLiteral("append")).toBool()) {
        activityDescription =
            activityDescription.isEmpty() ? desc : activityDescription + QStringLiteral("\r\n") + desc;
    } else
        activityDescription = desc;
    emit activityDescriptionChanged(activityDescription);
    QJsonObject main;
    main[QStringLiteral("content")] = activityDescription;
    main[QStringLiteral("msg")] = QStringLiteral("R_appendactivitydescription");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onGetSessionArray(TemplateInfoSender *tempSender) {
    QJsonObject main;
    main[QStringLiteral("content")] = sessionArray;
    main[QStringLiteral("msg")] = QStringLiteral("R_getsessionarray");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSaveTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QString fileName;
    QJsonArray rows;
    QJsonObject content;
    if ((content = msgContent.toObject()).isEmpty() ||
        (fileName = content.value(QStringLiteral("name")).toString()).isEmpty() ||
        (rows = content.value(QStringLiteral("list")).toArray()).isEmpty()) {
        return;
    }
    QList<trainrow> trainRows;
    trainRows.reserve(rows.size() + 1);
    for (const auto &r : qAsConst(rows)) {
        QJsonObject row = r.toObject();
        trainrow tR;
        if (row.contains(QStringLiteral("duration"))) {
            tR.duration = QTime::fromString(row[QStringLiteral("duration")].toString(), QStringLiteral("hh:mm:ss"));
            if (row.contains(QStringLiteral("speed"))) {
                tR.speed = row[QStringLiteral("speed")].toDouble();
            }
            if (row.contains(QStringLiteral("fanspeed"))) {
                tR.fanspeed = row[QStringLiteral("fanspeed")].toInt();
            }
            if (row.contains(QStringLiteral("inclination"))) {
                tR.inclination = row[QStringLiteral("inclination")].toDouble();
            }
            if (row.contains(QStringLiteral("resistance"))) {
                tR.resistance = row[QStringLiteral("resistance")].toInt();
            }
            if (row.contains(QStringLiteral("requested_peloton_resistance"))) {
                tR.requested_peloton_resistance = row[QStringLiteral("requested_peloton_resistance")].toInt();
            }
            if (row.contains(QStringLiteral("cadence"))) {
                tR.cadence = row[QStringLiteral("cadence")].toInt();
            }
            if (row.contains(QStringLiteral("forcespeed"))) {
                tR.forcespeed = (bool)row[QStringLiteral("forcespeed")].toInt();
            }
            if (row.contains(QStringLiteral("loopTimeHR"))) {
                tR.loopTimeHR = row[QStringLiteral("loopTimeHR")].toInt();
            }
            if (row.contains(QStringLiteral("zoneHR"))) {
                tR.zoneHR = row[QStringLiteral("zoneHR")].toInt();
            }
            if (row.contains(QStringLiteral("maxSpeed"))) {
                tR.maxSpeed = row[QStringLiteral("maxSpeed")].toInt();
            }
            trainRows.append(tR);
        }
    }
    QJsonObject main, outObj;
    QString trainingDir(homeform::getWritableAppDir() + QStringLiteral("training/"));
    QDir dir(trainingDir);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
    outObj[QStringLiteral("name")] = fileName;
    if (trainprogram::saveXML(trainingDir + fileName + QStringLiteral(".xml"), trainRows)) {
        outObj[QStringLiteral("list")] = trainRows.size();
    } else {
        outObj[QStringLiteral("list")] = 0;
    }
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_savetrainingprogram");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onDataReceived(const QByteArray &data) {
    TemplateInfoSender *sender = qobject_cast<TemplateInfoSender *>(this->sender());
    if (!sender) {
        return;
    }
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
    if (jsonResponse.isObject()) {
        QJsonObject jsonObject = jsonResponse.object();
        if (jsonObject.contains(QStringLiteral("msg"))) {
            QJsonValue msgType = jsonObject[QStringLiteral("msg")];
            if (msgType.isString()) {
                QString msg = msgType.toString();
                if (msg == QStringLiteral("getsettings")) {
                    onGetSettings(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setresistance")) {
                    onSetResistance(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setpower")) {
                    onSetPower(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setcadence")) {
                    onSetCadence(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setdifficult")) {
                    onSetDifficult(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setspeed")) {
                    onSetSpeed(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setfanspeed")) {
                    onSetFanSpeed(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("setsettings")) {
                    onSetSettings(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("loadtrainingprograms")) {
                    onLoadTrainingPrograms(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("appendactivitydescription")) {
                    onAppendActivityDescription(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("savetrainingprogram")) {
                    onSaveTrainingProgram(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("getsessionarray")) {
                    onGetSessionArray(sender);
                    return;
                }
            }
        }
    }
    qDebug() << QStringLiteral("Unrecognized message") << data;
}

void TemplateInfoSenderBuilder::buildContext(bool forceReinit) {
    QJSValue glob = engine->globalObject();
    QJSValue obj;
    if (!glob.hasOwnProperty(QStringLiteral("workout")) || forceReinit) {
        obj = engine->newObject();
        glob.setProperty(QStringLiteral("workout"), obj);
    } else
        obj = glob.property(QStringLiteral("workout"));

    if (!glob.hasOwnProperty(QStringLiteral("settings")) || forceReinit) {
        QJSValue sett = engine->newObject();
        glob.setProperty(QStringLiteral("settings"), sett);
        QVariant::Type typesett;
        QVariant valsett;
        int i = 0;
        auto allKeys_list = settings.allKeys();
        for (const auto &key : allKeys_list) {
            valsett.setValue(settings.value(key));
            typesett = valsett.type();
            if (typesett == QVariant::Int) {
                sett.setProperty(key, valsett.toInt());
            } else if (typesett == QVariant::Double) {
                sett.setProperty(key, valsett.toDouble());
            } else if (typesett == QVariant::String) {
                sett.setProperty(key, valsett.toString());
            } else if (typesett == QVariant::Bool) {
                sett.setProperty(key, valsett.toBool());
            } else if (typesett == QVariant::UInt) {
                sett.setProperty(key, valsett.toUInt());
            } else if (typesett == QVariant::StringList) {
                QStringList settL = valsett.toStringList();
                QJSValue settLJ = engine->newArray(settL.size());
                i = 0;
                for (const auto &settLK : qAsConst(settL)) {
                    settLJ.setProperty(i++, settLK);
                }
                sett.setProperty(key, settLJ);
            }
        }
        obj.setProperty(QStringLiteral("BIKE_TYPE"), (int)bluetoothdevice::BIKE);
        obj.setProperty(QStringLiteral("ELLIPTICAL_TYPE"), (int)bluetoothdevice::ELLIPTICAL);
        obj.setProperty(QStringLiteral("ROWING_TYPE"), (int)bluetoothdevice::ROWING);
        obj.setProperty(QStringLiteral("TREADMILL_TYPE"), (int)bluetoothdevice::TREADMILL);
        obj.setProperty(QStringLiteral("UNKNOWN_TYPE"), (int)bluetoothdevice::UNKNOWN);
    }
    if (!device) {
        obj.setProperty(QStringLiteral("deviceId"), QJSValue());
    } else {
        QTime el = device->elapsedTime();
        QString name;
        bluetoothdevice::BLUETOOTH_TYPE tp = device->deviceType();

        metric dep;
#ifdef Q_OS_IOS
        obj.setProperty("deviceId", device->bluetoothDevice.deviceUuid().toString());
#else
        obj.setProperty(QStringLiteral("deviceId"), device->bluetoothDevice.address().toString());
#endif
        obj.setProperty(QStringLiteral("deviceName"),
                        (name = device->bluetoothDevice.name()).isEmpty() ? QString(QStringLiteral("N/A")) : name);
        obj.setProperty(QStringLiteral("deviceRSSI"), device->bluetoothDevice.rssi());
        obj.setProperty(QStringLiteral("deviceType"), (int)device->deviceType());
        obj.setProperty(QStringLiteral("deviceConnected"), (bool)device->connected());
        obj.setProperty(QStringLiteral("devicePaused"), (bool)device->isPaused());
        obj.setProperty(QStringLiteral("elapsed_s"), el.second());
        obj.setProperty(QStringLiteral("elapsed_m"), el.minute());
        obj.setProperty(QStringLiteral("elapsed_h"), el.hour());
        el = device->currentPace();
        obj.setProperty(QStringLiteral("pace_s"), el.second());
        obj.setProperty(QStringLiteral("pace_m"), el.minute());
        obj.setProperty(QStringLiteral("pace_h"), el.hour());
        el = device->movingTime();
        obj.setProperty(QStringLiteral("moving_s"), el.second());
        obj.setProperty(QStringLiteral("moving_m"), el.minute());
        obj.setProperty(QStringLiteral("moving_h"), el.hour());
        obj.setProperty(QStringLiteral("speed"), (dep = device->currentSpeed()).value());
        obj.setProperty(QStringLiteral("speed_avg"), dep.average());
        obj.setProperty(QStringLiteral("calories"), device->calories());
        obj.setProperty(QStringLiteral("distance"), device->odometer());
        obj.setProperty(QStringLiteral("heart"), (dep = device->currentHeart()).value());
        obj.setProperty(QStringLiteral("heart_avg"), dep.average());
        obj.setProperty(QStringLiteral("jouls"), device->jouls().value());
        obj.setProperty(QStringLiteral("elevation"), device->elevationGain());
        obj.setProperty(QStringLiteral("difficult"), device->difficult());
        obj.setProperty(QStringLiteral("watts"), (dep = device->wattsMetric()).value());
        obj.setProperty(QStringLiteral("watts_avg"), dep.average());
        if (tp == bluetoothdevice::BIKE) {
            obj.setProperty(QStringLiteral("peloton_resistance"),
                            (dep = ((bike *)device)->pelotonResistance()).value());
            obj.setProperty(QStringLiteral("peloton_resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence"), (dep = ((bike *)device)->currentCadence()).value());
            obj.setProperty(QStringLiteral("cadence_avg"), dep.average());
            obj.setProperty(QStringLiteral("resistance"), (dep = ((bike *)device)->currentResistance()).value());
            obj.setProperty(QStringLiteral("resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("cranks"), ((bike *)device)->currentCrankRevolutions());
            obj.setProperty(QStringLiteral("cranktime"), ((bike *)device)->lastCrankEventTime());
        } else if (tp == bluetoothdevice::ROWING) {
            obj.setProperty(QStringLiteral("peloton_resistance"),
                            (dep = ((rower *)device)->pelotonResistance()).value());
            obj.setProperty(QStringLiteral("peloton_resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence"), (dep = ((rower *)device)->currentCadence()).value());
            obj.setProperty(QStringLiteral("cadence_avg"), dep.average());
            obj.setProperty(QStringLiteral("resistance"), (dep = ((rower *)device)->currentResistance()).value());
            obj.setProperty(QStringLiteral("resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("cranks"), ((rower *)device)->currentCrankRevolutions());
            obj.setProperty(QStringLiteral("cranktime"), ((rower *)device)->lastCrankEventTime());
            obj.setProperty(QStringLiteral("strokescount"), ((rower *)device)->currentStrokesCount().value());
            obj.setProperty(QStringLiteral("strokeslength"), ((rower *)device)->currentStrokesLength().value());
        } else {
            obj.setProperty(QStringLiteral("resistance"), (dep = ((treadmill *)device)->currentInclination()).value());
            obj.setProperty(QStringLiteral("resistance_avg"), dep.average());
        }
        QJsonObject o;
        o.fromVariantMap(obj.toVariant().toMap());
        sessionArray.append(o);
    }
}

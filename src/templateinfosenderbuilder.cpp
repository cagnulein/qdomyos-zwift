#include "templateinfosenderbuilder.h"
#include "devices/bike.h"
#include "treadmill.h"
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QTime>
#include <limits>
#ifdef Q_HTTPSERVER
#include "webserverinfosender.h"
#endif
#include "homeform.h"
#include "tcpclientinfosender.h"
#include "trainprogram.h"
#include <chrono>

using namespace std::chrono_literals;

#define TRAINPROGRAM_FIELD_TO_STRING()                                                                      \
    item[QStringLiteral("duration")] = row.duration.toString();                                             \
    item[QStringLiteral("duration_s")] = QTime(0,0,0).secsTo(row.duration);                                 \
    item[QStringLiteral("distance")] = row.distance;                                                        \
    item[QStringLiteral("speed")] = row.speed;                                                              \
    item[QStringLiteral("minspeed")] = row.minSpeed;                                                        \
    item[QStringLiteral("maxspeed")] = row.maxSpeed;                                                        \
    item[QStringLiteral("fanspeed")] = row.fanspeed;                                                        \
    item[QStringLiteral("inclination")] = row.inclination;                                                  \
    item[QStringLiteral("resistance")] = row.resistance;                                                    \
    item[QStringLiteral("maxresistance")] = row.maxResistance;                                              \
    item[QStringLiteral("mets")] = row.mets;                                                                \
    item[QStringLiteral("pace_intensity")] = row.pace_intensity;                                            \
    item[QStringLiteral("lower_resistance")] = row.lower_resistance;                                        \
    item[QStringLiteral("upper_resistance")] = row.upper_resistance;                                        \
    item[QStringLiteral("requested_peloton_resistance")] = row.requested_peloton_resistance;                \
    item[QStringLiteral("lower_requested_peloton_resistance")] = row.lower_requested_peloton_resistance;    \
    item[QStringLiteral("upper_requested_peloton_resistance")] = row.upper_requested_peloton_resistance;    \
    item[QStringLiteral("power")] = row.power;                                                              \
    item[QStringLiteral("cadence")] = row.cadence;                                                          \
    item[QStringLiteral("lower_cadence")] = row.lower_cadence;                                              \
    item[QStringLiteral("upper_cadence")] = row.upper_cadence;                                              \
    item[QStringLiteral("forcespeed")] = row.forcespeed;                                                    \
    item[QStringLiteral("loopTimeHR")] = row.loopTimeHR;                                                    \
    item[QStringLiteral("zoneHR")] = row.zoneHR;                                                            \
    item[QStringLiteral("HRmin")] = row.HRmin;                                                              \
    item[QStringLiteral("HRmax")] = row.HRmax;                                                              \
    item[QStringLiteral("maxSpeed")] = row.maxSpeed;                                                        \
    item[QStringLiteral("latitude")] = row.latitude;                                                        \
    item[QStringLiteral("longitude")] = row.longitude;                                                      \
    item[QStringLiteral("altitude")] = row.altitude;                                                        \
    item[QStringLiteral("azimuth")] = row.azimuth;


QHash<QString, TemplateInfoSenderBuilder *> TemplateInfoSenderBuilder::instanceMap;
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

TemplateInfoSenderBuilder *TemplateInfoSenderBuilder::getInstance(const QString &idInfo, const QStringList &folders,
                                                                  QObject *parent) {
    TemplateInfoSenderBuilder *instance = instanceMap.value(idInfo, nullptr);
    if (instance) {
        return instance;
    } else {
        instance = new TemplateInfoSenderBuilder(parent);
        instance->load(idInfo, folders);
        return instance;
    }
}

bool TemplateInfoSenderBuilder::validFileTemplateType(const QString &tp) const { return tp == TEMPLATE_TYPE_TCPCLIENT; }

void TemplateInfoSenderBuilder::createTemplatesFromFolder(const QString &idInfo, const QString &folder,
                                                          QStringList &dirTemplates) {
    QSettings settings;
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
                    templateId = idInfo + "_" + templateId;
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
            if (pathEl != QStringLiteral(".") && pathEl != QStringLiteral("..") && !dirTemplates.contains(pathEl)) {
                qDebug() << QStringLiteral("Template Dir Found") << filePath;
                dirTemplates += pathEl;
            }
        }
    }
}

void TemplateInfoSenderBuilder::load(const QString &idInfo, const QStringList &folders) {
    QSettings settings;
    stop();
    masterId = idInfo;
    foldersToLook = folders;
    templateInfoMap.clear();
    templateFilesList.clear();
    QStringList globalIdList, globalFolderList;
    int startIdIndex = 0;
    for (auto &tdir : folders) {
        qDebug() << QStringLiteral("Load start from") << tdir;
        startIdIndex = globalIdList.size();
        createTemplatesFromFolder(idInfo, tdir, globalIdList);
        for (int i = startIdIndex; i < globalIdList.size(); i++)
            globalFolderList.append(tdir + "/" + globalIdList.at(i));
    }
    if (!globalFolderList.isEmpty()) {
        QStringList addressList;
        qDebug() << QStringLiteral("Folder List") << globalFolderList;
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
        for (auto &address : QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
                addressList += address.toString();
            }
        }
        qDebug() << QStringLiteral("addressList ") << addressList;
        QString templateId = idInfo + "_" + QStringLiteral(TEMPLATE_PRIVATE_WEBSERVER_ID);
        settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_ips"), addressList);
        templateFilesList.insert(templateId, TEMPLATE_TYPE_WEBSERVER);
        QString temptype =
            settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_type"), QString()).toString();
        settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_folders"), globalFolderList);
        settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_ips"), addressList);
        if (temptype != TEMPLATE_TYPE_WEBSERVER) {
            settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_type"),
                              QString(TEMPLATE_TYPE_WEBSERVER));
            settings.setValue(QStringLiteral("template_") + templateId + QStringLiteral("_enabled"), false);
        } else if (settings.value(QStringLiteral("template_") + templateId + QStringLiteral("_enabled"), false)
                       .toBool()) {
            newTemplate(templateId, TEMPLATE_TYPE_WEBSERVER,
                        QStringLiteral("JSON.stringify({msg: \"workout\", content: this.workout})"));
        } else {
            qDebug() << QStringLiteral("Template") << templateId << QStringLiteral(" is disabled: not created");
        }
    }
    qDebug() << QStringLiteral("Setting template_ids") << templateFilesList.keys();
    settings.setValue(QStringLiteral("template_") + idInfo + QStringLiteral("_ids"),
                      QStringList(templateFilesList.keys()));
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

void TemplateInfoSenderBuilder::reinit() { load(masterId, foldersToLook); }

void TemplateInfoSenderBuilder::clearSessionArray() {
    int len = sessionArray.count();
    for (int i = 0; i < len; i++) {
        sessionArray.removeAt(0);
    }
}

void TemplateInfoSenderBuilder::start(bluetoothdevice *dev) {
    device = nullptr;
    clearSessionArray();
    buildContext(true);
    device = dev;
    activityDescription = QLatin1String("");
    updateTimer.start(1s);
}

QStringList TemplateInfoSenderBuilder::templateIdList() const { return templateFilesList.keys(); }

void TemplateInfoSenderBuilder::onGetSettings(const QJsonValue &val, TemplateInfoSender *tempSender) {
    QJsonObject outObj;
    QSettings settings;
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
            if ((res = resVal.toInt()) >= 0 && res < std::numeric_limits<resistance_t>::max()) {
                ((bike *)device)->changeResistance((resistance_t)res);
                outObj[QStringLiteral("value")] = res;
            }
        } else {
            double resd;
            ((treadmill *)device)->changeInclination(resVal.toDouble(), resd = resVal.toDouble());
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
    QSettings settings;
    for (auto &key : keys) {
        if (settings.contains(key)) {
            val = obj[key];
            valConv = val.toVariant();
            settingVal = settings.value(key);
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
                                                        fileXml + QStringLiteral(".xml"), (device ? device->deviceType() : bluetoothdevice::BIKE ));
        for (auto &row : lst) {
            QJsonObject item;
            TRAINPROGRAM_FIELD_TO_STRING();
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

void TemplateInfoSenderBuilder::onGetTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject main;
    QJsonArray outArr;
    QJsonObject outObj;
    QString fileXml;
    if (homeform::singleton() && homeform::singleton()->trainingProgram()) {
        QList<trainrow> lst = homeform::singleton()->trainingProgram()->loadedRows;
        for (auto &row : lst) {
            QJsonObject item;
            TRAINPROGRAM_FIELD_TO_STRING();
            outArr.append(item);
        }
    }
    outObj[QStringLiteral("list")] = outArr;
    outObj[QStringLiteral("name")] = fileXml;
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_gettrainingprogram");
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

void TemplateInfoSenderBuilder::onGetGPXBase64(TemplateInfoSender *tempSender) {
    if (!device)
        return;
    QJsonObject main;
    main[QStringLiteral("content")] = device->currentGPXBase64();
    main[QStringLiteral("msg")] = QStringLiteral("R_getgpxbase64");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onGetLatLon(TemplateInfoSender *tempSender) {
    if (!device)
        return;
    QJsonObject main;
    main[QStringLiteral("content")] = QString::number(device->currentCordinate().latitude(), 'g', 18) + "," +
                                      QString::number(device->currentCordinate().longitude(), 'g', 18) + "," +
                                      QString::number(device->currentCordinate().altitude(), 'g', 18) + "," +
                                      QString::number(device->currentAzimuth(), 'g', 18) + "," +
                                      QString::number(device->averageAzimuthNext300m());
    main[QStringLiteral("msg")] = QStringLiteral("R_getlatlon");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onNextInclination300Meters(TemplateInfoSender *tempSender) {
    if (!device)
        return;
    QJsonObject main;
    QList<MetersByInclination> ii = device->nextInclination300Meters();
    QString values = "";
    for (int i = 0; i < ii.length(); i++) {
        values += QString::number(ii.at(i).meters, 'g', 0) + "," + QString::number(ii.at(i).inclination, 'g', 1) + ",";
    }
    main[QStringLiteral("content")] = values;
    main[QStringLiteral("msg")] = QStringLiteral("R_getnextinclination");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onStart(TemplateInfoSender *tempSender) {
    emit Start();
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_start");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onPause(TemplateInfoSender *tempSender) {
    emit Pause();
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_pause");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onStop(TemplateInfoSender *tempSender) {
    emit Stop();
    QJsonObject main;
    main[QStringLiteral("msg")] = QStringLiteral("R_stop");
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
            if (row.contains(QStringLiteral("HRmin"))) {
                tR.HRmin = row[QStringLiteral("HRmin")].toInt();
            }
            if (row.contains(QStringLiteral("HRmax"))) {
                tR.HRmax = row[QStringLiteral("HRmax")].toInt();
            }
            if (row.contains(QStringLiteral("maxSpeed"))) {
                tR.maxSpeed = row[QStringLiteral("maxSpeed")].toInt();
            }
            if (row.contains(QStringLiteral("latitude"))) {
                tR.latitude = row[QStringLiteral("latitude")].toDouble();
            }
            if (row.contains(QStringLiteral("longitude"))) {
                tR.longitude = row[QStringLiteral("longitude")].toDouble();
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

void TemplateInfoSenderBuilder::onLap(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit lap();
    main[QStringLiteral("msg")] = QStringLiteral("R_lap");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onPelotonOffsetPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit pelotonOffset_Plus();
    main[QStringLiteral("msg")] = QStringLiteral("R_pelotonoffset_plus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onPelotonOffsetMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit pelotonOffset_Minus();
    main[QStringLiteral("msg")] = QStringLiteral("R_pelotonoffset_minus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onGearsPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit gears_Plus();
    main[QStringLiteral("msg")] = QStringLiteral("R_gears_plus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onGearsMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit gears_Minus();
    main[QStringLiteral("msg")] = QStringLiteral("R_gears_minus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onPelotonStartWorkout(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit peloton_start_workout();
    main[QStringLiteral("msg")] = QStringLiteral("R_peloton_start_workout");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onPelotonAbortWorkout(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit peloton_abort_workout();
    main[QStringLiteral("msg")] = QStringLiteral("R_peloton_abort_workout");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onFloatingClose(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit floatingClose();
    main[QStringLiteral("msg")] = QStringLiteral("R_floating_close");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onAutoresistance(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit autoResistance();
    main[QStringLiteral("msg")] = QStringLiteral("R_autoresistance");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSaveChart(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QString filename;
    QString image;
    QJsonObject content;
    if ((content = msgContent.toObject()).isEmpty() ||
        (filename = content.value(QStringLiteral("name")).toString()).isEmpty() ||
        (image = content.value(QStringLiteral("image")).toString()).isEmpty()) {
        return;
    }
    QString path = homeform::getWritableAppDir();
    QJsonObject main, outObj;
    QString filenameScreenshot =
        path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
        QStringLiteral("_") + filename.replace(QStringLiteral(":"), QStringLiteral("_")) + QStringLiteral(".png");

    QPixmap imagep;
    imagep.loadFromData(QByteArray::fromBase64(image.toLocal8Bit().replace("data:image/png;base64,", "")));
    imagep.save(filenameScreenshot);

    emit chartSaved(filenameScreenshot);

    outObj[QStringLiteral("name")] = filename;
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_savechart");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onGetPelotonImage(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject main;
    QString base64 = "";
    if (homeform::singleton() && !homeform::singleton()->currentPelotonImage().isEmpty())
        base64 = homeform::singleton()->currentPelotonImage().toBase64();
    main[QStringLiteral("content")] = base64;
    main[QStringLiteral("msg")] = QStringLiteral("R_getpelotonimage");
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
                } else if (msg == QStringLiteral("getlatlon")) {
                    onGetLatLon(sender);
                    return;
                } else if (msg == QStringLiteral("getnextinclination")) {
                    onNextInclination300Meters(sender);
                    return;
                } else if (msg == QStringLiteral("getgpxbase64")) {
                    onGetGPXBase64(sender);
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
                } else if (msg == QStringLiteral("gettrainingprogram")) {
                    onGetTrainingProgram(jsonObject[QStringLiteral("content")], sender);
                    return;                    
                } else if (msg == QStringLiteral("appendactivitydescription")) {
                    onAppendActivityDescription(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("savetrainingprogram")) {
                    onSaveTrainingProgram(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("savechart")) {
                    onSaveChart(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("getpelotonimage")) {
                    onGetPelotonImage(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("lap")) {
                    onLap(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("pelotonoffset_plus")) {
                    onPelotonOffsetPlus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("pelotonoffset_minus")) {
                    onPelotonOffsetMinus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("gears_plus")) {
                    onGearsPlus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("gears_minus")) {
                    onGearsMinus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("peloton_start_workout")) {
                    onPelotonStartWorkout(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("peloton_abort_workout")) {
                    onPelotonAbortWorkout(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("floating_close")) {
                    onFloatingClose(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("autoresistance")) {
                    onAutoresistance(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("getsessionarray")) {
                    onGetSessionArray(sender);
                    return;
                }
                if (msg == QStringLiteral("start")) {
                    onStart(sender);
                    return;
                }
                if (msg == QStringLiteral("pause")) {
                    onPause(sender);
                    return;
                }
                if (msg == QStringLiteral("stop")) {
                    onStop(sender);
                    return;
                }
            }
        }
    }
    // qDebug() << QStringLiteral("Unrecognized message") << data;
}

void TemplateInfoSenderBuilder::buildContext(bool forceReinit) {
    QJSValue glob = engine->globalObject();
    QJSValue obj;
    QSettings settings;

    if (!homeform::singleton()) {
        qDebug() << QStringLiteral("homeform::singleton() not available. You should never see this!");
        return;
    }

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
        QTime elLap = device->lapElapsedTime();
        QString name;
        QString nickName;
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
        obj.setProperty(QStringLiteral("lapelapsed_s"), elLap.second());
        obj.setProperty(QStringLiteral("lapelapsed_m"), elLap.minute());
        obj.setProperty(QStringLiteral("lapelapsed_h"), elLap.hour());
        el = device->currentPace();
        obj.setProperty(QStringLiteral("pace_s"), el.second());
        obj.setProperty(QStringLiteral("pace_m"), el.minute());
        obj.setProperty(QStringLiteral("pace_h"), el.hour());
        obj.setProperty(QStringLiteral("pace_color"), homeform::singleton()->pace->valueFontColor());
        el = device->averagePace();
        obj.setProperty(QStringLiteral("avgpace_s"), el.second());
        obj.setProperty(QStringLiteral("avgpace_m"), el.minute());
        obj.setProperty(QStringLiteral("avgpace_h"), el.hour());
        el = device->maxPace();
        obj.setProperty(QStringLiteral("maxpace_s"), el.second());
        obj.setProperty(QStringLiteral("maxpace_m"), el.minute());
        obj.setProperty(QStringLiteral("maxpace_h"), el.hour());
        el = device->movingTime();
        obj.setProperty(QStringLiteral("moving_s"), el.second());
        obj.setProperty(QStringLiteral("moving_m"), el.minute());
        obj.setProperty(QStringLiteral("moving_h"), el.hour());
        obj.setProperty(QStringLiteral("speed"), (dep = device->currentSpeed()).value());
        obj.setProperty(QStringLiteral("speed_avg"), dep.average());
        obj.setProperty(QStringLiteral("speed_color"), homeform::singleton()->speed->valueFontColor());
        obj.setProperty(QStringLiteral("speed_lapavg"), dep.lapAverage());
        obj.setProperty(QStringLiteral("speed_lapmax"), dep.lapMax());
        obj.setProperty(QStringLiteral("calories"), device->calories().value());
        obj.setProperty(QStringLiteral("distance"), device->odometer());
        obj.setProperty(QStringLiteral("heart"), (dep = device->currentHeart()).value());
        obj.setProperty(QStringLiteral("heart_color"), homeform::singleton()->heart->valueFontColor());
        obj.setProperty(QStringLiteral("heart_avg"), dep.average());
        obj.setProperty(QStringLiteral("heart_lapavg"), dep.lapAverage());
        obj.setProperty(QStringLiteral("heart_max"), dep.max());
        obj.setProperty(QStringLiteral("heart_lapmax"), dep.lapMax());
        obj.setProperty(QStringLiteral("jouls"), device->jouls().value());
        obj.setProperty(QStringLiteral("elevation"), device->elevationGain().value());
        obj.setProperty(QStringLiteral("difficult"), device->difficult());
        obj.setProperty(QStringLiteral("watts"), (dep = device->wattsMetric()).value());
        obj.setProperty(QStringLiteral("watts_avg"), dep.average());
        obj.setProperty(QStringLiteral("watts_color"), homeform::singleton()->watt->valueFontColor());
        obj.setProperty(QStringLiteral("watts_lapavg"), dep.lapAverage());
        obj.setProperty(QStringLiteral("watts_max"), dep.max());
        obj.setProperty(QStringLiteral("watts_lapmax"), dep.lapMax());
        obj.setProperty(QStringLiteral("kgwatts"), (dep = device->wattKg()).value());
        obj.setProperty(QStringLiteral("kgwatts_avg"), dep.average());
        obj.setProperty(QStringLiteral("kgwatts_max"), dep.max());
        obj.setProperty(QStringLiteral("workoutName"), workoutName);
        obj.setProperty(QStringLiteral("workoutStartDate"), workoutStartDate);
        obj.setProperty(QStringLiteral("instructorName"), instructorName);
        obj.setProperty(QStringLiteral("latitude"), device->currentCordinate().latitude());
        obj.setProperty(QStringLiteral("longitude"), device->currentCordinate().longitude());
        obj.setProperty(QStringLiteral("altitude"), device->currentCordinate().altitude());
        obj.setProperty(QStringLiteral("peloton_offset"), pelotonOffset());
        obj.setProperty(QStringLiteral("peloton_ask_start"), pelotonAskStart());
        obj.setProperty(QStringLiteral("autoresistance"), homeform::singleton()->autoResistance());
        obj.setProperty(QStringLiteral("nextrow"), homeform::singleton()->nextRows->value());
        if (homeform::singleton()->trainingProgram()) {
            el = homeform::singleton()->trainingProgram()->currentRowRemainingTime();
            obj.setProperty(QStringLiteral("row_remaining_time_s"), el.second());
            obj.setProperty(QStringLiteral("row_remaining_time_m"), el.minute());
            obj.setProperty(QStringLiteral("row_remaining_time_h"), el.hour());
        } else {
            obj.setProperty(QStringLiteral("row_remaining_time_s"), 0);
            obj.setProperty(QStringLiteral("row_remaining_time_m"), 0);
            obj.setProperty(QStringLiteral("row_remaining_time_h"), 0);
        }
        if (homeform::singleton()->trainingProgram()) {
            el = homeform::singleton()->trainingProgram()->remainingTime();
            obj.setProperty(QStringLiteral("remaining_time_s"), el.second());
            obj.setProperty(QStringLiteral("remaining_time_m"), el.minute());
            obj.setProperty(QStringLiteral("remaining_time_h"), el.hour());
        } else {
            obj.setProperty(QStringLiteral("remaining_time_s"), 0);
            obj.setProperty(QStringLiteral("remaining_time_m"), 0);
            obj.setProperty(QStringLiteral("remaining_time_h"), 0);
        }
        obj.setProperty(
            QStringLiteral("nickName"),
            (nickName = settings.value(QZSettings::user_nickname, QZSettings::default_user_nickname).toString())
                    .isEmpty()
                ? QString(QStringLiteral("N/A"))
                : nickName);
        if (tp == bluetoothdevice::BIKE) {
            obj.setProperty(QStringLiteral("gears"), ((bike *)device)->gears());
            obj.setProperty(QStringLiteral("target_resistance"), ((bike *)device)->lastRequestedResistance().value());
            obj.setProperty(QStringLiteral("target_peloton_resistance"),
                            ((bike *)device)->lastRequestedPelotonResistance().value());
            obj.setProperty(QStringLiteral("target_cadence"), ((bike *)device)->lastRequestedCadence().value());
            obj.setProperty(QStringLiteral("target_power"), ((bike *)device)->lastRequestedPower().value());
            obj.setProperty(QStringLiteral("power_zone"), ((bike *)device)->currentPowerZone().value());
            obj.setProperty(QStringLiteral("power_zone_lapavg"), ((bike *)device)->currentPowerZone().lapAverage());
            obj.setProperty(QStringLiteral("power_zone_lapmax"), ((bike *)device)->currentPowerZone().lapMax());
            obj.setProperty(QStringLiteral("target_power_zone"), ((bike *)device)->targetPowerZone().value());
            obj.setProperty(QStringLiteral("power_zone_color"), homeform::singleton()->ftp->valueFontColor());
            obj.setProperty(QStringLiteral("target_power_zone_color"), homeform::singleton()->target_zone->valueFontColor());
            obj.setProperty(QStringLiteral("peloton_resistance"),
                            (dep = ((bike *)device)->pelotonResistance()).value());
            obj.setProperty(QStringLiteral("peloton_resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("peloton_resistance_color"), homeform::singleton()->peloton_resistance->valueFontColor());
            obj.setProperty(QStringLiteral("peloton_resistance_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("peloton_resistance_lapmax"), dep.lapMax());
            obj.setProperty(QStringLiteral("peloton_req_resistance"),
                            (dep = ((bike *)device)->lastRequestedPelotonResistance()).value());
            obj.setProperty(QStringLiteral("cadence"), (dep = ((bike *)device)->currentCadence()).value());
            obj.setProperty(QStringLiteral("cadence_color"), homeform::singleton()->cadence->valueFontColor());
            obj.setProperty(QStringLiteral("cadence_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("cadence_lapmax"), dep.lapMax());
            obj.setProperty(QStringLiteral("resistance"), (dep = ((bike *)device)->currentResistance()).value());
            obj.setProperty(QStringLiteral("resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("resistance_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("resistance_lapmax"), dep.lapMax());
            obj.setProperty(QStringLiteral("cranks"), ((bike *)device)->currentCrankRevolutions());
            obj.setProperty(QStringLiteral("cranktime"), ((bike *)device)->lastCrankEventTime());
            obj.setProperty(QStringLiteral("req_power"), (dep = ((bike *)device)->lastRequestedPower()).value());
            obj.setProperty(QStringLiteral("req_cadence"), (dep = ((bike *)device)->lastRequestedCadence()).value());
            obj.setProperty(QStringLiteral("req_resistance"),
                            (dep = ((bike *)device)->lastRequestedResistance()).value());
            obj.setProperty(QStringLiteral("inclination"),
                            (dep = ((bike *)device)->currentInclination()).value());
            obj.setProperty(QStringLiteral("inclination_avg"), dep.average());
        } else if (tp == bluetoothdevice::ROWING) {
            obj.setProperty(QStringLiteral("gears"), ((rower *)device)->gears());
            el = ((rower *)device)->lastRequestedPace();
            obj.setProperty(QStringLiteral("target_speed"), ((rower *)device)->lastRequestedSpeed().value());
            obj.setProperty(QStringLiteral("target_pace_s"), el.second());
            obj.setProperty(QStringLiteral("target_pace_m"), el.minute());
            obj.setProperty(QStringLiteral("target_pace_h"), el.hour());
            obj.setProperty(QStringLiteral("peloton_resistance"),
                            (dep = ((rower *)device)->pelotonResistance()).value());
            obj.setProperty(QStringLiteral("peloton_resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence"), (dep = ((rower *)device)->currentCadence()).value());
            obj.setProperty(QStringLiteral("cadence_color"), homeform::singleton()->cadence->valueFontColor());
            obj.setProperty(QStringLiteral("cadence_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("cadence_lapmax"), dep.lapMax());

            // use to preserve compatibility to dochart.js and floating.htm
            obj.setProperty(QStringLiteral("req_cadence"), (dep = ((rower *)device)->lastRequestedCadence()).value());
            obj.setProperty(QStringLiteral("target_cadence"), (dep = ((rower *)device)->lastRequestedCadence()).value());
            
            obj.setProperty(QStringLiteral("resistance"), (dep = ((rower *)device)->currentResistance()).value());
            obj.setProperty(QStringLiteral("resistance_avg"), dep.average());
            obj.setProperty(QStringLiteral("cranks"), ((rower *)device)->currentCrankRevolutions());
            obj.setProperty(QStringLiteral("cranktime"), ((rower *)device)->lastCrankEventTime());
            obj.setProperty(QStringLiteral("strokescount"), ((rower *)device)->currentStrokesCount().value());
            obj.setProperty(QStringLiteral("strokeslength"), ((rower *)device)->currentStrokesLength().value());
        } else if (tp == bluetoothdevice::TREADMILL) {
            obj.setProperty(QStringLiteral("target_speed"), ((treadmill *)device)->lastRequestedSpeed().value());
            el = ((treadmill *)device)->lastRequestedPace();
            obj.setProperty(QStringLiteral("target_pace_s"), el.second());
            obj.setProperty(QStringLiteral("target_pace_m"), el.minute());
            obj.setProperty(QStringLiteral("target_pace_h"), el.hour());
            obj.setProperty(QStringLiteral("target_inclination"),
                            ((treadmill *)device)->lastRequestedInclination().value());
            obj.setProperty(QStringLiteral("cadence"), (dep = ((treadmill *)device)->currentCadence()).value());
            obj.setProperty(QStringLiteral("cadence_color"), homeform::singleton()->cadence->valueFontColor());
            obj.setProperty(QStringLiteral("cadence_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("cadence_lapmax"), dep.lapMax());
            obj.setProperty(QStringLiteral("inclination"), (dep = ((treadmill *)device)->currentInclination()).value());
            obj.setProperty(QStringLiteral("inclination_avg"), dep.average());
            obj.setProperty(QStringLiteral("inclination_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("inclination_lapmax"), dep.lapMax());
            obj.setProperty(QStringLiteral("stridelength"),
                            (dep = ((treadmill *)device)->currentStrideLength()).value());
            obj.setProperty(QStringLiteral("groundcontact"),
                            (dep = ((treadmill *)device)->currentGroundContact()).value());
            obj.setProperty(QStringLiteral("verticaloscillation"),
                            (dep = ((treadmill *)device)->currentVerticalOscillation()).value());
        } else if (tp == bluetoothdevice::ELLIPTICAL) {
            obj.setProperty(QStringLiteral("cadence"), (dep = ((elliptical *)device)->currentCadence()).value());
            obj.setProperty(QStringLiteral("cadence_color"), homeform::singleton()->cadence->valueFontColor());
            obj.setProperty(QStringLiteral("cadence_avg"), dep.average());
            obj.setProperty(QStringLiteral("cadence_lapavg"), dep.lapAverage());
            obj.setProperty(QStringLiteral("cadence_lapmax"), dep.lapMax());
            obj.setProperty(QStringLiteral("inclination"),
                            (dep = ((elliptical *)device)->currentInclination()).value());
            obj.setProperty(QStringLiteral("inclination_avg"), dep.average());
        }
        if (!device->isPaused()) {
            sessionArray.append(QJsonObject::fromVariantMap(obj.toVariant().toMap()));
        }
    }
}

void TemplateInfoSenderBuilder::workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state) {
    if (state == bluetoothdevice::STARTED) {
        clearSessionArray();
    }
}

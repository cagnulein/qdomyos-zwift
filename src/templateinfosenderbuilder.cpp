#include "templateinfosenderbuilder.h"
#include "devices/bike.h"
#include "treadmill.h"
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QTime>
#include <QQmlFile>
#include <limits>
#include <QRegularExpression>>
#ifdef Q_HTTPSERVER
#include "webserverinfosender.h"
#endif
#include "homeform.h"
#include "tcpclientinfosender.h"
#include "trainprogram.h"
#include "zwiftworkout.h"
#include "qzsettings.h"
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

namespace {
QString sanitizeTrainingProgramName(const QString &input) {
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        trimmed = QStringLiteral("Workout");
    }
    QRegularExpression invalid(QStringLiteral("[^A-Za-z0-9_\\- ]"));
    trimmed.replace(invalid, QStringLiteral("_"));
    trimmed.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral("_"));
    return trimmed;
}

QString deviceTypeToKey(BLUETOOTH_TYPE type) {
    switch (type) {
    case TREADMILL:
        return QStringLiteral("treadmill");
    case BIKE:
        return QStringLiteral("bike");
    case ELLIPTICAL:
        return QStringLiteral("elliptical");
    case ROWING:
        return QStringLiteral("rower");
    default:
        return QStringLiteral("treadmill");
    }
}
} // namespace


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

void TemplateInfoSenderBuilder::clearPreviewSessionArray() {
    int len = previewSessionArray.count();
    for (int i = 0; i < len; i++) {
        previewSessionArray.removeAt(0);
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
                QRegularExpression regex(key.mid(1));
                for (auto &keypresent : settings.allKeys()) {
                    QRegularExpressionMatch match = regex.match(keypresent);
                    if (match.hasMatch()) {
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
        BLUETOOTH_TYPE tp = device->deviceType();
        if (tp == BIKE || tp == ROWING) {
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
        (device->deviceType() == BIKE || device->deviceType() == ROWING)) {
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
        (device->deviceType() == BIKE || device->deviceType() == ROWING)) {
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
        device->deviceType() == TREADMILL && (vald = resVal.toDouble()) >= 0) {
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

    // Get path from content
    QString path;
    QJsonObject content = msgContent.toObject();
    if (content.contains(QStringLiteral("path"))) {
        path = content.value(QStringLiteral("path")).toString();
    }

    // Build full directory path
    QString basePath = homeform::getWritableAppDir() + QStringLiteral("training");
    QString fullPath = path.isEmpty() ? basePath : (basePath + QStringLiteral("/") + path);

    // Iterate through directory
    QDir dir(fullPath);
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst | QDir::Name);
    dir.setNameFilters(QStringList() << "*.xml" << "*.zwo");

    QFileInfoList list = dir.entryInfoList();
    for (const QFileInfo &fileInfo : list) {
        QJsonObject item;
        item[QStringLiteral("name")] = fileInfo.fileName();
        item[QStringLiteral("isFolder")] = fileInfo.isDir();
        item[QStringLiteral("path")] = fileInfo.absoluteFilePath();
        item[QStringLiteral("url")] = QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toString();
        outArr.append(item);
    }

    outObj[QStringLiteral("files")] = outArr;
    outObj[QStringLiteral("path")] = path;
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_loadtrainingprograms");
    main[QStringLiteral("type")] = QStringLiteral("trainingfiles");
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

void TemplateInfoSenderBuilder::onTrainingProgramPreview(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject content = msgContent.toObject();
    QString urlString = content.value(QStringLiteral("url")).toString();

    if (urlString.isEmpty()) {
        return;
    }

    QUrl url(urlString);
    QString filePath = QQmlFile::urlToLocalFileOrQrc(url);
    QString extension = QFileInfo(filePath).suffix().toUpper();

    // Load the training program - use zwiftworkout::load for ZWO files
    QList<trainrow> rows;
    QString description, tags;

    if (extension == QStringLiteral("ZWO")) {
        rows = zwiftworkout::load(filePath, &description, &tags);
    } else {
        rows = trainprogram::loadXML(filePath, (device ? device->deviceType() : BIKE));
    }

    // Build workout preview data
    QJsonObject main;
    QJsonObject outObj;
    QJsonArray watts, speed, inclination, resistance, cadence;

    if (!rows.isEmpty()) {
        // Calculate total duration
        int totalSeconds = 0;
        for (const trainrow &r : rows) {
            totalSeconds += (r.duration.hour() * 3600) + (r.duration.minute() * 60) + r.duration.second();
        }

        outObj[QStringLiteral("points")] = totalSeconds;
        outObj[QStringLiteral("description")] = description.isEmpty() ? QFileInfo(filePath).baseName() : description;
        outObj[QStringLiteral("tags")] = tags;

        // Build data arrays
        for (const trainrow &r : rows) {
            int duration = (r.duration.hour() * 3600) + (r.duration.minute() * 60) + r.duration.second();

            for (int i = 0; i < duration; i++) {
                int currentSecond = watts.size();

                // Watts
                QJsonObject wattPoint;
                wattPoint[QStringLiteral("x")] = currentSecond;
                wattPoint[QStringLiteral("y")] = r.power;
                watts.append(wattPoint);

                // Speed
                if (r.speed >= 0) {
                    QJsonObject speedPoint;
                    speedPoint[QStringLiteral("x")] = currentSecond;
                    speedPoint[QStringLiteral("y")] = r.speed;
                    speed.append(speedPoint);
                }

                // Inclination
                if (r.inclination > -200) {
                    QJsonObject incPoint;
                    incPoint[QStringLiteral("x")] = currentSecond;
                    incPoint[QStringLiteral("y")] = r.inclination;
                    inclination.append(incPoint);
                }

                // Resistance
                if (r.resistance >= 0) {
                    QJsonObject resPoint;
                    resPoint[QStringLiteral("x")] = currentSecond;
                    resPoint[QStringLiteral("y")] = r.resistance;
                    resistance.append(resPoint);
                }

                // Cadence
                if (r.cadence >= 0) {
                    QJsonObject cadPoint;
                    cadPoint[QStringLiteral("x")] = currentSecond;
                    cadPoint[QStringLiteral("y")] = r.cadence;
                    cadence.append(cadPoint);
                }
            }
        }

        // Determine device type
        QString deviceType = QStringLiteral("bike");
        if (speed.size() > 0 && watts.size() == 0) {
            deviceType = QStringLiteral("treadmill");
        } else if (watts.size() == 0 && resistance.size() > 0) {
            deviceType = QStringLiteral("elliptical");
        }

        outObj[QStringLiteral("watts")] = watts;
        outObj[QStringLiteral("speed")] = speed;
        outObj[QStringLiteral("inclination")] = inclination;
        outObj[QStringLiteral("resistance")] = resistance;
        outObj[QStringLiteral("cadence")] = cadence;
        outObj[QStringLiteral("deviceType")] = deviceType;
    }

    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_trainprogram_preview");
    main[QStringLiteral("type")] = QStringLiteral("workoutpreview");
    QJsonDocument response(main);
    tempSender->send(response.toJson());
}

void TemplateInfoSenderBuilder::onGetWorkoutPreview(TemplateInfoSender *tempSender) {
    if (!homeform::singleton()) {
        return;
    }

    homeform *hf = homeform::singleton();

    // Build workout preview data from homeform properties
    QJsonObject main;
    QJsonObject outObj;
    QJsonArray watts, speed, inclination, resistance, cadence;

    int points = hf->preview_workout_points();

    if (points > 0) {
        QList<double> wattsData = hf->preview_workout_watt();
        QList<double> speedData = hf->preview_workout_speed();
        QList<double> inclinationData = hf->preview_workout_inclination();
        QList<double> resistanceData = hf->preview_workout_resistance();
        QList<double> cadenceData = hf->preview_workout_cadence();

        outObj[QStringLiteral("points")] = points;
        outObj[QStringLiteral("description")] = hf->previewWorkoutDescription();
        outObj[QStringLiteral("tags")] = hf->previewWorkoutTags();

        // Build data arrays with x,y points
        for (int i = 0; i < points; i++) {
            // Watts
            if (i < wattsData.size()) {
                QJsonObject wattPoint;
                wattPoint[QStringLiteral("x")] = i;
                wattPoint[QStringLiteral("y")] = wattsData[i];
                watts.append(wattPoint);
            }

            // Speed
            if (i < speedData.size()) {
                QJsonObject speedPoint;
                speedPoint[QStringLiteral("x")] = i;
                speedPoint[QStringLiteral("y")] = speedData[i];
                speed.append(speedPoint);
            }

            // Inclination
            if (i < inclinationData.size()) {
                QJsonObject incPoint;
                incPoint[QStringLiteral("x")] = i;
                incPoint[QStringLiteral("y")] = inclinationData[i];
                inclination.append(incPoint);
            }

            // Resistance
            if (i < resistanceData.size()) {
                QJsonObject resPoint;
                resPoint[QStringLiteral("x")] = i;
                resPoint[QStringLiteral("y")] = resistanceData[i];
                resistance.append(resPoint);
            }

            // Cadence
            if (i < cadenceData.size()) {
                QJsonObject cadPoint;
                cadPoint[QStringLiteral("x")] = i;
                cadPoint[QStringLiteral("y")] = cadenceData[i];
                cadence.append(cadPoint);
            }
        }

        // Determine device type
        QString deviceType = QStringLiteral("bike");
        if (speed.size() > 0 && watts.size() == 0) {
            deviceType = QStringLiteral("treadmill");
        } else if (watts.size() == 0 && resistance.size() > 0) {
            deviceType = QStringLiteral("elliptical");
        }

        outObj[QStringLiteral("watts")] = watts;
        outObj[QStringLiteral("speed")] = speed;
        outObj[QStringLiteral("inclination")] = inclination;
        outObj[QStringLiteral("resistance")] = resistance;
        outObj[QStringLiteral("cadence")] = cadence;
        outObj[QStringLiteral("deviceType")] = deviceType;

        // Add miles_unit setting
        QSettings settings;
        outObj[QStringLiteral("miles_unit")] = settings.value(QStringLiteral("miles_unit"), false).toBool();
    }

    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_workoutpreview");
    main[QStringLiteral("type")] = QStringLiteral("workoutpreview");
    QJsonDocument response(main);
    tempSender->send(response.toJson());
}

void TemplateInfoSenderBuilder::onTrainingProgramOpen(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject content = msgContent.toObject();
    QString urlString = content.value(QStringLiteral("url")).toString();

    if (urlString.isEmpty() || !homeform::singleton()) {
        return;
    }

    QUrl url(urlString);
    QMetaObject::invokeMethod(homeform::singleton(), "trainprogram_open_clicked", Qt::QueuedConnection, Q_ARG(QUrl, url));
}

void TemplateInfoSenderBuilder::onTrainingProgramAutostart(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    qDebug() << "[TemplateInfoSenderBuilder] onTrainingProgramAutostart called";
    if (!homeform::singleton()) {
        qDebug() << "[TemplateInfoSenderBuilder] homeform singleton is null!";
        return;
    }

    // Get the QML stack and emit the signal there, so main.qml can intercept it
    // This is important for WorkoutEditor to close properly
    QQmlApplicationEngine *engine = homeform::singleton()->getEngine();
    if (engine && !engine->rootObjects().isEmpty()) {
        QObject *stack = engine->rootObjects().constFirst();
        qDebug() << "[TemplateInfoSenderBuilder] Emitting trainprogram_autostart_requested signal on QML stack";
        QMetaObject::invokeMethod(stack, "trainprogram_autostart_requested", Qt::QueuedConnection);
    } else {
        qDebug() << "[TemplateInfoSenderBuilder] QML stack is null, calling homeform slot directly";
        QMetaObject::invokeMethod(homeform::singleton(), "trainprogram_autostart_requested", Qt::QueuedConnection);
    }
}

void TemplateInfoSenderBuilder::onWorkoutEditorEnv(TemplateInfoSender *tempSender) {
    QJsonObject outObj;
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    outObj[QStringLiteral("miles")] = miles;
    if (device) {
        outObj[QStringLiteral("device")] = deviceTypeToKey(device->deviceType());
    } else {
        outObj[QStringLiteral("device")] = QStringLiteral("treadmill");
    }
    QJsonObject main;
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_workouteditor_env");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onWorkoutEditorStart(const QJsonValue &msgContent,
                                                     TemplateInfoSender *tempSender) {
    QString programName;
    if (msgContent.isObject()) {
        programName = msgContent.toObject().value(QStringLiteral("name")).toString();
    } else if (msgContent.isString()) {
        programName = msgContent.toString();
    }
    if (programName.endsWith(QStringLiteral(".xml"), Qt::CaseInsensitive)) {
        programName.chop(4);
    }
    programName = sanitizeTrainingProgramName(programName);

    QString filename = programName + QStringLiteral(".xml");
    QString fullPath = homeform::getWritableAppDir() + QStringLiteral("training/") + filename;

    bool ok = false;
    if (homeform::singleton()) {
        // Check if file exists
        if (QFile::exists(fullPath)) {
            // Use the same approach as workoutmodel.cpp - create QUrl from local file
            QUrl fileUrl = QUrl::fromLocalFile(fullPath);
            homeform::singleton()->trainprogram_open_clicked(fileUrl);

            // Automatically start the workout
            // Check if device is currently paused
            bluetoothdevice *dev = nullptr;
            if (homeform::singleton()->bluetoothManager) {
                dev = homeform::singleton()->bluetoothManager->device();
            }

            if (dev && !dev->isPaused()) {
                // Device is running (not paused), need to pause first then start
                qDebug() << "Device is running, calling Start() twice (pause then start)";
                QMetaObject::invokeMethod(homeform::singleton(), "Start", Qt::QueuedConnection);
                QThread::msleep(200); // Small delay between calls
                QMetaObject::invokeMethod(homeform::singleton(), "Start", Qt::QueuedConnection);
            } else {
                // Device is paused or stopped, just start
                qDebug() << "Device is paused/stopped, calling Start() once";
                QMetaObject::invokeMethod(homeform::singleton(), "Start", Qt::QueuedConnection);
            }

            ok = true;
        } else {
            qDebug() << "Training program file not found:" << fullPath;
            ok = false;
        }
    }

    QJsonObject outObj;
    outObj[QStringLiteral("ok")] = ok;
    outObj[QStringLiteral("name")] = programName;
    outObj[QStringLiteral("path")] = fullPath;
    if (!ok) {
        outObj[QStringLiteral("message")] = QStringLiteral("Unable to start training program");
    }

    QJsonObject main;
    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_workouteditor_start");
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

void TemplateInfoSenderBuilder::onGetPreviewSessionArray(TemplateInfoSender *tempSender) {
    QJsonObject main;
    main[QStringLiteral("content")] = previewSessionArray;
    main[QStringLiteral("msg")] = QStringLiteral("R_getpreviewsessionarray");
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
    if (fileName.endsWith(QStringLiteral(".xml"), Qt::CaseInsensitive)) {
        fileName.chop(4);
    }
    fileName = sanitizeTrainingProgramName(fileName);
    QList<trainrow> trainRows;
    trainRows.reserve(rows.size() + 1);
    for (const auto &r : qAsConst(rows)) {
        QJsonObject row = r.toObject();
        trainrow tR;
        if (row.contains(QStringLiteral("duration"))) {
            tR.duration = QTime::fromString(row[QStringLiteral("duration")].toString(), QStringLiteral("hh:mm:ss"));
            if (row.contains(QStringLiteral("distance"))) {
                tR.distance = row[QStringLiteral("distance")].toDouble();
            }
            if (row.contains(QStringLiteral("speed"))) {
                tR.speed = row[QStringLiteral("speed")].toDouble();
            }
            if (row.contains(QStringLiteral("minSpeed"))) {
                tR.minSpeed = row[QStringLiteral("minSpeed")].toDouble();
            }
            if (row.contains(QStringLiteral("maxSpeed"))) {
                tR.maxSpeed = row[QStringLiteral("maxSpeed")].toDouble();
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
            if (row.contains(QStringLiteral("maxResistance"))) {
                tR.maxResistance = row[QStringLiteral("maxResistance")].toInt();
            }
            if (row.contains(QStringLiteral("requested_peloton_resistance"))) {
                tR.requested_peloton_resistance = row[QStringLiteral("requested_peloton_resistance")].toInt();
            }
            if (row.contains(QStringLiteral("power"))) {
                tR.power = row[QStringLiteral("power")].toInt();
            }
            if (row.contains(QStringLiteral("cadence"))) {
                tR.cadence = row[QStringLiteral("cadence")].toInt();
            }
            if (row.contains(QStringLiteral("mets"))) {
                tR.mets = row[QStringLiteral("mets")].toInt();
            }
            if (row.contains(QStringLiteral("forcespeed"))) {
                tR.forcespeed = row.value(QStringLiteral("forcespeed")).toVariant().toBool();
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
            if (row.contains(QStringLiteral("latitude"))) {
                tR.latitude = row[QStringLiteral("latitude")].toDouble();
            }
            if (row.contains(QStringLiteral("longitude"))) {
                tR.longitude = row[QStringLiteral("longitude")].toDouble();
            }
            if (row.contains(QStringLiteral("altitude"))) {
                tR.altitude = row[QStringLiteral("altitude")].toDouble();
            }
            if (row.contains(QStringLiteral("azimuth"))) {
                tR.azimuth = row[QStringLiteral("azimuth")].toDouble();
            }
            // Load textEvents if present
            if (row.contains(QStringLiteral("textEvents"))) {
                QJsonArray textEventsArray = row[QStringLiteral("textEvents")].toArray();
                for (const auto &te : qAsConst(textEventsArray)) {
                    QJsonObject textEvent = te.toObject();
                    if (textEvent.contains(QStringLiteral("timeoffset")) &&
                        textEvent.contains(QStringLiteral("message"))) {
                        trainrow::TextEvent evt;
                        evt.timeoffset = textEvent[QStringLiteral("timeoffset")].toInt();
                        evt.message = textEvent[QStringLiteral("message")].toString();
                        tR.textEvents.append(evt);
                    }
                }
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

void TemplateInfoSenderBuilder::onDeleteTrainingProgram(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    QJsonObject content;
    QString fileUrl;
    if ((content = msgContent.toObject()).isEmpty() ||
        (fileUrl = content.value(QStringLiteral("url")).toString()).isEmpty()) {
        qDebug() << "onDeleteTrainingProgram: invalid content";
        return;
    }

    // Convert URL to local file path
    QUrl url(fileUrl);
    QString filePath = url.toLocalFile();

    qDebug() << "onDeleteTrainingProgram: attempting to delete" << filePath;

    QJsonObject main, outObj;
    bool success = false;

    if (QFile::exists(filePath)) {
        if (QFile::remove(filePath)) {
            qDebug() << "onDeleteTrainingProgram: successfully deleted" << filePath;
            success = true;
            outObj[QStringLiteral("success")] = true;
            outObj[QStringLiteral("message")] = QStringLiteral("Workout deleted successfully");

            // Create a marker file to prevent repopulation of default files
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            QString directory = fileInfo.absolutePath();
            QString markerPath = directory + "/.deleted_" + fileName;

            QFile markerFile(markerPath);
            if (markerFile.open(QIODevice::WriteOnly)) {
                markerFile.write("This file was intentionally deleted by the user");
                markerFile.close();
                qDebug() << "onDeleteTrainingProgram: created deletion marker at" << markerPath;
            } else {
                qDebug() << "onDeleteTrainingProgram: failed to create deletion marker at" << markerPath;
            }
        } else {
            qDebug() << "onDeleteTrainingProgram: failed to delete" << filePath;
            outObj[QStringLiteral("success")] = false;
            outObj[QStringLiteral("message")] = QStringLiteral("Failed to delete file");
        }
    } else {
        qDebug() << "onDeleteTrainingProgram: file does not exist" << filePath;
        outObj[QStringLiteral("success")] = false;
        outObj[QStringLiteral("message")] = QStringLiteral("File not found");
    }

    main[QStringLiteral("content")] = outObj;
    main[QStringLiteral("msg")] = QStringLiteral("R_deletetrainingprogram");
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

void TemplateInfoSenderBuilder::onSpeedPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit speed_Plus();
    main[QStringLiteral("msg")] = QStringLiteral("R_speed_plus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onSpeedMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit speed_Minus();
    main[QStringLiteral("msg")] = QStringLiteral("R_speed_minus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onInclinationPlus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit inclination_Plus();
    main[QStringLiteral("msg")] = QStringLiteral("R_inclination_plus");
    QJsonDocument out(main);
    tempSender->send(out.toJson());
}

void TemplateInfoSenderBuilder::onInclinationMinus(const QJsonValue &msgContent, TemplateInfoSender *tempSender) {
    Q_UNUSED(msgContent);
    QJsonObject main, outObj;
    emit inclination_Minus();
    main[QStringLiteral("msg")] = QStringLiteral("R_inclination_minus");
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
                } else if (msg == QStringLiteral("trainprogram_preview")) {
                    onTrainingProgramPreview(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("getworkoutpreview")) {
                    onGetWorkoutPreview(sender);
                    return;
                } else if (msg == QStringLiteral("trainprogram_open_clicked")) {
                    onTrainingProgramOpen(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("trainprogram_autostart_requested")) {
                    onTrainingProgramAutostart(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("gettrainingprogram")) {
                    onGetTrainingProgram(jsonObject[QStringLiteral("content")], sender);
                    return;                    
                } else if (msg == QStringLiteral("workouteditor_env")) {
                    onWorkoutEditorEnv(sender);
                    return;
                } else if (msg == QStringLiteral("workouteditor_start")) {
                    onWorkoutEditorStart(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("appendactivitydescription")) {
                    onAppendActivityDescription(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("savetrainingprogram")) {
                    onSaveTrainingProgram(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("deletetrainingprogram")) {
                    onDeleteTrainingProgram(jsonObject[QStringLiteral("content")], sender);
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
                } else if (msg == QStringLiteral("speed_plus")) {
                    onSpeedPlus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("speed_minus")) {
                    onSpeedMinus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("inclination_plus")) {
                    onInclinationPlus(jsonObject[QStringLiteral("content")], sender);
                    return;
                } else if (msg == QStringLiteral("inclination_minus")) {
                    onInclinationMinus(jsonObject[QStringLiteral("content")], sender);
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
                } else if (msg == QStringLiteral("getpreviewsessionarray")) {
                    onGetPreviewSessionArray(sender);
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
        obj.setProperty(QStringLiteral("BIKE_TYPE"), (int)BIKE);
        obj.setProperty(QStringLiteral("ELLIPTICAL_TYPE"), (int)ELLIPTICAL);
        obj.setProperty(QStringLiteral("ROWING_TYPE"), (int)ROWING);
        obj.setProperty(QStringLiteral("TREADMILL_TYPE"), (int)TREADMILL);
        obj.setProperty(QStringLiteral("UNKNOWN_TYPE"), (int)UNKNOWN);
    }
    if (!device) {
        obj.setProperty(QStringLiteral("deviceId"), QJSValue());
    } else {
        QTime el = device->elapsedTime();
        QTime elLap = device->lapElapsedTime();
        QString name;
        QString nickName;
        BLUETOOTH_TYPE tp = device->deviceType();

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
        obj.setProperty(QStringLiteral("watts"), (device->wattsMetricforUI()));
        dep = device->wattsMetric();
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
        if (tp == BIKE) {
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
        } else if (tp == ROWING) {
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
        } else if (tp == TREADMILL) {
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
        } else if (tp == ELLIPTICAL) {
            obj.setProperty(QStringLiteral("resistance"), (dep = ((elliptical *)device)->currentResistance()).value());
            obj.setProperty(QStringLiteral("resistance_avg"), dep.average());
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

void TemplateInfoSenderBuilder::previewSessionOnChart(QList<SessionLine> *session, FIT_SPORT sport) {
    previewSessionOnChart(session, sport, "");
}

void TemplateInfoSenderBuilder::previewSessionOnChart(QList<SessionLine> *session, FIT_SPORT sport, const QString &workoutName) {
    auto startTime = std::chrono::high_resolution_clock::now();
    qDebug() << "previewSessionOnChart: Starting with" << session->size() << "elements";
    
    clearPreviewSessionArray();
    auto afterClear = std::chrono::high_resolution_clock::now();
    qDebug() << "Clear took:" << std::chrono::duration_cast<std::chrono::milliseconds>(afterClear - startTime).count() << "ms";
    
    buildContext(true);
    auto afterContext = std::chrono::high_resolution_clock::now();
    qDebug() << "buildContext took:" << std::chrono::duration_cast<std::chrono::milliseconds>(afterContext - afterClear).count() << "ms";
    
    if (session->isEmpty()) {
        return;
    }
    
    // Pre-calculate stats for performance
    double avgWatts = 0, maxWatts = 0, avgHeart = 0, maxHeart = 0, avgSpeed = 0, maxSpeed = 0, avgCadence = 0, maxCadence = 0;
    double totalWatts = 0, totalHeart = 0, totalSpeed = 0, totalCadence = 0;
    int validWatts = 0, validHeart = 0, validSpeed = 0, validCadence = 0;
    
    // Single optimized pass
    for (const SessionLine &s : *session) {
        if (s.watt > 0) { totalWatts += s.watt; validWatts++; maxWatts = qMax((double)s.watt, maxWatts); }
        if (s.heart > 0) { totalHeart += s.heart; validHeart++; maxHeart = qMax((double)s.heart, maxHeart); }
        if (s.speed > 0) { totalSpeed += s.speed; validSpeed++; maxSpeed = qMax(s.speed, maxSpeed); }
        if (s.cadence > 0) { totalCadence += s.cadence; validCadence++; maxCadence = qMax((double)s.cadence, maxCadence); }
    }
    
    avgWatts = validWatts > 0 ? totalWatts / validWatts : 0;
    avgHeart = validHeart > 0 ? totalHeart / validHeart : 0;
    avgSpeed = validSpeed > 0 ? totalSpeed / validSpeed : 0;
    avgCadence = validCadence > 0 ? totalCadence / validCadence : 0;
    
    auto afterStats = std::chrono::high_resolution_clock::now();
    qDebug() << "Stats calculation took:" << std::chrono::duration_cast<std::chrono::milliseconds>(afterStats - afterContext).count() << "ms";
    
    // Pre-calculate common values outside the loop
    QSettings settings;
    const QString nickName = settings.value(QZSettings::user_nickname, QZSettings::default_user_nickname).toString();
    const QString displayNickName = nickName.isEmpty() ? QStringLiteral("N/A") : nickName;
    const QString displayWorkoutName = workoutName.isEmpty() ? QStringLiteral("FIT Workout") : workoutName;
    const QString instructorName = QStringLiteral("");
    
    // Get properly formatted date from first session element for summary display
    const QString workoutStartDateFormatted = !session->isEmpty() ? session->first().time.toString() : QStringLiteral("");
    
    auto afterPreCalc = std::chrono::high_resolution_clock::now();
    qDebug() << "Pre-calculation took:" << std::chrono::duration_cast<std::chrono::milliseconds>(afterPreCalc - afterStats).count() << "ms";
    
    // Reserve space for better performance
    previewSessionArray = QJsonArray();
    
    qDebug() << "Starting main loop with" << session->size() << "elements";
    int processedItems = 0;
    
    // Pre-create template QJsonObject for reuse
    QJsonObject itemTemplate;
    itemTemplate[QStringLiteral("speed_avg")] = avgSpeed;
    itemTemplate[QStringLiteral("speed_max")] = maxSpeed;
    itemTemplate[QStringLiteral("heart_avg")] = avgHeart;
    itemTemplate[QStringLiteral("heart_max")] = maxHeart;
    itemTemplate[QStringLiteral("watts_avg")] = avgWatts;
    itemTemplate[QStringLiteral("watts_max")] = maxWatts;
    itemTemplate[QStringLiteral("workoutName")] = displayWorkoutName;
    itemTemplate[QStringLiteral("instructorName")] = instructorName;
    itemTemplate[QStringLiteral("nickName")] = displayNickName;
    itemTemplate[QStringLiteral("cadence_avg")] = avgCadence;
    itemTemplate[QStringLiteral("cadence_max")] = maxCadence;
    itemTemplate[QStringLiteral("workoutStartDate")] = workoutStartDateFormatted;
    
    // Simple optimized loop - no coordinates
    for (const SessionLine &s : *session) {
        QJsonObject item = itemTemplate; // Copy template (includes workoutStartDate)
        
        // Time calculation (optimized)
        const int totalSeconds = s.elapsedTime;
        item[QStringLiteral("elapsed_s")] = totalSeconds % 60;
        item[QStringLiteral("elapsed_m")] = (totalSeconds % 3600) / 60;
        item[QStringLiteral("elapsed_h")] = totalSeconds / 3600;
        
        // Variable properties only
        item[QStringLiteral("speed")] = s.speed;
        item[QStringLiteral("calories")] = s.calories;
        item[QStringLiteral("distance")] = s.distance;
        item[QStringLiteral("heart")] = s.heart;
        item[QStringLiteral("elevation")] = s.elevationGain;
        item[QStringLiteral("watts")] = s.watt;
        
        // Sport-specific properties
        if (sport == FIT_SPORT_CYCLING) {
            item[QStringLiteral("cadence")] = s.cadence;
            item[QStringLiteral("resistance")] = s.resistance;
        } else if (sport == FIT_SPORT_ROWING) {
            item[QStringLiteral("cadence")] = s.cadence;
            item[QStringLiteral("resistance")] = s.resistance;
            item[QStringLiteral("strokescount")] = static_cast<int>(s.totalStrokes);
            item[QStringLiteral("strokeslength")] = static_cast<double>(s.avgStrokesLength);
        } else if (sport == FIT_SPORT_RUNNING || sport == FIT_SPORT_WALKING) {
            item[QStringLiteral("inclination")] = s.inclination;
            item[QStringLiteral("stridelength")] = s.instantaneousStrideLengthCM;
            item[QStringLiteral("groundcontact")] = s.groundContactMS;
            item[QStringLiteral("verticaloscillation")] = s.verticalOscillationMM;
        } else if (sport == FIT_SUB_SPORT_ELLIPTICAL) {
            item[QStringLiteral("cadence")] = s.cadence;
            item[QStringLiteral("resistance")] = s.resistance;
            item[QStringLiteral("inclination")] = s.inclination;
        }
        
        previewSessionArray.append(item);
        processedItems++;
        
        // Log progress every 1000 items
        if (processedItems % 1000 == 0) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            qDebug() << "Processed" << processedItems << "items, elapsed:" 
                     << std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - afterPreCalc).count() << "ms";
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    auto loopTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - afterPreCalc).count();
    
    qDebug() << "previewSessionOnChart: Added" << previewSessionArray.size() << "elements to preview array";
    qDebug() << "Total time:" << totalTime << "ms, Main loop time:" << loopTime << "ms";
    qDebug() << "Average per item:" << (session->size() > 0 ? (double)loopTime / session->size() : 0) << "ms";
}

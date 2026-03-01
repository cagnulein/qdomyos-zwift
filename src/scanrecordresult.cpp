#include "scanrecordresult.h"

#if defined(Q_OS_ANDROID)
#include <QJniObject>
#endif

static const int ScanTypeId = qRegisterMetaType<ScanRecordResult>();

#if defined(Q_OS_ANDROID)
ScanRecordResult ScanRecordResult::fromJObject(JNIEnv *env, jobject java) {
    if (!java)
        return ScanRecordResult();
    QJniObject srr(java);
    // qDebug() << "SRR tos "<<srr.toString();
    int rssi = srr.callMethod<jint>("getRssi");
    QJniObject tmp = srr.callObjectMethod("getName", "()Ljava/lang/String;");
    QString name(tmp.toString());
    tmp = srr.callObjectMethod("getAddress", "()Ljava/lang/String;");
    QString address(tmp.toString());
    tmp = srr.callObjectMethod("getData", "()[B");
    jbyteArray dataArray = tmp.object<jbyteArray>();
    QByteArray data;
    if (dataArray) {
        jsize dataSize = env->GetArrayLength(dataArray);
        if (dataSize) {
            jbyte *dataEls = env->GetByteArrayElements(dataArray, 0);
            data.append((const char *)dataEls, dataSize);
            env->ReleaseByteArrayElements(dataArray, dataEls, 0);
        }
    }
    return ScanRecordResult(rssi, name, address, data);
}
#endif

bool ScanRecordResult::isValid() const { return !address.isEmpty(); }

ScanRecordResult::ScanRecordResult(int rss, const QString &nam, const QString &addres, const QByteArray &dat) {
    rssi = rss;
    name = nam;
    address = addres;
    data = dat;
}

ScanRecordResult::ScanRecordResult() {}

ScanRecordResult::~ScanRecordResult() {}

ScanRecordResult::ScanRecordResult(const ScanRecordResult &srr) {
    rssi = srr.rssi;
    name = srr.name;
    address = srr.address;
    data = srr.data;
}

QString ScanRecordResult::toString() const {
    return isValid() ? name + QStringLiteral(" (") + address + QStringLiteral(")[") + QString::number(rssi) +
                           QStringLiteral("] ") + data.toHex(' ')
                     : QStringLiteral("N/A");
}

int ScanRecordResult::getRssi() const { return rssi; }

void ScanRecordResult::setRssi(int rssi) { this->rssi = rssi; }

QByteArray ScanRecordResult::getData() const { return data; }

void ScanRecordResult::setData(const QByteArray &data) { this->data = data; }

QString ScanRecordResult::getName() const { return name; }

void ScanRecordResult::setName(const QString &name) { this->name = name; }

QString ScanRecordResult::getAddress() const { return address; }

void ScanRecordResult::setAddress(const QString &address) { this->address = address; }

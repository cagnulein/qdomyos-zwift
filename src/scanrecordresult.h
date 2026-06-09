#ifndef SCANRECORDRESULT_H
#define SCANRECORDRESULT_H
#include <QByteArray>
#include <QMetaType>
#include <QString>
#if defined(Q_OS_ANDROID)
#include <QJniObject>
#endif

class ScanRecordResult {
    int rssi = -1;
    QByteArray data;
    QString name, address;

  public:
    ScanRecordResult();
    ~ScanRecordResult();
    ScanRecordResult(int rss, const QString &nam, const QString &addres, const QByteArray &dat);
#if defined(Q_OS_ANDROID)
    static ScanRecordResult fromJObject(JNIEnv *env, jobject java);
#endif
    ScanRecordResult(const ScanRecordResult &sr);
    QString toString() const;
    int getRssi() const;
    void setRssi(int rssi);
    QByteArray getData() const;
    void setData(const QByteArray &data);
    QString getName() const;
    void setName(const QString &name);
    QString getAddress() const;
    void setAddress(const QString &address);
    bool isValid() const;
};

Q_DECLARE_METATYPE(ScanRecordResult)

#endif // SCANRECORDRESULT_H

#ifndef ANDROIDACTIVITYRESULTRECEIVER_H
#define ANDROIDACTIVITYRESULTRECEIVER_H
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QObject>
#include <QJniEnvironment>
#include <QJniObject>
#include <QCoreApplication>

class AndroidActivityResultReceiver : public QObject {
  public:
    AndroidActivityResultReceiver();
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const QJniObject &data);
};
#endif
#endif // ANDROIDACTIVITYRESULTRECEIVER_H

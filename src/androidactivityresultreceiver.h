#ifndef ANDROIDACTIVITYRESULTRECEIVER_H
#define ANDROIDACTIVITYRESULTRECEIVER_H
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidActivityResultReceiver>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QtAndroid>

class AndroidActivityResultReceiver : public QAndroidActivityResultReceiver {
  public:
    AndroidActivityResultReceiver();
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data);
};
#endif
#endif // ANDROIDACTIVITYRESULTRECEIVER_H

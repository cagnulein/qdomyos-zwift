#ifndef ANDROIDACTIVITYRESULTRECEIVER_H
#define ANDROIDACTIVITYRESULTRECEIVER_H

#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidActivityResultReceiver>

class AndroidActivityResultReceiver : public QAndroidActivityResultReceiver
{
public:
    AndroidActivityResultReceiver();
    virtual void handleActivityResult(int receiverRequestCode, int resultCode, const QAndroidJniObject &data);
};

#endif // ANDROIDACTIVITYRESULTRECEIVER_H

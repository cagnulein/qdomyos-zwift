#include "androidactivityresultreceiver.h"
#ifdef Q_OS_ANDROID
AndroidActivityResultReceiver::AndroidActivityResultReceiver() { qDebug() << "AndroidActivityResultReceiver"; }

void AndroidActivityResultReceiver::handleActivityResult(int receiverRequestCode, int resultCode,
                                                         const QAndroidJniObject &data) {
    qDebug() << "AndroidActivityResultReceiver::handleActivityResult" << receiverRequestCode << resultCode;
    QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/MediaProjection", "startService",
                                              "(Landroid/content/Context;ILandroid/content/Intent;)V",
                                              QtAndroid::androidContext().object(), resultCode, data.object<jobject>());
}
#endif

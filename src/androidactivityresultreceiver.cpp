#include "androidactivityresultreceiver.h"
#ifdef Q_OS_ANDROID
#include <QJniObject>
AndroidActivityResultReceiver::AndroidActivityResultReceiver() { qDebug() << "AndroidActivityResultReceiver"; }

void AndroidActivityResultReceiver::handleActivityResult(int receiverRequestCode, int resultCode,
                                                         const QJniObject &data) {
    qDebug() << "AndroidActivityResultReceiver::handleActivityResult" << receiverRequestCode << resultCode;
    QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/MediaProjection", "startService",
                                              "(Landroid/content/Context;ILandroid/content/Intent;)V",
                                              context.object(), resultCode, data.object<jobject>());
}
#endif

// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#include "applicationui.hpp"

#include <QGuiApplication>
#include <QtQml>

#include <QDir>
#include <QFile>

#include <QDebug>

#if defined(Q_OS_ANDROID)
#include <QtAndroid>
#endif

ApplicationUI::ApplicationUI(const QString &pth, QObject *parent) : QObject(parent), mShareUtils(new ShareUtils(this)) {
    // this is a demo application where we deal with an Image and a PDF as example
    // Image and PDF are delivered as qrc:/ resources at /data_assets
    // to start the tests as first we must copy these 2 files from assets into APP DATA
    // so we can simulate HowTo view, edit or send files from inside your APP DATA to other APPs
    // in a real life app you'll have your own workflows
    // I made copyAssetsToAPPData() INVOKABLE to be able to reset to origin files
    mAppDataFilesPath = pth;
    mAppDataFilesPath =
        (mAppDataFilesPath.isEmpty() ? QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).value(0)
                                     : mAppDataFilesPath)
            .append(QStringLiteral("/settings"));
    if (!QDir(mAppDataFilesPath).exists()) {
        if (QDir("").mkpath(mAppDataFilesPath)) {
            qDebug() << "Created Documents Location work directory. " << mAppDataFilesPath;
        } else {
            qWarning() << "Failed to create Documents Location work directory. " << mAppDataFilesPath;
        }
    }
}

void ApplicationUI::addContextProperty(QQmlContext *context) {
    context->setContextProperty("appui", this);
    context->setContextProperty("shareUtils", mShareUtils);
}

QString ApplicationUI::filePathDocumentsLocation(const QString &sourceFilePath) {
    QFileInfo fi(sourceFilePath);
    QString destinationFilePath = sourceFilePath;
    if (fi.exists() && fi.isFile()) {
        destinationFilePath = mAppDataFilesPath + QStringLiteral("/") + fi.fileName();
        qDebug() << "Destination is" << destinationFilePath;
        if (QFile::exists(destinationFilePath)) {
            bool removed = QFile::remove(destinationFilePath);
            if (!removed) {
                qWarning() << "Failed to remove " << destinationFilePath;
                return destinationFilePath;
            }
        }
        bool copied = QFile::copy(sourceFilePath, destinationFilePath);
        if (!copied) {
            qWarning() << "Failed to copy " << sourceFilePath << " to " << destinationFilePath;
            //#if defined(Q_OS_ANDROID)
            //        emit noDocumentsWorkLocation();
            //#endif
        }
    }
    return destinationFilePath;
}

void ApplicationUI::simulateIntentReceived(const QString &suffix, const QUrl &sourceFilePath) {
    QString path = mAppDataFilesPath.left(mAppDataFilesPath.lastIndexOf("/")).append("/") + suffix;
    if (!QDir(path).exists()) {
        if (QDir("").mkpath(path)) {
            qDebug() << "Created Documents Location work directory. " << path;
        } else {
            qWarning() << "Failed to create Documents Location work directory. " << path;
        }
    }
    mShareUtils->simulateIntentReceived(sourceFilePath, path);
}

#if defined(Q_OS_ANDROID)
void ApplicationUI::onApplicationStateChanged(Qt::ApplicationState applicationState) {
    qDebug() << "S T A T E changed into: " << applicationState;
    if (applicationState == Qt::ApplicationState::ApplicationSuspended) {
        // nothing to do
        return;
    }
    if (applicationState == Qt::ApplicationState::ApplicationActive) {
        // if App was launched from VIEW or SEND Intent
        // there's a race collision: the event will be lost,
        // because App and UI wasn't completely initialized
        // workaround: QShareActivity remembers that an Intent is pending
        if (!mPendingIntentsChecked) {
            mPendingIntentsChecked = true;
            mShareUtils->checkPendingIntents(mAppDataFilesPath);
        }
    }
}
// we don't need permissions if we only share files to other apps using FileProvider
// but we need permissions if other apps share their files with out app and we must access those files
bool ApplicationUI::checkPermission() {
    QtAndroid::PermissionResult r = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
    if (r == QtAndroid::PermissionResult::Denied) {
        QtAndroid::requestPermissionsSync(QStringList() << "android.permission.WRITE_EXTERNAL_STORAGE");
        r = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if (r == QtAndroid::PermissionResult::Denied) {
            qDebug() << "Permission denied";
            emit noDocumentsWorkLocation();
            return false;
        }
    }
    qDebug() << "YEP: Permission OK";
    return true;
}
#endif

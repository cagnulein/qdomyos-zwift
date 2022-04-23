// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#ifndef APPLICATIONUI_HPP
#define APPLICATIONUI_HPP

#include <QObject>

#include "shareutils.hpp"
#include <QtQml>

class ApplicationUI : public QObject {
    Q_OBJECT

  public:
    ApplicationUI(const QString &pth = QStringLiteral(""), QObject *parent = 0);

    void addContextProperty(QQmlContext *context);

    Q_INVOKABLE
    void simulateIntentReceived(const QString &suffix, const QUrl &sourceFilePath);

    Q_INVOKABLE
    QString filePathDocumentsLocation(const QString &sourceFilePath);

#if defined(Q_OS_ANDROID)
    Q_INVOKABLE
    bool checkPermission();
#endif

  signals:
    void noDocumentsWorkLocation();

  public slots:
#if defined(Q_OS_ANDROID)
    void onApplicationStateChanged(Qt::ApplicationState applicationState);
#endif

  private:
    ShareUtils *mShareUtils;
    QString mAppDataFilesPath;
    bool mPendingIntentsChecked;
};

#endif // APPLICATIONUI_HPP

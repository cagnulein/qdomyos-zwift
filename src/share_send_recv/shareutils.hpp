// (c) 2017 Ekkehard Gentz (ekke)
// this project is based on ideas from
// http://blog.lasconic.com/share-on-ios-and-android-using-qml/
// see github project https://github.com/lasconic/ShareUtils-QML
// also inspired by:
// https://www.androidcode.ninja/android-share-intent-example/
// https://www.calligra.org/blogs/sharing-with-qt-on-android/
// https://stackoverflow.com/questions/7156932/open-file-in-another-app
// http://www.qtcentre.org/threads/58668-How-to-use-QAndroidJniObject-for-intent-setData
// see also /COPYRIGHT and /LICENSE

// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#ifndef SHAREUTILS_H
#define SHAREUTILS_H

#include <QObject>

#include <QDebug>

class PlatformShareUtils : public QObject
{
    Q_OBJECT
signals:
    void shareEditDone(int requestCode);
    void shareFinished(int requestCode);
    void shareNoAppAvailable(int requestCode);
    void shareError(int requestCode, QString message);
    void fileUrlReceived(QString url);
    void fileReceivedAndSaved(QString url);

public:
    PlatformShareUtils(QObject *parent = 0) : QObject(parent){}
    virtual ~PlatformShareUtils() {}
    virtual bool checkMimeTypeView(const QString &mimeType){
        qDebug() << "check view for " << mimeType;
        return true;}
    virtual bool checkMimeTypeEdit(const QString &mimeType){
        qDebug() << "check edit for " << mimeType;
        return true;}
    virtual void share(const QString &text, const QUrl &url){ qDebug() << text << url; }
    virtual void sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl){
        qDebug() << filePath << " - " << title << "requestId " << requestId << " - " << mimeType << "altImpl? " << altImpl; }
    virtual void viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl){
        qDebug() << filePath << " - " << title << " requestId: " << requestId << " - " << mimeType << "altImpl? " << altImpl; }
    virtual void editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl){
        qDebug() << filePath << " - " << title << " requestId: " << requestId << " - " << mimeType << "altImpl? " << altImpl; }

    virtual void checkPendingIntents(const QString workingDirPath){
        qDebug() << "checkPendingIntents " << workingDirPath; }
};

class ShareUtils : public QObject
{
    Q_OBJECT


signals:
    void shareEditDone(int requestCode);
    void shareFinished(int requestCode);
    void shareNoAppAvailable(int requestCode);
    void shareError(int requestCode, QString message);
    void fileUrlReceived(QString url);
    void fileReceivedAndSaved(QString url);

public slots:
    void onShareEditDone(int requestCode);
    void onShareFinished(int requestCode);
    void onShareNoAppAvailable(int requestCode);
    void onShareError(int requestCode, QString message);
    void onFileUrlReceived(QString url);
    void onFileReceivedAndSaved(QString url);

public:
    explicit ShareUtils(QObject *parent = 0);
    Q_INVOKABLE bool checkMimeTypeView(const QString &mimeType);
    Q_INVOKABLE bool checkMimeTypeEdit(const QString &mimeType);
    Q_INVOKABLE void share(const QString &text, const QUrl &url);
    Q_INVOKABLE void sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl);
    Q_INVOKABLE void viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl);
    Q_INVOKABLE void editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl);
    Q_INVOKABLE void checkPendingIntents(const QString workingDirPath);

private:
    PlatformShareUtils* mPlatformShareUtils;

};

#endif //SHAREUTILS_H

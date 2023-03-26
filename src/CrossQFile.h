#ifndef CROSSQFILE_H
#define CROSSQFILE_H

#include <QObject>
#include <QFile>
#include <QSharedPointer>
#ifdef __ANDROID__
#include <QtAndroidExtras/QAndroidJniObject>
#endif

class CrossQFile : public QFile
{
public:
    CrossQFile(const QString& nameOrUri, const bool isUri = false);
    virtual qint64 size() const override;

    //if working with uri, it uses QAndroidjniObject to open the file using the uri.
    //otherwise it would act like a normal QFile.
    bool open(CrossQFile::OpenMode openMode) override;
    bool remove();
    //if working with uri, it does nothing.
    //otherwise it would act like a normal QFile.
    bool rename(const QString& newName);
    bool exists() const;
    //returns the display name of the file.
    QString displayName();
    void setFileName(const QString& nameOrUri, bool isUri = false);
private:
    bool isWorkingWithUri{false};
#ifdef __ANDROID__
    QAndroidJniObject mainActivityObj;
    QAndroidJniObject contentResolverObj;
    bool checkJenvExceptions() const;
    QAndroidJniObject parseUriString(const QString& uriString) const;
#endif
};


#endif // CROSSQFILE_H


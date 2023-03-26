#include "CrossQFile.h"
#include <QFileInfo>
#ifdef __ANDROID__
#include <jni.h>
#include <QtAndroidExtras/QtAndroid>
#include <QtAndroidExtras/qandroidjnienvironment.h>
#endif
#include <QCoreApplication>

CrossQFile::CrossQFile(const QString& nameOrUri, const bool isUri) : QFile(nameOrUri), isWorkingWithUri(isUri){
#ifdef __ANDROID__
    mainActivityObj = QtAndroid::androidActivity();
    contentResolverObj = mainActivityObj.callObjectMethod
            ("getContentResolver","()Landroid/content/ContentResolver;");
    checkJenvExceptions();
#endif
}

#ifdef __ANDROID__
bool CrossQFile::checkJenvExceptions() const{
    QAndroidJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }
    return false;
}
QAndroidJniObject CrossQFile::parseUriString(const QString& uriString) const{
    return QAndroidJniObject::callStaticObjectMethod
                ("android/net/Uri" , "parse",
                 "(Ljava/lang/String;)Landroid/net/Uri;",
                 QAndroidJniObject::fromString(uriString).object());
}
#endif

void CrossQFile::setFileName(const QString& nameOrUri, bool isUri){
    QFile::setFileName(nameOrUri);
    isWorkingWithUri = isUri;
}


qint64 CrossQFile::size() const{
#ifdef __ANDROID__
    if(isWorkingWithUri){
        QAndroidJniObject cursorObj {contentResolverObj.callObjectMethod
                    ("query",
                     "(Landroid/net/Uri;[Ljava/lang/String;Landroid/os/Bundle;Landroid/os/CancellationSignal;)Landroid/database/Cursor;",
                     parseUriString(fileName()).object(), QAndroidJniObject().object(), QAndroidJniObject().object()
                     , QAndroidJniObject().object(), QAndroidJniObject().object())};
        int sizeIndex {cursorObj.callMethod<jint>
                    ("getColumnIndex","(Ljava/lang/String;)I",
                     QAndroidJniObject::getStaticObjectField<jstring>
                     ("android/provider/OpenableColumns","SIZE").object())};
        cursorObj.callMethod<jboolean>("moveToFirst");
        qint64 ret {cursorObj.callMethod<jlong>("getLong","(I)J",sizeIndex)};
        if(checkJenvExceptions()){
            ret = 0;
        }
        return ret;
    }
#endif
    return QFile::size();
}

bool CrossQFile::open(CrossQFile::OpenMode openMode){
#ifdef __ANDROID__
    if(isWorkingWithUri){
        QAndroidJniObject jopenMode {QAndroidJniObject::fromString("rw")};
        switch (openMode){
            case QFile::ReadOnly:
                jopenMode = QAndroidJniObject::fromString("r");
                break;
            case QFile::WriteOnly:
                jopenMode = QAndroidJniObject::fromString("w");
                break;
            default:
                jopenMode = QAndroidJniObject::fromString("rw");
        }
        QAndroidJniObject pfdObj{contentResolverObj.callObjectMethod
                    ("openFileDescriptor", "(Landroid/net/Uri;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
                     parseUriString(fileName()).object(), jopenMode.object())};
        int fd{pfdObj.callMethod<jint>("detachFd")};
        bool ret {false};
        if(QFile::open(fd, openMode)){
            ret = true;
        }
        if(checkJenvExceptions()){
            ret = false;
        }
        return ret;
    }
#endif
    return QFile::open(openMode);
}
QString CrossQFile::displayName(){
#ifdef __ANDROID__
    if(isWorkingWithUri){
        QAndroidJniObject cursorObj {contentResolverObj.callObjectMethod
                    ("query",
                     "(Landroid/net/Uri;[Ljava/lang/String;Landroid/os/Bundle;Landroid/os/CancellationSignal;)Landroid/database/Cursor;",
                     parseUriString(fileName()).object(), QAndroidJniObject().object(), QAndroidJniObject().object(),
                     QAndroidJniObject().object(), QAndroidJniObject().object())};
        cursorObj.callMethod<jboolean>("moveToFirst");
        QAndroidJniObject retObj{cursorObj.callObjectMethod
                    ("getString","(I)Ljava/lang/String;", cursorObj.callMethod<jint>
                     ("getColumnIndex","(Ljava/lang/String;)I",
                      QAndroidJniObject::getStaticObjectField<jstring>
                      ("android/provider/OpenableColumns","DISPLAY_NAME").object()))};
        QString ret {retObj.toString()};
        if(checkJenvExceptions()){
            ret = "";
        }
        return ret;
    }
#endif
    QFileInfo fileInfo(fileName());
    return fileInfo.fileName();
}

bool CrossQFile::remove(){
#ifdef __ANDROID__
    if(isWorkingWithUri){
        bool ret {static_cast<bool>(QAndroidJniObject::callStaticMethod<jboolean>
                                    ("android/provider/DocumentsContract", "deleteDocument",
                                     "(Landroid/content/ContentResolver;Landroid/net/Uri;)Z",
                                     contentResolverObj.object(), parseUriString(fileName()).object()))};
        if(checkJenvExceptions()){
            ret = false;
        }
        return ret;
    }
#endif
    return QFile::remove();
}

bool CrossQFile::rename(const QString& newName){
    if(!isWorkingWithUri){
        return QFile::rename(newName);
    }else{
        return false;
    }
}

bool CrossQFile::exists() const{
#ifdef __ANDROID__
    if(isWorkingWithUri){
        bool ret {static_cast<bool>(QAndroidJniObject::callStaticMethod<jboolean>
                                    ("android/provider/DocumentsContract", "isDocumentUri",
                                     "(Landroid/content/Context;Landroid/net/Uri;)Z",
                                     mainActivityObj.object(), parseUriString(fileName()).object()))};
        if(checkJenvExceptions()){
            ret = false;
        }
        return ret;
    }
#endif
    return QFile::exists();
}







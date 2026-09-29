#ifndef KEEPAWAKEHELPER_H
#define KEEPAWAKEHELPER_H

#include <QtCore>

#ifdef Q_OS_ANDROID

#include <QApplication>
#include <QSettings>
#include <QCoreApplication>
#include <QJniObject>
#include <QJniEnvironment>

static QJniObject activity;
static QJniObject *ant;

class KeepAwakeHelper {
  public:
    KeepAwakeHelper();
    virtual ~KeepAwakeHelper();
    static QJniObject *antObject(bool forceCreate);
    static int heart();
    void keepScreenOn(bool on);

  private:
    QJniObject m_wakeLock;
};

#endif
#endif // KEEPAWAKEHELPER_H

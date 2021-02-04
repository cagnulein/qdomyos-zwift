#ifndef KEEPAWAKEHELPER_H
#define KEEPAWAKEHELPER_H
#ifdef Q_OS_ANDROID

#include <QAndroidJniObject>

static QAndroidJniObject activity;

class KeepAwakeHelper
{
public:
    KeepAwakeHelper();
    virtual ~KeepAwakeHelper();

private:
    QAndroidJniObject m_wakeLock;
};

#endif
#endif // KEEPAWAKEHELPER_H

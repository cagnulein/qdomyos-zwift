#ifndef KEEPAWAKEHELPER_H
#define KEEPAWAKEHELPER_H
#ifdef Q_OS_ANDROID

#include <QAndroidJniObject>

static QAndroidJniObject activity;
static QAndroidJniObject* ant;

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

#include "virtualdevice.h"
#include "objectfactory.h"
#include <QDebug>

virtualdevice::virtualdevice(QObject *parent)
    : QObject{parent}
{
    this->lockscreenFunctions = ObjectFactory::createLockscreenFunctions();
}

QZLockscreenFunctions *virtualdevice::getLockscreenFunctions() const {
    return this->lockscreenFunctions;
}



virtualdevice::~virtualdevice() {
    qDebug() << "Deleting virtual device";

    if(this->lockscreenFunctions)
        delete this->lockscreenFunctions;
}

#include "virtualdevice.h"
#include "objectfactory.h"

#include <QDebug>

QZLockscreenFunctions *virtualdevice::getLockscreenFunctions() const {
    return this->lockscreenFunctions;
}

virtualdevice::virtualdevice(QObject *parent)
    : QObject{parent}
{
    this->lockscreenFunctions = ObjectFactory::createLockscreenFunctions();
}

virtualdevice::~virtualdevice() {
    qDebug() << "Deleting virtual device";

    if(this->lockscreenFunctions)
        delete this->lockscreenFunctions;
}

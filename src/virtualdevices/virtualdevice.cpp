#include "virtualdevice.h"
#include <QDebug>

virtualdevice::virtualdevice(QObject *parent)
    : QObject{parent}
{

}

virtualdevice::~virtualdevice() {
    qDebug() << "Deleting virtual device";
}

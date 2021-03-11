#include "M3iIOS-Interface.h"
#include "../scanrecordresult.h"
#include <QMetaObject>
#include <QtDebug>
#include <Foundation/NSString.h>

void m3i_callback(void * objref, m3i_result_t * res) {
    QByteArray data((const char *)res->bytes, res->nbytes);
    ScanRecordResult srr(res->rssi, res->name, res->uuid, data);
    qDebug() << "NEW ADV " << srr.toString();
    QMetaObject::invokeMethod((QObject *)objref, "processAdvertising", Qt::QueuedConnection,
                              Q_ARG(QByteArray, data));
}
void qt_log(const void * msg) {
    qDebug() << QString::fromNSString((const NSString *) msg);
}

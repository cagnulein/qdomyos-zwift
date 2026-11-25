#ifndef IOS_FLOATING_METRICS_H
#define IOS_FLOATING_METRICS_H

#include <QList>
#include <QString>

class iosFloatingMetrics {
public:
    static bool supported();
    static void start();
    static void stop();
    struct Metric {
        QString name;
        QString value;
        QString valueFontColor;
    };
    static void update(const QList<Metric> &metrics, const QString &emptyLabel);
    static bool running();
};
#endif

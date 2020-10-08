#ifndef TRAINPROGRAM_H
#define TRAINPROGRAM_H
#include <QTime>

class trainrow
{
public:
    QTime duration;
    double speed;
    double inclination;
    bool forcespeed;
};

class trainprogram
{
public:
    trainprogram(QList<trainrow>);
    void save(QString filename);
    static trainprogram* load(QString filename);

    QList<trainrow> rows;
private:        
};

#endif // TRAINPROGRAM_H

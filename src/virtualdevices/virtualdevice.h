#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include <QObject>

class virtualdevice : public QObject
{
    Q_OBJECT
public:
    explicit virtualdevice(QObject *parent = nullptr);
    ~virtualdevice() override;
    virtual bool connected()=0;

signals:

};

#endif // VIRTUALDEVICE_H

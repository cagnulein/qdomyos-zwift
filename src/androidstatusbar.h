#ifndef ANDROIDSTATUSBAR_H
#define ANDROIDSTATUSBAR_H

#include <QObject>
#include <QQmlEngine>

class AndroidStatusBar : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int height READ height CONSTANT)

public:
    explicit AndroidStatusBar(QObject *parent = nullptr);
    
    static void registerQmlType();
    
    int height() const;

private:
    int getStatusBarHeightFromAndroid() const;
    mutable int m_cachedHeight;
};

#endif // ANDROIDSTATUSBAR_H
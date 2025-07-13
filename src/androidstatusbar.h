#ifndef ANDROIDSTATUSBAR_H
#define ANDROIDSTATUSBAR_H

#include <QObject>
#include <QQmlEngine>
#include <QScreen>

class AndroidStatusBar : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int navigationBarHeight READ navigationBarHeight NOTIFY navigationBarHeightChanged)
    Q_PROPERTY(int apiLevel READ apiLevel CONSTANT)

public:
    explicit AndroidStatusBar(QObject *parent = nullptr);
    
    static void registerQmlType();
    
    int height() const;
    int navigationBarHeight() const;
    int apiLevel() const;

signals:
    void heightChanged();
    void navigationBarHeightChanged();

private slots:
    void onOrientationChanged();
    
private:
    int getStatusBarHeightFromAndroid() const;
    int getNavigationBarHeightFromAndroid() const;
    int getApiLevelFromAndroid() const;
    void invalidateCache();
    mutable int m_cachedHeight;
    mutable int m_cachedNavigationBarHeight;
};

#endif // ANDROIDSTATUSBAR_H
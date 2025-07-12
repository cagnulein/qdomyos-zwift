#ifndef SAFEAREA_H
#define SAFEAREA_H

#include <QObject>
#include <QQmlEngine>

class SafeArea : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int top READ top NOTIFY safeAreaChanged)
    Q_PROPERTY(int right READ right NOTIFY safeAreaChanged)
    Q_PROPERTY(int bottom READ bottom NOTIFY safeAreaChanged)
    Q_PROPERTY(int left READ left NOTIFY safeAreaChanged)

public:
    explicit SafeArea(QObject *parent = nullptr);

    static void registerQmlType();

    int top() const;
    int right() const;
    int bottom() const;
    int left() const;

public slots:
    void updateSafeArea(int top, int right, int bottom, int left);

signals:
    void safeAreaChanged();

private:
    int m_top;
    int m_right;
    int m_bottom;
    int m_left;
};

#endif // SAFEAREA_H
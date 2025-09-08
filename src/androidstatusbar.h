#ifndef ANDROIDSTATUSBAR_H
#define ANDROIDSTATUSBAR_H

#include <QObject>
#include <QQmlEngine>

class AndroidStatusBar : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int height READ height NOTIFY insetsChanged)
    Q_PROPERTY(int navigationBarHeight READ navigationBarHeight NOTIFY insetsChanged)
    Q_PROPERTY(int leftInset READ leftInset NOTIFY insetsChanged)
    Q_PROPERTY(int rightInset READ rightInset NOTIFY insetsChanged)
    Q_PROPERTY(int apiLevel READ apiLevel CONSTANT)

public:
    explicit AndroidStatusBar(QObject *parent = nullptr);

    static void registerQmlType();
    static AndroidStatusBar* instance();

    int height() const { return m_top; }
    int navigationBarHeight() const { return m_bottom; }
    int leftInset() const { return m_left; }
    int rightInset() const { return m_right; }
    int apiLevel() const;

public slots:
    void onInsetsChanged(int top, int bottom, int left, int right);

signals:
    void insetsChanged();

private:
    int m_top = 0;
    int m_bottom = 0;
    int m_left = 0;
    int m_right = 0;

    static AndroidStatusBar* m_instance;
};

#endif // ANDROIDSTATUSBAR_H

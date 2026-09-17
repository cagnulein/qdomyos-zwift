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
    Q_PROPERTY(int waterfallTopInset READ waterfallTopInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallBottomInset READ waterfallBottomInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallLeftInset READ waterfallLeftInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallRightInset READ waterfallRightInset NOTIFY insetsChanged)
    Q_PROPERTY(bool hasWaterfallDisplay READ hasWaterfallDisplay NOTIFY insetsChanged)
    Q_PROPERTY(int apiLevel READ apiLevel CONSTANT)

public:
    explicit AndroidStatusBar(QObject *parent = nullptr);

    static void registerQmlType();
    static AndroidStatusBar* instance();

    int height() const { return m_top; }
    int navigationBarHeight() const { return m_bottom; }
    int leftInset() const { return m_left; }
    int rightInset() const { return m_right; }
    int waterfallTopInset() const { return m_waterfallTop; }
    int waterfallBottomInset() const { return m_waterfallBottom; }
    int waterfallLeftInset() const { return m_waterfallLeft; }
    int waterfallRightInset() const { return m_waterfallRight; }
    bool hasWaterfallDisplay() const { return m_waterfallTop > 0 || m_waterfallBottom > 0 || m_waterfallLeft > 0 || m_waterfallRight > 0; }
    int apiLevel() const;

public slots:
    void onInsetsChanged(int top, int bottom, int left, int right, int waterfallTop, int waterfallBottom,
                         int waterfallLeft, int waterfallRight);

signals:
    void insetsChanged();

private:
    int m_top = 0;
    int m_bottom = 0;
    int m_left = 0;
    int m_right = 0;
    int m_waterfallTop = 0;
    int m_waterfallBottom = 0;
    int m_waterfallLeft = 0;
    int m_waterfallRight = 0;

    static AndroidStatusBar* m_instance;
};

#endif // ANDROIDSTATUSBAR_H

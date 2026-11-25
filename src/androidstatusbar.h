#ifndef ANDROIDSTATUSBAR_H
#define ANDROIDSTATUSBAR_H

#include <QColor>
#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>

class AndroidStatusBar : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int height READ height NOTIFY insetsChanged)
    Q_PROPERTY(int navigationBarHeight READ navigationBarHeight NOTIFY insetsChanged)
    Q_PROPERTY(int leftInset READ leftInset NOTIFY insetsChanged)
    Q_PROPERTY(int rightInset READ rightInset NOTIFY insetsChanged)
    // Left and right insets of the system bars alone, without the display cutout
    Q_PROPERTY(int systemBarLeftInset READ systemBarLeftInset NOTIFY insetsChanged)
    Q_PROPERTY(int systemBarRightInset READ systemBarRightInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallTopInset READ waterfallTopInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallBottomInset READ waterfallBottomInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallLeftInset READ waterfallLeftInset NOTIFY insetsChanged)
    Q_PROPERTY(int waterfallRightInset READ waterfallRightInset NOTIFY insetsChanged)
    Q_PROPERTY(bool hasWaterfallDisplay READ hasWaterfallDisplay NOTIFY insetsChanged)
    Q_PROPERTY(int apiLevel READ apiLevel CONSTANT)

public:
    explicit AndroidStatusBar(QObject *parent = nullptr);
    ~AndroidStatusBar() override;

    static void registerQmlType();
    static AndroidStatusBar* instance();

    int height() const { return m_top; }
    int navigationBarHeight() const { return m_bottom; }
    int leftInset() const { return m_left; }
    int rightInset() const { return m_right; }
    int systemBarLeftInset() const { return m_systemBarLeft; }
    int systemBarRightInset() const { return m_systemBarRight; }
    int waterfallTopInset() const { return m_waterfallTop; }
    int waterfallBottomInset() const { return m_waterfallBottom; }
    int waterfallLeftInset() const { return m_waterfallLeft; }
    int waterfallRightInset() const { return m_waterfallRight; }
    bool hasWaterfallDisplay() const { return m_waterfallTop > 0 || m_waterfallBottom > 0 || m_waterfallLeft > 0 || m_waterfallRight > 0; }
    int apiLevel() const;
    // System text size: Android font scale, iOS Dynamic Type; 1.0 elsewhere.
    // Read again when the app comes back to the foreground, the user changes it in the system settings.
    // In the iOS qmake build (IO_UNDER_QT) it comes from the app font Qt sets at start-up, so a change
    // there needs a restart.
    Q_INVOKABLE double systemFontScale() const;
    // Night mode of the system (Configuration.uiMode), for the modern look. Read on demand: QML asks
    // again when the app comes back to the foreground and on systemThemeChanged().
    // iOS: the dark appearance of the system (iOS 13+), read the same way.
    // Android before 10 (API 29, no dark theme in the system), iOS before 13, the iOS qmake build
    // (IO_UNDER_QT) and other systems: true (the app is dark by default).
    Q_INVOKABLE bool systemDarkMode() const;
    // iOS: the window of Qt covers only the safe area, and the system paints the areas under the
    // status bar and the home indicator black, with light status bar text. This gives them a colour
    // and the status bar text for a dark (light text) or light page, and from then on keeps the
    // status bar for a full screen window, which Qt would hide.
    // Android 6+ (API 23): modern true – the status and navigation bars without the black background
    // of the Qt 5.15 theme: transparent from API 31 (the window is under them), the page colour
    // below; icons dark on a light page (dark false). modern false puts back the bars of the window
    // as they were before the first modern call (color is then not used).
    // Nothing elsewhere.
    Q_INVOKABLE void setSystemBarsColor(const QColor &color, bool dark, bool modern) const;
    // iOS 11+: the safe area (status bar and cutout, home indicator, sides held sideways) in points,
    // and the size of the UIWindow it belongs to (the screen, or the window of the app on an iPad
    // with several apps on the screen), as { top, bottom, left, right, width, height }. Read again
    // on safeAreaChanged(). Zeros elsewhere.
    Q_INVOKABLE QVariantMap safeAreaInsets() const;

public slots:
    void onInsetsChanged(int top, int bottom, int left, int right, int waterfallTop, int waterfallBottom,
                         int waterfallLeft, int waterfallRight);
    void onSystemBarSideInsetsChanged(int left, int right);

signals:
    void insetsChanged();
    // The system theme may have changed, systemDarkMode() may answer differently now. Android: the
    // system configuration changed (night mode among others). iOS 13+: the system switched between
    // light and dark appearance while the app runs (Qt 5.15 does not report it to QML)
    void systemThemeChanged();
    // iOS: UIKit changed the safe area of the Qt window, safeAreaInsets() may answer differently now
    // (the status bar shown or hidden, the phone turned, the app window resized on an iPad). Neither
    // Qt 5.15 nor the Screen object of QML reports it
    void safeAreaChanged();

private:
    int m_top = 0;
    int m_bottom = 0;
    int m_left = 0;
    int m_right = 0;
    int m_systemBarLeft = 0;
    int m_systemBarRight = 0;
    int m_waterfallTop = 0;
    int m_waterfallBottom = 0;
    int m_waterfallLeft = 0;
    int m_waterfallRight = 0;

    static AndroidStatusBar* m_instance;
};

#endif // ANDROIDSTATUSBAR_H

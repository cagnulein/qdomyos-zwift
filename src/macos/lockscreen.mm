#include <Foundation/NSString.h>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

// These test-only dependencies are isolated in the macOS implementation so
// the portable main.cpp patch does not collide with the Qt6/MSVC2022 branch.
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QTimer>
#include <QtGlobal>
#include <stdio.h>

IOPMAssertionID assertion;

void lockScreen() {
    // CI exercises the app inside the mounted DMG. This runs after main.qml
    // and homeform have loaded, immediately before QApplication::exec().
    if (qEnvironmentVariableIsSet("QZ_CI_GUI_SMOKE_TEST")) {
        bool visibleQmlWindow = false;
        for (QWindow *window : QGuiApplication::allWindows()) {
            if (qobject_cast<QQuickWindow *>(window) && window->isVisible()) {
                visibleQmlWindow = true;
                break;
            }
        }

        if (!visibleQmlWindow) {
            fputs("QZ_QML_WINDOW_NOT_VISIBLE\n", stderr);
            fflush(stderr);
            QTimer::singleShot(0, [] { QCoreApplication::exit(1); });
        } else {
            fputs("QZ_QML_LOADED\n", stderr);
            fflush(stderr);
            QTimer::singleShot(5000, [] { QCoreApplication::quit(); });
        }
        // No sleep-prevention assertion needed for a five-second smoke test.
        return;
    }

    CFURLRef bundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
    CFRelease(bundleURL);

    IOReturn result = IOPMAssertionCreateWithDescription(kIOPMAssertionTypePreventUserIdleSystemSleep,
        CFSTR("A name which makes sense for your app"),
        CFSTR("Some details"), CFSTR("A human-readable reason"),
        bundlePath, 0, NULL, &assertion);
    CFRelease(bundlePath);
}

void unlockScreen()
{
    IOPMAssertionRelease(assertion);
}

#include <Foundation/NSString.h>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

IOPMAssertionID assertion;

void lockScreen() {
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

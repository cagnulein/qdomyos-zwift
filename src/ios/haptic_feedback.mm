#import <UIKit/UIKit.h>
#import <dispatch/dispatch.h>

// A light confirmation tick, respecting the system's haptics preferences.
extern "C" void qzHapticFeedback() {
    dispatch_async(dispatch_get_main_queue(), ^{
        static UISelectionFeedbackGenerator *generator = nil;
        if (!generator)
            generator = [[UISelectionFeedbackGenerator alloc] init];
        [generator selectionChanged];
        [generator prepare];
    });
}

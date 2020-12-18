#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

void lockscreen::setTimerDisabled() {
     [[UIApplication sharedApplication] setIdleTimerDisabled: YES]
}

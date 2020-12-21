#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "ios/lockscreen.h"

void lockscreen::setTimerDisabled() {
     [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
}

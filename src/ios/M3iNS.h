//
//  M3iIOS.h
//  qdomyoszwift
//
//  Created by MatteoFederico on 11/03/21.
//

#ifndef M3iIOS_h
#define M3iIOS_h
#import "M3iIOS-Interface.h"
#include <Foundation/Foundation.h>
#include <CoreBluetooth/CoreBluetooth.h>

@interface M3iNS:NSObject<CBCentralManagerDelegate>

- (instancetype)initWithObj:(void *) obj;
- (void)startScan: (m3i_result_t *)initStr;
- (void)stopScan;

@end

#endif /* M3iIOS_h */

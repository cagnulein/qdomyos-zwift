//
//  M3iIOS.h
//  qdomyoszwift
//
//  Created by MatteoFederico on 11/03/21.
//

#ifndef M3iIOS_h
#define M3iIOS_h
#import "M3iIOS-Interface.h"
@import CoreBluetooth;

@interface M3iNS:NSObject<CBCentralManagerDelegate>
{
    m3i_result_t * conf;
    void * objref;
    NSUUID * devUid;
    CBCentralManager *cbCentralManager;
    BOOL startRequested;
}

- (instancetype)initWithObj:(void *) obj;
- (void)startScan: (m3i_result_t * initStr);
- (void)stopScan();

@end

#endif /* M3iIOS_h */

#import "M3iNS.h"
#import "M3iIOS-Interface.h"
#include <Foundation/NSString.h>


M3iIOS::M3iIOS ( void ) {
    self = 0;
}

M3iIOS::~M3iIOS( void ) {
    NSLog(@"Deleting m3ios self = %p", self);
    if (self) {
        CFRelease(self);
        self = 0;
    }
}

void M3iIOS::init( void * objref ) {
    self = (void *)CFBridgingRetain([[M3iNS alloc] initWithObj:objref]);//tst
    NSLog(@"self is %p", self);
}

void  M3iIOS::startScan( m3i_result_t * conf) {
    NSLog(@"self is %p", self);
    if (self)
        [(id)self startScan:conf];
}

bool M3iIOS::isScanning() const
{
    NSLog(@"self is %p", self);
    if (self)
        return (bool)[(id)self isScanning];
    else
        return false;
}
    
void M3iIOS::stopScan() {
    if (self)
        [(id)self stopScan];
}

@implementation M3iNS
m3i_result_t * conf;
void * objref;
NSUUID * devUid;
CBCentralManager *cbCentralManager;
BOOL startRequested;
char logs[512];
NSTimeInterval lastRestart = -1;
//dispatch_queue_t myQueue;

- (instancetype)initWithObj:(void *) obj {
    self = [super init];
    if (self) {
        objref = obj;
        conf = 0;
        devUid = 0;
        startRequested = NO;
        qt_log("cbc about to init");
        //myQueue = dispatch_queue_create("BLEM3I", 0);
        cbCentralManager = [[CBCentralManager alloc] initWithDelegate:self queue: dispatch_get_main_queue()];
        qt_log("cbc inited");
    }
    return self;
}

-(void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state == CBManagerStatePoweredOn && startRequested){
        [self startScan:conf];
    } else {
        NSLog(@"bluetooth not on");
    }
}

- (BOOL)isScanning {
    return [cbCentralManager isScanning];
}

- (void)stopScan {
    if ([cbCentralManager isScanning])
        [cbCentralManager stopScan];
}

- (void)startScan:(m3i_result_t *) config {
    qt_log("in startscan");
    conf = config;
    devUid = [[NSUUID alloc] initWithUUIDString:[NSString stringWithUTF8String:conf->uuid]];
    if (cbCentralManager.state == CBManagerStatePoweredOn) {
        //NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBCentralManagerScanOptionAllowDuplicatesKey, nil];
        qt_log("about to start scan");
        [ cbCentralManager scanForPeripheralsWithServices:nil options:nil ];
        qt_log("scan started");
        startRequested = NO;
        lastRestart = [[NSDate date] timeIntervalSince1970];
    }
    else
        startRequested = YES;
}

-(void)logOnQt:(NSString *) logString {
    NSLog(@"%@", logString);
    [logString getCString:logs maxLength:sizeof(logs)/sizeof(*logs)-1 encoding:NSUTF8StringEncoding];
    qt_log(logs);
}

-(void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *,id> *)advertisementData RSSI:(NSNumber *)RSSI {
    NSString * name = [peripheral name];
    if (name == 0) name = @"N/A";
    NSString * uuidstring = [peripheral.identifier UUIDString];
    NSString * logString = [NSString stringWithFormat:@"Received %@ (%@)[%d]", uuidstring, name, (int)[RSSI integerValue]];
    NSTimeInterval now = [[NSDate date] timeIntervalSince1970];
    [self logOnQt:logString];
    if (peripheral.identifier && [devUid isEqual:peripheral.identifier]) {
        NSData * data = [advertisementData objectForKey:@"kCBAdvDataManufacturerData"];
        if (data) {
            NSUInteger dataLength = [data length];
            const unsigned char *arr = (const unsigned char *)[data bytes];
            if (arr && dataLength>=10) {
                conf->rssi = (int)[RSSI integerValue];
                conf->nbytes = dataLength > sizeof(conf->bytes)?sizeof(conf->bytes): (int)dataLength;
                memcpy(conf->bytes, arr, conf->nbytes);
                [name getCString:conf->name maxLength:sizeof(conf->name)/sizeof(*conf->name)-1 encoding:NSUTF8StringEncoding];
                m3i_callback(objref, conf);
            }
        }
    }
    if (now-lastRestart >= 60) {
        [self stopScan];
        [self startScan:conf];
        lastRestart = now;
    }
}

@end

#import "M3iNS.h"
#import "M3iIOS-Interface.h"
#include <Foundation/NSString.h>


M3iIOS::M3iIOS ( void ) {
    self = 0;
}

M3iIOS::~M3iIOS( void ) {
    if (self)
        [(id)self dealloc];
}

void M3iIOS::init( void * objref ) {
    self = [[M3iNS alloc] initWithObj:objref];
}

void  M3iIOS::startScan( m3i_result_t * conf) {
    if (self)
        [(id)self startScan:conf];
}

bool M3iIOS::isScanning() const {
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

- (instancetype)initWithObj:(void *) obj {
    self = [super init];
    if (self) {
        objref = obj;
        conf = 0;
        devUid = 0;
        startRequested = NO;
        cbCentralManager = [[CBCentralManager alloc] initWithDelegate:self queue: dispatch_get_main_queue()];
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

- (void)stopScan {
    if ([cbCentralManager isScanning])
        [cbCentralManager stopScan];
}

- (void)startScan:(m3i_result_t *) config {
    conf = config;
    if (cbCentralManager.state == CBManagerStatePoweredOn) {
        NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], CBCentralManagerScanOptionAllowDuplicatesKey, nil];
        [ cbCentralManager scanForPeripheralsWithServices:nil options:options ];
        startRequested = NO;
    }
}

-(void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary<NSString *,id> *)advertisementData RSSI:(NSNumber *)RSSI {
    NSString * name = [peripheral name];
    if (name == 0) name = @"N/A";
    NSString * uuidstring = [peripheral.identifier UUIDString];
    NSString * logString = [NSString stringWithFormat:@"Received %@ (%@)[%d]", uuidstring, name, (int)[RSSI integerValue]];
    NSLog(@"%@", logString);
    [logString getCString:conf->uuid maxLength:sizeof(logs)/sizeof(*logs)-1 encoding:NSUTF8StringEncoding];
    qt_log(logString);
    if (devUid != 0) {
        NSData * data = [advertisementData objectForKey:@"kCBAdvDataManufacturerData"];
        if (data) {
            NSUInteger dataLength = [data length];
            const unsigned char *arr = (const unsigned char *)[data bytes];
            if (arr && dataLength>=10) {
                unsigned int index = 0;
                if (arr[index] == 2 && arr[index + 1] == 1)
                    index += 2;
                unsigned char mayor = arr[index];
                index += 1;
                unsigned char minor = arr[index];
                index += 1;
                if (mayor == conf->major && minor == conf->minor && dataLength > index + 13) {
                    unsigned char dt = arr[index];
                    if ((dt == 0 || dt >= 128 || dt <= 227) && conf->idval == arr[index+1]) {
                        devUid = peripheral.identifier;
                        [uuidstring getCString:conf->uuid maxLength:sizeof(conf->uuid)/sizeof(*conf->uuid)-1 encoding:NSUTF8StringEncoding];
                    }
                }
            }
        }
    }
    if (devUid != 0) {
        if ([devUid isEqual:peripheral.identifier]) {
            NSData * data = [advertisementData objectForKey:@"kCBAdvDataManufacturerData"];
            if (data) {
                NSUInteger dataLength = [data length];
                const unsigned char *arr = (const unsigned char *)[data bytes];
                if (arr && dataLength>=10) {
                    conf->rssi = (int)[RSSI integerValue];
                    conf->nbytes = dataLength > sizeof(conf->nbytes)?sizeof(conf->nbytes): dataLength;
                    memcpy(conf->bytes, arr, conf->nbytes);
                    [name getCString:conf->name maxLength:sizeof(conf->name)/sizeof(*conf->name)-1 encoding:NSUTF8StringEncoding];
                    m3i_callback(objref, conf);
                }
            }
        }
    }
}

@end

#import "ios_echelonconnectsport.h"

@implementation ios_echelonconnectsport

- (instancetype)init:(NSString *)deviceName qtDevice:(void *)qtDevice {
    self = [super init];
    if (self) {
        (void)deviceName;
        (void)qtDevice;
    }
    return self;
}

- (void)writeCharacteristc:(unsigned char *)qdata length:(unsigned char)length {
    (void)qdata;
    (void)length;
}

@end

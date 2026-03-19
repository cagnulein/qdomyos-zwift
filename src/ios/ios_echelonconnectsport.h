#ifndef IOS_ECHELONCONNECTSPORT_H
#define IOS_ECHELONCONNECTSPORT_H

#import <Foundation/Foundation.h>

@interface ios_echelonconnectsport : NSObject

- (nonnull instancetype)init:(nonnull NSString *)deviceName qtDevice:(void *_Nullable)qtDevice;
- (void)writeCharacteristc:(unsigned char *_Nonnull)qdata length:(unsigned char)length;

@end

#endif

//
//  AdbClient.h
//  adb-ios
//
//  Created by Li Zonghai on 9/28/15.
//  Copyright © 2015 Li Zonghai. All rights reserved.
//

#import <Foundation/Foundation.h>


typedef void (^ResponseBlock)(BOOL succ, NSString *result);


typedef NSUInteger ADBInstallFlag;
NS_ENUM(ADBInstallFlag) {
    
    ADBInstallFlag_Sdcard = 0x1,
    ADBInstallFlag_GrantAllRuntimePermission = 0x2,
    ADBInstallFlag_Replace = 0x4,
    
};


@interface AdbClient : NSObject


-(id)init;
-(id)initWithVerbose:(BOOL)flg;

-(void) devices:(ResponseBlock)block;

-(void) connect: (NSString *)addr didResponse:(ResponseBlock)block;
-(void) disconnect: (NSString *)addr didResponse:(ResponseBlock)block;

-(void) installApk: (NSString *)apkPath flags:(ADBInstallFlag)flags didResponse:(ResponseBlock)block;
-(void) uninstallApk: (NSString *)packageName didResponse:(ResponseBlock)block;

-(void) shell: (NSString *)cmd didResponse:(ResponseBlock)block;

@end

//
//  AdbClient.m
//  adb-ios
//
//  Created by Li Zonghai on 9/28/15.
//  Copyright © 2015 Li Zonghai. All rights reserved.
//

#import "AdbClient.h"
#import "../../swiftDebugBridge.h"

#include <stdio.h>
#include "adb.h"
#include "adb_client.h"
#include "sysdeps.h"


#define BUF_SIZE 4096


extern const char* get_basename(const char* filename);
extern int do_sync_push(const char *lpath, const char *rpath, int verifyApk);


@interface AdbClient ()

@property(strong) dispatch_queue_t queue;

-(void) read:(int)fd toResponse:(ResponseBlock)block;
-(void) shell:(NSString *) cmd toResponse:(ResponseBlock)block;
-(void) pm:(NSString *)cmd toResponse:(ResponseBlock)block;
-(void) logMessage:(NSString *)message;
-(NSString *)stringFromCString:(const char *)cString;
@end


@implementation AdbClient
@synthesize queue = _queue;


-(id) init
{
    if ((self = [super init]))
    {
        adb_sysdeps_init();
        _queue = dispatch_queue_create("adb-queue", DISPATCH_QUEUE_SERIAL);
        [self logMessage:@"ADBClient initialized"];
        
        NSString *path = [[NSBundle mainBundle] bundlePath];
        if (setenv("HOME", path.UTF8String, YES) != 0)
        {
            [self logMessage:@"WARN! can't set HOME directory. adb may not find adbkey files"];
        }
        else
        {
            [self logMessage:[NSString stringWithFormat:@"ADB HOME set to %@", path]];
        }
        
    }
    
    return self;
}


-(id)initWithVerbose:(BOOL)flg
{
    if ((self = [self init]))
    {
        if (flg) {
            adb_trace_init("all");
            [self logMessage:@"ADB verbose tracing enabled"];
        }
    }
    return self;
}


-(void) devices:(ResponseBlock)block
{
    dispatch_async(_queue, ^{
        
        char *buf = "host:devices-l" ;
        char *tmp;
        
        [self logMessage:@"ADB devices command issued"];
        tmp = adb_query(buf);
        if (tmp)
        {
            if (block)
            {
                NSString *result = [NSString stringWithFormat:@"List of devices attached\n%s", tmp];
                [self logMessage:result];
                block(YES, result);
            }
            free(tmp);
        }
        else
        {
            const char *error = adb_error();
            NSString *errorMessage = [self stringFromCString:error];
            [self logMessage:errorMessage];
            if(block)
                block(NO, errorMessage);
        }
        
    });
}


-(void) connect:(NSString *)addr didResponse:(ResponseBlock)block
{
    
    dispatch_async(_queue, ^{

        char buf[256];
        char *tmp;
        
        snprintf(buf, sizeof buf, "host:connect:%s", addr.UTF8String);
        [self logMessage:[NSString stringWithFormat:@"ADB connect request -> %s", buf]];
        tmp = adb_query(buf);
        if (tmp)
        {
            if (block)
            {
                NSString *result = [NSString stringWithUTF8String:tmp];
                [self logMessage:[NSString stringWithFormat:@"ADB connect response: %@", result]];
                if ([result rangeOfString:@"connected"].location != NSNotFound)
                    block(YES, result);
                else
                    block(NO, result);
            }
            free(tmp);
        }
        else
        {
            const char *error = adb_error();
            NSString *errorMessage = [self stringFromCString:error];
            [self logMessage:errorMessage];
            if(block)
                block(NO, errorMessage);
        }
       
    });
    
    
}


-(void) disconnect:(NSString *)addr didResponse:(ResponseBlock)block
{
    dispatch_async(_queue, ^{
       
        char buf[256];
        char *tmp;
        
        if (addr)
            snprintf(buf, sizeof buf, "host:disconnect:%s", addr.UTF8String);
        else
            snprintf(buf, sizeof buf, "host:disconnect:");

        [self logMessage:[NSString stringWithFormat:@"ADB disconnect request -> %s", buf]];
        tmp = adb_query(buf);
        if (tmp)
        {
            NSString *response = [NSString stringWithUTF8String:tmp];
            [self logMessage:[NSString stringWithFormat:@"ADB disconnect response: %@", response]];
            if (block)
                block(YES, response);
            free(tmp);
        }
        else
        {
            const char *error = adb_error();
            NSString *errorMessage = [self stringFromCString:error];
            [self logMessage:errorMessage];
            if(block) block(NO, errorMessage);
        }
        

    });
}


-(void) installApk:(NSString *)apkPath flags:(ADBInstallFlag)flags didResponse:(ResponseBlock)block
{
    dispatch_async(_queue, ^{

        static const char *const DATA_DEST = "/data/local/tmp/%s";
        static const char *const SD_DEST = "/sdcard/tmp/%s";
        const char* where = DATA_DEST;
        char apk_dest[PATH_MAX];
        const char* apk_file;
        int err;
        
        if (flags & ADBInstallFlag_Sdcard)
        {
            where = SD_DEST;
        }
        
        if ([apkPath length] == 0)
        {
            NSString *message = @"can't find filename in arguments";
            [self logMessage:message];
            if (block) block(NO, message);
            return;
        }
        
        apk_file = apkPath.UTF8String;
        snprintf(apk_dest, sizeof apk_dest, where, get_basename(apk_file));
        [self logMessage:[NSString stringWithFormat:@"ADB install push %@ -> %s", apkPath, apk_dest]];
        err = do_sync_push(apk_file, apk_dest, 1 /* verify APK */);
        if (err) {
           [self logMessage:@"ADB install: push failed"];
           if(block) block(NO, nil);
            return;
        }

        NSString *installCmd = [NSString stringWithFormat:@"shell:pm install"];
        if (flags & ADBInstallFlag_Replace)
            installCmd = [installCmd stringByAppendingString:@" -r"];
        
        if (flags & ADBInstallFlag_Sdcard)
            installCmd = [installCmd stringByAppendingString:@" -s"];
        
        if (flags & ADBInstallFlag_GrantAllRuntimePermission)
            installCmd = [installCmd stringByAppendingString:@" -g"];
        
        installCmd = [installCmd stringByAppendingFormat:@" %s", apk_dest];

        int fd = adb_connect(installCmd.UTF8String);
        if(fd >= 0)
        {
            [self logMessage:[NSString stringWithFormat:@"ADB install command sent: %@", installCmd]];
            char buf[BUF_SIZE];
            int len;
            
            while(1) {
                
                len = adb_read(fd, buf, BUF_SIZE);
                if(len == 0) {
                    break;
                }
                
                if(len < 0) {
                    if(errno == EINTR) continue;
                    break;
                }
                
                if (block)
                {
                    *(buf + len) = '\0';
                    NSString *message = [self stringFromCString:buf];
                    [self logMessage:message];
                    if (*buf == 'S')
                    {
                        block(YES, message);
                    }
                    else if (*buf == 'F')
                    {
                        block(NO, message);
                    }
                }
            }

            adb_close(fd);
        }
        else
        {
            const char *error = adb_error();
            NSString *errorMessage = [self stringFromCString:error];
            [self logMessage:errorMessage];
            if(block)
            {
                block(NO, errorMessage);
            }
        }
 
        [self shell:[NSString stringWithFormat:@"rm %s", apk_dest] toResponse:nil];
        
    });
}


-(void) uninstallApk:(NSString *)packageName didResponse:(ResponseBlock)block
{
    dispatch_async(_queue, ^{
        [self pm:[NSString stringWithFormat:@"uninstall %@", packageName] toResponse:block];
    });
}


-(void) shell:(NSString *)cmd didResponse:(ResponseBlock)block
{
    dispatch_async(_queue, ^{
        
        if ([cmd length] != 0)
            [self shell:cmd toResponse:block];
        else if (block)
        {
            NSString *message = @"must have command";
            [self logMessage:message];
            block(NO, message);
        }
    });
}


-(void) pm:(NSString *)cmd toResponse:(ResponseBlock)block
{
    NSString *tmp = [NSString stringWithFormat:@"pm %@", cmd];
    [self shell:tmp toResponse:block];
}


-(void) shell:(NSString *)cmd toResponse:(ResponseBlock)block
{
    
    int fd = adb_connect([NSString stringWithFormat:@"shell:%@", cmd].UTF8String);
    if(fd >= 0)
    {
        [self logMessage:[NSString stringWithFormat:@"ADB shell command opened: %@", cmd]];
        [self read:fd toResponse:block];
        adb_close(fd);
    }
    else
        
    {
        const char *error = adb_error();
        NSString *errorMessage = [self stringFromCString:error];
        [self logMessage:errorMessage];
        if(block) block(NO, errorMessage);
    }

}


-(void) read:(int)fd toResponse:(ResponseBlock)block
{
    char buf[BUF_SIZE];
    int len;
    
    while(fd >= 0)
    {
        len = adb_read(fd, buf, BUF_SIZE);
        if(len == 0)
            break;
     
        if(len < 0)
        {
            if(errno == EINTR) continue;
            break;
        }
        
        if (block)
        {
            *(buf + len) = '\0';
            NSString *message = [self stringFromCString:buf];
            [self logMessage:message];
            block(YES, message);
        }
    }

}

-(void) logMessage:(NSString *)message
{
    if (message.length == 0)
    {
        return;
    }
    SwiftDebugLogNSString(message);
}

-(NSString *)stringFromCString:(const char *)cString
{
    if (!cString)
    {
        return nil;
    }
    NSString *message = [[NSString alloc] initWithCString:cString encoding:NSUTF8StringEncoding];
    if (!message)
    {
        message = [NSString stringWithFormat:@"(invalid UTF8) %s", cString];
    }
    return message;
}

@end

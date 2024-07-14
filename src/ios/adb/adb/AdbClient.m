//
//  AdbClient.m
//  adb-ios
//
//  Created by Li Zonghai on 9/28/15.
//  Copyright © 2015 Li Zonghai. All rights reserved.
//

#import "AdbClient.h"

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
@end


@implementation AdbClient
@synthesize queue = _queue;


-(id) init
{
    if ((self = [super init]))
    {
        adb_sysdeps_init();
        _queue = dispatch_queue_create("adb-queue", DISPATCH_QUEUE_SERIAL);
        
        NSString *path = [[NSBundle mainBundle] bundlePath];
        if (setenv("HOME", path.UTF8String, YES) != 0)
        {
            NSLog(@"WARN! can't set HOME directory. adb may not find adbkey files");
        }
        
    }
    
    return self;
}


-(id)initWithVerbose:(BOOL)flg
{
    if ((self = [self init]))
    {
        if (flg) adb_trace_init("all");
    }
    return self;
}


-(void) devices:(ResponseBlock)block
{
    dispatch_async(_queue, ^{
        
        char *buf = "host:devices-l" ;
        char *tmp;
        
        tmp = adb_query(buf);
        if (tmp)
        {
            if (block)
            {
                NSString *result = [NSString stringWithFormat:@"List of devices attached\n%s", tmp];
                block(YES, result);
            }
            free(tmp);
        }
        else
        {
            if(block)
                block(NO, [NSString stringWithUTF8String:adb_error()]);
        }
        
    });
}


-(void) connect:(NSString *)addr didResponse:(ResponseBlock)block
{
    
    dispatch_async(_queue, ^{

        char buf[256];
        char *tmp;
        
        snprintf(buf, sizeof buf, "host:connect:%s", addr.UTF8String);
        tmp = adb_query(buf);
        if (tmp)
        {
            if (block)
            {
                NSString *result = [NSString stringWithUTF8String:tmp];
                if ([result rangeOfString:@"connected"].location != NSNotFound)
                    block(YES, result);
                else
                    block(NO, result);
            }
            free(tmp);
        }
        else
        {
            if(block)
                block(NO, [NSString stringWithUTF8String:adb_error()]);
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

        tmp = adb_query(buf);
        if (tmp)
        {
            if (block)
                block(YES, [NSString stringWithUTF8String:tmp]);
            free(tmp);
        }
        else
        {
            if(block) block(NO, [NSString stringWithUTF8String:adb_error()]);
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
            if (block) block(NO, @"can't find filename in arguments");
            return;
        }
        
        apk_file = apkPath.UTF8String;
        snprintf(apk_dest, sizeof apk_dest, where, get_basename(apk_file));
        err = do_sync_push(apk_file, apk_dest, 1 /* verify APK */);
        if (err) {
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
                    if (*buf == 'S')
                        block(YES, [NSString stringWithUTF8String:buf]);
                    else if (*buf == 'F')
                        block(NO, [NSString stringWithUTF8String:buf]);
                }
            }

            adb_close(fd);
        }
        else
        {
            if(block)
                block(NO, [NSString stringWithCString:adb_error() encoding:NSUTF8StringEncoding]);
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
            block(NO, @"must have command");
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
        [self read:fd toResponse:block];
        adb_close(fd);
    }
    else
        
    {
        if(block) block(NO, [NSString stringWithUTF8String:adb_error()]);
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
            block(YES, [NSString stringWithUTF8String:buf]);
        }
    }

}

@end

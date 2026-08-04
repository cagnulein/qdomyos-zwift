#ifndef IO_UNDER_QT
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <QDebug>

#include "ios/ios_background_keepalive.h"

namespace {

// 1 second of 8 kHz mono 16 bit silence, wrapped in a minimal WAV container.
NSData *QZSilentWavData() {
    const uint32_t sampleRate = 8000;
    const uint16_t channels = 1;
    const uint16_t bitsPerSample = 16;
    const uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    const uint16_t blockAlign = channels * (bitsPerSample / 8);
    const uint32_t dataSize = byteRate; // one second
    const uint32_t riffSize = 36 + dataSize;
    const uint32_t fmtSize = 16;
    const uint16_t audioFormat = 1; // PCM

    NSMutableData *wav = [NSMutableData dataWithCapacity:44 + dataSize];
    [wav appendBytes:"RIFF" length:4];
    [wav appendBytes:&riffSize length:4];
    [wav appendBytes:"WAVE" length:4];
    [wav appendBytes:"fmt " length:4];
    [wav appendBytes:&fmtSize length:4];
    [wav appendBytes:&audioFormat length:2];
    [wav appendBytes:&channels length:2];
    [wav appendBytes:&sampleRate length:4];
    [wav appendBytes:&byteRate length:4];
    [wav appendBytes:&blockAlign length:2];
    [wav appendBytes:&bitsPerSample length:2];
    [wav appendBytes:"data" length:4];
    [wav appendBytes:&dataSize length:4];
    [wav increaseLengthBy:dataSize]; // zero filled == silence

    return wav;
}

} // namespace

@interface QZBackgroundKeepAlive : NSObject <AVAudioPlayerDelegate>
@property(nonatomic, strong) AVAudioPlayer *player;
@property(nonatomic, strong) NSTimer *watchdog;
@property(nonatomic, assign) BOOL running;
+ (instancetype)shared;
- (void)start;
- (void)stop;
@end

@implementation QZBackgroundKeepAlive

+ (instancetype)shared {
    static QZBackgroundKeepAlive *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[QZBackgroundKeepAlive alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(handleInterruption:)
                                                     name:AVAudioSessionInterruptionNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(handleMediaServicesReset:)
                                                     name:AVAudioSessionMediaServicesWereResetNotification
                                                   object:nil];
    }
    return self;
}

- (BOOL)activateSession {
    NSError *error = nil;
    AVAudioSession *session = [AVAudioSession sharedInstance];

    // MixWithOthers: we must never stop the music/podcast the user is listening
    // to while training.  Playback is the only category that keeps running when
    // the screen is locked.
    if (![session setCategory:AVAudioSessionCategoryPlayback
                  withOptions:AVAudioSessionCategoryOptionMixWithOthers
                        error:&error]) {
        qDebug() << "QZ iOS keepalive: setCategory failed"
                 << QString::fromUtf8(error.localizedDescription.UTF8String);
        return NO;
    }

    if (![session setActive:YES error:&error]) {
        qDebug() << "QZ iOS keepalive: setActive failed"
                 << QString::fromUtf8(error.localizedDescription.UTF8String);
        return NO;
    }

    return YES;
}

- (void)start {
    if (self.running) {
        return;
    }

    if (![self activateSession]) {
        return;
    }

    NSError *error = nil;
    self.player = [[AVAudioPlayer alloc] initWithData:QZSilentWavData() error:&error];
    if (self.player == nil) {
        qDebug() << "QZ iOS keepalive: player creation failed"
                 << QString::fromUtf8(error.localizedDescription.UTF8String);
        return;
    }

    self.player.delegate = self;
    self.player.numberOfLoops = -1; // forever
    self.player.volume = 1.0;       // the samples are silence anyway
    [self.player prepareToPlay];

    if (![self.player play]) {
        qDebug() << "QZ iOS keepalive: play failed";
        self.player = nil;
        return;
    }

    self.running = YES;

    // iOS can stop the playback behind our back (interruptions, route changes,
    // media services restart).  If that happens while we are in background the
    // app gets suspended a few seconds later, so keep an eye on it.
    self.watchdog = [NSTimer scheduledTimerWithTimeInterval:10.0
                                                     target:self
                                                   selector:@selector(checkPlayback)
                                                   userInfo:nil
                                                    repeats:YES];

    qDebug() << "QZ iOS keepalive: started";
}

- (void)stop {
    if (!self.running) {
        return;
    }

    self.running = NO;

    [self.watchdog invalidate];
    self.watchdog = nil;

    [self.player stop];
    self.player = nil;

    NSError *error = nil;
    [[AVAudioSession sharedInstance] setActive:NO
                   withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation
                         error:&error];

    qDebug() << "QZ iOS keepalive: stopped";
}

- (void)checkPlayback {
    if (!self.running || self.player.isPlaying) {
        return;
    }

    qDebug() << "QZ iOS keepalive: playback stopped unexpectedly, restarting";
    if ([self activateSession]) {
        [self.player play];
    }
}

- (void)handleInterruption:(NSNotification *)notification {
    if (!self.running) {
        return;
    }

    NSNumber *type = notification.userInfo[AVAudioSessionInterruptionTypeKey];
    if (type.unsignedIntegerValue == AVAudioSessionInterruptionTypeEnded) {
        qDebug() << "QZ iOS keepalive: interruption ended, resuming";
        if ([self activateSession]) {
            [self.player play];
        }
    } else {
        qDebug() << "QZ iOS keepalive: interruption began";
    }
}

- (void)handleMediaServicesReset:(NSNotification *)notification {
    Q_UNUSED(notification)
    if (!self.running) {
        return;
    }

    qDebug() << "QZ iOS keepalive: media services were reset, rebuilding the player";
    self.running = NO;
    [self.watchdog invalidate];
    self.watchdog = nil;
    self.player = nil;
    [self start];
}

- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer *)player error:(NSError *)error {
    Q_UNUSED(player)
    qDebug() << "QZ iOS keepalive: decode error"
             << (error ? QString::fromUtf8(error.localizedDescription.UTF8String) : QStringLiteral("(null)"));
}

@end

void iosBackgroundKeepAlive::start() { [[QZBackgroundKeepAlive shared] start]; }

void iosBackgroundKeepAlive::stop() { [[QZBackgroundKeepAlive shared] stop]; }

bool iosBackgroundKeepAlive::isRunning() { return [QZBackgroundKeepAlive shared].running == YES; }

#endif

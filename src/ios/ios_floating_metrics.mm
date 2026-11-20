#ifndef IO_UNDER_QT
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>
#import <UIKit/UIKit.h>
#include <QDebug>
#include "ios/ios_floating_metrics.h"

// All AVKit and UIKit operations are performed on the main queue.
API_AVAILABLE(ios(15.0))
@interface QZFloatingMetrics : NSObject <AVPictureInPictureControllerDelegate, AVPictureInPictureSampleBufferPlaybackDelegate>
@property(nonatomic, strong) AVSampleBufferDisplayLayer *layer;
@property(nonatomic, strong) AVPictureInPictureController *controller;
@property(nonatomic, strong) UIView *preview;
@property(nonatomic, copy) NSArray<NSArray<NSString *> *> *metrics;
@property(nonatomic, copy) NSString *lastFrame;
@property(nonatomic, copy) NSString *emptyLabel;
@property(nonatomic) BOOL pending;
@property(nonatomic) BOOL observing;
+ (instancetype)shared;
- (void)start;
- (void)render;
- (void)finish;
@end

@implementation QZFloatingMetrics
+ (instancetype)shared {
    static QZFloatingMetrics *instance;
    static dispatch_once_t once;
    dispatch_once(&once, ^{ instance = [QZFloatingMetrics new]; });
    return instance;
}
- (void)start {
    qDebug() << "Floating metrics: native start, controller exists" << (self.controller != nil)
             << "supported" << [AVPictureInPictureController isPictureInPictureSupported]
             << "application state" << (int)UIApplication.sharedApplication.applicationState;
    if (self.controller || ![AVPictureInPictureController isPictureInPictureSupported]) return;
    UIWindow *window = nil;
    for (UIWindow *candidate in UIApplication.sharedApplication.windows) {
        if (candidate.isKeyWindow) { window = candidate; break; }
    }
    if (!window) { qWarning() << "Floating metrics: no key window, count" << (int)UIApplication.sharedApplication.windows.count; return; }
    qDebug() << "Floating metrics: key window found" << window.bounds.size.width << window.bounds.size.height;
    NSError *error = nil;
    AVAudioSession *audio = AVAudioSession.sharedInstance;
    if (![audio setCategory:AVAudioSessionCategoryPlayback withOptions:AVAudioSessionCategoryOptionMixWithOthers error:&error] ||
        ![audio setActive:YES error:&error]) {
        qWarning() << "Floating metrics: audio session failed" << error.localizedDescription.UTF8String;
        return;
    }
    qDebug() << "Floating metrics: audio session activated; metric count" << (int)self.metrics.count;
    self.preview = [[UIView alloc] initWithFrame:CGRectMake(8, window.bounds.size.height - 100, 160, 90)];
    self.preview.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleRightMargin;
    self.preview.userInteractionEnabled = NO;
    [window addSubview:self.preview];
    self.layer = [AVSampleBufferDisplayLayer new];
    self.layer.frame = self.preview.bounds;
    self.layer.videoGravity = AVLayerVideoGravityResizeAspect;
    [self.preview.layer addSublayer:self.layer];
    AVPictureInPictureControllerContentSource *source = [[AVPictureInPictureControllerContentSource alloc]
        initWithSampleBufferDisplayLayer:self.layer playbackDelegate:self];
    self.controller = [[AVPictureInPictureController alloc] initWithContentSource:source];
    if (!self.controller) { qWarning() << "Floating metrics: controller creation failed"; [self finish]; return; }
    self.controller.delegate = self;
    self.controller.requiresLinearPlayback = YES;
    self.controller.canStartPictureInPictureAutomaticallyFromInline = YES;
    self.pending = YES;
    [self render];
    [self.controller addObserver:self forKeyPath:@"pictureInPicturePossible" options:NSKeyValueObservingOptionNew context:NULL];
    self.observing = YES;
    qDebug() << "Floating metrics: controller ready; PiP possible" << self.controller.isPictureInPicturePossible << "layer status" << (int)self.layer.status;
    if (self.controller.isPictureInPicturePossible) { qDebug() << "Floating metrics: requesting PiP immediately"; self.pending = NO; [self.controller startPictureInPicture]; }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 8 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        if (self.pending) { qWarning() << "Floating metrics: PiP unavailable after timeout; layer status" << (int)self.layer.status << "error" << (self.layer.error ? self.layer.error.localizedDescription.UTF8String : "none"); [self finish]; }
    });
}
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    Q_UNUSED(change); Q_UNUSED(context);
    if (object == self.controller && [keyPath isEqualToString:@"pictureInPicturePossible"])
        qDebug() << "Floating metrics: PiP possible changed" << self.controller.isPictureInPicturePossible << "pending" << self.pending;
    if (object == self.controller && [keyPath isEqualToString:@"pictureInPicturePossible"] && self.pending && self.controller.isPictureInPicturePossible) {
        qDebug() << "Floating metrics: requesting PiP after availability change";
        self.pending = NO;
        [self.controller startPictureInPicture];
    }
}
- (void)render {
    if (!self.layer) return;
    NSArray<NSArray<NSString *> *> *all = self.metrics ?: @[];
    NSUInteger pages = MAX((NSUInteger)1, (all.count + 5) / 6);
    NSUInteger page = ((NSUInteger)(NSDate.date.timeIntervalSince1970 / 6)) % pages;
    NSRange range = NSMakeRange(MIN(page * 6, all.count), MIN((NSUInteger)6, all.count - MIN(page * 6, all.count)));
    NSArray<NSArray<NSString *> *> *shown = [all subarrayWithRange:range];
    NSString *signature = [NSString stringWithFormat:@"%@/%lu/%lu/%@", shown, (unsigned long)page, (unsigned long)pages, self.emptyLabel ?: @""];
    if ([signature isEqualToString:self.lastFrame] && self.layer.status != AVQueuedSampleBufferRenderingStatusFailed) return;
    CVPixelBufferRef pixel = NULL;
    NSDictionary *attrs = @{(id)kCVPixelBufferCGImageCompatibilityKey:@YES, (id)kCVPixelBufferCGBitmapContextCompatibilityKey:@YES};
    if (CVPixelBufferCreate(kCFAllocatorDefault, 480, 270, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef)attrs, &pixel) != kCVReturnSuccess) return;
    CVPixelBufferLockBaseAddress(pixel, 0);
    CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(CVPixelBufferGetBaseAddress(pixel), 480, 270, 8, CVPixelBufferGetBytesPerRow(pixel), color, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    CGColorSpaceRelease(color);
    if (!ctx) { CVPixelBufferUnlockBaseAddress(pixel, 0); CVPixelBufferRelease(pixel); return; }
    CGContextTranslateCTM(ctx, 0, 270); CGContextScaleCTM(ctx, 1, -1);
    UIGraphicsPushContext(ctx);
    [[UIColor colorWithWhite:0.06 alpha:1] setFill]; UIRectFill(CGRectMake(0, 0, 480, 270));
    NSMutableParagraphStyle *style = [NSMutableParagraphStyle new]; style.alignment = NSTextAlignmentCenter; style.lineBreakMode = NSLineBreakByTruncatingTail;
    NSDictionary *label = @{NSFontAttributeName:[UIFont systemFontOfSize:18 weight:UIFontWeightMedium], NSForegroundColorAttributeName:UIColor.lightGrayColor, NSParagraphStyleAttributeName:style};
    NSDictionary *value = @{NSFontAttributeName:[UIFont monospacedDigitSystemFontOfSize:35 weight:UIFontWeightBold], NSForegroundColorAttributeName:UIColor.whiteColor, NSParagraphStyleAttributeName:style};
    for (NSUInteger i = 0; i < shown.count; ++i) {
        CGFloat x = (i % 2) * 240, y = (i / 2) * 87;
        [shown[i][0] drawInRect:CGRectMake(x + 5, y + 4, 230, 24) withAttributes:label];
        [shown[i][1] drawInRect:CGRectMake(x + 5, y + 27, 230, 48) withAttributes:value];
    }
    if (shown.count == 0) [(self.emptyLabel ?: @"") drawInRect:CGRectMake(0, 110, 480, 30) withAttributes:label];
    if (pages > 1) [[NSString stringWithFormat:@"%lu / %lu", (unsigned long)page + 1, (unsigned long)pages] drawInRect:CGRectMake(0, 250, 480, 20) withAttributes:label];
    UIGraphicsPopContext(); CGContextRelease(ctx);
    CVPixelBufferUnlockBaseAddress(pixel, 0);
    CMVideoFormatDescriptionRef format = NULL; CMSampleBufferRef sample = NULL;
    if (CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, pixel, &format) == noErr) {
        CMSampleTimingInfo timing = {kCMTimeInvalid, CMClockGetTime(CMClockGetHostTimeClock()), kCMTimeInvalid};
        if (CMSampleBufferCreateReadyWithImageBuffer(kCFAllocatorDefault, pixel, format, &timing, &sample) == noErr) {
            CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sample, true);
            if (attachments && CFArrayGetCount(attachments)) CFDictionarySetValue((CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 0), kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);
            if (self.layer.status == AVQueuedSampleBufferRenderingStatusFailed) [self.layer flush];
            if (self.layer.readyForMoreMediaData) { [self.layer enqueueSampleBuffer:sample]; self.lastFrame = signature; }
            CFRelease(sample);
        }
        CFRelease(format);
    }
    CVPixelBufferRelease(pixel);
}
- (void)finish {
    qDebug() << "Floating metrics: cleanup";
    self.pending = NO;
    if (self.observing) { [self.controller removeObserver:self forKeyPath:@"pictureInPicturePossible"]; self.observing = NO; }
    [self.preview removeFromSuperview]; self.preview = nil; self.layer = nil; self.controller = nil; self.lastFrame = nil;
}
- (void)pictureInPictureControllerDidStartPictureInPicture:(AVPictureInPictureController *)controller { Q_UNUSED(controller); qDebug() << "Floating metrics: PiP started"; self.preview.alpha = 0.01; }
- (void)pictureInPictureController:(AVPictureInPictureController *)controller restoreUserInterfaceForPictureInPictureStopWithCompletionHandler:(void (^)(BOOL))completion { Q_UNUSED(controller); completion(YES); }
- (void)pictureInPictureControllerDidStopPictureInPicture:(AVPictureInPictureController *)controller { Q_UNUSED(controller); qDebug() << "Floating metrics: PiP stopped"; [self finish]; }
- (void)pictureInPictureController:(AVPictureInPictureController *)controller failedToStartPictureInPictureWithError:(NSError *)error { Q_UNUSED(controller); qWarning() << "Floating metrics: PiP start failed" << error.localizedDescription.UTF8String; [self finish]; }
- (void)pictureInPictureController:(AVPictureInPictureController *)controller setPlaying:(BOOL)playing { Q_UNUSED(controller); Q_UNUSED(playing); }
- (CMTimeRange)pictureInPictureControllerTimeRangeForPlayback:(AVPictureInPictureController *)controller { Q_UNUSED(controller); return CMTimeRangeMake(kCMTimeZero, kCMTimePositiveInfinity); }
- (BOOL)pictureInPictureControllerIsPlaybackPaused:(AVPictureInPictureController *)controller { Q_UNUSED(controller); return NO; }
- (void)pictureInPictureController:(AVPictureInPictureController *)controller didTransitionToRenderSize:(CMVideoDimensions)size { Q_UNUSED(controller); Q_UNUSED(size); }
- (void)pictureInPictureController:(AVPictureInPictureController *)controller skipByInterval:(CMTime)interval completionHandler:(void (^)(void))completion { Q_UNUSED(controller); Q_UNUSED(interval); completion(); }
@end

bool iosFloatingMetrics::supported() {
    if (@available(iOS 15.0, *)) return [AVPictureInPictureController isPictureInPictureSupported];
    return false;
}
void iosFloatingMetrics::start() { if (@available(iOS 15.0, *)) dispatch_async(dispatch_get_main_queue(), ^{ [[QZFloatingMetrics shared] start]; }); }
void iosFloatingMetrics::stop() { if (@available(iOS 15.0, *)) dispatch_async(dispatch_get_main_queue(), ^{ QZFloatingMetrics *pip = [QZFloatingMetrics shared]; if (pip.controller.isPictureInPictureActive) [pip.controller stopPictureInPicture]; else [pip finish]; }); }
bool iosFloatingMetrics::running() { if (@available(iOS 15.0, *)) return [QZFloatingMetrics shared].controller != nil; return false; }
void iosFloatingMetrics::update(const QList<QPair<QString, QString>> &metrics, const QString &emptyLabel) {
    if (@available(iOS 15.0, *)) {
        NSMutableArray *items = [NSMutableArray arrayWithCapacity:metrics.size()];
        for (const auto &metric : metrics) [items addObject:@[[NSString stringWithUTF8String:metric.first.toUtf8().constData()], [NSString stringWithUTF8String:metric.second.toUtf8().constData()]]];
        NSString *emptyText = [NSString stringWithUTF8String:emptyLabel.toUtf8().constData()];
        dispatch_async(dispatch_get_main_queue(), ^{ [QZFloatingMetrics shared].metrics = items; [QZFloatingMetrics shared].emptyLabel = emptyText; [[QZFloatingMetrics shared] render]; });
    }
}
#endif

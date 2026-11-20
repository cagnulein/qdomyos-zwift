#ifndef IO_UNDER_QT
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>
#import <UIKit/UIKit.h>
#include <QDebug>
#include "ios/ios_floating_metrics.h"

// Draw the complete text on one line, scaling it to fit rather than truncating it.
static void QZDrawFittedText(NSString *text, CGRect rect, UIFont *font, UIColor *color) {
    if (!text.length || rect.size.width <= 0 || rect.size.height <= 0) return;
    NSString *singleLine = [[text componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]] componentsJoinedByString:@" "];
    NSDictionary *attributes = @{NSFontAttributeName:font, NSForegroundColorAttributeName:color};
    CGSize size = [singleLine sizeWithAttributes:attributes];
    if (size.width <= 0 || size.height <= 0) return;
    CGFloat scale = MIN(1.0, MIN(rect.size.width / size.width, rect.size.height / size.height));
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    CGContextTranslateCTM(ctx, rect.origin.x + (rect.size.width - size.width * scale) / 2,
                          rect.origin.y + (rect.size.height - size.height * scale) / 2);
    CGContextScaleCTM(ctx, scale, scale);
    [singleLine drawAtPoint:CGPointZero withAttributes:attributes];
    CGContextRestoreGState(ctx);
}

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
- (void)scheduleForegroundStart:(NSUInteger)attempt controller:(AVPictureInPictureController *)controller;
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
    self.pending = UIDevice.currentDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad;
    [self render];
    [self.controller addObserver:self forKeyPath:@"pictureInPicturePossible" options:NSKeyValueObservingOptionNew context:NULL];
    self.observing = YES;
    qDebug() << "Floating metrics: controller ready; PiP possible" << self.controller.isPictureInPicturePossible << "layer status" << (int)self.layer.status;
    if (self.pending) {
        qDebug() << "Floating metrics: iPad foreground start scheduled";
        [self scheduleForegroundStart:0 controller:self.controller];
    } else {
        qDebug() << "Floating metrics: iPhone armed for automatic start when leaving QZ";
    }
}
- (void)scheduleForegroundStart:(NSUInteger)attempt controller:(AVPictureInPictureController *)controller {
    if (attempt >= 8) {
        if (self.controller == controller) {
            self.pending = NO;
            qWarning() << "Floating metrics: foreground PiP unavailable after 8 attempts; automatic background start remains armed";
        }
        return;
    }
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        if (self.controller != controller || controller.isPictureInPictureActive) return;
        qDebug() << "Floating metrics: foreground start check" << (int)attempt + 1
                 << "possible" << controller.isPictureInPicturePossible << "layer status" << (int)self.layer.status
                 << "ready" << self.layer.readyForMoreMediaData;
        if (controller.isPictureInPicturePossible) {
            self.pending = NO;
            qDebug() << "Floating metrics: requesting foreground PiP";
            [controller startPictureInPicture];
        } else {
            [self scheduleForegroundStart:attempt + 1 controller:controller];
        }
    });
}
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    Q_UNUSED(change); Q_UNUSED(context);
    if (object == self.controller && [keyPath isEqualToString:@"pictureInPicturePossible"])
        qDebug() << "Floating metrics: PiP possible changed" << self.controller.isPictureInPicturePossible << "pending" << self.pending;
}
- (void)render {
    if (!self.layer) return;
    NSArray<NSArray<NSString *> *> *all = self.metrics ?: @[];
    NSUInteger pages = MAX((NSUInteger)1, (all.count + 8) / 9);
    NSUInteger page = ((NSUInteger)(NSDate.date.timeIntervalSince1970 / 6)) % pages;
    NSRange range = NSMakeRange(MIN(page * 9, all.count), MIN((NSUInteger)9, all.count - MIN(page * 9, all.count)));
    NSArray<NSArray<NSString *> *> *shown = [all subarrayWithRange:range];
    NSString *signature = [NSString stringWithFormat:@"%@/%lu/%lu/%@", shown, (unsigned long)page, (unsigned long)pages, self.emptyLabel ?: @""];
    if ([signature isEqualToString:self.lastFrame] && self.layer.status != AVQueuedSampleBufferRenderingStatusFailed) return;
    CVPixelBufferRef pixel = NULL;
    NSDictionary *attrs = @{(id)kCVPixelBufferCGImageCompatibilityKey:@YES, (id)kCVPixelBufferCGBitmapContextCompatibilityKey:@YES, (id)kCVPixelBufferIOSurfacePropertiesKey:@{}};
    CVReturn pixelStatus = CVPixelBufferCreate(kCFAllocatorDefault, 480, 270, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef)attrs, &pixel);
    if (pixelStatus != kCVReturnSuccess || !pixel) { qWarning() << "Floating metrics: pixel buffer creation failed" << pixelStatus; return; }
    CVPixelBufferLockBaseAddress(pixel, 0);
    CGColorSpaceRef color = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctx = CGBitmapContextCreate(CVPixelBufferGetBaseAddress(pixel), 480, 270, 8, CVPixelBufferGetBytesPerRow(pixel), color, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    CGColorSpaceRelease(color);
    if (!ctx) { qWarning() << "Floating metrics: bitmap context creation failed"; CVPixelBufferUnlockBaseAddress(pixel, 0); CVPixelBufferRelease(pixel); return; }
    CGContextTranslateCTM(ctx, 0, 270); CGContextScaleCTM(ctx, 1, -1);
    UIGraphicsPushContext(ctx);
    [[UIColor colorWithWhite:0.06 alpha:1] setFill]; UIRectFill(CGRectMake(0, 0, 480, 270));
    UIFont *labelFont = [UIFont systemFontOfSize:17 weight:UIFontWeightMedium];
    UIFont *valueFont = [UIFont monospacedDigitSystemFontOfSize:31 weight:UIFontWeightBold];
    // Nine metrics: 3 columns and 3 rows. Reserve a separate footer for the page number.
    for (NSUInteger i = 0; i < shown.count; ++i) {
        CGFloat x = (i % 3) * 160, y = 9 + (i / 3) * 80;
        QZDrawFittedText(shown[i][0], CGRectMake(x + 7, y, 146, 22), labelFont, UIColor.lightGrayColor);
        QZDrawFittedText(shown[i][1], CGRectMake(x + 7, y + 23, 146, 45), valueFont, UIColor.whiteColor);
    }
    if (shown.count == 0) QZDrawFittedText(self.emptyLabel ?: @"", CGRectMake(10, 110, 460, 30), labelFont, UIColor.lightGrayColor);
    if (pages > 1) QZDrawFittedText([NSString stringWithFormat:@"%lu / %lu", (unsigned long)page + 1, (unsigned long)pages], CGRectMake(10, 250, 460, 17), labelFont, UIColor.lightGrayColor);
    UIGraphicsPopContext(); CGContextRelease(ctx);
    CVPixelBufferUnlockBaseAddress(pixel, 0);
    CMVideoFormatDescriptionRef format = NULL; CMSampleBufferRef sample = NULL;
    OSStatus formatStatus = CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, pixel, &format);
    if (formatStatus == noErr) {
        CMSampleTimingInfo timing = {kCMTimeInvalid, CMClockGetTime(CMClockGetHostTimeClock()), kCMTimeInvalid};
        OSStatus sampleStatus = CMSampleBufferCreateReadyWithImageBuffer(kCFAllocatorDefault, pixel, format, &timing, &sample);
        if (sampleStatus == noErr) {
            CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sample, true);
            if (attachments && CFArrayGetCount(attachments)) CFDictionarySetValue((CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 0), kCMSampleAttachmentKey_DisplayImmediately, kCFBooleanTrue);
            if (self.layer.status == AVQueuedSampleBufferRenderingStatusFailed) [self.layer flush];
            if (self.layer.readyForMoreMediaData) {
                BOOL firstFrame = self.lastFrame == nil;
                [self.layer enqueueSampleBuffer:sample]; self.lastFrame = signature;
                if (firstFrame) qDebug() << "Floating metrics: first IOSurface frame enqueued, layer status" << (int)self.layer.status;
            } else if (!self.lastFrame) qWarning() << "Floating metrics: layer not ready for first frame";
            CFRelease(sample);
        } else qWarning() << "Floating metrics: sample buffer creation failed" << sampleStatus;
        CFRelease(format);
    }
    else qWarning() << "Floating metrics: format description creation failed" << formatStatus;
    CVPixelBufferRelease(pixel);
}
- (void)finish {
    qDebug() << "Floating metrics: cleanup";
    self.pending = NO;
    if (self.observing) { [self.controller removeObserver:self forKeyPath:@"pictureInPicturePossible"]; self.observing = NO; }
    [self.preview removeFromSuperview]; self.preview = nil; self.layer = nil; self.controller = nil; self.lastFrame = nil;
}
- (void)pictureInPictureControllerWillStartPictureInPicture:(AVPictureInPictureController *)controller { Q_UNUSED(controller); qDebug() << "Floating metrics: PiP will start"; }
- (void)pictureInPictureControllerWillStopPictureInPicture:(AVPictureInPictureController *)controller { Q_UNUSED(controller); qDebug() << "Floating metrics: PiP will stop"; }
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

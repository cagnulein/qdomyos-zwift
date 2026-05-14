#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreImage/CoreImage.h>
#import <Foundation/Foundation.h>
#import <Photos/Photos.h>
#import <UIKit/UIKit.h>

#include "ios/workoutvideorecorder.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QUrl>

@class QZWorkoutVideoCaptureDelegate;

static inline id QZObjcRetain(id object) {
#if !__has_feature(objc_arc)
    return [object retain];
#else
    return object;
#endif
}

static inline void QZObjcRelease(id object) {
#if !__has_feature(objc_arc)
    [object release];
#else
    Q_UNUSED(object)
#endif
}

static char QZWorkoutVideoCaptureQueueKey;

class IOSWorkoutVideoRecorderPrivate {
  public:
    bool recording = false;
    bool paused = false;
    bool writingStarted = false;
    QStringList metrics;
    QMutex metricsMutex;
    NSURL *outputUrl = nil;
    AVCaptureSession *captureSession = nil;
    AVCaptureVideoDataOutput *videoOutput = nil;
    AVCaptureAudioDataOutput *audioOutput = nil;
    dispatch_queue_t captureQueue = nil;
    AVAssetWriter *assetWriter = nil;
    AVAssetWriterInput *videoInput = nil;
    AVAssetWriterInput *audioInput = nil;
    AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor = nil;
    CIContext *ciContext = nil;
    QZWorkoutVideoCaptureDelegate *captureDelegate = nil;

    void clearCaptureObjects() {
        if (videoOutput) {
            [videoOutput setSampleBufferDelegate:nil queue:nil];
        }
        if (audioOutput) {
            [audioOutput setSampleBufferDelegate:nil queue:nil];
        }
        if (captureQueue && dispatch_get_specific(&QZWorkoutVideoCaptureQueueKey) != &QZWorkoutVideoCaptureQueueKey) {
            dispatch_sync(captureQueue, ^{
            });
        }

        QZObjcRelease(outputUrl);
        QZObjcRelease(captureSession);
        QZObjcRelease(videoOutput);
        QZObjcRelease(audioOutput);
        QZObjcRelease(assetWriter);
        QZObjcRelease(videoInput);
        QZObjcRelease(audioInput);
        QZObjcRelease(pixelBufferAdaptor);
        QZObjcRelease(ciContext);
        QZObjcRelease(captureDelegate);

        outputUrl = nil;
        captureSession = nil;
        videoOutput = nil;
        audioOutput = nil;
        captureQueue = nil;
        assetWriter = nil;
        videoInput = nil;
        audioInput = nil;
        pixelBufferAdaptor = nil;
        ciContext = nil;
        captureDelegate = nil;
        writingStarted = false;
    }
};

static NSURL *urlFromOutputLocation(const QString &outputLocation) {
    const QUrl qurl(outputLocation);
    const QString localPath = qurl.isLocalFile() ? qurl.toLocalFile() : outputLocation;
    QDir().mkpath(QFileInfo(localPath).absolutePath());
    [[NSFileManager defaultManager] removeItemAtURL:[NSURL fileURLWithPath:localPath.toNSString()] error:nil];
    return [NSURL fileURLWithPath:localPath.toNSString()];
}

static AVCaptureDevice *cameraDevice(const QString &cameraPosition) {
    AVCaptureDevicePosition requestedPosition = cameraPosition.compare(QStringLiteral("front"), Qt::CaseInsensitive) == 0
                                                   ? AVCaptureDevicePositionFront
                                                   : AVCaptureDevicePositionBack;
    NSArray<AVCaptureDevice *> *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if (device.position == requestedPosition) {
            return device;
        }
    }
    return devices.count > 0 ? devices.firstObject : nil;
}

static void drawMetrics(NSArray<NSString *> *metrics, CVPixelBufferRef pixelBuffer) {
    if (metrics.count == 0) {
        return;
    }

    const size_t width = CVPixelBufferGetWidth(pixelBuffer);
    const size_t height = CVPixelBufferGetHeight(pixelBuffer);

    UIFont *font = [UIFont boldSystemFontOfSize:28.0];
    NSDictionary *attributes = @{
        NSFontAttributeName : font,
        NSForegroundColorAttributeName : UIColor.whiteColor
    };

    NSMutableArray<NSValue *> *lineSizes = [NSMutableArray arrayWithCapacity:metrics.count];
    CGFloat maxWidth = 0.0;
    CGFloat totalHeight = 0.0;
    for (NSString *line in metrics) {
        CGSize lineSize = [line sizeWithAttributes:attributes];
        [lineSizes addObject:[NSValue valueWithCGSize:lineSize]];
        maxWidth = MAX(maxWidth, lineSize.width);
        totalHeight += lineSize.height + 8.0;
    }
    totalHeight = MAX(0.0, totalHeight - 8.0);

    const CGFloat padding = 18.0;
    const CGFloat margin = 24.0;
    CGRect backgroundRect = CGRectMake(width - maxWidth - padding * 2.0 - margin,
                                       height - totalHeight - padding * 2.0 - margin,
                                       maxWidth + padding * 2.0,
                                       totalHeight + padding * 2.0);

    UIGraphicsImageRendererFormat *format = [UIGraphicsImageRendererFormat defaultFormat];
    format.opaque = NO;
    format.scale = 1.0;
    UIGraphicsImageRenderer *renderer = [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(width, height)
                                                                               format:format];
    UIImage *overlay = [renderer imageWithActions:^(UIGraphicsImageRendererContext *rendererContext) {
        Q_UNUSED(rendererContext);

        UIBezierPath *path = [UIBezierPath bezierPathWithRoundedRect:backgroundRect cornerRadius:14.0];
        [[UIColor colorWithWhite:0.0 alpha:0.62] setFill];
        [path fill];

        CGFloat y = backgroundRect.origin.y + padding;
        for (NSUInteger i = 0; i < metrics.count; i++) {
            NSString *line = metrics[i];
            CGSize lineSize = [lineSizes[i] CGSizeValue];
            CGRect textRect = CGRectMake(backgroundRect.origin.x + backgroundRect.size.width - padding - lineSize.width,
                                         y,
                                         lineSize.width,
                                         lineSize.height);
            [line drawInRect:textRect withAttributes:attributes];
            y += lineSize.height + 8.0;
        }
    }];

    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    void *baseAddress = CVPixelBufferGetBaseAddress(pixelBuffer);
    const size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(baseAddress, width, height, 8, bytesPerRow, colorSpace,
                                                 kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
    CGColorSpaceRelease(colorSpace);

    if (!context) {
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        QZObjcRelease(renderer);
        return;
    }

    CGContextDrawImage(context, CGRectMake(0.0, 0.0, width, height), overlay.CGImage);
    CGContextRelease(context);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    QZObjcRelease(renderer);
}

@interface QZWorkoutVideoCaptureDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate, AVCaptureAudioDataOutputSampleBufferDelegate>
@property(nonatomic, assign) IOSWorkoutVideoRecorderPrivate *state;
- (instancetype)initWithState:(IOSWorkoutVideoRecorderPrivate *)state;
@end

@implementation QZWorkoutVideoCaptureDelegate
- (instancetype)initWithState:(IOSWorkoutVideoRecorderPrivate *)state {
    self = [super init];
    if (self) {
        _state = state;
    }
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)output
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
           fromConnection:(AVCaptureConnection *)connection {
    Q_UNUSED(connection)

    IOSWorkoutVideoRecorderPrivate *state = _state;
    if (!state || !state->recording || state->paused || !CMSampleBufferDataIsReady(sampleBuffer)) {
        return;
    }

    AVAssetWriter *assetWriter = state->assetWriter;
    if (output == state->audioOutput) {
        AVAssetWriterInput *audioInput = state->audioInput;
        if (!assetWriter || !audioInput || !state->writingStarted || assetWriter.status != AVAssetWriterStatusWriting ||
            !audioInput.readyForMoreMediaData) {
            return;
        }
        [audioInput appendSampleBuffer:sampleBuffer];
        return;
    }

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    if (!imageBuffer) {
        return;
    }

    AVAssetWriterInput *videoInput = state->videoInput;
    AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor = state->pixelBufferAdaptor;
    CIContext *ciContext = state->ciContext;
    if (!assetWriter || !videoInput || !pixelBufferAdaptor || !ciContext) {
        return;
    }

    const CMTime presentationTime = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
    if (!state->writingStarted) {
        if (![assetWriter startWriting]) {
            qDebug() << "Workout video writer failed to start"
                     << (assetWriter.error ? QString::fromNSString(assetWriter.error.localizedDescription) : QString());
            return;
        }
        [assetWriter startSessionAtSourceTime:presentationTime];
        state->writingStarted = true;
    }

    if (assetWriter.status != AVAssetWriterStatusWriting || !videoInput.readyForMoreMediaData) {
        return;
    }

    CVPixelBufferPoolRef pixelBufferPool = pixelBufferAdaptor.pixelBufferPool;
    if (!pixelBufferPool) {
        return;
    }

    CVPixelBufferRef outputBuffer = nil;
    CVReturn result = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault,
                                                         pixelBufferPool,
                                                         &outputBuffer);
    if (result != kCVReturnSuccess || !outputBuffer) {
        return;
    }

    CIImage *sourceImage = [CIImage imageWithCVPixelBuffer:imageBuffer];
    [ciContext render:sourceImage toCVPixelBuffer:outputBuffer];

    QStringList metricLines;
    {
        QMutexLocker locker(&state->metricsMutex);
        metricLines = state->metrics;
    }
    NSMutableArray<NSString *> *nativeMetrics = [NSMutableArray arrayWithCapacity:metricLines.size()];
    for (const QString &line : metricLines) {
        if (!line.trimmed().isEmpty()) {
            [nativeMetrics addObject:line.toNSString()];
        }
    }
    drawMetrics(nativeMetrics, outputBuffer);

    [pixelBufferAdaptor appendPixelBuffer:outputBuffer withPresentationTime:presentationTime];
    CVPixelBufferRelease(outputBuffer);
}
@end

IOSWorkoutVideoRecorder::IOSWorkoutVideoRecorder(QObject *parent) : QObject(parent), d(new IOSWorkoutVideoRecorderPrivate) {}

IOSWorkoutVideoRecorder::~IOSWorkoutVideoRecorder() {
    stopRecording();
    delete d;
}

bool IOSWorkoutVideoRecorder::available() const {
    return [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] != nil;
}

bool IOSWorkoutVideoRecorder::recording() const { return d->recording; }

bool IOSWorkoutVideoRecorder::paused() const { return d->paused; }

bool IOSWorkoutVideoRecorder::startRecording(const QString &outputLocation, const QString &cameraPosition) {
    if (d->recording) {
        return true;
    }

    AVAuthorizationStatus authorizationStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    if (authorizationStatus == AVAuthorizationStatusDenied || authorizationStatus == AVAuthorizationStatusRestricted) {
        emit errorOccurred(QStringLiteral("Camera permission is required for workout video recording."));
        return false;
    }
    if (authorizationStatus == AVAuthorizationStatusNotDetermined) {
        IOSWorkoutVideoRecorder *self = this;
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
            dispatch_async(dispatch_get_main_queue(), ^{
                emit self->errorOccurred(granted ? QStringLiteral("Camera permission granted. Press record again to start recording.")
                                                : QStringLiteral("Camera permission is required for workout video recording."));
            });
        }];
        return false;
    }

    AVAuthorizationStatus audioAuthorizationStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
    if (audioAuthorizationStatus == AVAuthorizationStatusDenied || audioAuthorizationStatus == AVAuthorizationStatusRestricted) {
        emit errorOccurred(QStringLiteral("Microphone permission is required for workout video recording."));
        return false;
    }
    if (audioAuthorizationStatus == AVAuthorizationStatusNotDetermined) {
        IOSWorkoutVideoRecorder *self = this;
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
            dispatch_async(dispatch_get_main_queue(), ^{
                emit self->errorOccurred(granted ? QStringLiteral("Microphone permission granted. Press record again to start recording.")
                                                : QStringLiteral("Microphone permission is required for workout video recording."));
            });
        }];
        return false;
    }

    AVCaptureDevice *device = cameraDevice(cameraPosition);
    if (!device) {
        emit errorOccurred(QStringLiteral("No iOS camera is available for workout video recording."));
        return false;
    }
    AVCaptureDevice *audioDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeAudio];
    if (!audioDevice) {
        emit errorOccurred(QStringLiteral("No iOS microphone is available for workout video recording."));
        return false;
    }

    d->outputUrl = QZObjcRetain(urlFromOutputLocation(outputLocation));
    NSError *error = nil;
    d->assetWriter = [[AVAssetWriter alloc] initWithURL:d->outputUrl fileType:AVFileTypeQuickTimeMovie error:&error];
    if (!d->assetWriter || error) {
        emit errorOccurred(error ? QString::fromNSString(error.localizedDescription)
                                : QStringLiteral("Unable to create the workout video file."));
        d->clearCaptureObjects();
        return false;
    }

    d->captureSession = [[AVCaptureSession alloc] init];
    d->captureSession.sessionPreset = AVCaptureSessionPreset1280x720;

    AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:device error:&error];
    if (!input || error || ![d->captureSession canAddInput:input]) {
        emit errorOccurred(error ? QString::fromNSString(error.localizedDescription)
                                : QStringLiteral("Unable to access the selected iOS camera."));
        d->clearCaptureObjects();
        return false;
    }
    [d->captureSession addInput:input];

    AVCaptureDeviceInput *audioDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:audioDevice error:&error];
    if (!audioDeviceInput || error || ![d->captureSession canAddInput:audioDeviceInput]) {
        emit errorOccurred(error ? QString::fromNSString(error.localizedDescription)
                                : QStringLiteral("Unable to access the iOS microphone."));
        d->clearCaptureObjects();
        return false;
    }
    [d->captureSession addInput:audioDeviceInput];

    d->videoOutput = [[AVCaptureVideoDataOutput alloc] init];
    d->videoOutput.videoSettings = @{(__bridge NSString *)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)};
    d->videoOutput.alwaysDiscardsLateVideoFrames = YES;
    d->captureQueue = dispatch_queue_create("org.cagnulein.qdomyoszwift.workoutVideo", DISPATCH_QUEUE_SERIAL);
    dispatch_queue_set_specific(d->captureQueue, &QZWorkoutVideoCaptureQueueKey, &QZWorkoutVideoCaptureQueueKey, NULL);
    d->captureDelegate = [[QZWorkoutVideoCaptureDelegate alloc] initWithState:d];
    [d->videoOutput setSampleBufferDelegate:d->captureDelegate queue:d->captureQueue];
    if (![d->captureSession canAddOutput:d->videoOutput]) {
        emit errorOccurred(QStringLiteral("Unable to start the iOS camera video output."));
        d->clearCaptureObjects();
        return false;
    }
    [d->captureSession addOutput:d->videoOutput];

    d->audioOutput = [[AVCaptureAudioDataOutput alloc] init];
    [d->audioOutput setSampleBufferDelegate:d->captureDelegate queue:d->captureQueue];
    if (![d->captureSession canAddOutput:d->audioOutput]) {
        emit errorOccurred(QStringLiteral("Unable to start the iOS microphone audio output."));
        d->clearCaptureObjects();
        return false;
    }
    [d->captureSession addOutput:d->audioOutput];

    AVCaptureConnection *videoConnection = [d->videoOutput connectionWithMediaType:AVMediaTypeVideo];
    if (videoConnection.supportsVideoOrientation) {
        videoConnection.videoOrientation = AVCaptureVideoOrientationPortrait;
    }
    if (videoConnection.supportsVideoMirroring && device.position == AVCaptureDevicePositionFront) {
        videoConnection.videoMirrored = YES;
    }

    NSDictionary *outputSettings = @{
        AVVideoCodecKey : AVVideoCodecTypeH264,
        AVVideoWidthKey : @(720),
        AVVideoHeightKey : @(1280)
    };
    d->videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:outputSettings];
    d->videoInput = QZObjcRetain(d->videoInput);
    d->videoInput.expectsMediaDataInRealTime = YES;
    NSDictionary *sourceAttributes = @{
        (__bridge NSString *)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
        (__bridge NSString *)kCVPixelBufferWidthKey : @(720),
        (__bridge NSString *)kCVPixelBufferHeightKey : @(1280)
    };
    d->pixelBufferAdaptor = [AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:d->videoInput
                                                                                             sourcePixelBufferAttributes:sourceAttributes];
    d->pixelBufferAdaptor = QZObjcRetain(d->pixelBufferAdaptor);
    if (![d->assetWriter canAddInput:d->videoInput]) {
        emit errorOccurred(QStringLiteral("Unable to configure the workout video writer."));
        d->clearCaptureObjects();
        return false;
    }
    [d->assetWriter addInput:d->videoInput];

    NSDictionary *audioSettings = @{
        AVFormatIDKey : @(kAudioFormatMPEG4AAC),
        AVNumberOfChannelsKey : @(1),
        AVSampleRateKey : @(44100),
        AVEncoderBitRateKey : @(64000)
    };
    d->audioInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio outputSettings:audioSettings];
    d->audioInput = QZObjcRetain(d->audioInput);
    d->audioInput.expectsMediaDataInRealTime = YES;
    if (![d->assetWriter canAddInput:d->audioInput]) {
        emit errorOccurred(QStringLiteral("Unable to configure the workout audio writer."));
        d->clearCaptureObjects();
        return false;
    }
    [d->assetWriter addInput:d->audioInput];

    d->ciContext = QZObjcRetain([CIContext contextWithOptions:nil]);
    d->writingStarted = false;
    d->paused = false;
    d->recording = true;
    [d->captureSession startRunning];
    emit recordingChanged();
    emit pausedChanged();
    qDebug() << "Native iOS camera workout recording started" << outputLocation << cameraPosition;
    return true;
}

void IOSWorkoutVideoRecorder::updateMetrics(const QStringList &metrics) {
    QMutexLocker locker(&d->metricsMutex);
    d->metrics = metrics;
}

void IOSWorkoutVideoRecorder::pauseRecording() {
    if (!d->recording || d->paused) {
        return;
    }

    d->paused = true;
    emit pausedChanged();
    qDebug() << "Native iOS camera workout recording paused";
}

void IOSWorkoutVideoRecorder::stopRecording() {
    if (!d->recording) {
        return;
    }

    d->recording = false;
    d->paused = false;
    emit recordingChanged();
    emit pausedChanged();

    [d->captureSession stopRunning];

    AVAssetWriter *writer = d->assetWriter;
    AVAssetWriterInput *videoInput = d->videoInput;
    AVAssetWriterInput *audioInput = d->audioInput;
    NSURL *url = d->outputUrl;
    const QString outputLocation = QString::fromNSString(url.absoluteString);
    const bool hadFrames = d->writingStarted;

    QZObjcRetain(writer);
    QZObjcRetain(videoInput);
    QZObjcRetain(audioInput);
    QZObjcRetain(url);

    d->clearCaptureObjects();

    if (!hadFrames || writer.status != AVAssetWriterStatusWriting) {
        emit errorOccurred(QStringLiteral("Workout video recording stopped before receiving video frames."));
        QZObjcRelease(writer);
        QZObjcRelease(videoInput);
        QZObjcRelease(audioInput);
        QZObjcRelease(url);
        return;
    }

    [videoInput markAsFinished];
    [audioInput markAsFinished];
    IOSWorkoutVideoRecorder *self = this;
    [writer finishWritingWithCompletionHandler:^{
        if (writer.status == AVAssetWriterStatusCompleted) {
            if (UIVideoAtPathIsCompatibleWithSavedPhotosAlbum(url.path)) {
                UISaveVideoAtPathToSavedPhotosAlbum(url.path, nil, nil, nil);
            }
            dispatch_async(dispatch_get_main_queue(), ^{
                emit self->saved(outputLocation);
            });
        } else if (writer.error) {
            dispatch_async(dispatch_get_main_queue(), ^{
                emit self->errorOccurred(QString::fromNSString(writer.error.localizedDescription));
            });
        }
        QZObjcRelease(writer);
        QZObjcRelease(videoInput);
        QZObjcRelease(audioInput);
        QZObjcRelease(url);
    }];
}

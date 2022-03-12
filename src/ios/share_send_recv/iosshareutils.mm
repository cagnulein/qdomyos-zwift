// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#import "iosshareutils.hpp"

#import <UIKit/UIKit.h>
#import <QGuiApplication>
#import <QQuickWindow>
#import <QDesktopServices>
#import <QUrl>
#import <QFileInfo>

#import <UIKit/UIDocumentInteractionController.h>

#import "docviewcontroller.hpp"

IosShareUtils::IosShareUtils(QObject *parent) : PlatformShareUtils(parent)
{
    // Sharing Files from other iOS Apps I got the ideas and some code contribution from:
    // Thomas K. Fischer (@taskfabric) - http://taskfabric.com - thx
    QDesktopServices::setUrlHandler("file", this, "handleFileUrlReceived");
}

bool IosShareUtils::checkMimeTypeView(const QString &mimeType) {
#pragma unused (mimeType)
    // dummi implementation on iOS
    // MimeType not used yet
    return true;
}

bool IosShareUtils::checkMimeTypeEdit(const QString &mimeType) {
#pragma unused (mimeType)
    // dummi implementation on iOS
    // MimeType not used yet
    return true;
}

void IosShareUtils::share(const QString &text, const QUrl &url) {

    NSMutableArray *sharingItems = [NSMutableArray new];

    if (!text.isEmpty()) {
        [sharingItems addObject:text.toNSString()];
    }
    if (url.isValid()) {
        [sharingItems addObject:url.toNSURL()];
    }

    // get the main window rootViewController
    UIViewController *qtUIViewController = [[UIApplication sharedApplication].keyWindow rootViewController];

    UIActivityViewController *activityController = [[UIActivityViewController alloc] initWithActivityItems:sharingItems applicationActivities:nil];
    if ( [activityController respondsToSelector:@selector(popoverPresentationController)] ) { // iOS8
        activityController.popoverPresentationController.sourceView = qtUIViewController.view;
    }
    [qtUIViewController presentViewController:activityController animated:YES completion:nil];
}

// altImpl not used yet on iOS, on Android twi ways to use JNI
void IosShareUtils::sendFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl) {
#pragma unused (title, mimeType, altImpl)

    NSString* nsFilePath = filePath.toNSString();
    NSURL *nsFileUrl = [NSURL fileURLWithPath:nsFilePath];

    static DocViewController* docViewController = nil;
    if(docViewController!=nil)
    {
        [docViewController removeFromParentViewController];
        [docViewController release];
    }

    UIDocumentInteractionController* documentInteractionController = nil;
    documentInteractionController = [UIDocumentInteractionController interactionControllerWithURL:nsFileUrl];

    UIViewController* qtUIViewController = [[[[UIApplication sharedApplication]windows] firstObject]rootViewController];
    if(qtUIViewController!=nil)
    {
        docViewController = [[DocViewController alloc] init];

        docViewController.requestId = requestId;
        // we need this to be able to execute handleDocumentPreviewDone() method,
        // when preview was finished
        docViewController.mIosShareUtils = this;

        [qtUIViewController addChildViewController:docViewController];
        documentInteractionController.delegate = docViewController;
        // [documentInteractionController presentPreviewAnimated:YES];
        if(![documentInteractionController presentPreviewAnimated:YES])
        {
            emit shareError(0, tr("No App found to open: %1").arg(filePath));
        }
    }
}


void IosShareUtils::viewFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl) {
#pragma unused (title, mimeType)

    sendFile(filePath, title, mimeType, requestId, altImpl);
}

void IosShareUtils::editFile(const QString &filePath, const QString &title, const QString &mimeType, const int &requestId, const bool &altImpl) {
#pragma unused (title, mimeType)

    sendFile(filePath, title, mimeType, requestId, altImpl);
}

void IosShareUtils::handleDocumentPreviewDone(const int &requestId)
{
    // documentInteractionControllerDidEndPreview
    qDebug() << "handleShareDone: " << requestId;
    emit shareFinished(requestId);
}

void IosShareUtils::handleFileUrlReceived(const QUrl &url)
{
    QString incomingUrl = url.toString();
    if(incomingUrl.isEmpty()) {
        qWarning() << "setFileUrlReceived: we got an empty URL";
        emit shareError(0, tr("Empty URL received"));
        return;
    }
    qDebug() << "IosShareUtils setFileUrlReceived: we got the File URL from iOS: " << incomingUrl;
    QString myUrl;
    if(incomingUrl.startsWith("file://")) {
        myUrl= incomingUrl.right(incomingUrl.length()-7);
        qDebug() << "QFile needs this URL: " << myUrl;
    } else {
        myUrl= incomingUrl;
    }

    // check if File exists
    QFileInfo fileInfo = QFileInfo(myUrl);
    if(fileInfo.exists()) {
        emit fileUrlReceived(myUrl);
    } else {
        qDebug() << "setFileUrlReceived: FILE does NOT exist ";
        emit shareError(0, tr("File does not exist: %1").arg(myUrl));
    }
}



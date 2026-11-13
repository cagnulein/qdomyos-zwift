#import <UIKit/UIKit.h>
#import <MessageUI/MessageUI.h>
#import <Foundation/Foundation.h>
#include <string>

// Isolated from lockscreen.mm so it is compiled even with IO_UNDER_QT.
@interface QZSupportMailDelegate : NSObject <MFMailComposeViewControllerDelegate>
@end

@implementation QZSupportMailDelegate
- (void)mailComposeController:(MFMailComposeViewController *)controller
          didFinishWithResult:(MFMailComposeResult)result
                        error:(NSError *)error {
    (void)result;
    (void)error;
    [controller dismissViewControllerAnimated:YES completion:nil];
}
@end

static QZSupportMailDelegate *qzMailDelegate() {
    static QZSupportMailDelegate *delegate = nil;
    if (!delegate)
        delegate = [[QZSupportMailDelegate alloc] init];
    return delegate;
}

static UIViewController *qzPresentingController() {
    UIApplication *application = [UIApplication sharedApplication];
    UIWindow *window = application.keyWindow;
    if (!window) {
        for (UIWindow *candidate in application.windows) {
            if (candidate.isKeyWindow) {
                window = candidate;
                break;
            }
        }
    }
    UIViewController *controller = window.rootViewController;
    while (controller.presentedViewController)
        controller = controller.presentedViewController;
    return controller;
}

// C ABI: the caller's Qt QByteArrays are copied before posting to the main queue.
extern "C" void qzComposeSupportEmail(const char *recipient, const char *subject,
                                      const char *body, const char *attachmentPath) {
    const std::string recipientCopy = recipient ? recipient : "";
    const std::string subjectCopy = subject ? subject : "";
    const std::string bodyCopy = body ? body : "";
    const std::string pathCopy = attachmentPath ? attachmentPath : "";

    dispatch_async(dispatch_get_main_queue(), ^{
        UIViewController *presenter = qzPresentingController();
        if (!presenter)
            return;

        NSString *to = [NSString stringWithUTF8String:recipientCopy.c_str()];
        NSString *title = [NSString stringWithUTF8String:subjectCopy.c_str()];
        NSString *message = [NSString stringWithUTF8String:bodyCopy.c_str()];
        NSString *path = [NSString stringWithUTF8String:pathCopy.c_str()];

        if ([MFMailComposeViewController canSendMail]) {
            MFMailComposeViewController *mail = [[MFMailComposeViewController alloc] init];
            mail.mailComposeDelegate = qzMailDelegate();
            [mail setToRecipients:@[to]];
            [mail setSubject:title];
            [mail setMessageBody:message isHTML:NO];
            if (path.length > 0) {
                NSData *data = [NSData dataWithContentsOfFile:path];
                if (data)
                    [mail addAttachmentData:data mimeType:@"application/gzip"
                                  fileName:[path lastPathComponent]];
            }
            [presenter presentViewController:mail animated:YES completion:nil];
            return;
        }

        // No Apple Mail account configured (e.g. Gmail-only users). The share
        // sheet can pass the .gz to another mail app. Recipient and subject are
        // also included as text because third-party extensions need not honour them.
        NSString *shareText = [NSString stringWithFormat:@"To: %@\nSubject: %@\n\n%@",
                               to, title, message];
        NSMutableArray *items = [NSMutableArray arrayWithObject:shareText];
        if (path.length > 0)
            [items addObject:[NSURL fileURLWithPath:path]];
        UIActivityViewController *share =
            [[UIActivityViewController alloc] initWithActivityItems:items
                                              applicationActivities:nil];
        if (share.popoverPresentationController) {
            share.popoverPresentationController.sourceView = presenter.view;
            share.popoverPresentationController.sourceRect =
                CGRectMake(CGRectGetMidX(presenter.view.bounds),
                           CGRectGetMidY(presenter.view.bounds), 1, 1);
            share.popoverPresentationController.permittedArrowDirections = 0;
        }
        [presenter presentViewController:share animated:YES completion:nil];
    });
}

// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#import "DocViewController.hpp"

#include <QDebug>

@interface DocViewController ()
@end
@implementation DocViewController
#pragma mark -
#pragma mark View Life Cycle
- (void)viewDidLoad {
    [super viewDidLoad];
}
#pragma mark -
#pragma mark Document Interaction Controller Delegate Methods
- (UIViewController *) documentInteractionControllerViewControllerForPreview: (UIDocumentInteractionController *) controller {
#pragma unused (controller)
    return self;
}
- (void)documentInteractionControllerDidEndPreview:(UIDocumentInteractionController *)controller
{
#pragma unused (controller)
    qDebug() << "end preview";

    self.mIosShareUtils->handleDocumentPreviewDone(self.requestId);

    [self removeFromParentViewController];
}
@end

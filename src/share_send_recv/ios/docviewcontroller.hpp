// (c) 2017 Ekkehard Gentz (ekke) @ekkescorner
// my blog about Qt for mobile: http://j.mp/qt-x
// see also /COPYRIGHT and /LICENSE

#ifndef DOCVIEWCONTROLLER_HPP
#define DOCVIEWCONTROLLER_HPP

#import <UIKit/UIKit.h>
#import "iosshareutils.hpp"

@interface DocViewController : UIViewController <UIDocumentInteractionControllerDelegate>

@property int requestId;

@property IosShareUtils *mIosShareUtils;

@end



#endif // DOCVIEWCONTROLLER_HPP

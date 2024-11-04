//
//  swiftDebug.m
//  qdomyoszwift
//
//  Created by Roberto Viola on 14/12/23.
//
#import "swiftDebug.h"
#import "homeform.h"
#import "bike.h"
#include <QDebug>

@implementation swiftDebug

- (void)qtDebug:(NSString *)inputString {
    qDebug() << inputString;
}

- (void)gearUp {
    ((bike*)(homeform::singleton()->bluetoothManager->device()))->gearUp();
}

- (void)gearDown {
    ((bike*)(homeform::singleton()->bluetoothManager->device()))->gearDown();
}

@end

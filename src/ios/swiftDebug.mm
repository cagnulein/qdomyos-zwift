//
//  swiftDebug.m
//  qdomyoszwift
//
//  Created by Roberto Viola on 14/12/23.
//
#import "swiftDebug.h"
#include <QDebug>

@implementation swiftDebug

- (void)qtDebug:(NSString *)inputString {
    qDebug() << inputString;
}

@end

#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <AVFoundation/AVFoundation.h>
#import <ConnectIQ/ConnectIQ.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/lockscreen.h"
#include <QDebug>
#include "ios/AdbClient.h"
#include "ios/ios_eliteariafan.h"

@class virtualbike_ios_swift;
@class virtualbike_zwift;
@class virtualrower;
@class virtualtreadmill_zwift;
@class healthkit;

static healthkit* h = 0;
static virtualbike_ios_swift* _virtualbike = nil;
static virtualbike_zwift* _virtualbike_zwift = nil;
static virtualrower* _virtualrower = nil;
static virtualtreadmill_zwift* _virtualtreadmill_zwift = nil;

static GarminConnect* Garmin = 0;

static AdbClient *_adb = 0;

static ios_eliteariafan* ios_eliteAriaFan = nil;

static zwift_protobuf_layer* zwiftProtobufLayer = nil;

static NSString* profile_selected;

void lockscreen::setTimerDisabled() {
     [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
}

void lockscreen::request()
{
    h = [[healthkit alloc] init];
    [h request];
    zwiftProtobufLayer = [[zwift_protobuf_layer alloc] init];
    if (@available(iOS 13, *)) {
        Garmin = [[GarminConnect alloc] init];
    }
    _adb = [[AdbClient alloc] initWithVerbose:YES];
}

long lockscreen::heartRate()
{
    return [h heartRate];
}

long lockscreen::stepCadence()
{
    return [h stepCadence];
}

void lockscreen::setKcal(double kcal)
{
    [h setKcalWithKcal:kcal];
}

void lockscreen::setDistance(double distance)
{
    [h setDistanceWithDistance:distance * 0.621371];
}

void lockscreen::setPower(double power)
{
    [h setPowerWithPower:power];
}
void lockscreen::setCadence(double cadence)
{
    [h setCadenceWithCadence:cadence];
}
void lockscreen::setSpeed(double speed)
{
    [h setSpeedWithSpeed:speed];
}


void lockscreen::virtualbike_ios()
{
    _virtualbike = [[virtualbike_ios_swift alloc] init];
}

void lockscreen::virtualbike_setHeartRate(unsigned char heartRate)
{
    if(_virtualbike != nil)
        [_virtualbike updateHeartRateWithHeartRate:heartRate];
    if(_virtualbike_zwift != nil)
        [_virtualbike_zwift updateHeartRateWithHeartRate:heartRate];
}

void lockscreen::virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime)
{
    if(_virtualbike != nil)
        [_virtualbike updateCadenceWithCrankRevolutions:crankRevolutions LastCrankEventTime:lastCrankEventTime];
}

void lockscreen::virtualbike_zwift_ios(bool disable_hr, bool garmin_bluetooth_compatibility, bool zwift_play_emulator, bool watt_bike_emulator)
{
    _virtualbike_zwift = [[virtualbike_zwift alloc] initWithDisable_hr:disable_hr garmin_bluetooth_compatibility:garmin_bluetooth_compatibility zwift_play_emulator:zwift_play_emulator watt_bike_emulator:watt_bike_emulator];
}

void lockscreen::virtualrower_ios()
{
    _virtualrower = [[virtualrower alloc] init];
}

double lockscreen::virtualbike_getCurrentSlope()
{
    if(_virtualbike_zwift != nil)
    {
        return [_virtualbike_zwift readCurrentSlope];
    }
    return 0;
}

double lockscreen::virtualbike_getCurrentCRR()
{
    if(_virtualbike_zwift != nil)
    {
        return [_virtualbike_zwift readCurrentCRR];
    }
    return 0;
}

double lockscreen::virtualbike_getCurrentCW()
{
    if(_virtualbike_zwift != nil)
    {
        return [_virtualbike_zwift readCurrentCW];
    }
    return 0;
}

double lockscreen::virtualbike_getPowerRequested()
{
    if(_virtualbike_zwift != nil)
    {
        return [_virtualbike_zwift readPowerRequested];
    }
    return 0;
}

bool lockscreen::virtualbike_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt, UInt16 CrankRevolutions, UInt16 LastCrankEventTime, signed short Gears)
{
    if(_virtualbike_zwift != nil)
        return [_virtualbike_zwift updateFTMSWithNormalizeSpeed:normalizeSpeed currentCadence:currentCadence currentResistance:currentResistance currentWatt:currentWatt CrankRevolutions:CrankRevolutions LastCrankEventTime:LastCrankEventTime Gears:Gears];
    return 0;
}

bool lockscreen::virtualrower_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt, UInt16 CrankRevolutions, UInt16 LastCrankEventTime, UInt16 StrokesCount, UInt32 Distance, UInt16 KCal, UInt16 Pace)
{
    if(_virtualrower != nil)
        return [_virtualrower updateFTMSWithNormalizeSpeed:normalizeSpeed currentCadence:currentCadence currentResistance:currentResistance currentWatt:currentWatt CrankRevolutions:CrankRevolutions LastCrankEventTime:LastCrankEventTime StrokesCount:StrokesCount Distance:Distance KCal:KCal Pace:Pace];
    return 0;
}

void lockscreen::virtualrower_setHeartRate(unsigned char heartRate)
{
    if(_virtualrower != nil)
        [_virtualrower updateHeartRateWithHeartRate:heartRate];
}


// virtual treadmill
void lockscreen::virtualtreadmill_zwift_ios()
{
    _virtualtreadmill_zwift = [[virtualtreadmill_zwift alloc] init];
}

void lockscreen::virtualtreadmill_setHeartRate(unsigned char heartRate)
{
    if(_virtualtreadmill_zwift != nil)
        [_virtualtreadmill_zwift updateHeartRateWithHeartRate:heartRate];
}

double lockscreen::virtualtreadmill_getCurrentSlope()
{
    if(_virtualtreadmill_zwift != nil)
    {
        return [_virtualtreadmill_zwift readCurrentSlope];
    }
    return 0;
}

uint64_t lockscreen::virtualtreadmill_lastChangeCurrentSlope()
{
    if(_virtualtreadmill_zwift != nil)
    {
        return [_virtualtreadmill_zwift lastChangeCurrentSlope];
    }
    return 0;
}

double lockscreen::virtualtreadmill_getPowerRequested()
{
    if(_virtualtreadmill_zwift != nil)
    {
        return [_virtualtreadmill_zwift readPowerRequested];
    }
    return 0;
}

bool lockscreen::virtualtreadmill_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt, UInt16 currentInclination, UInt64 currentDistance)
{
    if(_virtualtreadmill_zwift != nil)
        return [_virtualtreadmill_zwift updateFTMSWithNormalizeSpeed:normalizeSpeed currentCadence:currentCadence currentResistance:currentResistance currentWatt:currentWatt currentInclination:currentInclination currentDistance:currentDistance];
    return 0;
}

int lockscreen::virtualbike_getLastFTMSMessage(unsigned char* message) {
    if(message) {
        if(_virtualbike_zwift != nil) {
            NSData* data = [_virtualbike_zwift getLastFTMSMessage];
            [data getBytes:message length:data.length];
            return (int)data.length;
        }
        return 0;
    }
    return 0;
}

int lockscreen::virtualrower_getLastFTMSMessage(unsigned char* message) {
    if(message) {
        if(_virtualrower != nil) {
            NSData* data = [_virtualrower getLastFTMSMessage];
            [data getBytes:message length:data.length];
            return (int)data.length;
        }
        return 0;
    }
    return 0;
}

void lockscreen::garminconnect_init() {
    [[ConnectIQ sharedInstance] initializeWithUrlScheme:@"org.cagnulein.connectiqcomms-ciq"
                                 uiOverrideDelegate:nil];
    
    [[ConnectIQ sharedInstance] showConnectIQDeviceSelection];
}

bool lockscreen::urlParser(const char *url) {
    NSString *sURL = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    NSURL *URL = [NSURL URLWithString:[sURL stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    [Garmin urlParser: URL];
}

int lockscreen::getHR() {
    return [Garmin getHR];
}

int lockscreen::getFootCad() {
    return [Garmin getFootCad];
}

int lockscreen::getPower() {
    return [Garmin getPower];
}

double lockscreen::getSpeed() {
    return [Garmin getSpeed];
}

// getVolume

double lockscreen::getVolume()
{
    [[AVAudioSession sharedInstance] setActive:true error:0];
    return [[AVAudioSession sharedInstance] outputVolume];
}

void lockscreen::debug(const char* debugstring) {
    qDebug() << debugstring;
}

void lockscreen::nslog(const char* log) {
    NSLog([[NSString alloc] initWithUTF8String:log]);
}

void lockscreen::set_action_profile(const char* profile) {
    profile_selected = [[NSString alloc] initWithUTF8String:profile];
}

const char* lockscreen::get_action_profile() {
    return [profile_selected UTF8String];
}

void lockscreen::adb_connect(const char*  IP) {
    if(_adb == 0) return;
    
    [_adb connect:[NSString stringWithCString:IP encoding:NSASCIIStringEncoding] didResponse:^(BOOL succ, NSString *result) {
        
        qDebug() << result;

    }];
}
    
void lockscreen::adb_sendcommand(const char* command) {
    if(_adb == 0) return;
    
    [_adb shell:[NSString stringWithCString:command encoding:NSASCIIStringEncoding] didResponse:^(BOOL succ, NSString *result) {
        
        qDebug() << result;

    }];
}

void lockscreen::eliteAriaFan() {
    ios_eliteAriaFan = [[ios_eliteariafan alloc] init];
}

void lockscreen::eliteAriaFan_fanSpeedRequest(unsigned char speed) {
    if(ios_eliteAriaFan) {
        [ios_eliteAriaFan fanSpeedRequest:speed];
    }
}

void lockscreen::zwift_api_decodemessage_player(const char* data, int len) {
    NSData *d = [NSData dataWithBytes:data length:len];
    [zwiftProtobufLayer getPlayerStateWithValue:d];
}

float lockscreen::zwift_api_getaltitude() {
    return [zwiftProtobufLayer getAltitude];
}

int lockscreen::zwift_api_getdistance() {
    return [zwiftProtobufLayer getDistance];
}

float lockscreen::zwift_api_getlatitude() {
    return [zwiftProtobufLayer getLatitude];
}

float lockscreen::zwift_api_getlongitude() {
    return [zwiftProtobufLayer getLongitude];
}
#endif

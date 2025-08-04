#ifndef IO_UNDER_QT
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <WatchConnectivity/WatchConnectivity.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <AVFoundation/AVFoundation.h>
#import <ConnectIQ/ConnectIQ.h>
#import "qdomyoszwift-Swift2.h"
#include "ios/lockscreen.h"
#include "devices/bluetoothdevice.h"
#include <QDebug>
#include "ios/AdbClient.h"
#include "ios/ios_eliteariafan.h"
#include "ios/ios_echelonconnectsport.h"
#include "ios/ios_wahookickrsnapbike.h"
#include "ios/ios_zwiftclickremote.h"

@class virtualbike_ios_swift;
@class virtualbike_zwift;
@class virtualrower_zwift;
@class virtualtreadmill_zwift;
@class healthkit;
@class WorkoutTracking;

static healthkit* h = 0;
static virtualbike_ios_swift* _virtualbike = nil;
static virtualbike_zwift* _virtualbike_zwift = nil;
static virtualrower_zwift* _virtualrower = nil;
static virtualtreadmill_zwift* _virtualtreadmill_zwift = nil;
static WorkoutTracking* workoutTracking = nil;

static GarminConnect* Garmin = 0;

static AdbClient *_adb = 0;

static ios_eliteariafan* ios_eliteAriaFan = nil;
static ios_echelonconnectsport* ios_echelonConnectSport = nil;

static zwift_protobuf_layer* zwiftProtobufLayer = nil;

static NSString* profile_selected;

bool lockscreen::appleWatchAppInstalled() {
    if ([WCSession isSupported]) {
        // Get the default session
        WCSession *session = [WCSession defaultSession];
        
        // Activate the session
        [session activateSession];
        
        // Check if a watch is paired and the app is installed
        if (session.isPaired && session.isWatchAppInstalled) {
            // An Apple Watch is paired and has the companion app installed
            qDebug() << "Apple Watch is paired and app is installed";
            return true;
        } else if (session.isPaired) {
            // An Apple Watch is paired but doesn't have the companion app
            qDebug() << "Apple Watch is paired but app is not installed";
            return false;
        } else {
            // No Apple Watch is paired
            qDebug() << "No Apple Watch is paired";
            return false;
        }
    } else {
        // This device doesn't support Watch connectivity
        qDebug() << "Watch connectivity is not supported on this device";
        return false;
    }
    return false;
}

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
    // just to be sure, I built the library for iOS17 only but theorically we can use any iOS version
    if (@available(iOS 17, *)) {
        workoutTracking = [[WorkoutTracking alloc] init];
        [WorkoutTracking requestAuth];
        _adb = [[AdbClient alloc] initWithVerbose:YES];
    }
}

void lockscreen::startWorkout(unsigned short deviceType) {
    if(workoutTracking != nil && !appleWatchAppInstalled())
        [workoutTracking startWorkOutWithDeviceType:deviceType];
}

void lockscreen::stopWorkout() {
    if(workoutTracking != nil && !appleWatchAppInstalled())
        [workoutTracking stopWorkOut];
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

void lockscreen::setSteps(double steps)
{
    [h setStepsWithSteps:steps];
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

void lockscreen::workoutTrackingUpdate(double speed, unsigned short cadence, unsigned short watt, unsigned short currentCalories, unsigned long long currentSteps, unsigned char deviceType, double currentDistance) {
    if(workoutTracking != nil && !appleWatchAppInstalled())
        [workoutTracking addMetricsWithPower:watt cadence:cadence*2 speed:speed * 100 kcal:currentCalories steps:currentSteps deviceType:deviceType distance:currentDistance];
}

void lockscreen::virtualbike_zwift_ios(bool disable_hr, bool garmin_bluetooth_compatibility, bool zwift_play_emulator, bool watt_bike_emulator)
{
    _virtualbike_zwift = [[virtualbike_zwift alloc] initWithDisable_hr:disable_hr garmin_bluetooth_compatibility:garmin_bluetooth_compatibility zwift_play_emulator:zwift_play_emulator watt_bike_emulator:watt_bike_emulator];
}

void lockscreen::virtualrower_ios()
{
    _virtualrower = [[virtualrower_zwift alloc] init];
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

bool lockscreen::virtualbike_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt, UInt16 CrankRevolutions, UInt16 LastCrankEventTime, signed short Gears, UInt16 currentCalories, UInt32 Distance)
{
    if(workoutTracking != nil && !appleWatchAppInstalled())
        [workoutTracking addMetricsWithPower:currentWatt cadence:currentCadence speed:normalizeSpeed kcal:currentCalories steps:0 deviceType: bluetoothdevice::BLUETOOTH_TYPE::BIKE distance:Distance];

    if(_virtualbike_zwift != nil)
        return [_virtualbike_zwift updateFTMSWithNormalizeSpeed:normalizeSpeed currentCadence:currentCadence currentResistance:currentResistance currentWatt:currentWatt CrankRevolutions:CrankRevolutions LastCrankEventTime:LastCrankEventTime Gears:Gears];
    return 0;
}

bool lockscreen::virtualrower_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt, UInt16 CrankRevolutions, UInt16 LastCrankEventTime, UInt16 StrokesCount, UInt32 Distance, UInt16 KCal, UInt16 Pace, UInt16 currentCalories)
{
    if(workoutTracking != nil && !appleWatchAppInstalled())
        [workoutTracking addMetricsWithPower:currentWatt cadence:currentCadence speed:normalizeSpeed kcal:currentCalories steps:0 deviceType: bluetoothdevice::BLUETOOTH_TYPE::ROWING distance:Distance];

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
void lockscreen::virtualtreadmill_zwift_ios(bool garmin_bluetooth_compatibility)
{
    _virtualtreadmill_zwift = [[virtualtreadmill_zwift alloc] initWithGarmin_bluetooth_compatibility:garmin_bluetooth_compatibility];
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

double lockscreen::virtualtreadmill_getRequestedSpeed()
{
    if(workoutTracking != nil && !appleWatchAppInstalled())
        [workoutTracking addMetricsWithPower:currentWatt cadence:currentCadence speed:normalizeSpeed kcal:currentCalories steps:currentSteps deviceType:bluetoothdevice::BLUETOOTH_TYPE::TREADMILL distance:currentDistance elapsedTimeSeconds:elapsedSeconds];

    if(_virtualtreadmill_zwift != nil)
    {
        return [_virtualtreadmill_zwift readRequestedSpeed];
    }
    return 0;
}

bool lockscreen::virtualtreadmill_updateFTMS(UInt16 normalizeSpeed, UInt8 currentResistance, UInt16 currentCadence, UInt16 currentWatt, UInt16 currentInclination, UInt64 currentDistance, unsigned short currentCalories, 
                                     qint32 currentSteps,  unsigned short elapsedSeconds)
{
    if(_virtualtreadmill_zwift != nil)
        return [_virtualtreadmill_zwift updateFTMSWithNormalizeSpeed:normalizeSpeed currentCadence:currentCadence currentResistance:currentResistance currentWatt:currentWatt currentInclination:currentInclination currentDistance:currentDistance elapsedTimeSeconds:elapsedSeconds];
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

void lockscreen::echelonConnectSport(const char*  Name, void* deviceClass) {
    NSString *deviceName = [NSString stringWithCString:Name encoding:NSASCIIStringEncoding];
    ios_echelonConnectSport = [[ios_echelonconnectsport alloc] init:deviceName qtDevice:deviceClass];
}

void lockscreen::echelonConnectSport_WriteCharacteristic(unsigned char* qdata, unsigned char length) {
    if(ios_echelonConnectSport) {
        [ios_echelonConnectSport writeCharacteristc:qdata length:length ];
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

QByteArray lockscreen::zwift_hub_inclinationCommand(double inclination) {
    NSError *error = nil;
    NSData *command = [ZwiftHubBike inclinationCommandWithInclination:inclination error:&error];

    if (error) {
        qDebug() << "error zwift_hub_inclinationCommand: " << error;
        return QByteArray();
    } else {
        const char* bytes = static_cast<const char*>([command bytes]);
        NSUInteger length = [command length];
        return QByteArray(bytes, length);
    }
}

QByteArray lockscreen::zwift_hub_setGearsCommand(unsigned int gears) {
    NSError *error = nil;
    NSData *command = [ZwiftHubBike setGearCommandWithGears:gears error:&error];

    if (error) {
        qDebug() << "error zwift_hub_setGearsCommand: " << error;
        return QByteArray();
    } else {
        const char* bytes = static_cast<const char*>([command bytes]);
        NSUInteger length = [command length];
        return QByteArray(bytes, length);
    }
}

// Function to get power value from buffer data
uint32_t lockscreen::zwift_hub_getPowerFromBuffer(const QByteArray& buffer) {
    NSData *data = [NSData dataWithBytes:buffer.constData() length:buffer.length()];
    
    uint32_t power = [ZwiftHubBike getPowerFromBufferWithBuffer:data];
    return power;
}

// Function to get cadence value from buffer data
uint32_t lockscreen::zwift_hub_getCadenceFromBuffer(const QByteArray& buffer) {
    NSData *data = [NSData dataWithBytes:buffer.constData() length:buffer.length()];
    
    uint32_t cadence = [ZwiftHubBike getCadenceFromBufferWithBuffer:data];
    return cadence;
}

static ios_wahookickrsnapbike* ios_wahooKickrSnapBike = nil;

void lockscreen::wahooKickrSnapBike(const char* Name, void* deviceClass) {
    NSString *deviceName = [NSString stringWithCString:Name encoding:NSASCIIStringEncoding];
    ios_wahooKickrSnapBike = [[ios_wahookickrsnapbike alloc] init:deviceName qtDevice:deviceClass];
}

void lockscreen::writeCharacteristic(unsigned char* qdata, unsigned char length) {
    if(ios_wahooKickrSnapBike) {
        [ios_wahooKickrSnapBike writeCharacteristic:qdata length:length];
    }
}

static NSMutableDictionary<NSValue*, ios_zwiftclickremote*>* ios_zwiftClickRemotes = nil;

void lockscreen::zwiftClickRemote(const char* Name, const char* UUID, void* deviceClass) {
    NSString *deviceName = [NSString stringWithCString:Name encoding:NSASCIIStringEncoding];
    NSString *deviceUUID = [NSString stringWithCString:UUID encoding:NSASCIIStringEncoding];
    
    // Initialize the dictionary if needed
    if (ios_zwiftClickRemotes == nil) {
        ios_zwiftClickRemotes = [[NSMutableDictionary alloc] init];
    }
    
    // Create a key using the pointer value
    NSValue *key = [NSValue valueWithPointer:deviceClass];
    
    // Create a new instance for this device, passing both name and UUID
    ios_zwiftclickremote *remote = [[ios_zwiftclickremote alloc] initWithNameAndUUID:deviceName uuid:deviceUUID qtDevice:deviceClass];
    
    // Store in dictionary using the device pointer as key
    [ios_zwiftClickRemotes setObject:remote forKey:key];
}


void lockscreen::zwiftClickRemote_WriteCharacteristic(unsigned char* qdata, unsigned char length, void* deviceClass) {
    if (ios_zwiftClickRemotes == nil) return;
    
    // Get the specific remote for this device
    NSValue *key = [NSValue valueWithPointer:deviceClass];
    ios_zwiftclickremote *remote = [ios_zwiftClickRemotes objectForKey:key];
    
    if(remote) {
        [remote writeCharacteristic:qdata length:length];
    }
}
#endif

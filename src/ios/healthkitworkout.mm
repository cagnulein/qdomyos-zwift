#ifdef IO_UNDER_QT
#import <HealthKit/HealthKit.h>
#include "ios/lockscreen.h"
#include <QDebug>

static HKWorkoutActivityType HistoricalHealthKitActivityType(unsigned short sport)
{
    switch (sport) {
    case 1: // FIT_SPORT_RUNNING
        return HKWorkoutActivityTypeRunning;
    case 11: // FIT_SPORT_WALKING
        return HKWorkoutActivityTypeWalking;
    case 2: // FIT_SPORT_CYCLING
        return HKWorkoutActivityTypeCycling;
    case 15: // FIT_SPORT_ROWING
        return HKWorkoutActivityTypeRowing;
    case 4: // FIT_SPORT_FITNESS_EQUIPMENT
        return HKWorkoutActivityTypeElliptical;
    default:
        return HKWorkoutActivityTypeOther;
    }
}

static HKQuantityType *HistoricalHealthKitDistanceType(unsigned short sport)
{
    switch (sport) {
    case 1:
    case 11:
        return [HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierDistanceWalkingRunning];
    case 2:
    case 4:
        return [HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierDistanceCycling];
    case 15:
        if (@available(iOS 18.0, *)) {
            return [HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierDistanceRowing];
        }
        return nil;
    default:
        return nil;
    }
}

bool lockscreen::canWriteHistoricalWorkoutToHealthKit(unsigned short sport) {
    if (![HKHealthStore isHealthDataAvailable]) {
        return false;
    }

    HKHealthStore *healthStore = [[HKHealthStore alloc] init];
    HKWorkoutType *workoutType = [HKObjectType workoutType];
    HKQuantityType *energyType = [HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierActiveEnergyBurned];
    HKQuantityType *distanceType = HistoricalHealthKitDistanceType(sport);

    if ([healthStore authorizationStatusForType:workoutType] != HKAuthorizationStatusSharingAuthorized ||
        !energyType || [healthStore authorizationStatusForType:energyType] != HKAuthorizationStatusSharingAuthorized) {
        return false;
    }

    return !distanceType || [healthStore authorizationStatusForType:distanceType] == HKAuthorizationStatusSharingAuthorized;
}

bool lockscreen::saveHistoricalWorkoutToHealthKit(unsigned short sport, double startTimestamp, double endTimestamp,
                                                   double distanceMeters, double calories) {
    if (endTimestamp <= startTimestamp || !canWriteHistoricalWorkoutToHealthKit(sport)) {
        return false;
    }

    HKHealthStore *healthStore = [[HKHealthStore alloc] init];
    HKWorkoutConfiguration *configuration = [[HKWorkoutConfiguration alloc] init];
    configuration.activityType = HistoricalHealthKitActivityType(sport);
    configuration.locationType = HKWorkoutSessionLocationTypeIndoor;

    HKWorkoutBuilder *builder = [[HKWorkoutBuilder alloc] initWithHealthStore:healthStore
                                                                configuration:configuration
                                                                       device:[HKDevice localDevice]];
    NSDate *startDate = [NSDate dateWithTimeIntervalSince1970:startTimestamp];
    NSDate *endDate = [NSDate dateWithTimeIntervalSince1970:endTimestamp];

    [builder beginCollectionWithStartDate:startDate completion:^(BOOL success, NSError *error) {
        if (!success) {
            qDebug() << "Historical Apple Health workout begin failed:" << error.localizedDescription.UTF8String;
            return;
        }

        NSMutableArray<HKSample *> *samples = [NSMutableArray array];
        HKQuantityType *energyType = [HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierActiveEnergyBurned];
        if (calories > 0 && energyType) {
            HKQuantity *energy = [HKQuantity quantityWithUnit:[HKUnit kilocalorieUnit] doubleValue:calories];
            [samples addObject:[HKQuantitySample quantitySampleWithType:energyType
                                                               quantity:energy
                                                              startDate:startDate
                                                                endDate:endDate]];
        }

        HKQuantityType *distanceType = HistoricalHealthKitDistanceType(sport);
        if (distanceMeters > 0 && distanceType) {
            HKQuantity *distance = [HKQuantity quantityWithUnit:[HKUnit meterUnit] doubleValue:distanceMeters];
            [samples addObject:[HKQuantitySample quantitySampleWithType:distanceType
                                                               quantity:distance
                                                              startDate:startDate
                                                                endDate:endDate]];
        }

        void (^finishWorkout)(void) = ^{
            [builder endCollectionWithEndDate:endDate completion:^(BOOL endSuccess, NSError *endError) {
                if (!endSuccess) {
                    qDebug() << "Historical Apple Health workout end failed:" << endError.localizedDescription.UTF8String;
                    return;
                }
                [builder finishWorkoutWithCompletion:^(HKWorkout *workout, NSError *finishError) {
                    if (finishError) {
                        qDebug() << "Historical Apple Health workout save failed:" << finishError.localizedDescription.UTF8String;
                    } else {
                        qDebug() << "Historical workout saved to Apple Health";
                    }
                }];
            }];
        };

        if (samples.count == 0) {
            finishWorkout();
        } else {
            [builder addSamples:samples completion:^(BOOL addSuccess, NSError *addError) {
                if (!addSuccess) {
                    qDebug() << "Historical Apple Health workout samples failed:" << addError.localizedDescription.UTF8String;
                    return;
                }
                finishWorkout();
            }];
        }
    }];

    return true;
}
#endif

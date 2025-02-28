//
//  SmartControl.h
//
//  Copyright © 2017 Kinetic. All rights reserved.
//

#ifndef SmartControl_h
#define SmartControl_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

static const char SMART_CONTROL_SERVICE_UUID[]         = "E9410200-B434-446B-B5CC-36592FC4C724";
static const char SMART_CONTROL_SERVICE_POWER_UUID[]   = "E9410201-B434-446B-B5CC-36592FC4C724";
static const char SMART_CONTROL_SERVICE_CONFIG_UUID[]  = "E9410202-B434-446B-B5CC-36592FC4C724";
static const char SMART_CONTROL_SERVICE_CONTROL_UUID[] = "E9410203-B434-446B-B5CC-36592FC4C724";

/*! Smart Control Resistance Mode */
typedef enum smart_control_mode
{
    SMART_CONTROL_MODE_ERG          = 0x00,
    SMART_CONTROL_MODE_FLUID        = 0x01,
    SMART_CONTROL_MODE_BRAKE        = 0x02,
    SMART_CONTROL_MODE_SIMULATION   = 0x03
} smart_control_mode;

/*! Smart Control Power Data */
typedef struct smart_control_power_data
{
    /*! Current Resistance Mode */
    smart_control_mode mode;

    /*! Current Power (Watts) */
    uint16_t power;

    /*! Current Speed (KPH) */
    double speedKPH;

    /*! Current Cadence (Virtual RPM) */
    uint8_t cadenceRPM;

    /*! Current wattage the RU is Targetting */
    uint16_t targetResistance;

} smart_control_power_data;

/*!
 Deserialize the raw power data (bytes) broadcast by Smart Control.

 @param data The raw data broadcast from the [Power Service -> Power] Characteristic
 @param size The size of the data array

 @return Smart Control Power Data Struct
 */
smart_control_power_data smart_control_process_power_data(uint8_t *data, size_t size);

/*! Smart Control Calibration State */
typedef enum smart_control_calibration_state
{
    SMART_CONTROL_CALIBRATION_STATE_NOT_PERFORMED       = 0,
    SMART_CONTROL_CALIBRATION_STATE_INITIALIZING        = 1,
    SMART_CONTROL_CALIBRATION_STATE_SPEED_UP            = 2,
    SMART_CONTROL_CALIBRATION_STATE_START_COASTING      = 3,
    SMART_CONTROL_CALIBRATION_STATE_COASTING            = 4,
    SMART_CONTROL_CALIBRATION_STATE_SPEED_UP_DETECTED   = 5,
    SMART_CONTROL_CALIBRATION_STATE_COMPLETE           = 10
} smart_control_calibration_state;

/*! Smart Control Configuration Data */
typedef struct smart_control_config_data
{
    /*! Power Data Update Rate (Hz) */
    uint8_t updateRate;

    /*! Current Calibration State of the RU */
    smart_control_calibration_state calibrationState;

    /*! Current Spindown Time being applied to the Power Data */
    double spindownTime;

    /*! Calibration Speed Threshold (KPH) */
    double calibrationThresholdKPH;

    /*! Brake Calibration Speed Threshold (KPH) */
    double brakeCalibrationThresholdKPH;

    /*! Clock Speed of Data Update (Hz) */
    uint32_t tickRate;

    /*! System Health Status (non-zero indicates problem) */
    uint16_t systemStatus;

    /*! Firmware Update State (Internal Use Only) */
    uint8_t firmwareUpdateState;

    /*! Normalized Brake Strength calculated by a Brake Calibration */
    uint8_t brakeStrength;

    /*! Normalized Brake Offset calculated by a Brake Calibration */
    uint8_t brakeOffset;

    /*! Noise Filter Strength */
    uint8_t noiseFilter;

} smart_control_config_data;

/*!
 Deserialize the raw config data (bytes) broadcast by Smart Control.

 @param data The raw data broadcast from the [Power Service -> Config] Characteristic
 @param size The size of the data array

 @return Smart Control Config Data Struct
 */
smart_control_config_data smart_control_process_config_data(uint8_t *data, size_t size);

/*! Command Structs to write to the Control Point Characteristic */
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct smart_control_set_mode_erg_data
{
    uint8_t bytes[5];
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
smart_control_set_mode_erg_data;

typedef struct smart_control_set_mode_fluid_data
{
    uint8_t bytes[4];
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
smart_control_set_mode_fluid_data;

typedef struct smart_control_set_mode_brake_data
{
    uint8_t bytes[5];
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
smart_control_set_mode_brake_data;

typedef struct smart_control_set_mode_simulation_data
{
    uint8_t bytes[13];
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
smart_control_set_mode_simulation_data;

typedef struct smart_control_calibration_command_data
{
    uint8_t bytes[4];
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
smart_control_calibration_command_data;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

/*!
 Creates the Command to put the Resistance Unit into ERG mode with a target wattage.

 @param targetWatts The target wattage the RU should try to maintain by adjusting the brake position

 @return Write the bytes of the struct to the Control Point Characteristic (w/ response)
 */
smart_control_set_mode_erg_data smart_control_set_mode_erg_command(uint16_t targetWatts);

/*!
 Creates the Command to put the Resistance Unit into a "Fluid" mode, mimicking a fluid trainer.
 This mode is a simplified interface for the Simulation Mode, where:
    Rider + Bike weight is 85kg
    Rolling Coeff is 0.004
    Wind Resistance is 0.60
    Grade is equal to the "level" parameter
    Wind Speed is 0.0

 @param level Difficulty level (0-9) the RU should apply (simulated grade %)

 @return Write the bytes of the struct to the Control Point Characteristic (w/ response)
 */
smart_control_set_mode_fluid_data smart_control_set_mode_fluid_command(uint8_t level);

/*!
 Creates the Command to put the Resistance Unit Brake at a specific position (as a percent).

 @param percent Percent (0-1) of brake resistance to apply.

 @return Write the bytes of the struct to the Control Point Characteristic (w/ response)
 */
smart_control_set_mode_brake_data smart_control_set_mode_brake_command(float percent);

/*!
 Creates the Command to put the Resistance Unit into Simulation mode.

 @param weightKG Weight of Rider and Bike in Kilograms (kg)
 @param rollingCoeff Rolling Resistance Coefficient (0.004 for asphault)
 @param windCoeff Wind Resistance Coeffienct (0.6 default)
 @param grade Grade (-45 to 45) of simulated hill
 @param windSpeedMPS Head or Tail wind speed (meters / second)

 @return Write the bytes of the struct to the Control Point Characteristic (w/ response)
 */
smart_control_set_mode_simulation_data smart_control_set_mode_simulation_command(float weightKG, float rollingCoeff, float windCoeff, float grade, float windSpeedMPS);

/*!
 Creates the Command to start the Calibration Process.

 @param brakeCalibration Calibrates the brake (only needs to be done once, result is stored on unit)

 @return Write the bytes of the struct to the Control Point Characteristic (w/ response)
 */
smart_control_calibration_command_data smart_control_start_calibration_command(bool brakeCalibration);

/*!
 Creates the Command to stop the Calibration Process.
 This is not necessary if the calibration process is allowed to complete.

 @return Write the bytes of the struct to the Control Point Characteristic (w/ response)
 */
smart_control_calibration_command_data smart_control_stop_calibration_command(void);

#endif /* SmartControl_h */

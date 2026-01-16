#include "qfit.h"

#include <QSettings>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <QDir>


#include "fit_date_time.hpp"
#include "fit_encode.hpp"

#include "fit_decode.hpp"
#include "fit_developer_field_description.hpp"
#include "fit_field_description_mesg.hpp"
#include "fit_developer_field.hpp"
#include "fit_mesg_broadcaster.hpp"
#include "fit_timestamp_correlation_mesg.hpp"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#endif

using namespace std;

qfit::qfit(QObject *parent) : QObject(parent) {}

void qfit::save(const QString &filename, QList<SessionLine> session, BLUETOOTH_TYPE type,
                uint32_t processFlag, FIT_SPORT overrideSport, QString workoutName, QString bluetooth_device_name,
                QString workoutSource, QString pelotonWorkoutId, QString pelotonUrl, QString trainingProgramFile) {
    QSettings settings;
    bool strava_virtual_activity =
        settings.value(QZSettings::strava_virtual_activity, QZSettings::default_strava_virtual_activity).toBool();
    bool strava_treadmill =
        settings.value(QZSettings::strava_treadmill, QZSettings::default_strava_treadmill).toBool();
    bool powr_sensor_running_cadence_half_on_strava =
        settings
            .value(QZSettings::powr_sensor_running_cadence_half_on_strava,
                   QZSettings::default_powr_sensor_running_cadence_half_on_strava)
            .toBool();
    std::list<fit::RecordMesg> records;
    fit::Encode encode(fit::ProtocolVersion::V20);
    if (session.isEmpty()) {
        return;
    }
    std::fstream file;
    uint32_t firstRealIndex = 0;
    for (int i = 0; i < session.length(); i++) {
        if ((session.at(i).speed > 0 && (type == TREADMILL || type == ELLIPTICAL)) ||
            (session.at(i).cadence > 0 && (type == BIKE || type == ROWING))) {
            firstRealIndex = i;
            break;
        }
    }
    double startingDistanceOffset = 0.0;
    if (!session.isEmpty()) {
        startingDistanceOffset = session.at(firstRealIndex).distance;
    }

#ifdef _WIN32
    file.open(QString(filename).toLocal8Bit().constData(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
#else
    file.open(filename.toStdString(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
#endif

    if (!file.is_open()) {
        qDebug() << "Error opening file stream";
        return;
    }

    bool fit_file_garmin_device_training_effect = settings.value(QZSettings::fit_file_garmin_device_training_effect, QZSettings::default_fit_file_garmin_device_training_effect).toBool();
    int fit_file_garmin_device_training_effect_device = settings.value(QZSettings::fit_file_garmin_device_training_effect_device, QZSettings::default_fit_file_garmin_device_training_effect_device).toInt();
    uint32_t garmin_device_serial = settings.value(QZSettings::garmin_device_serial, QZSettings::default_garmin_device_serial).toUInt();
    bool is_zwift_device = (fit_file_garmin_device_training_effect_device == 99999);
    bool is_tacx_device = (fit_file_garmin_device_training_effect_device == 88888);
    fit::FileIdMesg fileIdMesg; // Every FIT file requires a File ID message
    fileIdMesg.SetType(FIT_FILE_ACTIVITY);
    if(bluetooth_device_name.toUpper().startsWith("DOMYOS") && !is_zwift_device && !is_tacx_device && !fit_file_garmin_device_training_effect)
        fileIdMesg.SetManufacturer(FIT_MANUFACTURER_DECATHLON);
    else {
        if(is_zwift_device)
            fileIdMesg.SetManufacturer(FIT_MANUFACTURER_ZWIFT);
        else if(is_tacx_device)
            fileIdMesg.SetManufacturer(FIT_MANUFACTURER_TACX);
        else if(fit_file_garmin_device_training_effect)
            fileIdMesg.SetManufacturer(FIT_MANUFACTURER_GARMIN);
        else
            fileIdMesg.SetManufacturer(FIT_MANUFACTURER_DEVELOPMENT);
    }
    if(fit_file_garmin_device_training_effect || is_zwift_device || is_tacx_device) {
        if(is_zwift_device)
            fileIdMesg.SetProduct(3288);
        else if(is_tacx_device)
            fileIdMesg.SetProduct(20533);
        else
            fileIdMesg.SetProduct(fit_file_garmin_device_training_effect_device);
        fileIdMesg.SetSerialNumber(garmin_device_serial);
    } else {
        fileIdMesg.SetProduct(1);
        fileIdMesg.SetSerialNumber(12345);
    }
    fileIdMesg.SetTimeCreated(session.at(firstRealIndex).time.toSecsSinceEpoch() - 631065600L);

    fit::UserProfileMesg userMesg;
    userMesg.SetWeight(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat());
    userMesg.SetAge(settings.value(QZSettings::age, QZSettings::default_age).toUInt());
    userMesg.SetGender(settings.value(QZSettings::sex, QZSettings::default_sex).toString().startsWith(QZSettings::default_sex) ? FIT_GENDER_MALE
                                                                                                   : FIT_GENDER_FEMALE);
    userMesg.SetFriendlyName(
        settings.value(QZSettings::user_nickname, QZSettings::default_user_nickname).toString().toStdWString());

    fit::FileCreatorMesg fileCreatorMesg;
    if(fit_file_garmin_device_training_effect) {
        fileCreatorMesg.SetSoftwareVersion(975);
        fileCreatorMesg.SetHardwareVersion(255);
    } else {
        fileCreatorMesg.SetSoftwareVersion(2119);
    }

    fit::DeviceInfoMesg deviceInfoMesg;
    deviceInfoMesg.SetDeviceIndex(FIT_DEVICE_INDEX_CREATOR);
    if(is_zwift_device) {
        deviceInfoMesg.SetManufacturer(FIT_MANUFACTURER_ZWIFT);
        deviceInfoMesg.SetSerialNumber(garmin_device_serial);
        deviceInfoMesg.SetProduct(3288);
        deviceInfoMesg.SetSoftwareVersion(21.19);
    } else if(is_tacx_device) {
        deviceInfoMesg.SetManufacturer(FIT_MANUFACTURER_TACX);
        deviceInfoMesg.SetSerialNumber(garmin_device_serial);
        deviceInfoMesg.SetProduct(20533);
        deviceInfoMesg.SetSoftwareVersion(1.30);
    } else if(fit_file_garmin_device_training_effect) {
        deviceInfoMesg.SetManufacturer(FIT_MANUFACTURER_GARMIN);
        deviceInfoMesg.SetSerialNumber(garmin_device_serial);
        deviceInfoMesg.SetProduct(fit_file_garmin_device_training_effect_device);
        deviceInfoMesg.SetGarminProduct(fit_file_garmin_device_training_effect_device);
        deviceInfoMesg.SetSoftwareVersion(21.19);
    } else {
        deviceInfoMesg.SetManufacturer(FIT_MANUFACTURER_DEVELOPMENT);
        deviceInfoMesg.SetSerialNumber(12345);
        deviceInfoMesg.SetProduct(1);
        deviceInfoMesg.SetSoftwareVersion(21.19);
    }
    deviceInfoMesg.SetSourceType(FIT_SOURCE_TYPE_LOCAL);

    bool gps_data = false;
    double max_alt = 0;
    double min_alt = 99999;
    double speed_acc = 0;
    int speed_count = 0;
    int lap_index = 0;
    double speed_avg = 0;

    // Variables for training load calculation
    double hr_sum = 0;
    int hr_count = 0;
    uint8_t min_hr = 255;
    uint8_t max_hr = 0;
    double watt_sum = 0;
    int watt_count = 0;

    // Additional statistics variables
    double cadence_sum = 0;
    int cadence_count = 0;
    uint8_t max_cadence = 0;
    double total_cycles = 0;
    double fractional_cadence_sum = 0;

    uint16_t max_watt = 0;
    double total_work = 0;  // Joules
    std::vector<double> power_samples_for_np;  // For normalized power calculation

    double max_speed = 0;
    double altitude_sum = 0;
    int altitude_count = 0;

    // Treadmill specific
    double step_length_sum = 0;
    int step_length_count = 0;
    double vertical_oscillation_sum = 0;
    int vertical_oscillation_count = 0;

    // HR zones for training effect calculation (5 zones)
    double time_in_hr_zone[5] = {0, 0, 0, 0, 0};
    int num_laps_count = 0;

    for (int i = firstRealIndex; i < session.length(); i++) {
        if (session.at(i).coordinate.isValid()) {
            gps_data = true;
            break;
        }
    }
    for (int i = firstRealIndex; i < session.length(); i++) {
        if (gps_data) {
            if (session.at(i).coordinate.isValid()) {
                if (min_alt > session.at(i).coordinate.altitude())
                    min_alt = session.at(i).coordinate.altitude();
                if (max_alt < session.at(i).coordinate.altitude())
                    max_alt = session.at(i).coordinate.altitude();
            }
        } else {
            min_alt = 0;
            if (max_alt < session.at(i).elevationGain)
                max_alt = session.at(i).elevationGain;
        }

        if (session.at(i).speed > 0) {
            speed_count++;
            speed_acc += session.at(i).speed;
            if (session.at(i).speed > max_speed) {
                max_speed = session.at(i).speed;
            }
        }

        // Collect heart rate data for training load
        if (session.at(i).heart > 0) {
            hr_sum += session.at(i).heart;
            hr_count++;
            if (session.at(i).heart < min_hr) {
                min_hr = session.at(i).heart;
            }
            if (session.at(i).heart > max_hr) {
                max_hr = session.at(i).heart;
            }
        }

        // Collect power data for TSS calculation
        if (session.at(i).watt > 0) {
            watt_sum += session.at(i).watt;
            watt_count++;
            if (session.at(i).watt > max_watt) {
                max_watt = session.at(i).watt;
            }
            power_samples_for_np.push_back(session.at(i).watt);
            // total_work in Joules = watts * seconds (assuming 1 second per sample)
            total_work += session.at(i).watt;
        }

        // Collect cadence data
        if (session.at(i).cadence > 0) {
            cadence_sum += session.at(i).cadence;
            cadence_count++;
            if (session.at(i).cadence > max_cadence) {
                max_cadence = session.at(i).cadence;
            }
            fractional_cadence_sum += session.at(i).cadence;
            // total_cycles += cadence / 60.0 (cadence is rpm, we want cycles per second)
            total_cycles += session.at(i).cadence / 60.0;
        }

        // Collect altitude data (if not from GPS)
        if (!gps_data && session.at(i).elevationGain > 0) {
            altitude_sum += session.at(i).elevationGain;
            altitude_count++;
        }

        // Collect treadmill-specific data
        if (type == TREADMILL) {
            if (session.at(i).instantaneousStrideLengthCM > 0) {
                step_length_sum += session.at(i).instantaneousStrideLengthCM;
                step_length_count++;
            }
            if (session.at(i).verticalOscillationMM > 0) {
                vertical_oscillation_sum += session.at(i).verticalOscillationMM;
                vertical_oscillation_count++;
            }
        }

        // Count laps
        if (session.at(i).lapTrigger) {
            num_laps_count++;
        }
    }

    if (speed_count > 0) {
        speed_avg = speed_acc / ((double)speed_count);
        qDebug() << "average speed from the fit file" << speed_avg;
    }

    // Calculate additional statistics
    double avg_hr = 0;
    if (hr_count > 0) {
        avg_hr = hr_sum / hr_count;
    }

    double avg_cadence = 0;
    double avg_fractional_cadence = 0;
    if (cadence_count > 0) {
        avg_cadence = cadence_sum / cadence_count;
        avg_fractional_cadence = fractional_cadence_sum / cadence_count;
    }

    double avg_watt = 0;
    if (watt_count > 0) {
        avg_watt = watt_sum / watt_count;
    }

    // Calculate Normalized Power (NP) using 30-second rolling average
    uint16_t normalized_power = 0;
    if (power_samples_for_np.size() >= 30) {
        std::vector<double> rolling_avg_30s;
        for (size_t i = 0; i < power_samples_for_np.size(); i++) {
            double sum = 0;
            int count = 0;
            for (size_t j = (i >= 29 ? i - 29 : 0); j <= i; j++) {
                sum += power_samples_for_np[j];
                count++;
            }
            rolling_avg_30s.push_back(sum / count);
        }

        // NP = fourth root of average of fourth powers
        double sum_fourth_power = 0;
        for (double avg : rolling_avg_30s) {
            sum_fourth_power += std::pow(avg, 4);
        }
        normalized_power = static_cast<uint16_t>(std::pow(sum_fourth_power / rolling_avg_30s.size(), 0.25));
        qDebug() << "Normalized Power calculated:" << normalized_power << "W";
    }

    double avg_altitude = 0;
    if (altitude_count > 0) {
        avg_altitude = altitude_sum / altitude_count;
    }

    double avg_step_length = 0;
    if (step_length_count > 0) {
        avg_step_length = step_length_sum / step_length_count;  // in cm
    }

    double avg_vertical_ratio = 0;
    if (vertical_oscillation_count > 0 && step_length_count > 0) {
        double avg_vertical_oscillation = vertical_oscillation_sum / vertical_oscillation_count;  // in mm
        // Vertical ratio = vertical oscillation / step length (both need same units)
        // avg_step_length is in cm, avg_vertical_oscillation is in mm
        if (avg_step_length > 0) {
            avg_vertical_ratio = (avg_vertical_oscillation / 10.0) / avg_step_length;  // convert mm to cm
        }
    }

    // Calculate training load: TSS for cycling with power, TRIMP otherwise
    float training_load = 0.0f;
    float tss = 0.0f;  // Training Stress Score (for cycling with power)
    uint32_t duration_seconds = session.last().elapsedTime;
    bool has_tss = false;

    // For cycling with power data, calculate TSS (Training Stress Score)
    if (type == BIKE && watt_count > 0) {
        double avg_watt = watt_sum / watt_count;
        float ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toFloat();

        if (ftp > 0 && avg_watt > 0) {
            // TSS formula: (duration_seconds × average_power × IF) / (FTP × 36)
            // where IF (Intensity Factor) = average_power / FTP
            double intensity_factor = avg_watt / ftp;
            tss = (duration_seconds * avg_watt * intensity_factor) / (ftp * 36.0);
            training_load = tss;  // Use TSS as training load in the worst scenario
            has_tss = true;

            qDebug() << "Training Load (TSS) calculated:" << tss
                     << "Duration:" << (duration_seconds / 60) << "min"
                     << "Avg Power:" << avg_watt << "W"
                     << "FTP:" << ftp << "W"
                     << "IF:" << intensity_factor;
        }
    }

    // Always calculate TRIMP if we have HR data (fallback or additional metric)
    if (hr_count > 0) {
        double avg_hr = hr_sum / hr_count;
        uint32_t duration_minutes = duration_seconds / 60;

        // Get max HR: use override if enabled, otherwise calculate from age
        bool max_hr_override_enabled = settings.value(QZSettings::heart_max_override_enable,
                                                       QZSettings::default_heart_max_override_enable).toBool();
        uint8_t max_hr;
        if (max_hr_override_enabled) {
            max_hr = settings.value(QZSettings::heart_max_override_value,
                                   QZSettings::default_heart_max_override_value).toUInt();
        } else {
            uint8_t user_age = settings.value(QZSettings::age, QZSettings::default_age).toUInt();
            max_hr = 220 - user_age;
        }

        // Get resting HR from settings
        uint8_t resting_hr = settings.value(QZSettings::heart_rate_resting,
                                            QZSettings::default_heart_rate_resting).toUInt();

        // Bannister's TRIMP formula: D * HR_ratio * exp(b * HR_ratio)
        // where HR_ratio = (avg_hr - resting_hr) / (max_hr - resting_hr)
        //
        // COEFFICIENT SELECTION:
        // Standard Bannister formula uses b = 1.92 (men) and b = 1.67 (women)
        // However, Garmin devices (Fenix, etc.) appear to use b ≈ 1.67 for all users
        // to match Garmin's training load calculations more closely.
        // We use b = 1.67 for everyone to ensure compatibility with Garmin Connect's
        // acute training load and training status features.
        double hr_ratio = 0;
        if (max_hr > resting_hr) {
            hr_ratio = (avg_hr - resting_hr) / (double)(max_hr - resting_hr);
        }

        // Use coefficient 1.67 (matches Garmin implementation)
        double b = 1.67;

        // Calculate TRIMP
        if (hr_ratio > 0 && hr_ratio < 2.0) {  // Sanity check
            training_load = duration_minutes * hr_ratio * std::exp(b * hr_ratio);
            qDebug() << "Training Load (TRIMP) calculated:" << training_load
                     << "Duration:" << duration_minutes << "min"
                     << "Avg HR:" << avg_hr
                     << "Max HR:" << max_hr << (max_hr_override_enabled ? "(override)" : "(calculated)")
                     << "Resting HR:" << resting_hr;
        }
    }

    // Calculate Training Effect (Aerobic and Anaerobic)
    float aerobic_training_effect = 0.0f;
    float anaerobic_training_effect = 0.0f;

    // Get max HR for zone calculation (same logic as TRIMP)
    bool max_hr_override_enabled = settings.value(QZSettings::heart_max_override_enable,
                                                   QZSettings::default_heart_max_override_enable).toBool();
    uint8_t user_max_hr;
    if (max_hr_override_enabled) {
        user_max_hr = settings.value(QZSettings::heart_max_override_value,
                               QZSettings::default_heart_max_override_value).toUInt();
    } else {
        uint8_t user_age = settings.value(QZSettings::age, QZSettings::default_age).toUInt();
        user_max_hr = 220 - user_age;
    }
    uint8_t resting_hr = settings.value(QZSettings::heart_rate_resting,
                                        QZSettings::default_heart_rate_resting).toUInt();

    // Calculate Aerobic Training Effect based on HR zones
    if (hr_count > 0 && user_max_hr > resting_hr) {
        // Define HR zones (5 zones based on % of HRR - Heart Rate Reserve)
        // HRR = max_hr - resting_hr
        // Zone 1: 50-60% HRR
        // Zone 2: 60-70% HRR
        // Zone 3: 70-80% HRR
        // Zone 4: 80-90% HRR
        // Zone 5: 90-100% HRR

        double hrr = user_max_hr - resting_hr;

        // Calculate time in each zone
        for (int i = firstRealIndex; i < session.length(); i++) {
            if (session.at(i).heart > 0) {
                double hr_percent = (session.at(i).heart - resting_hr) / hrr;

                if (hr_percent < 0.60) {
                    time_in_hr_zone[0] += 1.0;  // Zone 1
                } else if (hr_percent < 0.70) {
                    time_in_hr_zone[1] += 1.0;  // Zone 2
                } else if (hr_percent < 0.80) {
                    time_in_hr_zone[2] += 1.0;  // Zone 3
                } else if (hr_percent < 0.90) {
                    time_in_hr_zone[3] += 1.0;  // Zone 4
                } else {
                    time_in_hr_zone[4] += 1.0;  // Zone 5
                }
            }
        }

        // Calculate Aerobic TE using zone-based formula
        // Weights based on Firstbeat/Garmin research (approximation)
        double zone_weights[5] = {0.2, 0.5, 1.0, 1.5, 2.0};
        double weighted_time = 0;
        for (int z = 0; z < 5; z++) {
            weighted_time += (time_in_hr_zone[z] / 60.0) * zone_weights[z];  // convert to minutes
        }

        double duration_minutes = duration_seconds / 60.0;
        if (duration_minutes > 0) {
            // Scale to 0-5.0 range (Garmin scale)
            aerobic_training_effect = (weighted_time / duration_minutes) * 2.0;

            // Cap at 5.0 (max Garmin scale)
            if (aerobic_training_effect > 5.0) {
                aerobic_training_effect = 5.0;
            }

            qDebug() << "Aerobic Training Effect calculated:" << aerobic_training_effect
                     << "Duration:" << duration_minutes << "min"
                     << "Avg HR:" << avg_hr << "Max HR:" << user_max_hr;
        }
    }

    // Calculate Anaerobic Training Effect
    if (watt_count > 0) {
        // For activities with power data
        float ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toFloat();

        if (ftp > 0 && avg_watt > 0) {
            // Count time above anaerobic threshold (typically ~105% FTP)
            double anaerobic_threshold = ftp * 1.05;
            double time_above_threshold = 0;
            double intensity_above_threshold = 0;

            for (int i = firstRealIndex; i < session.length(); i++) {
                if (session.at(i).watt > anaerobic_threshold) {
                    time_above_threshold += 1.0;  // seconds
                    // Weight by intensity above threshold
                    double intensity_factor = session.at(i).watt / anaerobic_threshold;
                    intensity_above_threshold += intensity_factor;
                }
            }

            // Calculate anaerobic TE
            double duration_minutes = duration_seconds / 60.0;
            if (duration_minutes > 0 && time_above_threshold > 0) {
                double avg_intensity = intensity_above_threshold / time_above_threshold;
                double percent_time_above = (time_above_threshold / duration_seconds) * 100.0;

                // Scale based on time above threshold and intensity
                anaerobic_training_effect = (percent_time_above / 20.0) * avg_intensity;

                // Cap at 5.0
                if (anaerobic_training_effect > 5.0) {
                    anaerobic_training_effect = 5.0;
                }

                qDebug() << "Anaerobic Training Effect calculated:" << anaerobic_training_effect
                         << "Time above threshold:" << (time_above_threshold / 60.0) << "min"
                         << "Avg intensity:" << avg_intensity;
            }
        }
    } else if (hr_count > 0 && user_max_hr > resting_hr) {
        // For activities without power (e.g., running), use HR-based anaerobic estimate
        // Time in high intensity zones (Zone 4 and 5)
        double high_intensity_time = time_in_hr_zone[3] + time_in_hr_zone[4];  // seconds
        double duration_minutes = duration_seconds / 60.0;

        if (duration_minutes > 0 && high_intensity_time > 0) {
            double percent_high_intensity = (high_intensity_time / duration_seconds) * 100.0;

            // Scale to 0-5.0 range
            anaerobic_training_effect = (percent_high_intensity / 25.0) * 2.0;

            // Boost if mostly in Zone 5
            if (time_in_hr_zone[4] > time_in_hr_zone[3]) {
                anaerobic_training_effect *= 1.3;
            }

            // Cap at 5.0
            if (anaerobic_training_effect > 5.0) {
                anaerobic_training_effect = 5.0;
            }

            qDebug() << "Anaerobic Training Effect (HR-based) calculated:" << anaerobic_training_effect
                     << "High intensity time:" << (high_intensity_time / 60.0) << "min";
        }
    }

    encode.Open(file);
    encode.Write(fileIdMesg);
    encode.Write(userMesg);

    // Declare developer field descriptions (but don't write them yet)
    fit::FieldDescriptionMesg activityTitle;
    activityTitle.SetDeveloperDataIndex(0);
    activityTitle.SetFieldDefinitionNumber(0);
    activityTitle.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    activityTitle.SetFieldName(0, L"Activity Title");
    activityTitle.SetUnits(0, L"Title");
    activityTitle.SetNativeMesgNum(FIT_MESG_NUM_SESSION);

    fit::FieldDescriptionMesg targetCadenceMesg;
    targetCadenceMesg.SetDeveloperDataIndex(0);
    targetCadenceMesg.SetFieldDefinitionNumber(1);
    targetCadenceMesg.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT64);
    targetCadenceMesg.SetFieldName(0, L"Target Cadence");
    targetCadenceMesg.SetUnits(0, L"rpm");
    targetCadenceMesg.SetNativeMesgNum(FIT_MESG_NUM_RECORD);

    fit::FieldDescriptionMesg targetWattMesg;
    targetWattMesg.SetDeveloperDataIndex(0);
    targetWattMesg.SetFieldDefinitionNumber(2);
    targetWattMesg.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT64);
    targetWattMesg.SetFieldName(0, L"Target Watt");
    targetWattMesg.SetUnits(0, L"watts");
    targetWattMesg.SetNativeMesgNum(FIT_MESG_NUM_RECORD);

    fit::FieldDescriptionMesg targetResistanceMesg;
    targetResistanceMesg.SetDeveloperDataIndex(0);
    targetResistanceMesg.SetFieldDefinitionNumber(3);
    targetResistanceMesg.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT64);
    targetResistanceMesg.SetFieldName(0, L"Target Resistance");
    targetResistanceMesg.SetUnits(0, L"resistance");
    targetResistanceMesg.SetNativeMesgNum(FIT_MESG_NUM_RECORD);

    fit::FieldDescriptionMesg ftpSessionMesg;
    ftpSessionMesg.SetDeveloperDataIndex(0);
    ftpSessionMesg.SetFieldDefinitionNumber(4);
    ftpSessionMesg.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT64);
    ftpSessionMesg.SetFieldName(0, L"FTP");
    ftpSessionMesg.SetUnits(0, L"FTP");
    ftpSessionMesg.SetNativeMesgNum(FIT_MESG_NUM_SESSION);

    // Peloton and workout source fields
    fit::FieldDescriptionMesg workoutSourceMesg;
    workoutSourceMesg.SetDeveloperDataIndex(0);
    workoutSourceMesg.SetFieldDefinitionNumber(8);
    workoutSourceMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    workoutSourceMesg.SetFieldName(0, L"Workout Source");
    workoutSourceMesg.SetUnits(0, L"source");
    workoutSourceMesg.SetNativeMesgNum(FIT_MESG_NUM_SESSION);

    fit::FieldDescriptionMesg pelotonWorkoutIdMesg;
    pelotonWorkoutIdMesg.SetDeveloperDataIndex(0);
    pelotonWorkoutIdMesg.SetFieldDefinitionNumber(9);
    pelotonWorkoutIdMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    pelotonWorkoutIdMesg.SetFieldName(0, L"Peloton Workout ID");
    pelotonWorkoutIdMesg.SetUnits(0, L"id");
    pelotonWorkoutIdMesg.SetNativeMesgNum(FIT_MESG_NUM_SESSION);

    fit::FieldDescriptionMesg pelotonUrlMesg;
    pelotonUrlMesg.SetDeveloperDataIndex(0);
    pelotonUrlMesg.SetFieldDefinitionNumber(10);
    pelotonUrlMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    pelotonUrlMesg.SetFieldName(0, L"Peloton URL");
    pelotonUrlMesg.SetUnits(0, L"url");
    pelotonUrlMesg.SetNativeMesgNum(FIT_MESG_NUM_SESSION);

    fit::FieldDescriptionMesg trainingProgramFileMesg;
    trainingProgramFileMesg.SetDeveloperDataIndex(0);
    trainingProgramFileMesg.SetFieldDefinitionNumber(11);
    trainingProgramFileMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    trainingProgramFileMesg.SetFieldName(0, L"Training Program File");
    trainingProgramFileMesg.SetUnits(0, L"filename");
    trainingProgramFileMesg.SetNativeMesgNum(FIT_MESG_NUM_SESSION);

    fit::SessionMesg sessionMesg;
    sessionMesg.SetTimestamp(session.at(firstRealIndex).time.toSecsSinceEpoch() - 631065600L);
    sessionMesg.SetStartTime(session.at(firstRealIndex).time.toSecsSinceEpoch() - 631065600L);
    sessionMesg.SetTotalElapsedTime(session.last().elapsedTime);
    sessionMesg.SetTotalTimerTime(session.last().elapsedTime);
    sessionMesg.SetTotalDistance((session.last().distance - startingDistanceOffset) * 1000.0); // meters
    sessionMesg.SetTotalCalories(session.last().calories);
    sessionMesg.SetTotalMovingTime(session.last().elapsedTime);
    sessionMesg.SetTotalAscent(session.last().elevationGain);  // Total elevation gain (meters)
    sessionMesg.SetTotalDescent(session.last().negativeElevationGain);  // Total elevation loss/descent (meters)
    sessionMesg.SetMinAltitude(min_alt);
    sessionMesg.SetMaxAltitude(max_alt);
    sessionMesg.SetEvent(FIT_EVENT_SESSION);
    sessionMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    sessionMesg.SetFirstLapIndex(0);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);
    sessionMesg.SetMessageIndex(FIT_MESSAGE_INDEX_RESERVED);

    // Set training load in FIT file
    // Always set training_load_peak (Garmin uses this for acute training load)
    if (training_load > 0) {
        sessionMesg.SetTrainingLoadPeak(training_load);
        qDebug() << "Setting training_load_peak in FIT file:" << training_load;
    }
    
    // For cycling with power, also set training_stress_score (TSS)
    if (has_tss) {
        sessionMesg.SetTrainingStressScore(tss);
        qDebug() << "Setting training_stress_score (TSS) in FIT file:" << tss;
    }

    // Add all new session statistics (only if > 0)

    // Heart Rate statistics
    if (hr_count > 0) {
        sessionMesg.SetAvgHeartRate(static_cast<uint8_t>(avg_hr));
        qDebug() << "Setting avg_heart_rate:" << static_cast<uint8_t>(avg_hr);
    }
    if (max_hr > 0) {
        sessionMesg.SetMaxHeartRate(max_hr);
        qDebug() << "Setting max_heart_rate:" << max_hr;
    }
    if (min_hr < 255) {
        sessionMesg.SetMinHeartRate(min_hr);
        qDebug() << "Setting min_heart_rate:" << min_hr;
    }

    // Cadence statistics (skip for ROWING, which uses stroke-specific fields)
    if (cadence_count > 0 && type != ROWING) {
        sessionMesg.SetAvgCadence(static_cast<uint8_t>(avg_cadence));
        qDebug() << "Setting avg_cadence:" << static_cast<uint8_t>(avg_cadence);

        if (avg_fractional_cadence > 0) {
            sessionMesg.SetAvgFractionalCadence(static_cast<float>(avg_fractional_cadence));
            qDebug() << "Setting avg_fractional_cadence:" << static_cast<float>(avg_fractional_cadence);
        }
    }
    if (max_cadence > 0 && type != ROWING) {
        sessionMesg.SetMaxCadence(max_cadence);
        sessionMesg.SetMaxFractionalCadence(static_cast<float>(max_cadence));
        qDebug() << "Setting max_cadence:" << max_cadence;
    }
    if (total_cycles > 0) {
        sessionMesg.SetTotalCycles(static_cast<uint32_t>(total_cycles));
        qDebug() << "Setting total_cycles:" << static_cast<uint32_t>(total_cycles);
    }

    // Power statistics (for all device types with power data)
    if (watt_count > 0) {
        sessionMesg.SetAvgPower(static_cast<uint16_t>(avg_watt));
        qDebug() << "Setting avg_power:" << static_cast<uint16_t>(avg_watt);
    }
    if (max_watt > 0) {
        sessionMesg.SetMaxPower(max_watt);
        qDebug() << "Setting max_power:" << max_watt;
    }
    if (normalized_power > 0) {
        sessionMesg.SetNormalizedPower(normalized_power);
        qDebug() << "Setting normalized_power:" << normalized_power;
    }
    if (total_work > 0) {
        sessionMesg.SetTotalWork(static_cast<uint32_t>(total_work));
        qDebug() << "Setting total_work:" << static_cast<uint32_t>(total_work) << "J";
    }

    // Speed statistics (enhanced)
    if (speed_avg > 0) {
        sessionMesg.SetEnhancedAvgSpeed(static_cast<float>(speed_avg / 3.6));  // convert km/h to m/s
        qDebug() << "Setting enhanced_avg_speed:" << static_cast<float>(speed_avg / 3.6) << "m/s";
    }
    if (max_speed > 0) {
        sessionMesg.SetEnhancedMaxSpeed(static_cast<float>(max_speed / 3.6));  // convert km/h to m/s
        qDebug() << "Setting enhanced_max_speed:" << static_cast<float>(max_speed / 3.6) << "m/s";
    }

    // Altitude statistics (if not GPS)
    if (!gps_data && avg_altitude > 0) {
        sessionMesg.SetEnhancedAvgAltitude(static_cast<float>(avg_altitude));
        qDebug() << "Setting enhanced_avg_altitude:" << static_cast<float>(avg_altitude) << "m";
    }

    // Treadmill-specific statistics
    if (type == TREADMILL) {
        if (avg_step_length > 0) {
            // Convert cm to mm (FIT expects mm)
            sessionMesg.SetAvgStepLength(static_cast<float>(avg_step_length * 10.0));
            qDebug() << "Setting avg_step_length:" << static_cast<float>(avg_step_length * 10.0) << "mm";
        }
        if (avg_vertical_ratio > 0) {
            sessionMesg.SetAvgVerticalRatio(static_cast<float>(avg_vertical_ratio));
            qDebug() << "Setting avg_vertical_ratio:" << static_cast<float>(avg_vertical_ratio);
        }
    }

    // Training Effects
    if (aerobic_training_effect > 0) {
        sessionMesg.SetTotalTrainingEffect(aerobic_training_effect);
        qDebug() << "Setting total_training_effect (aerobic):" << aerobic_training_effect;
    }
    if (anaerobic_training_effect > 0) {
        sessionMesg.SetTotalAnaerobicTrainingEffect(anaerobic_training_effect);
        qDebug() << "Setting total_anaerobic_training_effect:" << anaerobic_training_effect;
    }

    // Lap count
    if (num_laps_count > 0) {
        sessionMesg.SetNumLaps(num_laps_count + 1);  // +1 for the final lap
        qDebug() << "Setting num_laps:" << (num_laps_count + 1);
    }

    // Sport profile name (based on device type and sport)
    std::wstring sport_profile_name;
    if (type == TREADMILL) {
        sport_profile_name = L"Treadmill Running";
    } else if (type == BIKE) {
        sport_profile_name = L"Indoor Cycling";
    } else if (type == ROWING) {
        sport_profile_name = L"Indoor Rowing";
    } else if (type == ELLIPTICAL) {
        sport_profile_name = L"Elliptical";
    } else if (type == STAIRCLIMBER) {
        sport_profile_name = L"Stair Climber";
    } else if (type == JUMPROPE) {
        sport_profile_name = L"Jump Rope";
    }

    if (!sport_profile_name.empty()) {
        sessionMesg.SetSportProfileName(sport_profile_name);
        qDebug() << "Setting sport_profile_name:" << QString::fromStdWString(sport_profile_name);
    }

    // First, set sport and subsport based on device type
    if (type == TREADMILL) {
        if(session.last().stepCount > 0)
            sessionMesg.SetTotalStrides(session.last().stepCount);

        if (speed_avg == 0 || speed_avg > 6.5 || strava_virtual_activity)
            sessionMesg.SetSport(FIT_SPORT_RUNNING);
        else
            sessionMesg.SetSport(FIT_SPORT_WALKING);

        if (strava_virtual_activity) {
            sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
        } else {
            if(strava_treadmill)
                sessionMesg.SetSubSport(FIT_SUB_SPORT_TREADMILL);
        }
    } else if (type == ELLIPTICAL) {
        if (strava_virtual_activity) {
            if (speed_avg == 0 || speed_avg > 6.5)
                sessionMesg.SetSport(FIT_SPORT_RUNNING);
            else
                sessionMesg.SetSport(FIT_SPORT_WALKING);
            sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
        } else {
            sessionMesg.SetSport(FIT_SPORT_FITNESS_EQUIPMENT);
            sessionMesg.SetSubSport(FIT_SUB_SPORT_ELLIPTICAL);
        }
    } else if (type == ROWING) {

        sessionMesg.SetSport(FIT_SPORT_ROWING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_ROWING);
        if (session.last().totalStrokes)
            sessionMesg.SetTotalStrokes(session.last().totalStrokes);
        if (session.last().avgStrokesRate)
            sessionMesg.SetAvgStrokeCount(session.last().avgStrokesRate);
        if (session.last().maxStrokesRate)
            sessionMesg.SetMaxCadence(session.last().maxStrokesRate);
        if (session.last().avgStrokesLength)
            sessionMesg.SetAvgStrokeDistance(session.last().avgStrokesLength);
    } else if (type == STAIRCLIMBER) {

        sessionMesg.SetSport(FIT_SPORT_GENERIC);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_STAIR_CLIMBING);
    } else if (type == JUMPROPE) {

        sessionMesg.SetSport(FIT_SPORT_JUMP_ROPE);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_GENERIC);
        if (session.last().stepCount)
            sessionMesg.SetJumpCount(session.last().stepCount);
    } else {

        sessionMesg.SetSport(FIT_SPORT_CYCLING);
        if (strava_virtual_activity) {
            sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
        }
    }

    // Then, override the sport if requested (keeping the subsport from above)
    if (overrideSport != FIT_SPORT_INVALID) {
        sessionMesg.SetSport(overrideSport);
        qDebug() << "overriding FIT sport to" << overrideSport << "keeping subsport from device type";
    }

    fit::DeveloperDataIdMesg devIdMesg;
    // QZ companion app
    // 3746ce54-a9e9-42b0-9be9-b867f8d20f7d
    // Core App
    // 6957fe68-83fe-4ed6-8613-413f70624bb5
    devIdMesg.SetApplicationId(0, 0x37);
    devIdMesg.SetApplicationId(1, 0x46);
    devIdMesg.SetApplicationId(2, 0xce);
    devIdMesg.SetApplicationId(3, 0x54);
    devIdMesg.SetApplicationId(4, 0xa9);
    devIdMesg.SetApplicationId(5, 0xe9);
    devIdMesg.SetApplicationId(6, 0x42);
    devIdMesg.SetApplicationId(7, 0xb0);
    devIdMesg.SetApplicationId(8, 0x9b);
    devIdMesg.SetApplicationId(9, 0xe9);
    devIdMesg.SetApplicationId(10, 0xb8);
    devIdMesg.SetApplicationId(11, 0x67);
    devIdMesg.SetApplicationId(12, 0xf8);
    devIdMesg.SetApplicationId(13, 0xd2);
    devIdMesg.SetApplicationId(14, 0x0f);
    devIdMesg.SetApplicationId(15, 0x7d);
    devIdMesg.SetDeveloperDataIndex(0);
    devIdMesg.SetApplicationVersion(70);

           // Create developer field descriptions for custom temperature fields
    fit::FieldDescriptionMesg coreTemperatureFieldDesc;
    coreTemperatureFieldDesc.SetDeveloperDataIndex(0);
    coreTemperatureFieldDesc.SetFieldDefinitionNumber(5);
    coreTemperatureFieldDesc.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT32);
    coreTemperatureFieldDesc.SetFieldName(0, L"core_temperature");
    coreTemperatureFieldDesc.SetUnits(0, L"°C");
        coreTemperatureFieldDesc.SetNativeMesgNum(FIT_MESG_NUM_RECORD);
    coreTemperatureFieldDesc.SetNativeFieldNum(139);

    fit::FieldDescriptionMesg skinTemperatureFieldDesc;
    skinTemperatureFieldDesc.SetDeveloperDataIndex(0);
    skinTemperatureFieldDesc.SetFieldDefinitionNumber(6);
    skinTemperatureFieldDesc.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT32);
    skinTemperatureFieldDesc.SetFieldName(0, L"skin_temperature");
    skinTemperatureFieldDesc.SetUnits(0, L"°C");
        skinTemperatureFieldDesc.SetNativeMesgNum(FIT_MESG_NUM_RECORD);
    skinTemperatureFieldDesc.SetNativeFieldNum(255); // Use invalid field number to indicate custom field

    fit::FieldDescriptionMesg heatStrainIndexFieldDesc;
    heatStrainIndexFieldDesc.SetDeveloperDataIndex(0);
    heatStrainIndexFieldDesc.SetFieldDefinitionNumber(7);
    heatStrainIndexFieldDesc.SetFitBaseTypeId(FIT_BASE_TYPE_FLOAT32);
    heatStrainIndexFieldDesc.SetFieldName(0, L"heat_strain_index");
    heatStrainIndexFieldDesc.SetUnits(0, L"a.u.");
    heatStrainIndexFieldDesc.SetNativeMesgNum(FIT_MESG_NUM_RECORD);
    heatStrainIndexFieldDesc.SetNativeFieldNum(255); // Use invalid field number to indicate custom field

    fit::DeveloperField ftpSessionField(ftpSessionMesg, devIdMesg);
    ftpSessionField.AddValue(settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble());

    fit::DeveloperField activityTitleField(activityTitle, devIdMesg);
    activityTitleField.SetSTRINGValue(workoutName.toStdWString());

    // Create our new developer fields
    fit::DeveloperField workoutSourceField(workoutSourceMesg, devIdMesg);
    if (!workoutSource.isEmpty()) {
        workoutSourceField.SetSTRINGValue(workoutSource.toStdWString());
    } else {
        workoutSourceField.SetSTRINGValue(L"QZ"); // Default to QZ if not specified
    }

    fit::DeveloperField pelotonWorkoutIdField(pelotonWorkoutIdMesg, devIdMesg);
    if (!pelotonWorkoutId.isEmpty()) {
        pelotonWorkoutIdField.SetSTRINGValue(pelotonWorkoutId.toStdWString());
    }

    fit::DeveloperField pelotonUrlField(pelotonUrlMesg, devIdMesg);
    if (!pelotonUrl.isEmpty()) {
        pelotonUrlField.SetSTRINGValue(pelotonUrl.toStdWString());
    }

    fit::DeveloperField trainingProgramFileField(trainingProgramFileMesg, devIdMesg);
    if (!trainingProgramFile.isEmpty()) {
        trainingProgramFileField.SetSTRINGValue(trainingProgramFile.toStdWString());
    }

    sessionMesg.AddDeveloperField(activityTitleField);
    sessionMesg.AddDeveloperField(ftpSessionField);
    sessionMesg.AddDeveloperField(workoutSourceField);
    if (!pelotonWorkoutId.isEmpty()) {
        sessionMesg.AddDeveloperField(pelotonWorkoutIdField);
    }
    if (!pelotonUrl.isEmpty()) {
        sessionMesg.AddDeveloperField(pelotonUrlField);
    }
    if (!trainingProgramFile.isEmpty()) {
        sessionMesg.AddDeveloperField(trainingProgramFileField);
    }

    fit::ActivityMesg activityMesg;
    activityMesg.SetTimestamp(session.at(firstRealIndex).time.toSecsSinceEpoch() - 631065600L);
    activityMesg.SetTotalTimerTime(session.last().elapsedTime);
    activityMesg.SetNumSessions(1);
    activityMesg.SetType(FIT_ACTIVITY_MANUAL);
    activityMesg.SetEvent(FIT_EVENT_WORKOUT);
    activityMesg.SetEventType(FIT_EVENT_TYPE_START);
    activityMesg.SetLocalTimestamp(fit::DateTime((time_t)session.last().time.toSecsSinceEpoch())
                                       .GetTimeStamp()); // seconds since 00:00 Dec d31 1989 in local time zone
    activityMesg.SetEvent(FIT_EVENT_ACTIVITY);
    activityMesg.SetEventType(FIT_EVENT_TYPE_STOP);

    fit::EventMesg eventMesg;
    eventMesg.SetEvent(FIT_EVENT_TIMER);
    eventMesg.SetEventType(FIT_EVENT_TYPE_START);
    eventMesg.SetData(0);
    eventMesg.SetEventGroup(0);
    eventMesg.SetTimestamp(session.at(firstRealIndex).time.toSecsSinceEpoch() - 631065600L);
    encode.Write(fileCreatorMesg);
    encode.Write(devIdMesg);
    
    // Write developer field descriptions (declared earlier)
    encode.Write(activityTitle);
    encode.Write(targetCadenceMesg);
    encode.Write(targetWattMesg);
    encode.Write(targetResistanceMesg);
    encode.Write(ftpSessionMesg);
    encode.Write(workoutSourceMesg);
    encode.Write(pelotonWorkoutIdMesg);
    encode.Write(pelotonUrlMesg);
    encode.Write(trainingProgramFileMesg);
    
    encode.Write(coreTemperatureFieldDesc);
    encode.Write(skinTemperatureFieldDesc);
    encode.Write(heatStrainIndexFieldDesc);
    encode.Write(deviceInfoMesg);

    // Add Timestamp Correlation record
    // This correlates the UTC timestamp with the system timestamp and local timestamp
    fit::TimestampCorrelationMesg timestampCorrelationMesg;
    FIT_DATE_TIME sessionStartTimestamp = session.at(firstRealIndex).time.toSecsSinceEpoch() - 631065600L;

    // Timestamp: UTC timestamp at session start
    timestampCorrelationMesg.SetTimestamp(sessionStartTimestamp);

    // System Timestamp: Same as timestamp (session start)
    timestampCorrelationMesg.SetSystemTimestamp(sessionStartTimestamp);

    // Local Timestamp: User's local time at session start
    // Convert the local time to FIT format
    fit::DateTime localDateTime((time_t)session.at(firstRealIndex).time.toSecsSinceEpoch());
    timestampCorrelationMesg.SetLocalTimestamp(localDateTime.GetTimeStamp());

    encode.Write(timestampCorrelationMesg);

    if (workoutName.length() > 0) {
        fit::TrainingFileMesg trainingFile;
        trainingFile.SetTimestamp(sessionMesg.GetTimestamp());
        trainingFile.SetTimeCreated(sessionMesg.GetTimestamp());
        trainingFile.SetType(FIT_FILE_WORKOUT);
        encode.Write(trainingFile);

        fit::WorkoutMesg workout;
        workout.SetSport(sessionMesg.GetSport());
        workout.SetSubSport(sessionMesg.GetSubSport());
#ifndef _WIN32
        workout.SetWktName(workoutName.toStdWString());
#endif
        workout.SetNumValidSteps(1);
        encode.Write(workout);

        fit::WorkoutStepMesg workoutStep;
        workoutStep.SetDurationTime(sessionMesg.GetTotalTimerTime());
        workoutStep.SetTargetValue(0);
        workoutStep.SetCustomTargetValueHigh(0);
        workoutStep.SetCustomTargetValueLow(0);
        workoutStep.SetMessageIndex(0);
        workoutStep.SetDurationType(FIT_WKT_STEP_DURATION_TIME);
        workoutStep.SetTargetType(FIT_WKT_STEP_TARGET_SPEED);
        workoutStep.SetIntensity(FIT_INTENSITY_INTERVAL);
        encode.Write(workoutStep);
    }

    encode.Write(eventMesg);

    fit::DateTime date((time_t)session.first().time.toSecsSinceEpoch());

    fit::LapMesg lapMesg;
    lapMesg.SetIntensity(FIT_INTENSITY_ACTIVE);
    lapMesg.SetStartTime(date.GetTimeStamp() + firstRealIndex);
    lapMesg.SetTimestamp(date.GetTimeStamp() + firstRealIndex);
    lapMesg.SetEvent(FIT_EVENT_WORKOUT);
    lapMesg.SetSubSport(FIT_SUB_SPORT_GENERIC);
    lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    lapMesg.SetTotalElapsedTime(0);
    lapMesg.SetTotalTimerTime(0);
    if (overrideSport != FIT_SPORT_INVALID) {

        lapMesg.SetSport(FIT_SPORT_GENERIC);
    } else if (type == TREADMILL) {

        lapMesg.SetSport(FIT_SPORT_RUNNING);
    } else if (type == ELLIPTICAL) {

        lapMesg.SetSport(FIT_SPORT_RUNNING);
    } else if (type == ROWING) {

        lapMesg.SetSport(FIT_SPORT_ROWING);
    } else if (type == JUMPROPE) {

        lapMesg.SetSport(FIT_SPORT_JUMP_ROPE);
    } else if (type == STAIRCLIMBER) {

        lapMesg.SetSport(FIT_SPORT_GENERIC);
        lapMesg.SetSubSport(FIT_SUB_SPORT_STAIR_CLIMBING);
    } else {

        lapMesg.SetSport(FIT_SPORT_CYCLING);
    }

    encode.Write(sessionMesg);
    encode.Write(activityMesg);

    SessionLine sl;
    if (processFlag & QFIT_PROCESS_DISTANCENOISE) {
        double distanceOld = -1.0;
        int startIdx = -1;
        for (int i = firstRealIndex; i < session.length(); i++) {

            sl = session.at(i);
            if (sl.distance != distanceOld || i == session.length() - 1) {
                if (i == session.length() - 1 && sl.distance == distanceOld) {
                    i++;
                }
                if (startIdx >= 0) {
                    for (int j = startIdx; j < i; j++) {
                        session[j].distance += 0.1 * (j - startIdx) / (i - startIdx);
                    }
                }
                distanceOld = sl.distance;
                startIdx = i;
            }
        }
    }

    uint32_t lastLapTimer = 0;
    double lastLapOdometer = startingDistanceOffset;
    for (int i = firstRealIndex; i < session.length(); i++) {

        fit::RecordMesg newRecord;
        sl = session.at(i);

        fit::DeveloperField targetCadenceField(targetCadenceMesg, devIdMesg);
        targetCadenceField.AddValue(sl.target_cadence);
        newRecord.AddDeveloperField(targetCadenceField);

        fit::DeveloperField targetWattField(targetWattMesg, devIdMesg);
        targetWattField.AddValue(sl.target_watt);
        newRecord.AddDeveloperField(targetWattField);

        fit::DeveloperField targetResistanceField(targetResistanceMesg, devIdMesg);
        targetResistanceField.AddValue(sl.target_resistance);
        newRecord.AddDeveloperField(targetResistanceField);

        // fit::DateTime date((time_t)session.at(i).time.toSecsSinceEpoch());
        newRecord.SetHeartRate(sl.heart);
        uint8_t cad = sl.cadence;
        if (powr_sensor_running_cadence_half_on_strava)
            cad = cad / 2;
        newRecord.SetCadence(cad);
        newRecord.SetDistance((sl.distance - startingDistanceOffset) * 1000.0); // meters
        newRecord.SetSpeed(sl.speed / 3.6);                                     // meter per second
        newRecord.SetPower(sl.watt);
        newRecord.SetResistance(sl.resistance);
        newRecord.SetCalories(sl.calories);
        if (type == TREADMILL) {
            newRecord.SetStepLength(sl.instantaneousStrideLengthCM * 10);
            newRecord.SetVerticalOscillation(sl.verticalOscillationMM);
            newRecord.SetStanceTime(sl.groundContactMS);
        }

               // Add custom developer fields for temperature data
        if (sl.coreTemp) {
            fit::DeveloperField coreTemperatureField(coreTemperatureFieldDesc, devIdMesg);
            coreTemperatureField.SetFLOAT32Value((float)sl.coreTemp);
            newRecord.AddDeveloperField(coreTemperatureField);
        }
        if (sl.bodyTemp) {
            fit::DeveloperField skinTemperatureField(skinTemperatureFieldDesc, devIdMesg);
            skinTemperatureField.SetFLOAT32Value((float)sl.bodyTemp);
            newRecord.AddDeveloperField(skinTemperatureField);
        }
        if (sl.heatStrainIndex) {
            fit::DeveloperField heatStrainIndexField(heatStrainIndexFieldDesc, devIdMesg);
            heatStrainIndexField.SetFLOAT32Value((float)sl.heatStrainIndex);
            newRecord.AddDeveloperField(heatStrainIndexField);
        }

               // if a gps track contains a point without the gps information, it has to be discarded, otherwise the database
               // structure is corrupted and 2 tracks are saved in the FIT file causing mapping issue.
        if (!sl.coordinate.isValid() && gps_data) {
            continue;
        }

        if (sl.coordinate.isValid()) {
            newRecord.SetAltitude(sl.coordinate.altitude());
            newRecord.SetPositionLat(pow(2, 31) * (sl.coordinate.latitude()) / 180.0);
            newRecord.SetPositionLong(pow(2, 31) * (sl.coordinate.longitude()) / 180.0);
        } else {
            newRecord.SetAltitude(sl.elevationGain);
        }

               // using just the start point as reference in order to avoid pause time
               // strava ignore the elapsed field
               // this workaround could leads an accuracy issue.
        newRecord.SetTimestamp(date.GetTimeStamp() + i);
        encode.Write(newRecord);

        if (sl.lapTrigger) {

            lapMesg.SetTotalDistance((sl.distance - lastLapOdometer) * 1000.0); // meters
            lapMesg.SetTotalElapsedTime(sl.elapsedTime - lastLapTimer);
            lapMesg.SetTotalTimerTime(sl.elapsedTime - lastLapTimer);
            lapMesg.SetEvent(FIT_EVENT_LAP);
            lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
            lapMesg.SetMessageIndex(lap_index++);
            lapMesg.SetLapTrigger(FIT_LAP_TRIGGER_DISTANCE);
            if (type == JUMPROPE)
                lapMesg.SetRepetitionNum(session.at(i - 1).inclination);
            lastLapTimer = sl.elapsedTime;
            lastLapOdometer = sl.distance;

            encode.Write(lapMesg);

            lapMesg.SetStartTime(date.GetTimeStamp() + i);
            lapMesg.SetTimestamp(date.GetTimeStamp() + i);
            lapMesg.SetEvent(FIT_EVENT_WORKOUT);
            lapMesg.SetEventType(FIT_EVENT_LAP);
        }
    }

    lapMesg.SetTotalDistance((session.last().distance - lastLapOdometer) * 1000.0); // meters
    lapMesg.SetTotalElapsedTime(session.last().elapsedTime - lastLapTimer);
    lapMesg.SetTotalTimerTime(session.last().elapsedTime - lastLapTimer);
    lapMesg.SetEvent(FIT_EVENT_LAP);
    lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    lapMesg.SetLapTrigger(FIT_LAP_TRIGGER_SESSION_END);
    lapMesg.SetMessageIndex(lap_index++);
    encode.Write(lapMesg);

    if (!encode.Close()) {

        printf("Error closing encode.\n");
        return;
    }
    file.close();

    printf("Encoded FIT file ExampleActivity.fit.\n");
    return;
}

class Listener : public fit::FileIdMesgListener,
                 public fit::UserProfileMesgListener,
                 public fit::MonitoringMesgListener,
                 public fit::DeviceInfoMesgListener,
                 public fit::MesgListener,
                 public fit::DeveloperFieldDescriptionListener,
                 public fit::RecordMesgListener,
                 public fit::SessionMesgListener {
  public:
    QList<SessionLine> *sessionOpening = nullptr;
    FIT_SPORT *sport = nullptr;
    QString *workoutName = nullptr;
    QString *workoutSource = nullptr;
    QString *pelotonWorkoutId = nullptr;
    QString *pelotonUrl = nullptr;
    QString *trainingProgramFile = nullptr;

    static void PrintValues(const fit::FieldBase &field) {
        for (FIT_UINT8 j = 0; j < (FIT_UINT8)field.GetNumValues(); j++) {
            // std::wcout << L"       Val" << j << L": ";
            switch (field.GetType()) {
            // Get float 64 values for numeric types to receive values that have
            // their scale and offset properly applied.
            case FIT_BASE_TYPE_ENUM:
            case FIT_BASE_TYPE_BYTE:
            case FIT_BASE_TYPE_SINT8:
            case FIT_BASE_TYPE_UINT8:
            case FIT_BASE_TYPE_SINT16:
            case FIT_BASE_TYPE_UINT16:
            case FIT_BASE_TYPE_SINT32:
            case FIT_BASE_TYPE_UINT32:
            case FIT_BASE_TYPE_SINT64:
            case FIT_BASE_TYPE_UINT64:
            case FIT_BASE_TYPE_UINT8Z:
            case FIT_BASE_TYPE_UINT16Z:
            case FIT_BASE_TYPE_UINT32Z:
            case FIT_BASE_TYPE_UINT64Z:
            case FIT_BASE_TYPE_FLOAT32:
            case FIT_BASE_TYPE_FLOAT64:
                // std::wcout << field.GetFLOAT64Value(j);
                break;
            case FIT_BASE_TYPE_STRING:
                // std::wcout << field.GetSTRINGValue(j);
                break;
            default:
                break;
            }
            // std::wcout << L" " << field.GetUnits().c_str() << L"\n";;
        }
    }

    void OnMesg(fit::Mesg &mesg) {
        // printf("On Mesg:\n");
        // std::wcout << L"   New Mesg: " << mesg.GetName().c_str() << L".  It has " << mesg.GetNumFields() << L"
        // field(s) and " << mesg.GetNumDevFields() << " developer field(s).\n";

        for (FIT_UINT16 i = 0; i < (FIT_UINT16)mesg.GetNumFields(); i++) {
            fit::Field *field = mesg.GetFieldByIndex(i);
            // std::wcout << L"   Field" << i << " (" << field->GetName().c_str() << ") has " << field->GetNumValues()
            // << L" value(s)\n";
            PrintValues(*field);
        }

        for (auto devField : mesg.GetDeveloperFields()) {
            // std::wcout << L"   Developer Field(" << devField.GetName().c_str() << ") has " << devField.GetNumValues()
            // << L" value(s)\n";
            PrintValues(devField);
        }
    }

    void OnMesg(fit::FileIdMesg &mesg) {
        printf("File ID:\n");
        if (mesg.IsTypeValid())
            printf("   Type: %d\n", mesg.GetType());
        if (mesg.IsManufacturerValid())
            printf("   Manufacturer: %d\n", mesg.GetManufacturer());
        if (mesg.IsProductValid())
            printf("   Product: %d\n", mesg.GetProduct());
        if (mesg.IsSerialNumberValid())
            printf("   Serial Number: %u\n", mesg.GetSerialNumber());
        if (mesg.IsNumberValid())
            printf("   Number: %d\n", mesg.GetNumber());
    }

    void OnMesg(fit::UserProfileMesg &mesg) {
        printf("User profile:\n");
        if (mesg.IsFriendlyNameValid())
            // std::wcout << L"   Friendly Name: " << mesg.GetFriendlyName().c_str() << L"\n";
            if (mesg.GetGender() == FIT_GENDER_MALE)
                printf("   Gender: Male\n");
        if (mesg.GetGender() == FIT_GENDER_FEMALE)
            printf("   Gender: Female\n");
        if (mesg.IsAgeValid())
            printf("   Age [years]: %d\n", mesg.GetAge());
        if (mesg.IsWeightValid())
            printf("   Weight [kg]: %0.2f\n", mesg.GetWeight());
    }

    void OnMesg(fit::DeviceInfoMesg &mesg) {
        printf("Device info:\n");

        if (mesg.IsTimestampValid())
            printf("   Timestamp: %d\n", mesg.GetTimestamp());

        switch (mesg.GetBatteryStatus()) {
        case FIT_BATTERY_STATUS_CRITICAL:
            printf("   Battery status: Critical\n");
            break;
        case FIT_BATTERY_STATUS_GOOD:
            printf("   Battery status: Good\n");
            break;
        case FIT_BATTERY_STATUS_LOW:
            printf("   Battery status: Low\n");
            break;
        case FIT_BATTERY_STATUS_NEW:
            printf("   Battery status: New\n");
            break;
        case FIT_BATTERY_STATUS_OK:
            printf("   Battery status: OK\n");
            break;
        default:
            printf("   Battery status: Invalid\n");
            break;
        }
    }

    void OnMesg(fit::MonitoringMesg &mesg) {
        printf("Monitoring:\n");

        if (mesg.IsTimestampValid()) {
            printf("   Timestamp: %d\n", mesg.GetTimestamp());
        }

        if (mesg.IsActivityTypeValid()) {
            printf("   Activity type: %d\n", mesg.GetActivityType());
        }


        switch (mesg.GetActivityType()) // The Cycling field is dynamic
        {
        case FIT_ACTIVITY_TYPE_WALKING:
        case FIT_ACTIVITY_TYPE_RUNNING: // Intentional fallthrough
            if (mesg.IsStepsValid()) {
                printf("   Steps: %d\n", mesg.GetSteps());
            }
            break;
        case FIT_ACTIVITY_TYPE_CYCLING:
        case FIT_ACTIVITY_TYPE_SWIMMING: // Intentional fallthrough
            if (mesg.IsStrokesValid()) {
                printf("Strokes: %d\n", mesg.GetStrokes());
            }
            break;
        default:
            if (mesg.IsCyclesValid()) {
                printf("Cycles: %d\n", mesg.GetCycles());
            }
            break;
        }
    }

    static void PrintOverrideValues(const fit::Mesg &mesg, FIT_UINT8 fieldNum) {
        std::vector<const fit::FieldBase *> fields = mesg.GetOverrideFields(fieldNum);
        const fit::Profile::FIELD *profileField = fit::Profile::GetField(mesg.GetNum(), fieldNum);
        FIT_BOOL namePrinted = FIT_FALSE;

        for (const fit::FieldBase *field : fields) {
            if (!namePrinted) {
                printf("   %s:\n", profileField->name.c_str());
                namePrinted = FIT_TRUE;
            }

            if (FIT_NULL != dynamic_cast<const fit::Field *>(field)) {
                // Native Field
                printf("      native: ");
            } else {
                // Developer Field
                printf("      override: ");
            }

            switch (field->GetType()) {
            // Get float 64 values for numeric types to receive values that have
            // their scale and offset properly applied.
            case FIT_BASE_TYPE_ENUM:
            case FIT_BASE_TYPE_BYTE:
            case FIT_BASE_TYPE_SINT8:
            case FIT_BASE_TYPE_UINT8:
            case FIT_BASE_TYPE_SINT16:
            case FIT_BASE_TYPE_UINT16:
            case FIT_BASE_TYPE_SINT32:
            case FIT_BASE_TYPE_UINT32:
            case FIT_BASE_TYPE_SINT64:
            case FIT_BASE_TYPE_UINT64:
            case FIT_BASE_TYPE_UINT8Z:
            case FIT_BASE_TYPE_UINT16Z:
            case FIT_BASE_TYPE_UINT32Z:
            case FIT_BASE_TYPE_UINT64Z:
            case FIT_BASE_TYPE_FLOAT32:
            case FIT_BASE_TYPE_FLOAT64:
                printf("%f\n", field->GetFLOAT64Value());
                break;
            case FIT_BASE_TYPE_STRING:
                printf("%ls\n", field->GetSTRINGValue().c_str());
                break;
            default:
                break;
            }
        }
    }

    void OnMesg(fit::RecordMesg &record) override {
        if (sessionOpening != nullptr) {
            SessionLine s;
            s.heart = record.GetHeartRate();
            s.cadence = record.GetCadence();
            s.distance = record.GetDistance() / 1000;
            s.speed = record.GetSpeed() * 3.6;
            s.watt = record.GetPower();
            s.resistance = record.GetResistance();
            s.calories = record.GetCalories();
            s.instantaneousStrideLengthCM = record.GetStepLength() / 10;
            s.verticalOscillationMM = record.GetVerticalOscillation();
            s.groundContactMS = record.GetStanceTime();
            s.coordinate.setAltitude(record.GetAltitude());
            s.coordinate.setLatitude((record.GetPositionLat() * 180) / pow(2, 31));
            s.coordinate.setLongitude((record.GetPositionLong() * 180) / pow(2, 31));
            if (!s.coordinate.isValid()) {
                s.elevationGain = record.GetAltitude();
            }
            s.time = QDateTime::fromSecsSinceEpoch(record.GetTimestamp() + 631065600L);
            s.elapsedTime = (sessionOpening->count() ? sessionOpening->at(0).time : s.time).secsTo(s.time);
            sessionOpening->append(s);
        }
    }

    void OnDeveloperFieldDescription(const fit::DeveloperFieldDescription &desc) override {
        printf("New Developer Field Description\n");
        printf("   App Version: %d\n", desc.GetApplicationVersion());
        printf("   Field Number: %d\n", desc.GetFieldDefinitionNumber());
    }

    void OnMesg(fit::SessionMesg &mesg) override {
        printf("Session Message:\n");
        
        // Extract sport type from SessionMesg
        if (sport != nullptr && mesg.IsSportValid()) {
            *sport = mesg.GetSport();
            printf("   Sport type from session: %d\n", static_cast<int>(*sport));
        }
        
        if (workoutName != nullptr) {
            for (auto devField : mesg.GetDeveloperFields()) {
                std::string fieldName = devField.GetName();
                if (fieldName == "Activity Title") {
                    std::wstring wWorkoutName = devField.GetSTRINGValue(0);
                    *workoutName = QString::fromStdWString(wWorkoutName);
                    printf("   Found Activity Title: %s\n", workoutName->toStdString().c_str());
                } else if (fieldName == "Workout Source" && workoutSource != nullptr) {
                    std::wstring wWorkoutSource = devField.GetSTRINGValue(0);
                    *workoutSource = QString::fromStdWString(wWorkoutSource);
                    printf("   Found Workout Source: %s\n", workoutSource->toStdString().c_str());
                } else if (fieldName == "Peloton Workout ID" && pelotonWorkoutId != nullptr) {
                    std::wstring wPelotonWorkoutId = devField.GetSTRINGValue(0);
                    *pelotonWorkoutId = QString::fromStdWString(wPelotonWorkoutId);
                    printf("   Found Peloton Workout ID: %s\n", pelotonWorkoutId->toStdString().c_str());
                } else if (fieldName == "Peloton URL" && pelotonUrl != nullptr) {
                    std::wstring wPelotonUrl = devField.GetSTRINGValue(0);
                    *pelotonUrl = QString::fromStdWString(wPelotonUrl);
                    printf("   Found Peloton URL: %s\n", pelotonUrl->toStdString().c_str());
                } else if (fieldName == "Training Program File" && trainingProgramFile != nullptr) {
                    std::wstring wTrainingProgramFile = devField.GetSTRINGValue(0);
                    *trainingProgramFile = QString::fromStdWString(wTrainingProgramFile);
                    printf("   Found Training Program File: %s\n", trainingProgramFile->toStdString().c_str());
                } else if (fieldName == "Instructor Name" || fieldName == "Coach Name") {
                    // Future: handle instructor name if needed
                    printf("   Found Instructor/Coach field: %s\n", fieldName.c_str());
                } else if (!fieldName.empty()) {
                    printf("   Other developer field: %s\n", fieldName.c_str());
                }
            }
        }
    }
};

void qfit::open(const QString &filename, QList<SessionLine> *output, FIT_SPORT *sport) {
    // Call the version with workoutName
    open(filename, output, sport, nullptr);
}

void qfit::open(const QString &filename, QList<SessionLine> *output, FIT_SPORT *sport, QString *workoutName) {
    // Call the full version with nullptr for optional parameters
    open(filename, output, sport, workoutName, nullptr, nullptr, nullptr, nullptr);
}


void qfit::open(const QString &filename, QList<SessionLine> *output, FIT_SPORT *sport, QString *workoutName, 
                QString *workoutSource, QString *pelotonWorkoutId, QString *pelotonUrl, QString *trainingProgramFile) {
    std::fstream file;
#ifdef _WIN32
    file.open(QString(filename).toLocal8Bit().constData(), std::ios::in | std::ios::binary);
#else
    file.open(filename.toStdString(), std::ios::in | std::ios::binary);
#endif

    if (!file.is_open()) {
        std::system_error(errno, std::system_category(), "failed to open " + filename.toStdString());
        qDebug() << "opened " << filename << errno;
        printf("Error opening file ExampleActivity.fit\n");
        return;
    }

    fit::Decode decode;
    std::istream &s = file;
    fit::MesgBroadcaster mesgBroadcaster;
    Listener listener;
    listener.sport = sport;
    listener.sessionOpening = output;
    listener.workoutName = workoutName;
    listener.workoutSource = workoutSource;
    listener.pelotonWorkoutId = pelotonWorkoutId;
    listener.pelotonUrl = pelotonUrl;
    listener.trainingProgramFile = trainingProgramFile;
    mesgBroadcaster.AddListener((fit::FileIdMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::UserProfileMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::MonitoringMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::DeviceInfoMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::RecordMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::SessionMesgListener &)listener);
    mesgBroadcaster.AddListener((fit::MesgListener &)listener);
    decode.Read(&s, &mesgBroadcaster, &mesgBroadcaster, &listener);

    file.close();
}

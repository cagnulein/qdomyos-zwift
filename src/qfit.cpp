#include "qfit.h"

#include <QSettings>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <QDir>


#include "fit_date_time.hpp"
#include "fit_encode.hpp"
#include "fit_hrv_mesg.hpp"

#include "fit_decode.hpp"
#include "fit_developer_field_description.hpp"
#include "fit_field_description_mesg.hpp"
#include "fit_developer_field.hpp"
#include "fit_mesg_broadcaster.hpp"
#include "fit_timestamp_correlation_mesg.hpp"
#include "fit_zones_target_mesg.hpp"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#endif

using namespace std;

qfit::qfit(QObject *parent) : QObject(parent) {}

void qfit::save(const QString &filename, QList<SessionLine> session, BLUETOOTH_TYPE type,
                uint32_t processFlag, FIT_SPORT overrideSport, QString workoutName, QString bluetooth_device_name,
                QString workoutSource, QString pelotonWorkoutId, QString pelotonUrl, QString trainingProgramFile,
                int workoutRpe, int workoutFeel) {
    QSettings settings;
    bool strava_virtual_activity =
        settings.value(QZSettings::strava_virtual_activity, QZSettings::default_strava_virtual_activity).toBool();
    bool strava_treadmill =
        settings.value(QZSettings::strava_treadmill, QZSettings::default_strava_treadmill).toBool();
    bool treadmill_force_running_activity =
        settings
            .value(QZSettings::treadmill_force_running_activity,
                   QZSettings::default_treadmill_force_running_activity)
            .toBool();
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

    // Compute user physiology once — shared by userMesg, zones_target, and training effect.
    bool user_max_hr_override = settings.value(QZSettings::heart_max_override_enable,
                                               QZSettings::default_heart_max_override_enable).toBool();
    uint8_t user_max_hr = user_max_hr_override
        ? (uint8_t)settings.value(QZSettings::heart_max_override_value,
                                  QZSettings::default_heart_max_override_value).toUInt()
        : (uint8_t)(220 - settings.value(QZSettings::age, QZSettings::default_age).toUInt());
    uint8_t user_resting_hr = settings.value(QZSettings::heart_rate_resting,
                                             QZSettings::default_heart_rate_resting).toUInt();

    fit::UserProfileMesg userMesg;
    userMesg.SetWeight(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat());
    userMesg.SetAge(settings.value(QZSettings::age, QZSettings::default_age).toUInt());
    userMesg.SetGender(settings.value(QZSettings::sex, QZSettings::default_sex).toString().startsWith(QZSettings::default_sex) ? FIT_GENDER_MALE
                                                                                                   : FIT_GENDER_FEMALE);
    userMesg.SetFriendlyName(
        settings.value(QZSettings::user_nickname, QZSettings::default_user_nickname).toString().toStdWString());
    userMesg.SetHeight(settings.value(QZSettings::height, QZSettings::default_height).toFloat() / 100.0f);
    userMesg.SetDefaultMaxHeartRate(user_max_hr);
    userMesg.SetRestingHeartRate(user_resting_hr);

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

    // Cadence, power, and speed summaries (all device types)
    double cadence_sum = 0;
    int cadence_count = 0;
    uint8_t max_cadence = 0;
    uint16_t max_watt = 0;
    uint32_t total_work_joules = 0;
    double max_speed_ms = 0;
    std::vector<double> np_power_samples;

    // Variables for core temperature summaries used by Garmin Connect.
    double core_temp_sum = 0;
    double core_temp_min = 0;
    double core_temp_max = 0;
    int core_temp_count = 0;

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

        // Collect power data for TSS / NP / session stats
        if (session.at(i).watt > 0) {
            watt_sum += session.at(i).watt;
            watt_count++;
            if ((uint16_t)session.at(i).watt > max_watt)
                max_watt = (uint16_t)session.at(i).watt;
            total_work_joules += (uint32_t)session.at(i).watt;
        }
        np_power_samples.push_back(session.at(i).watt > 0 ? session.at(i).watt : 0.0);

        // Collect cadence data (all device types)
        if (session.at(i).cadence > 0) {
            cadence_sum += session.at(i).cadence;
            cadence_count++;
            if (session.at(i).cadence > max_cadence)
                max_cadence = session.at(i).cadence;
        }

        // Max speed (m/s — session.speed is in km/h)
        double speed_ms = session.at(i).speed / 3.6;
        if (speed_ms > max_speed_ms)
            max_speed_ms = speed_ms;

        if (session.at(i).coreTemp > 0) {
            double coreTemp = session.at(i).coreTemp;
            core_temp_sum += coreTemp;
            if (core_temp_count == 0 || coreTemp < core_temp_min) {
                core_temp_min = coreTemp;
            }
            if (core_temp_count == 0 || coreTemp > core_temp_max) {
                core_temp_max = coreTemp;
            }
            core_temp_count++;
        }
    }

    if (speed_count > 0) {
        speed_avg = speed_acc / ((double)speed_count);
        qDebug() << "average speed from the fit file" << speed_avg;
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
        if (user_max_hr > user_resting_hr) {
            hr_ratio = (avg_hr - user_resting_hr) / (double)(user_max_hr - user_resting_hr);
        }

        // Use coefficient 1.67 (matches Garmin implementation)
        double b = 1.67;

        // Calculate TRIMP
        if (hr_ratio > 0 && hr_ratio < 2.0) {  // Sanity check
            training_load = duration_minutes * hr_ratio * std::exp(b * hr_ratio);
            qDebug() << "Training Load (TRIMP) calculated:" << training_load
                     << "Duration:" << duration_minutes << "min"
                     << "Avg HR:" << avg_hr
                     << "Max HR:" << user_max_hr << (user_max_hr_override ? "(override)" : "(calculated)")
                     << "Resting HR:" << user_resting_hr;
        }
    }

    // Normalized Power: 30-second rolling average → 4th-power mean → 4th root
    uint16_t normalized_power = 0;
    if (np_power_samples.size() >= 30) {
        std::vector<double> rolling30;
        rolling30.reserve(np_power_samples.size());
        for (size_t idx = 0; idx < np_power_samples.size(); idx++) {
            double sum = 0;
            size_t start = idx >= 29 ? idx - 29 : 0;
            for (size_t j = start; j <= idx; j++)
                sum += np_power_samples[j];
            rolling30.push_back(sum / (idx - start + 1));
        }
        double sum4 = 0;
        for (double v : rolling30)
            sum4 += std::pow(v, 4.0);
        normalized_power = (uint16_t)std::pow(sum4 / rolling30.size(), 0.25);
    }

    // Training Effect (aerobic + anaerobic) from HR zones and power
    float aerobic_te = 0.0f;
    float anaerobic_te = 0.0f;
    if (hr_count > 0 && user_max_hr > user_resting_hr) {
        float zone1_pct = settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toFloat();
        float zone2_pct = settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toFloat();
        float zone3_pct = settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toFloat();
        float zone4_pct = settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4).toFloat();
        double z1 = zone1_pct / 100.0 * user_max_hr;
        double z2 = zone2_pct / 100.0 * user_max_hr;
        double z3 = zone3_pct / 100.0 * user_max_hr;
        double z4 = zone4_pct / 100.0 * user_max_hr;

        double time_in_zone[5] = {0, 0, 0, 0, 0};
        for (int i = firstRealIndex; i < session.length(); i++) {
            double hr = session.at(i).heart;
            if (hr <= 0) continue;
            if (hr < z1)      time_in_zone[0] += 1.0;
            else if (hr < z2) time_in_zone[1] += 1.0;
            else if (hr < z3) time_in_zone[2] += 1.0;
            else if (hr < z4) time_in_zone[3] += 1.0;
            else              time_in_zone[4] += 1.0;
        }

        // Aerobic TE: prefer the accumulated TRIMP load already calculated above.
        // Garmin Training Effect accumulates during the activity, so a duration-normalized
        // zone average underestimates steady aerobic workouts.
        const double zone_weights[5] = {0.2, 0.5, 1.0, 1.5, 2.0};
        double weighted = 0;
        for (int z = 0; z < 5; z++)
            weighted += (time_in_zone[z] / 60.0) * zone_weights[z];
        double dur_min = duration_seconds / 60.0;
        if (dur_min > 0) {
            if (training_load > 0) {
                aerobic_te = std::min(5.0f,
                                       (float)(5.0 * (1.0 - std::exp(-training_load / 90.0))));
            } else {
                aerobic_te = std::min(5.0f, (float)((weighted / dur_min) * 2.0));
            }
            qDebug() << "Aerobic TE:" << aerobic_te
                     << "Z1-Z5 min:" << time_in_zone[0]/60 << time_in_zone[1]/60
                     << time_in_zone[2]/60 << time_in_zone[3]/60 << time_in_zone[4]/60;
        }

        // Anaerobic TE: from power if available, else from time in Z4+Z5
        if (watt_count > 0) {
            float ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toFloat();
            if (ftp > 0) {
                double threshold_w = ftp * 1.05;
                double time_above = 0, intensity_above = 0;
                for (int i = firstRealIndex; i < session.length(); i++) {
                    if (session.at(i).watt > threshold_w) {
                        time_above += 1.0;
                        intensity_above += session.at(i).watt / threshold_w;
                    }
                }
                if (time_above > 0 && dur_min > 0) {
                    double pct = (time_above / duration_seconds) * 100.0;
                    anaerobic_te = std::min(5.0f, (float)((pct / 20.0) * (intensity_above / time_above)));
                }
            }
        } else {
            double high_intensity = time_in_zone[3] + time_in_zone[4];
            if (high_intensity > 0 && dur_min > 0) {
                double pct = (high_intensity / duration_seconds) * 100.0;
                anaerobic_te = std::min(5.0f, (float)((pct / 25.0) * 2.0));
                if (time_in_zone[4] > time_in_zone[3])
                    anaerobic_te = std::min(5.0f, anaerobic_te * 1.3f);
            }
        }
        qDebug() << "Anaerobic TE:" << anaerobic_te;
    }

    encode.Open(file);
    encode.Write(fileIdMesg);
    encode.Write(userMesg);

    // zones_target: gives Garmin Connect the user's FTP and HR zones for load calculations
    {
        fit::ZonesTargetMesg zonesTargetMesg;
        zonesTargetMesg.SetMaxHeartRate(user_max_hr);
        float zone3_pct = settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toFloat();
        zonesTargetMesg.SetThresholdHeartRate((uint8_t)(zone3_pct / 100.0f * user_max_hr));
        uint16_t ftp = (uint16_t)settings.value(QZSettings::ftp, QZSettings::default_ftp).toFloat();
        if (ftp > 0)
            zonesTargetMesg.SetFunctionalThresholdPower(ftp);
        zonesTargetMesg.SetHrCalcType(FIT_HR_ZONE_CALC_PERCENT_MAX_HR);
        zonesTargetMesg.SetPwrCalcType(FIT_PWR_ZONE_CALC_PERCENT_FTP);
        encode.Write(zonesTargetMesg);
    }

    // Declare developer field descriptions (but don't write them yet)
    fit::FieldDescriptionMesg activityTitle;
    activityTitle.SetDeveloperDataIndex(0);
    activityTitle.SetFieldDefinitionNumber(0);
    activityTitle.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    activityTitle.SetFieldName(0, L"Activity Title");
    activityTitle.SetUnits(0, L"Title");
    activityTitle.SetNativeMesgNum(FIT_MESG_NUM_WORKOUT);  // Workout message for developer metadata

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
    ftpSessionMesg.SetNativeMesgNum(FIT_MESG_NUM_WORKOUT);  // Workout message for developer metadata

    // Peloton and workout source fields
    fit::FieldDescriptionMesg workoutSourceMesg;
    workoutSourceMesg.SetDeveloperDataIndex(0);
    workoutSourceMesg.SetFieldDefinitionNumber(8);
    workoutSourceMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    workoutSourceMesg.SetFieldName(0, L"Workout Source");
    workoutSourceMesg.SetUnits(0, L"source");
    workoutSourceMesg.SetNativeMesgNum(FIT_MESG_NUM_WORKOUT);  // Workout message for developer metadata

    fit::FieldDescriptionMesg pelotonWorkoutIdMesg;
    pelotonWorkoutIdMesg.SetDeveloperDataIndex(0);
    pelotonWorkoutIdMesg.SetFieldDefinitionNumber(9);
    pelotonWorkoutIdMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    pelotonWorkoutIdMesg.SetFieldName(0, L"Peloton Workout ID");
    pelotonWorkoutIdMesg.SetUnits(0, L"id");
    pelotonWorkoutIdMesg.SetNativeMesgNum(FIT_MESG_NUM_WORKOUT);  // Workout message for developer metadata

    fit::FieldDescriptionMesg pelotonUrlMesg;
    pelotonUrlMesg.SetDeveloperDataIndex(0);
    pelotonUrlMesg.SetFieldDefinitionNumber(10);
    pelotonUrlMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    pelotonUrlMesg.SetFieldName(0, L"Peloton URL");
    pelotonUrlMesg.SetUnits(0, L"url");
    pelotonUrlMesg.SetNativeMesgNum(FIT_MESG_NUM_WORKOUT);  // Workout message for developer metadata

    fit::FieldDescriptionMesg trainingProgramFileMesg;
    trainingProgramFileMesg.SetDeveloperDataIndex(0);
    trainingProgramFileMesg.SetFieldDefinitionNumber(11);
    trainingProgramFileMesg.SetFitBaseTypeId(FIT_BASE_TYPE_STRING);
    trainingProgramFileMesg.SetFieldName(0, L"Training Program File");
    trainingProgramFileMesg.SetUnits(0, L"filename");
    trainingProgramFileMesg.SetNativeMesgNum(FIT_MESG_NUM_WORKOUT);  // Workout message for developer metadata

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
    if (speed_avg > 0) {
        sessionMesg.SetAvgSpeed(speed_avg / 3.6);  // Convert from km/h to m/s
    }
    sessionMesg.SetMinAltitude(min_alt);
    sessionMesg.SetMaxAltitude(max_alt);
    sessionMesg.SetEvent(FIT_EVENT_SESSION);
    sessionMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    sessionMesg.SetFirstLapIndex(0);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);
    sessionMesg.SetMessageIndex(FIT_MESSAGE_INDEX_RESERVED);

    // Perceived exertion (Borg CR10, 0-10 scale multiplied 10x) and how the user felt (0-100 scale)
    if (workoutRpe >= 0)
        sessionMesg.SetWorkoutRpe(static_cast<FIT_UINT8>(workoutRpe * 10));
    if (workoutFeel >= 0)
        sessionMesg.SetWorkoutFeel(static_cast<FIT_UINT8>(workoutFeel));

    // Set training load in FIT file
    // Always set training_load_peak (Garmin uses this for acute training load)
    // COMMENTED OUT: Garmin Connect doesn't properly reflect these values
    // Moving to developer data message instead
    if (training_load > 0) {
        //sessionMesg.SetTrainingLoadPeak(training_load);
        qDebug() << "Training load will be stored in developer data:" << training_load;
    }

    // For cycling with power, also set training_stress_score (TSS)
    // COMMENTED OUT: Moving to developer data message
    if (has_tss) {
        //sessionMesg.SetTrainingStressScore(tss);
        qDebug() << "TSS will be stored in developer data:" << tss;
    }

    const FIT_SPORT treadmill_activity_sport =
        (speed_avg == 0 || speed_avg > 6.5 || strava_virtual_activity || treadmill_force_running_activity)
            ? FIT_SPORT_RUNNING
            : FIT_SPORT_WALKING;

    // First, set sport and subsport based on device type
    if (type == TREADMILL) {
        if(session.last().stepCount > 0)
            sessionMesg.SetTotalStrides(session.last().stepCount);

        sessionMesg.SetSport(treadmill_activity_sport);

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

        sessionMesg.SetSport(FIT_SPORT_FITNESS_EQUIPMENT);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_STAIR_CLIMBING);
    } else if (type == JUMPROPE) {

        sessionMesg.SetSport(FIT_SPORT_JUMP_ROPE);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_GENERIC);
        if (session.last().stepCount)
            sessionMesg.SetJumpCount(session.last().stepCount);
        // Total cycles
        if (session.last().stepCount)
            sessionMesg.SetTotalCycles(session.last().stepCount);
        // Avg cadence (jump rate)
        if (cadence_count > 0)
            sessionMesg.SetAvgCadence((uint8_t)(cadence_sum / cadence_count));
        // Max cadence (max jump rate)
        if (max_cadence > 0)
            sessionMesg.SetMaxCadence(max_cadence);
    } else {

        sessionMesg.SetSport(FIT_SPORT_CYCLING);
        if (strava_virtual_activity) {
            sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
        } else {
            sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_CYCLING);
        }
    }

    // Then, override the sport if requested (keeping the subsport from above)
    if (overrideSport != FIT_SPORT_INVALID) {
        sessionMesg.SetSport(overrideSport);
        qDebug() << "overriding FIT sport to" << overrideSport << "keeping subsport from device type";
    }

    // Session statistics derived from record data
    if (hr_count > 0) {
        sessionMesg.SetAvgHeartRate((uint8_t)(hr_sum / hr_count));
        sessionMesg.SetMaxHeartRate(max_hr);
        sessionMesg.SetMinHeartRate(min_hr);
    }
    if (cadence_count > 0 && type != ROWING) {
        sessionMesg.SetAvgCadence((uint8_t)(cadence_sum / cadence_count));
        sessionMesg.SetMaxCadence(max_cadence);
    }
    if (watt_count > 0) {
        sessionMesg.SetAvgPower((uint16_t)(watt_sum / watt_count));
        sessionMesg.SetMaxPower(max_watt);
        if (normalized_power > 0)
            sessionMesg.SetNormalizedPower(normalized_power);
        if (total_work_joules > 0)
            sessionMesg.SetTotalWork(total_work_joules);
    }
    if (max_speed_ms > 0) {
        sessionMesg.SetEnhancedMaxSpeed((float)max_speed_ms);
    }
    if (aerobic_te > 0) {
        sessionMesg.SetTotalTrainingEffect(aerobic_te);
        sessionMesg.SetTotalAnaerobicTrainingEffect(anaerobic_te);
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

    fit::DeveloperDataIdMesg coreDevIdMesg;
    // CORE app: 6957fe68-83fe-4ed6-8613-413f70624bb5
    coreDevIdMesg.SetApplicationId(0, 0x69);
    coreDevIdMesg.SetApplicationId(1, 0x57);
    coreDevIdMesg.SetApplicationId(2, 0xfe);
    coreDevIdMesg.SetApplicationId(3, 0x68);
    coreDevIdMesg.SetApplicationId(4, 0x83);
    coreDevIdMesg.SetApplicationId(5, 0xfe);
    coreDevIdMesg.SetApplicationId(6, 0x4e);
    coreDevIdMesg.SetApplicationId(7, 0xd6);
    coreDevIdMesg.SetApplicationId(8, 0x86);
    coreDevIdMesg.SetApplicationId(9, 0x13);
    coreDevIdMesg.SetApplicationId(10, 0x41);
    coreDevIdMesg.SetApplicationId(11, 0x3f);
    coreDevIdMesg.SetApplicationId(12, 0x70);
    coreDevIdMesg.SetApplicationId(13, 0x62);
    coreDevIdMesg.SetApplicationId(14, 0x4b);
    coreDevIdMesg.SetApplicationId(15, 0xb5);
    coreDevIdMesg.SetDeveloperDataIndex(1);
    coreDevIdMesg.SetApplicationVersion(78);

    auto makeCoreFieldDescription = [](FIT_UINT8 fieldNumber,
                                       FIT_UINT8 baseType,
                                       const wchar_t *fieldName,
                                       const wchar_t *units,
                                       FIT_MESG_NUM nativeMesgNum,
                                       FIT_UINT8 nativeFieldNum) {
        fit::FieldDescriptionMesg desc;
        desc.SetDeveloperDataIndex(1);
        desc.SetFieldDefinitionNumber(fieldNumber);
        desc.SetFitBaseTypeId(baseType);
        desc.SetFieldName(0, fieldName);
        desc.SetUnits(0, units);
        desc.SetNativeMesgNum(nativeMesgNum);
        desc.SetNativeFieldNum(nativeFieldNum);
        return desc;
    };

    fit::FieldDescriptionMesg coreTemperatureFieldDesc =
        makeCoreFieldDescription(0, FIT_BASE_TYPE_FLOAT32, L"core_temperature", L"°C", FIT_MESG_NUM_RECORD, 139);
    fit::FieldDescriptionMesg coreSkinTemperatureFieldDesc =
        makeCoreFieldDescription(10, FIT_BASE_TYPE_FLOAT32, L"skin_temperature", L"°C", FIT_MESG_NUM_RECORD, 255);
    fit::FieldDescriptionMesg coreDataQualityFieldDesc =
        makeCoreFieldDescription(19, FIT_BASE_TYPE_SINT16, L"core_data_quality", L"Q", FIT_MESG_NUM_RECORD, 255);
    fit::FieldDescriptionMesg coreReservedFieldDesc =
        makeCoreFieldDescription(20, FIT_BASE_TYPE_SINT16, L"core_reserved", L"kcal", FIT_MESG_NUM_RECORD, 255);
    fit::FieldDescriptionMesg heatStrainIndexFieldDesc =
        makeCoreFieldDescription(95, FIT_BASE_TYPE_FLOAT32, L"heat_strain_index", L"a.u.", FIT_MESG_NUM_RECORD, 255);
    fit::FieldDescriptionMesg ciqCoreTemperatureFieldDesc =
        makeCoreFieldDescription(81, FIT_BASE_TYPE_FLOAT32, L"CIQ_core_temperature", L"°", FIT_MESG_NUM_RECORD, 255);
    fit::FieldDescriptionMesg ciqSkinTemperatureFieldDesc =
        makeCoreFieldDescription(82, FIT_BASE_TYPE_FLOAT32, L"CIQ_skin_temperature", L"°", FIT_MESG_NUM_RECORD, 255);
    fit::FieldDescriptionMesg lapAvgCoreTemperatureFieldDesc =
        makeCoreFieldDescription(1, FIT_BASE_TYPE_FLOAT32, L"avg_core_temperature", L"°", FIT_MESG_NUM_LAP, 158);
    fit::FieldDescriptionMesg lapMaxCoreTemperatureFieldDesc =
        makeCoreFieldDescription(2, FIT_BASE_TYPE_FLOAT32, L"max_core_temperature", L"°", FIT_MESG_NUM_LAP, 160);
    fit::FieldDescriptionMesg lapMinCoreTemperatureFieldDesc =
        makeCoreFieldDescription(3, FIT_BASE_TYPE_FLOAT32, L"min_core_temperature", L"°", FIT_MESG_NUM_LAP, 159);
    fit::FieldDescriptionMesg sessionAvgCoreTemperatureFieldDesc =
        makeCoreFieldDescription(5, FIT_BASE_TYPE_FLOAT32, L"avg_core_temperature", L"°", FIT_MESG_NUM_SESSION, 208);
    fit::FieldDescriptionMesg sessionMaxCoreTemperatureFieldDesc =
        makeCoreFieldDescription(6, FIT_BASE_TYPE_FLOAT32, L"max_core_temperature", L"°", FIT_MESG_NUM_SESSION, 210);
    fit::FieldDescriptionMesg sessionMinCoreTemperatureFieldDesc =
        makeCoreFieldDescription(7, FIT_BASE_TYPE_FLOAT32, L"min_core_temperature", L"°", FIT_MESG_NUM_SESSION, 209);
    fit::FieldDescriptionMesg ciqDeviceInfoFieldDesc =
        makeCoreFieldDescription(26, FIT_BASE_TYPE_UINT8, L"CIQ_device_info", L"°", FIT_MESG_NUM_SESSION, 255);

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

    auto addCoreTemperatureSummaryFields = [&](fit::Mesg &mesg,
                                               const fit::FieldDescriptionMesg &avgDesc,
                                               const fit::FieldDescriptionMesg &minDesc,
                                               const fit::FieldDescriptionMesg &maxDesc,
                                               double avgCoreTemp,
                                               double minCoreTemp,
                                               double maxCoreTemp) {
        fit::DeveloperField avgCoreTemperatureField(avgDesc, coreDevIdMesg);
        avgCoreTemperatureField.SetFLOAT32Value((float)avgCoreTemp);
        mesg.AddDeveloperField(avgCoreTemperatureField);

        fit::DeveloperField minCoreTemperatureField(minDesc, coreDevIdMesg);
        minCoreTemperatureField.SetFLOAT32Value((float)minCoreTemp);
        mesg.AddDeveloperField(minCoreTemperatureField);

        fit::DeveloperField maxCoreTemperatureField(maxDesc, coreDevIdMesg);
        maxCoreTemperatureField.SetFLOAT32Value((float)maxCoreTemp);
        mesg.AddDeveloperField(maxCoreTemperatureField);
    };

    if (core_temp_count > 0) {
        addCoreTemperatureSummaryFields(sessionMesg,
                                        sessionAvgCoreTemperatureFieldDesc,
                                        sessionMinCoreTemperatureFieldDesc,
                                        sessionMaxCoreTemperatureFieldDesc,
                                        core_temp_sum / core_temp_count,
                                        core_temp_min,
                                        core_temp_max);
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
    encode.Write(coreDevIdMesg);
    
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
    encode.Write(coreSkinTemperatureFieldDesc);
    encode.Write(coreDataQualityFieldDesc);
    encode.Write(coreReservedFieldDesc);
    encode.Write(heatStrainIndexFieldDesc);
    encode.Write(ciqCoreTemperatureFieldDesc);
    encode.Write(ciqSkinTemperatureFieldDesc);
    encode.Write(lapAvgCoreTemperatureFieldDesc);
    encode.Write(lapMaxCoreTemperatureFieldDesc);
    encode.Write(lapMinCoreTemperatureFieldDesc);
    encode.Write(sessionAvgCoreTemperatureFieldDesc);
    encode.Write(sessionMaxCoreTemperatureFieldDesc);
    encode.Write(sessionMinCoreTemperatureFieldDesc);
    encode.Write(ciqDeviceInfoFieldDesc);
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

    // Write workout message with developer metadata fields when workout name exists
    // This keeps workout-related metadata separate from session/activity for better compatibility
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

        // Add developer fields to workout message
        workout.AddDeveloperField(activityTitleField);
        workout.AddDeveloperField(ftpSessionField);
        workout.AddDeveloperField(workoutSourceField);
        if (!pelotonWorkoutId.isEmpty()) {
            workout.AddDeveloperField(pelotonWorkoutIdField);
        }
        if (!pelotonUrl.isEmpty()) {
            workout.AddDeveloperField(pelotonUrlField);
        }
        if (!trainingProgramFile.isEmpty()) {
            workout.AddDeveloperField(trainingProgramFileField);
        }

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

        lapMesg.SetSport(treadmill_activity_sport);
    } else if (type == ELLIPTICAL) {

        lapMesg.SetSport(FIT_SPORT_RUNNING);
    } else if (type == ROWING) {

        lapMesg.SetSport(FIT_SPORT_ROWING);
    } else if (type == JUMPROPE) {

        lapMesg.SetSport(FIT_SPORT_JUMP_ROPE);
    } else if (type == STAIRCLIMBER) {

        lapMesg.SetSport(FIT_SPORT_FITNESS_EQUIPMENT);
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
    double lapCoreTempSum = 0;
    double lapCoreTempMin = 0;
    double lapCoreTempMax = 0;
    int lapCoreTempCount = 0;
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
            fit::DeveloperField coreTemperatureField(coreTemperatureFieldDesc, coreDevIdMesg);
            coreTemperatureField.SetFLOAT32Value((float)sl.coreTemp);
            newRecord.AddDeveloperField(coreTemperatureField);

            fit::DeveloperField ciqCoreTemperatureField(ciqCoreTemperatureFieldDesc, coreDevIdMesg);
            ciqCoreTemperatureField.SetFLOAT32Value((float)sl.coreTemp);
            newRecord.AddDeveloperField(ciqCoreTemperatureField);

            fit::DeveloperField coreDataQualityField(coreDataQualityFieldDesc, coreDevIdMesg);
            coreDataQualityField.SetSINT16Value(20);
            newRecord.AddDeveloperField(coreDataQualityField);

            fit::DeveloperField coreReservedField(coreReservedFieldDesc, coreDevIdMesg);
            coreReservedField.SetSINT16Value(0x7fff);
            newRecord.AddDeveloperField(coreReservedField);
        }
        if (sl.bodyTemp) {
            fit::DeveloperField skinTemperatureField(coreSkinTemperatureFieldDesc, coreDevIdMesg);
            skinTemperatureField.SetFLOAT32Value((float)sl.bodyTemp);
            newRecord.AddDeveloperField(skinTemperatureField);

            fit::DeveloperField ciqSkinTemperatureField(ciqSkinTemperatureFieldDesc, coreDevIdMesg);
            ciqSkinTemperatureField.SetFLOAT32Value((float)sl.bodyTemp);
            newRecord.AddDeveloperField(ciqSkinTemperatureField);
        }
        if (sl.heatStrainIndex) {
            fit::DeveloperField heatStrainIndexField(heatStrainIndexFieldDesc, coreDevIdMesg);
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

        if (sl.coreTemp > 0) {
            double coreTemp = sl.coreTemp;
            lapCoreTempSum += coreTemp;
            if (lapCoreTempCount == 0 || coreTemp < lapCoreTempMin) {
                lapCoreTempMin = coreTemp;
            }
            if (lapCoreTempCount == 0 || coreTemp > lapCoreTempMax) {
                lapCoreTempMax = coreTemp;
            }
            lapCoreTempCount++;
        }

        encode.Write(newRecord);

        // Write HRV messages with RR-intervals (standard FIT format)
        // Each HrvMesg can contain up to 5 RR-interval values
        if (!sl.rrIntervals.isEmpty()) {
            for (int rrIdx = 0; rrIdx < sl.rrIntervals.size(); rrIdx += 5) {
                fit::HrvMesg hrvMesg;
                for (int j = 0; j < 5 && (rrIdx + j) < sl.rrIntervals.size(); j++) {
                    // Convert from milliseconds to seconds for FIT format
                    hrvMesg.SetTime(j, (float)(sl.rrIntervals.at(rrIdx + j) / 1000.0));
                }
                encode.Write(hrvMesg);
            }
        }

        if (sl.lapTrigger) {

            lapMesg.SetTotalDistance((sl.distance - lastLapOdometer) * 1000.0); // meters
            lapMesg.SetTotalElapsedTime(sl.elapsedTime - lastLapTimer);
            lapMesg.SetTotalTimerTime(sl.elapsedTime - lastLapTimer);
            lapMesg.SetEvent(FIT_EVENT_LAP);
            lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
            lapMesg.SetMessageIndex(lap_index++);
            lapMesg.SetLapTrigger(FIT_LAP_TRIGGER_DISTANCE);
            if (type == JUMPROPE)
                lapMesg.SetRepetitionNum(lap_index);
            lastLapTimer = sl.elapsedTime;
            lastLapOdometer = sl.distance;

            fit::LapMesg lapMesgToWrite(lapMesg);
            if (lapCoreTempCount > 0) {
                addCoreTemperatureSummaryFields(lapMesgToWrite,
                                                lapAvgCoreTemperatureFieldDesc,
                                                lapMinCoreTemperatureFieldDesc,
                                                lapMaxCoreTemperatureFieldDesc,
                                                lapCoreTempSum / lapCoreTempCount,
                                                lapCoreTempMin,
                                                lapCoreTempMax);
            }
            encode.Write(lapMesgToWrite);

            lapCoreTempSum = 0;
            lapCoreTempMin = 0;
            lapCoreTempMax = 0;
            lapCoreTempCount = 0;

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
    fit::LapMesg lapMesgToWrite(lapMesg);
    if (lapCoreTempCount > 0) {
        addCoreTemperatureSummaryFields(lapMesgToWrite,
                                        lapAvgCoreTemperatureFieldDesc,
                                        lapMinCoreTemperatureFieldDesc,
                                        lapMaxCoreTemperatureFieldDesc,
                                        lapCoreTempSum / lapCoreTempCount,
                                        lapCoreTempMin,
                                        lapCoreTempMax);
    }
    encode.Write(lapMesgToWrite);

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

        // Check if this is a Workout message with developer fields (new format)
        if (mesg.GetNum() == FIT_MESG_NUM_WORKOUT) {
            printf("Found Workout message with developer fields\n");
            // Read developer fields from workout message (new format)
            for (auto devField : mesg.GetDeveloperFields()) {
                std::string fieldName = devField.GetName();
                if (fieldName == "Activity Title" && workoutName != nullptr) {
                    std::wstring wWorkoutName = devField.GetSTRINGValue(0);
                    *workoutName = QString::fromStdWString(wWorkoutName);
                    printf("   Found Activity Title in workout: %s\n", workoutName->toStdString().c_str());
                } else if (fieldName == "Workout Source" && workoutSource != nullptr) {
                    std::wstring wWorkoutSource = devField.GetSTRINGValue(0);
                    *workoutSource = QString::fromStdWString(wWorkoutSource);
                    printf("   Found Workout Source in workout: %s\n", workoutSource->toStdString().c_str());
                } else if (fieldName == "Peloton Workout ID" && pelotonWorkoutId != nullptr) {
                    std::wstring wPelotonWorkoutId = devField.GetSTRINGValue(0);
                    *pelotonWorkoutId = QString::fromStdWString(wPelotonWorkoutId);
                    printf("   Found Peloton Workout ID in workout: %s\n", pelotonWorkoutId->toStdString().c_str());
                } else if (fieldName == "Peloton URL" && pelotonUrl != nullptr) {
                    std::wstring wPelotonUrl = devField.GetSTRINGValue(0);
                    *pelotonUrl = QString::fromStdWString(wPelotonUrl);
                    printf("   Found Peloton URL in workout: %s\n", pelotonUrl->toStdString().c_str());
                } else if (fieldName == "Training Program File" && trainingProgramFile != nullptr) {
                    std::wstring wTrainingProgramFile = devField.GetSTRINGValue(0);
                    *trainingProgramFile = QString::fromStdWString(wTrainingProgramFile);
                    printf("   Found Training Program File in workout: %s\n", trainingProgramFile->toStdString().c_str());
                }
            }
        }

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
            if (record.IsCoreTemperatureValid()) {
                s.coreTemp = record.GetCoreTemperature();
            }
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

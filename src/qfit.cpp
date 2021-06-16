#include "qfit.h"

#include <cstdlib>
#include <fstream>


#include "fit_date_time.hpp"
#include "fit_encode.hpp"

#include "fit_file_id_mesg.hpp"
#include "fit_mesg_broadcaster.hpp"

qfit::qfit(QObject *parent) : QObject(parent) {}


void qfit::save(const QString &filename, QList<SessionLine> session, bluetoothdevice::BLUETOOTH_TYPE type,
                uint32_t processFlag) {
    std::list<fit::RecordMesg> records;
    fit::Encode encode(fit::ProtocolVersion::V20);
    if (session.isEmpty()) {
        return;
    }
    std::fstream file;
    double startingDistanceOffset = 0.0;
    if (!session.isEmpty()) {
        startingDistanceOffset = session.first().distance;
    }

    file.open(filename.toStdString(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {

        printf("Error opening file ExampleActivity.fit\n");
        return;
    }

    QFile output(filename);
    output.open(QIODevice::WriteOnly);

    fit::FileIdMesg fileIdMesg; // Every FIT file requires a File ID message
    fileIdMesg.SetType(FIT_FILE_ACTIVITY);
    fileIdMesg.SetManufacturer(FIT_MANUFACTURER_DEVELOPMENT);
    fileIdMesg.SetProduct(1);
    fileIdMesg.SetSerialNumber(12345);
    fileIdMesg.SetTimeCreated(session.at(0).time.toSecsSinceEpoch() - 631065600L);

    fit::SessionMesg sessionMesg;
    sessionMesg.SetTimestamp(session.at(0).time.toSecsSinceEpoch() - 631065600L);
    sessionMesg.SetStartTime(session.at(0).time.toSecsSinceEpoch() - 631065600L);
    sessionMesg.SetTotalElapsedTime(session.last().elapsedTime);
    sessionMesg.SetTotalTimerTime(session.last().elapsedTime);
    sessionMesg.SetTotalDistance((session.last().distance - startingDistanceOffset) * 1000.0); // meters
    sessionMesg.SetTotalCalories(session.last().calories);
    sessionMesg.SetTotalMovingTime(session.last().elapsedTime);
    sessionMesg.SetMinAltitude(0);
    sessionMesg.SetMaxAltitude(session.last().elevationGain);
    sessionMesg.SetEvent(FIT_EVENT_SESSION);
    sessionMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    sessionMesg.SetFirstLapIndex(0);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);
    sessionMesg.SetMessageIndex(FIT_MESSAGE_INDEX_RESERVED);

    if (type == bluetoothdevice::TREADMILL) {

        sessionMesg.SetSport(FIT_SPORT_RUNNING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
    } else if (type == bluetoothdevice::ELLIPTICAL) {


        sessionMesg.SetSport(FIT_SPORT_RUNNING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
    } else if (type == bluetoothdevice::ROWING) {

        sessionMesg.SetSport(FIT_SPORT_ROWING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_ROWING);
        if(session.last().totalStrokes)
            sessionMesg.SetTotalStrokes(session.last().totalStrokes);
        if(session.last().avgStrokesRate)
            sessionMesg.SetAvgStrokeCount(session.last().avgStrokesRate);
        if(session.last().maxStrokesRate)
            sessionMesg.SetMaxCadence(session.last().maxStrokesRate);
        if(session.last().avgStrokesLength)
            sessionMesg.SetAvgStrokeDistance(session.last().avgStrokesLength);
    }
    else
    {
        sessionMesg.SetSport(FIT_SPORT_CYCLING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_CYCLING);
    }

    fit::DeveloperDataIdMesg devIdMesg;
    for (FIT_UINT8 i = 0; i < 16; i++) {

        devIdMesg.SetApplicationId(i, i);
    }
    devIdMesg.SetDeveloperDataIndex(0);

    fit::ActivityMesg activityMesg;
    activityMesg.SetTimestamp(session.first().time.toSecsSinceEpoch() - 631065600L);
    activityMesg.SetTotalTimerTime(session.last().elapsedTime);
    activityMesg.SetNumSessions(1);
    activityMesg.SetType(FIT_ACTIVITY_MANUAL);
    activityMesg.SetEvent(FIT_EVENT_WORKOUT);
    activityMesg.SetEventType(FIT_EVENT_TYPE_START);
    activityMesg.SetLocalTimestamp(fit::DateTime((time_t)session.last().time.toSecsSinceEpoch())
                                       .GetTimeStamp()); // seconds since 00:00 Dec d31 1989 in local time zone
    activityMesg.SetEvent(FIT_EVENT_ACTIVITY);
    activityMesg.SetEventType(FIT_EVENT_TYPE_STOP);

    fit::LapMesg lapMesg;
    lapMesg.SetIntensity(FIT_INTENSITY_ACTIVE);
    lapMesg.SetStartTime(session.first().time.toSecsSinceEpoch() - 631065600L);
    lapMesg.SetTimestamp(session.first().time.toSecsSinceEpoch() - 631065600L);
    lapMesg.SetEvent(FIT_EVENT_WORKOUT);
    lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    lapMesg.SetLapTrigger(FIT_LAP_TRIGGER_TIME);
    lapMesg.SetTotalElapsedTime(0);
    lapMesg.SetTotalTimerTime(0);
    if (type == bluetoothdevice::TREADMILL) {

        lapMesg.SetSport(FIT_SPORT_RUNNING);
    } else if (type == bluetoothdevice::ELLIPTICAL) {


        lapMesg.SetSport(FIT_SPORT_RUNNING);
    } else {


        lapMesg.SetSport(FIT_SPORT_CYCLING);
    }

    encode.Open(file);
    encode.Write(fileIdMesg);
    encode.Write(devIdMesg);
    encode.Write(sessionMesg);
    encode.Write(activityMesg);

    fit::DateTime date((time_t)session.first().time.toSecsSinceEpoch());
    SessionLine sl;
    if (processFlag & QFIT_PROCESS_DISTANCENOISE) {
        double distanceOld = -1.0;
        int startIdx = -1;
        for (int i = 0; i < session.length(); i++) {

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
    for (int i = 0; i < session.length(); i++) {

        fit::RecordMesg newRecord;
        sl = session.at(i);
        // fit::DateTime date((time_t)session.at(i).time.toSecsSinceEpoch());
        newRecord.SetHeartRate(sl.heart);
        newRecord.SetCadence(sl.cadence);
        newRecord.SetDistance((sl.distance - startingDistanceOffset) * 1000.0); // meters
        newRecord.SetSpeed(sl.speed / 3.6);                                     // meter per second
        newRecord.SetPower(sl.watt);
        newRecord.SetResistance(sl.resistance);
        newRecord.SetCalories(sl.calories);
        newRecord.SetAltitude(sl.elevationGain);

        // using just the start point as reference in order to avoid pause time
        // strava ignore the elapsed field
        // this workaround could leads an accuracy issue.
        newRecord.SetTimestamp(date.GetTimeStamp() + i);
        encode.Write(newRecord);

        if (sl.lapTrigger) {

            lapMesg.SetTotalElapsedTime(sl.elapsedTime - lapMesg.GetTotalElapsedTime());
            lapMesg.SetTotalTimerTime(sl.elapsedTime - lapMesg.GetTotalTimerTime());

            encode.Write(lapMesg);

            lapMesg.SetStartTime(sl.time.toSecsSinceEpoch() - 631065600L);
            lapMesg.SetTimestamp(sl.time.toSecsSinceEpoch() - 631065600L);
            lapMesg.SetEvent(FIT_EVENT_WORKOUT);
            lapMesg.SetEventType(FIT_EVENT_LAP);
        }
    }

    lapMesg.SetTotalElapsedTime(session.last().elapsedTime - lapMesg.GetTotalElapsedTime());
    lapMesg.SetTotalTimerTime(session.last().elapsedTime - lapMesg.GetTotalTimerTime());
    lapMesg.SetEvent(FIT_EVENT_LAP);
    lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    encode.Write(lapMesg);

    if (!encode.Close()) {

        printf("Error closing encode.\n");
        return;
    }
    file.close();

    printf("Encoded FIT file ExampleActivity.fit.\n");
    return;
}
#include "qfit.h"
#include <fstream>
#include <cstdlib>

#include "fit_encode.hpp"
#include "fit_mesg_broadcaster.hpp"
#include "fit_file_id_mesg.hpp"
#include "fit_date_time.hpp"


qfit::qfit(QObject *parent) : QObject(parent)
{

}

void qfit::save(QString filename, QList<SessionLine> session, bluetoothdevice::BLUETOOTH_TYPE type)
{
    std::list<fit::RecordMesg> records;
    fit::Encode encode( fit::ProtocolVersion::V20 );
    if(!session.length()) return;
    std::fstream file;

    file.open(filename.toStdString(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

    if (!file.is_open())
    {
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
    sessionMesg.SetTotalDistance(session.last().distance * 1000.0); //meters
    sessionMesg.SetTotalCalories(session.last().calories);
    sessionMesg.SetTotalMovingTime(session.last().elapsedTime);
    sessionMesg.SetMinAltitude(0);
    sessionMesg.SetMaxAltitude(session.last().elevationGain);
    sessionMesg.SetEvent(FIT_EVENT_SESSION);
    sessionMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    sessionMesg.SetFirstLapIndex(0);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);
    sessionMesg.SetMessageIndex(FIT_MESSAGE_INDEX_RESERVED);

    if(type == bluetoothdevice::TREADMILL)
    {
        sessionMesg.SetSport(FIT_SPORT_RUNNING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
    }
    else if(type == bluetoothdevice::ELLIPTICAL)
    {
        sessionMesg.SetSport(FIT_SPORT_RUNNING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_VIRTUAL_ACTIVITY);
    }
    else
    {
        sessionMesg.SetSport(FIT_SPORT_CYCLING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_INDOOR_CYCLING);
    }

    fit::DeveloperDataIdMesg devIdMesg;
    for (FIT_UINT8 i = 0; i < 16; i++)
    {
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
    activityMesg.SetLocalTimestamp(fit::DateTime((time_t)session.last().time.toSecsSinceEpoch()).GetTimeStamp());  //seconds since 00:00 Dec d31 1989 in local time zone
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
    if(type == bluetoothdevice::TREADMILL)
    {
        lapMesg.SetSport(FIT_SPORT_RUNNING);
    }
    else if(type == bluetoothdevice::ELLIPTICAL)
    {
        lapMesg.SetSport(FIT_SPORT_RUNNING);
    }
    else
    {
        lapMesg.SetSport(FIT_SPORT_CYCLING);
    }        

    encode.Open(file);
    encode.Write(fileIdMesg);
    encode.Write(devIdMesg);
    encode.Write(sessionMesg);
    encode.Write(activityMesg);

    fit::DateTime date((time_t)session.first().time.toSecsSinceEpoch());
    for (int i = 0; i < session.length(); i++)
    {
        fit::RecordMesg newRecord;
        //fit::DateTime date((time_t)session.at(i).time.toSecsSinceEpoch());
        newRecord.SetHeartRate(session.at(i).heart);
        newRecord.SetCadence(session.at(i).cadence);
        newRecord.SetDistance(session.at(i).distance * 1000.0); //meters
        newRecord.SetSpeed(session.at(i).speed / 3.6); // meter per second
        newRecord.SetPower(session.at(i).watt);
        newRecord.SetResistance(session.at(i).resistance);
        newRecord.SetCalories(session.at(i).calories);
        newRecord.SetAltitude(session.at(i).elevationGain);

        // using just the start point as reference in order to avoid pause time
        // strava ignore the elapsed field
        // this workaround could leads an accuracy issue.
        newRecord.SetTimestamp(date.GetTimeStamp() + i);
        encode.Write(newRecord);

        if(session.at(i).lapTrigger)
        {
            lapMesg.SetTotalElapsedTime(session.at(i).elapsedTime - lapMesg.GetTotalElapsedTime());
            lapMesg.SetTotalTimerTime(session.at(i).elapsedTime - lapMesg.GetTotalTimerTime());

            encode.Write(lapMesg);

            lapMesg.SetStartTime(session.at(i).time.toSecsSinceEpoch() - 631065600L);
            lapMesg.SetTimestamp(session.at(i).time.toSecsSinceEpoch() - 631065600L);
            lapMesg.SetEvent(FIT_EVENT_WORKOUT);
            lapMesg.SetEventType(FIT_EVENT_LAP);
        }
    }

    lapMesg.SetTotalElapsedTime(session.last().elapsedTime - lapMesg.GetTotalElapsedTime());
    lapMesg.SetTotalTimerTime(session.last().elapsedTime - lapMesg.GetTotalTimerTime());
    lapMesg.SetEvent(FIT_EVENT_LAP);
    lapMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    encode.Write(lapMesg);

    if (!encode.Close())
    {
       printf("Error closing encode.\n");
       return;
    }
    file.close();

    printf("Encoded FIT file ExampleActivity.fit.\n");
    return;
}

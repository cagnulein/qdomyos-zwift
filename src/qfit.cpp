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
    sessionMesg.SetTotalElapsedTime(session.last().time.toSecsSinceEpoch() - session.first().time.toSecsSinceEpoch());
    sessionMesg.SetTotalTimerTime(session.last().time.toSecsSinceEpoch() - session.first().time.toSecsSinceEpoch());
    sessionMesg.SetTotalDistance(session.last().distance);
    sessionMesg.SetTotalCalories(session.last().calories);
    sessionMesg.SetTotalMovingTime(session.last().time.toSecsSinceEpoch() - session.first().time.toSecsSinceEpoch());
    sessionMesg.SetEvent(FIT_EVENT_SESSION);
    sessionMesg.SetEventType(FIT_EVENT_TYPE_STOP);
    sessionMesg.SetFirstLapIndex(0);
    sessionMesg.SetTrigger(FIT_SESSION_TRIGGER_ACTIVITY_END);
    sessionMesg.SetMessageIndex(FIT_MESSAGE_INDEX_RESERVED);

    if(type == bluetoothdevice::TREADMILL)
    {
        sessionMesg.SetSport(FIT_SPORT_RUNNING);
        sessionMesg.SetSubSport(FIT_SUB_SPORT_TREADMILL);
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

    for (int i = 0; i < session.length(); i++)
    {
        fit::RecordMesg newRecord;
        fit::DateTime date((time_t)session.at(i).time.toSecsSinceEpoch());
        newRecord.SetHeartRate(session.at(i).heart);
        newRecord.SetCadence(session.at(i).cadence);
        newRecord.SetDistance(session.at(i).distance);
        newRecord.SetSpeed(session.at(i).speed / 3.6); // meter per second
        newRecord.SetPower(session.at(i).watt);
        newRecord.SetResistance(session.at(i).resistance);
        newRecord.SetCalories(session.at(i).calories);
        newRecord.SetAltitude(session.at(i).elevationGain);
        newRecord.SetTimestamp(date.GetTimeStamp());

        records.push_back(newRecord);
    }

    fit::ActivityMesg activityMesg;
    activityMesg.SetTimestamp(session.last().time.toSecsSinceEpoch() - session.first().time.toSecsSinceEpoch());
    activityMesg.SetTotalTimerTime(session.last().time.toSecsSinceEpoch() - session.first().time.toSecsSinceEpoch());
    activityMesg.SetNumSessions(1);
    activityMesg.SetType(FIT_ACTIVITY_MANUAL);
    activityMesg.SetEvent(FIT_EVENT_WORKOUT);
    activityMesg.SetEventType(FIT_EVENT_TYPE_START);
    activityMesg.SetLocalTimestamp(fit::DateTime((time_t)session.last().time.toSecsSinceEpoch()).GetTimeStamp());  //seconds since 00:00 Dec d31 1989 in local time zone
    activityMesg.SetEvent(FIT_EVENT_ACTIVITY);
    activityMesg.SetEventType(FIT_EVENT_TYPE_STOP);

    encode.Open(file);
    encode.Write(fileIdMesg);
    encode.Write(devIdMesg);
    encode.Write(sessionMesg);
    //encode.Write(activityMesg);

    for (auto record : records)
    {
        encode.Write(record);
    }

    if (!encode.Close())
    {
       printf("Error closing encode.\n");
       return;
    }
    file.close();

    printf("Encoded FIT file ExampleActivity.fit.\n");
    return;
}

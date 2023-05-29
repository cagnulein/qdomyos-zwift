#include "csafe.h"

csafe::csafe() {}

void csafe::initializeFlags() {
    flags["ExtendedStartFlag"] = 0xF0;
    flags["StandardStartFlag"] = 0xF1;
    flags["StopFlag"] = 0xF2;
    flags["StuffFlag"] = 0xF3;
}

void csafe::initializeCommands() {
    commands["GetStatus"] = {{"id", 0x80}, {"type", "short"}};
    commands["Reset"] = {{"id", 0x81}, {"type", "short"}};
    commands["GoIdle"] = {{"id", 0x82}, {"type", "short"}};
    commands["GoHaveID"] = {{"id", 0x83}, {"type", "short"}};
    commands["GoInUse"] = {{"id", 0x85}, {"type", "short"}};
    commands["GoFinished"] = {{"id", 0x86}, {"type", "short"}};
    commands["GoReady"] = {{"id", 0x87}, {"type", "short"}};
    commands["BadID"] = {{"id", 0x88}, {"type", "short"}};
    commands["GetVersion"] = {{"id", 0x91}, {"type", "short"}};
    commands["GetID"] = {{"id", 0x92}, {"type", "short"}};
    commands["GetUnits"] = {{"id", 0x93}, {"type", "short"}};
    commands["GetSerial"] = {{"id", 0x94}, {"type", "short"}};
    commands["GetList"] = {{"id", 0x98}, {"type", "short"}};
    commands["GetUtilization"] = {{"id", 0x99}, {"type", "short"}};
    commands["GetMotorCurrent"] = {{"id", 0x9A}, {"type", "short"}};
    commands["GetOdometer"] = {{"id", 0x9B}, {"type", "short"}};
    commands["GetErrorCode"] = {{"id", 0x9C}, {"type", "short"}};
    commands["GetServiceCode"] = {{"id", 0x9D}, {"type", "short"}};
    commands["GetUserCfg1"] = {{"id", 0x9E}, {"type", "short"}};
    commands["GetUserCfg2"] = {{"id", 0x9F}, {"type", "short"}};
    commands["GetTWork"] = {{"id", 0xA0}, {"type", "short"}};
    commands["GetHorizontal"] = {{"id", 0xA1}, {"type", "short"}};
    commands["GetVertical"] = {{"id", 0xA2}, {"type", "short"}};
    commands["GetCalories"] = {{"id", 0xA3}, {"type", "short"}};
    commands["GetProgram"] = {{"id", 0xA4}, {"type", "short"}};
    commands["GetSpeed"] = {{"id", 0xA5}, {"type", "short"}};
    commands["GetPace"] = {{"id", 0xA6}, {"type", "short"}};
    commands["GetCadence"] = {{"id", 0xA7}, {"type", "short"}};
    commands["GetGrade"] = {{"id", 0xA8}, {"type", "short"}};
    commands["GetGear"] = {{"id", 0xA9}, {"type", "short"}};
    commands["GetUpList"] = {{"id", 0xAA}, {"type", "short"}};
    commands["GetUserInfo"] = {{"id", 0xAB}, {"type", "short"}};
    commands["GetTorque"] = {{"id", 0xAC}, {"type", "short"}};
    commands["GetHRCur"] = {{"id", 0xB0}, {"type", "short"}};
    commands["GetHRTZone"] = {{"id", 0xB2}, {"type", "short"}};
    commands["GetMETS"] = {{"id", 0xB3}, {"type", "short"}};
    commands["GetPower"] = {{"id", 0xB4}, {"type", "short"}};
    commands["GetHRAvg"] = {{"id", 0xB5}, {"type", "short"}};
    commands["GetHRMax"] = {{"id", 0xB6}, {"type", "short"}};
    commands["GetUserData1"] = {{"id", 0xBE}, {"type", "short"}};
    commands["GetUserData2"] = {{"id", 0xBF}, {"type", "short"}};
    commands["GetAudioChannel"] = {{"id", 0xC0}, {"type", "short"}};
    commands["GetAudioVolume"] = {{"id", 0xC1}, {"type", "short"}};
    commands["GetAudioMute"] = {{"id", 0xC2}, {"type", "short"}};
    commands["DisplayPopup7"] = {{"id", 0xE1}, {"type", "short"}};
    commands["PMGetWorkoutType"] = {{"id", 0x89}, {"type", "long"}};
    commands["PMGetWorkTime"] = {{"id", 0xA0}, {"type", "long"}};
    commands["PMGetWorkDistance"] = {{"id", 0xA3}, {"type", "long"}};
    commands["PMGetStrokeState"] = {{"id", 0xBF}, {"type", "long"}};
    commands["PMGetDragFactor"] = {{"id", 0xC1}, {"type", "long"}};
}

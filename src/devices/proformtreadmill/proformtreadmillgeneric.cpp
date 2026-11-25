// SPDX-License-Identifier: GPL-3.0-or-later
#include "proformtreadmill.h"
#include "homeform.h"
#include <QSettings>
#include <algorithm>

using namespace FitProProtocol;
namespace {
QByteArray qbytes(const Bytes &b) { return QByteArray(reinterpret_cast<const char *>(b.data()), int(b.size())); }
Bytes bytes(const QByteArray &b) { return Bytes(b.begin(), b.end()); }
bool stoppedMode(int mode) { return mode==1 || mode==4; }
}
void proformtreadmill::genericClearRequests() {
    requestSpeed=-1; requestInclination=-100; requestStart=requestStop=requestPause=-1;
}
void proformtreadmill::genericReset() {
    genericFailed=genericReady=genericArmed=genericOwned=genericStartPending=false;
    genericStopping=genericPausing=genericAwaitRunning=genericArmPartial=false;
    genericWorkoutMode=0; genericActualSpeed=0;
    genericMinSpeed=genericMaxSpeed=genericMinGrade=genericMaxGrade=0;
    genericPending=genericNext=GenericStep::None;
    genericDeadline.invalidate(); genericMotionDeadline.invalidate();
    genericWrites.clear(); genericReply.clear(); genericAssembler.reset();
    genericDeviceInfo.clear(); genericSystemInfo.clear(); genericVersionInfo.clear(); genericUnlock.clear();
    genericClearRequests(); initDone=false;
    lastRefreshCharacteristicChanged=QDateTime::currentDateTime();
}
void proformtreadmill::genericFail(const QString &reason) {
    if (genericFailed) return;
    emit debug(QStringLiteral("FitPro generic FAILED: ")+reason);
    genericFailed=true; genericReady=initDone=false;
    genericPending=genericNext=GenericStep::None;
    genericWrites.clear(); genericReply.clear(); genericAssembler.reset();
    genericClearRequests();
    // A timed-out motor command might have reached the console. Never retry,
    // switch protocols, or claim that communication failure stopped the belt.
    if (homeform::singleton()) homeform::singleton()->setToastRequested(
        tr("FitPro generic communication failed. Use the treadmill's physical STOP button."));
}
void proformtreadmill::genericSend(GenericStep step, uint8_t device, uint8_t opcode, const Bytes &content) {
    if (genericFailed || genericPending!=GenericStep::None) return;
    genericWrites=encode(command(device,opcode,content));
    if (genericWrites.empty()) { genericFail(QStringLiteral("invalid outgoing frame")); return; }
    genericPending=step; genericExpectedOpcode=opcode; genericWriteIndex=0;
    genericReply.clear(); genericAssembler.reset(); genericDeadline.start();
    genericSendNextChunk();
}
void proformtreadmill::genericSendNextChunk() {
    if (genericFailed || genericPending==GenericStep::None || genericWriteIndex>=genericWrites.size()) return;
    if (!gattCommunicationChannelService || !gattWriteCharacteristic.isValid()) {
        genericFail(QStringLiteral("write characteristic unavailable")); return;
    }
    const QByteArray chunk=qbytes(genericWrites[genericWriteIndex]);
    emit debug(QStringLiteral("FitPro >> ")+chunk.toHex(' '));
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,chunk,QLowEnergyService::WriteWithResponse);
}
void proformtreadmill::genericWritten(const QByteArray &value) {
    if (genericFailed || genericPending==GenericStep::None || genericWriteIndex>=genericWrites.size()) return;
    if (value!=qbytes(genericWrites[genericWriteIndex])) { genericFail(QStringLiteral("unexpected BLE write acknowledgement")); return; }
    ++genericWriteIndex; genericSendNextChunk();
}
void proformtreadmill::genericNotification(const QByteArray &value) {
    if (genericFailed || genericPending==GenericStep::None) return;
    emit debug(QStringLiteral("FitPro << ")+value.toHex(' '));
    Bytes frame=genericAssembler.append(bytes(value));
    if (!frame.empty() && genericWriteIndex+1>=genericWrites.size() &&
        frame[0]==4 && frame[2]==genericExpectedOpcode && genericReply.empty()) genericReply=frame;
}
void proformtreadmill::genericWriteFields(GenericStep step, const Fields &fields) {
    const Bytes content=writeFields(fields);
    if (content.empty()) { genericFail(QStringLiteral("invalid field write")); return; }
    genericSend(step,4,2,content);
}
bool proformtreadmill::genericSpeedAllowed(double speed) const {
    QSettings s;
    const double userMin=s.value(QZSettings::treadmill_speed_min,QZSettings::default_treadmill_speed_min).toDouble();
    const double userMax=s.value(QZSettings::treadmill_speed_max,QZSettings::default_treadmill_speed_max).toDouble();
    if (!std::isfinite(userMin) || !std::isfinite(userMax)) return false;
    const double lo=std::max(genericMinSpeed,userMin);
    const double hi=std::min(std::min(genericMaxSpeed,22.0),userMax);
    return speed>0 && within(speed,lo,hi);
}
bool proformtreadmill::genericGradeAllowed(double grade) const {
    QSettings s;
    const double userMin=s.value(QZSettings::treadmill_incline_min,QZSettings::default_treadmill_incline_min).toDouble();
    const double userMax=s.value(QZSettings::treadmill_incline_max,QZSettings::default_treadmill_incline_max).toDouble();
    if (!std::isfinite(userMin) || !std::isfinite(userMax)) return false;
    const double lo=std::max(std::max(genericMinGrade,-3.0),userMin);
    const double hi=std::min(std::min(genericMaxGrade,15.0),userMax);
    return within(grade,lo,hi);
}
void proformtreadmill::genericStartBelt() {
    QSettings s;
    const double speed=std::max(std::max(genericMinSpeed,1.0),s.value(QZSettings::treadmill_speed_min,QZSettings::default_treadmill_speed_min).toDouble());
    genericStartPending=false; requestSpeed=-1; requestInclination=-100;
    if (!genericSpeedAllowed(speed) || speed>2.0 || !genericGradeAllowed(0.0)) {
        genericFail(QStringLiteral("safe initial speed/incline unavailable")); return;
    }
    genericWriteFields(GenericStep::Start,{{0,wireHundredths(speed)},{1,0},{12,2}});
}
void proformtreadmill::genericHandleReply(GenericStep step,const Bytes &frame) {
    if (!successfulReply(frame,genericExpectedOpcode,5)) {
        genericFail(QStringLiteral("console rejected command 0x%1 status %2").arg(int(genericExpectedOpcode),2,16,QLatin1Char('0')).arg(int(frame[3]))); return;
    }
    // Read and write share opcode 02. A delayed telemetry frame cannot
    // satisfy a write acknowledgement (captured successful write is 04 05 02 02 0d).
    if (genericExpectedOpcode == 2 && step != GenericStep::Limits && step != GenericStep::Poll && frame.size() != 5) {
        genericFail(QStringLiteral("unexpected write response length")); return;
    }
    Fields v;
    switch(step) {
    case GenericStep::DeviceInfo: genericDeviceInfo=frame; genericNext=GenericStep::SupportedDevices; return;
    case GenericStep::SupportedDevices: genericNext=GenericStep::SupportedCommands; return;
    case GenericStep::SupportedCommands: genericNext=GenericStep::SystemInfo; return;
    case GenericStep::SystemInfo: genericSystemInfo=frame; genericNext=GenericStep::VersionInfo; return;
    case GenericStep::VersionInfo:
        genericVersionInfo=frame; genericUnlock=computeUnlock(genericDeviceInfo,genericSystemInfo,genericVersionInfo);
        if (genericUnlock.empty()) { genericFail(QStringLiteral("invalid discovery")); return; }
        genericNext=GenericStep::Limits; return;
    case GenericStep::Limits:
        if (!decodeFields(frame,limitFields(),v)) { genericFail(QStringLiteral("invalid limits reply")); return; }
        genericMinGrade=signedHundredths(v.at(28)); genericMaxGrade=signedHundredths(v.at(27));
        genericMinSpeed=v.at(31)/100.0; genericMaxSpeed=v.at(30)/100.0;
        if (genericMaxSpeed<=0 || genericMinSpeed>genericMaxSpeed || genericMinGrade>genericMaxGrade) {
            genericFail(QStringLiteral("invalid machine limits")); return;
        }
        emit debug(QStringLiteral("FitPro limits speed %1..%2 incline %3..%4").arg(genericMinSpeed).arg(genericMaxSpeed).arg(genericMinGrade).arg(genericMaxGrade));
        genericNext=GenericStep::SerialNumber; return;
    case GenericStep::SerialNumber: genericNext=GenericStep::Poll; return;
    case GenericStep::Poll: {
        if (!decodeFields(frame,telemetryFields(),v)) { genericFail(QStringLiteral("invalid telemetry reply")); return; }
        const int mode=int(v.at(12));
        const double target=v.at(0)/100.0, incline=signedHundredths(v.at(1));
        genericActualSpeed=v.at(16)/100.0;
        if (target>genericMaxSpeed || genericActualSpeed>genericMaxSpeed || incline<genericMinGrade || incline>genericMaxGrade) {
            genericFail(QStringLiteral("telemetry outside machine limits")); return;
        }
        genericWorkoutMode=mode;
        // Target fallback is display only, never independent Runn motor feedback.
        const double speed=genericActualSpeed>0 ? genericActualSpeed : ((mode==2||mode==10||mode==11||mode==13)?target:0);
        QSettings settings;
        const double weight=settings.value(QZSettings::weight,QZSettings::default_weight).toDouble();
        const QDateTime now=QDateTime::currentDateTime();
        const qint64 dt=lastRefreshCharacteristicChanged.msecsTo(now);
        Speed=speed; Inclination=incline;
        if (genericReady && dt>0 && dt<=5000) {
            Distance+=speed*dt/3600000.0;
            if (watts(weight)) KCal+=((0.048*watts(weight)+1.19)*weight*3.5/200.0)*dt/60000.0;
        }
        lastRefreshCharacteristicChanged=now;
        if (settings.value(QZSettings::heart_rate_belt_name,QZSettings::default_heart_rate_belt_name).toString().startsWith(QStringLiteral("Disabled"))) {
            const uint32_t pulse=v.at(10)&255;
            if (pulse>0 && pulse<=240) Heart=pulse; else update_hr_from_external();
        }
        if (!cadenceFromAppleWatch() && speed>0 && settings.value(QZSettings::power_sensor_name,QZSettings::default_power_sensor_name).toString().startsWith(QStringLiteral("Disabled"))) {
            const double cadence=calculateCadenceFromSpeed(speed);
            if (cadence>0) { evaluateStepCount(); Cadence=cadence; }
        }
        if (!genericReady) {
            genericClearRequests(); genericReady=initDone=true;
            emit debug(QStringLiteral("FitPro generic telemetry ready"));
            emit connectedAndDiscovered();
        }
        if (genericAwaitRunning && mode==2) { genericAwaitRunning=false; emit tapeStarted(); }
        if (genericStopping && stoppedMode(mode) && genericActualSpeed==0) { genericStopping=false; genericOwned=false; }
        if (genericPausing && mode==3 && genericActualSpeed==0) genericPausing=false;
        if ((genericAwaitRunning||genericStopping||genericPausing) && genericMotionDeadline.isValid() && genericMotionDeadline.elapsed()>10000) {
            genericFail(QStringLiteral("requested workout state not confirmed")); return;
        }
        if (genericOwned && !genericAwaitRunning && !genericStopping && stoppedMode(mode)) {
            genericOwned=false; genericClearRequests();
        }
        return;
    }
    case GenericStep::Unlock: genericNext=GenericStep::ArmStart; return;
    case GenericStep::ArmStart: genericArmPartial=true; genericNext=GenericStep::ArmLockout; return;
    case GenericStep::ArmLockout: genericArmPartial=false; genericArmed=true; genericNext=GenericStep::Poll; return;
    case GenericStep::ToIdle: case GenericStep::Change: genericNext=GenericStep::Poll; return;
    case GenericStep::Start: genericOwned=true; genericAwaitRunning=true; genericMotionDeadline.start(); genericNext=GenericStep::Poll; return;
    case GenericStep::Stop: genericAwaitRunning=genericPausing=false; genericStopping=true; genericMotionDeadline.start(); genericNext=GenericStep::Poll; return;
    case GenericStep::Pause: genericAwaitRunning=false; genericPausing=true; genericMotionDeadline.start(); genericNext=GenericStep::Poll; return;
    case GenericStep::Release: genericArmed=genericArmPartial=false; genericNext=GenericStep::Poll; return;
    case GenericStep::None: return;
    }
}
void proformtreadmill::genericUpdate() {
    if (!m_control || m_control->state()!=QLowEnergyController::DiscoveredState || genericFailed) return;
    if (genericPending!=GenericStep::None) {
        if (!genericReply.empty() && genericWriteIndex==genericWrites.size()) {
            const GenericStep step=genericPending; const Bytes reply=genericReply;
            genericPending=GenericStep::None; genericReply.clear(); genericWrites.clear();
            genericHandleReply(step,reply);
        } else if (genericDeadline.elapsed()>5000) genericFail(QStringLiteral("transaction timeout"));
        return;
    }
    if (genericReady) {
        QSettings settings;
        update_metrics(true,watts(settings.value(QZSettings::weight,QZSettings::default_weight).toDouble()));
        if (requestStop!=-1 || requestPause!=-1 || requestSpeed==0) {
            const bool pause=requestPause!=-1 && requestSpeed!=0;
            genericStartPending=false; genericClearRequests();
            if (genericOwned) {
                genericNext=GenericStep::None;
                genericWriteFields(pause?GenericStep::Pause:GenericStep::Stop,{{12,pause?3u:4u}}); return;
            }
            if (genericNext==GenericStep::ArmStart || genericNext==GenericStep::ArmLockout || genericNext==GenericStep::ToIdle)
                genericNext=GenericStep::Poll;
        }
    } else genericClearRequests();
    const GenericStep next=genericNext; genericNext=GenericStep::None;
    switch(next) {
    case GenericStep::DeviceInfo: genericSend(next,2,0x81); return;
    case GenericStep::SupportedDevices: genericSend(next,4,0x80); return;
    case GenericStep::SupportedCommands: genericSend(next,4,0x88); return;
    case GenericStep::SystemInfo: genericSend(next,2,0x82,{0,0,0}); return;
    case GenericStep::VersionInfo: genericSend(next,2,0x84,{0,0}); return;
    case GenericStep::Limits: genericSend(next,4,2,readFields(limitFields())); return;
    case GenericStep::SerialNumber: genericSend(next,2,0x95); return;
    case GenericStep::Poll: genericSend(next,4,2,readFields(telemetryFields())); return;
    case GenericStep::Unlock: genericSend(next,4,0x90,genericUnlock); return;
    case GenericStep::ArmStart: if (genericStartPending) genericWriteFields(next,{{108,1}}); return;
    case GenericStep::ArmLockout: if (genericStartPending) genericWriteFields(next,{{95,0}}); return;
    default: break;
    }
    if (!genericReady) return;
    if (genericStopping||genericPausing||genericAwaitRunning) {
        requestSpeed=-1; requestInclination=-100; requestStart=-1;
        genericSend(GenericStep::Poll,4,2,readFields(telemetryFields())); return;
    }
    // If Stop cancelled arming after the first write, release that partial
    // state before allowing another explicit Start.
    if (genericArmPartial && !genericStartPending && !genericOwned && stoppedMode(genericWorkoutMode) && genericActualSpeed==0) {
        genericWriteFields(GenericStep::Release,{{108,0}}); return;
    }
    if (requestStart!=-1) {
        requestStart=-1;
        if ((stoppedMode(genericWorkoutMode)&&genericActualSpeed==0&&!genericOwned) ||
            (genericWorkoutMode==3&&genericOwned&&genericActualSpeed==0)) genericStartPending=true;
    }
    if (genericStartPending) {
        if (!genericArmed) { genericSend(GenericStep::Unlock,4,0x90,genericUnlock); return; }
        if (genericWorkoutMode==4&&genericActualSpeed==0) { genericWriteFields(GenericStep::ToIdle,{{12,1}}); return; }
        if ((genericWorkoutMode==1||(genericOwned&&genericWorkoutMode==3))&&genericActualSpeed==0) { genericStartBelt(); return; }
        genericStartPending=false;
    }
    if ((genericArmed||genericArmPartial)&&!genericOwned&&stoppedMode(genericWorkoutMode)&&genericActualSpeed==0) { genericWriteFields(GenericStep::Release,{{108,0}}); return; }
    Fields changes;
    if (genericArmed&&genericOwned&&genericWorkoutMode==2&&autoResistanceEnable) {
        if (requestSpeed!=-1&&genericSpeedAllowed(requestSpeed)) changes[0]=wireHundredths(requestSpeed);
        if (requestInclination!=-100&&genericGradeAllowed(requestInclination)) changes[1]=wireHundredths(requestInclination);
    }
    requestSpeed=-1; requestInclination=-100;
    if (!changes.empty()) genericWriteFields(GenericStep::Change,changes);
    else genericSend(GenericStep::Poll,4,2,readFields(telemetryFields()));
}

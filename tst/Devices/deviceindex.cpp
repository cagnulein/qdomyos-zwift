#include <QRegularExpression>
#include "deviceindex.h"

std::vector<QString> DeviceIndex::allDevices;
QMap<QString, QString> DeviceIndex::allIdentifiers;

static const QRegularExpression validIdentifierRegex("^\\w+$");

const QString DeviceIndex::AddDevice(const QString &name, const QString& identifier)
{
    if(name==nullptr)
        throw std::invalid_argument("Name must not be null.");

    if(identifier==nullptr || identifier=="")
        throw std::invalid_argument("Identifier must be provided.");

    if(!validIdentifierRegex.match(identifier).hasMatch())
        throw std::invalid_argument("Identifier must be alphanumeric");

    if(allIdentifiers.contains(name)) {
        throw std::invalid_argument("Device name already defined: " + name.toStdString());
    }

    allDevices.push_back(name);
    allIdentifiers[name] = identifier;

    return name;
}

DeviceIndex::DeviceIndex() {}


const QString DeviceIndex::Identifier(const QString &deviceName) { return allIdentifiers[deviceName]; }

#define DEFINE_DEVICE(DeviceKey, DeviceName) \
        const QString DeviceIndex::DeviceKey = AddDevice(QStringLiteral(DeviceName), QStringLiteral(#DeviceKey));


DEFINE_DEVICE(ActivioTreadmill, "Activio Treadmill");
DEFINE_DEVICE(ApexBike, "Apex Bike");
DEFINE_DEVICE(AntBike, "Ant Bike");
DEFINE_DEVICE(BHFitnessElliptical, "BH Fitness Elliptical");
DEFINE_DEVICE(BKoolBike, "BKool Bike");
DEFINE_DEVICE(BowflexT216Treadmill, "Bowflex T216 Treadmill");
DEFINE_DEVICE(Crossrope, "Crossrope");
DEFINE_DEVICE(CSafeRower, "CSafe Rower");
DEFINE_DEVICE(CSafeElliptical, "CSafe Elliptical");
DEFINE_DEVICE(CSCBike_Named, "CSC Bike (Named)");
DEFINE_DEVICE(CSCBike, "CSC Bike");
DEFINE_DEVICE(Chronobike, "Chronobike");
DEFINE_DEVICE(ComputrainerBike, "Computrainer Bike");
DEFINE_DEVICE(Concept2SkiErg, "Concept2 Ski Erg");
DEFINE_DEVICE(CyclopsPhantomBike, "Cyclops Phantom Bike");
DEFINE_DEVICE(DeerRunTreadmill, "DeerRun Treadmill");
DEFINE_DEVICE(DomyosBike, "Domyos Bike");
DEFINE_DEVICE(DomyosElliptical, "Domyos Elliptical");
DEFINE_DEVICE(DomyosRower, "Domyos Rower");
DEFINE_DEVICE(DomyosTreadmill, "Domyos Treadmill");
DEFINE_DEVICE(ESLinkerTreadmill, "ES Linker Treadmill");
DEFINE_DEVICE(EchelonConnectSportBike, "Echelon Connect Sport Bike");
DEFINE_DEVICE(EchelonRower, "Echelon Rower");
DEFINE_DEVICE(EchelonStrideTreadmill, "Echelon Stride Treadmill");
DEFINE_DEVICE(EliteSterzoSmart, "Elite Sterzo Smart");
DEFINE_DEVICE(FTMSAccessory, "FTMS Accessory");
DEFINE_DEVICE(FTMSBikeHammer, "FTMS Bike Hammer 64123");
DEFINE_DEVICE(FTMSBikeIConsole, "FTMS Bike IConsole");
DEFINE_DEVICE(FTMSBikeHammerRacerS, "FTMS Bike Hammer Racer S");
DEFINE_DEVICE(FTMSBike, "FTMS Bike");
DEFINE_DEVICE(FTMSBike2, "FTMS Bike 2");
DEFINE_DEVICE(FTMSBike3, "FTMS Bike 3");
DEFINE_DEVICE(FTMSKICKRCORE, "FTMS KICKR CORE");
DEFINE_DEVICE(FTMSRower, "FTMS Rower");
DEFINE_DEVICE(FakeBike, "Fake Bike");
DEFINE_DEVICE(FakeElliptical, "Fake Elliptical");
DEFINE_DEVICE(FakeRower, "Fake Rower");
DEFINE_DEVICE(FakeTreadmill, "Fake Treadmill");
DEFINE_DEVICE(FitPlusBike_MRK_NoSettings, "FitPlus Bike (MRK, no settings)");
DEFINE_DEVICE(FitPlusF5, "FitPlus F5");
DEFINE_DEVICE(FitShowBF, "FitShow BF");
DEFINE_DEVICE(FitShowFS, "FitShow FS");
DEFINE_DEVICE(FitShowSW, "FitShow SW");
DEFINE_DEVICE(FlywheelBike, "Flywheel Bike");
DEFINE_DEVICE(FlywheelLifeFitnessIC8, "Flywheel Life Fitness IC8");
DEFINE_DEVICE(FocusTreadmill, "Focus Treadmill");
DEFINE_DEVICE(HorizonGR7Bike, "Horizon GR7 Bike");
DEFINE_DEVICE(HorizonTreadmill_Bodytone, "Horizon Treadmill (Bodytone)");
DEFINE_DEVICE(HorizonTreadmill_DomyosTC, "Horizon Treadmill (Domyos TC)");
DEFINE_DEVICE(HorizonTreadmill_Toorx, "Horizon Treadmill (Toorx)");
DEFINE_DEVICE(HorizonTreadmill, "Horizon Treadmill");
DEFINE_DEVICE(InspireBike, "Inspire Bike");
DEFINE_DEVICE(KeepBike, "Keep Bike");
DEFINE_DEVICE(KingsmithR1ProTreadmill, "Kingsmith R1 Pro Treadmill");
DEFINE_DEVICE(KingsmithR2Treadmill, "Kingsmith R2 Treadmill");
DEFINE_DEVICE(LifeFitnessTreadmill, "Life Fitness Treadmill");
DEFINE_DEVICE(M3IBike, "M3I Bike");
DEFINE_DEVICE(MCFBike, "MCF Bike");
DEFINE_DEVICE(MepanelBike, "Mepanel Bike");
DEFINE_DEVICE(NPECableBike, "NPE Cable Bike");
DEFINE_DEVICE(NPECableBike_Excluding_FlywheelLifeFitnessIC8, "NPECable Bike (excluding Flywheel Life Fitness IC8)");
DEFINE_DEVICE(NautilusBike, "Nautilus Bike");
DEFINE_DEVICE(NautilusElliptical, "Nautilus Elliptical");
DEFINE_DEVICE(NautilusTreadmill, "Nautilus Treadmill");
DEFINE_DEVICE(NordictrackIFitADBBike, "Nordictrack IFit ADB Bike");
DEFINE_DEVICE(NordictrackIFitADBElliptical, "Nordictrack IFit ADB Elliptical");
DEFINE_DEVICE(NordictrackIFitADBTreadmill, "Nordictrack IFit ADB Treadmill");
DEFINE_DEVICE(NorditrackElliptical, "Norditrack Elliptical");
DEFINE_DEVICE(OctaneElliptical, "Octane Elliptical");
DEFINE_DEVICE(OctaneTreadmill, "Octane Treadmill");
DEFINE_DEVICE(PafersBike, "Pafers Bike");
DEFINE_DEVICE(PafersTreadmill, "Pafers Treadmill");
DEFINE_DEVICE(PitpatBike, "Pitpat Bike");
DEFINE_DEVICE(ProFormRower, "ProForm Rower");
DEFINE_DEVICE(ProFormTreadmill, "ProForm Treadmill");
DEFINE_DEVICE(ProFormWifiBike, "ProForm Wifi Bike");
DEFINE_DEVICE(ProFormTelnetBike, "ProForm Telnet Bike");
DEFINE_DEVICE(ProFormWifiTreadmill, "ProForm Wifi Treadmill");
DEFINE_DEVICE(ProformBike, "Proform Bike");
DEFINE_DEVICE(ProformEllipticalTrainer, "Proform Elliptical Trainer");
DEFINE_DEVICE(ProformElliptical, "Proform Elliptical");
DEFINE_DEVICE(RenphoBike_ToorxFTMS, "Renpho Bike (Toorx FTMS)");
DEFINE_DEVICE(RenphoBike, "Renpho Bike");
DEFINE_DEVICE(RenphoBike2, "Renpho Bike 2");
DEFINE_DEVICE(Schwinn170Bike, "Schwinn 170 Bike");
DEFINE_DEVICE(SchwinnIC4Bike, "Schwinn IC4 Bike");
DEFINE_DEVICE(Shuaa5Treadmill, "Shuaa5 Treadmill");
DEFINE_DEVICE(SkandikaWiryBike, "Skandika Wiry Bike");
DEFINE_DEVICE(SmartRowRower, "Smart Row Rower");
DEFINE_DEVICE(SnodeBikeFS, "Snode Bike FS");
DEFINE_DEVICE(SoleBike, "Sole Bike");
DEFINE_DEVICE(SoleElliptical, "Sole Elliptical");
DEFINE_DEVICE(SoleElliptical2, "Sole Elliptical 2");
DEFINE_DEVICE(SoleF80Treadmill, "Sole F80 Treadmill");
DEFINE_DEVICE(SoleF80Treadmill2, "Sole F80 Treadmill 2");
DEFINE_DEVICE(SoleF85Treadmill, "Sole F85 Treadmill");
DEFINE_DEVICE(SpiritTreadmill, "Spirit Treadmill");
DEFINE_DEVICE(SportsPlusBike, "Sports Plus Bike");
DEFINE_DEVICE(SportsTechBike, "Sports Tech Bike");
DEFINE_DEVICE(StagesBike_Assioma_PowerSensorDisabled, "Stages Bike (Assioma / Power Sensor disabled)");
DEFINE_DEVICE(StagesBike, "Stages Bike");
DEFINE_DEVICE(StagesPowerBike, "Stages Power Bike");
DEFINE_DEVICE(StrydeRunTreadmill_PowerSensor, "StrydeRun Treadmill / Power Sensor");
DEFINE_DEVICE(StrydeRunTreadmill_PowerSensor2, "StrydeRun Treadmill / Power Sensor 2");
DEFINE_DEVICE(TacxNeoBike, "Tacx Neo Bike");
DEFINE_DEVICE(TacxNeo2Bike, "Tacx Neo 2 Bike");
DEFINE_DEVICE(TechnoGymMyRunTreadmillRFComm, "TechnoGym MyRun Treadmill RF Comm");
DEFINE_DEVICE(TechnoGymMyRunTreadmill, "TechnoGym MyRun Treadmill");
DEFINE_DEVICE(ToorxAppGateUSBBike_EnabledInSettings, "Toorx AppGate USB Bike (Enabled in settings)");
DEFINE_DEVICE(ToorxAppGateUSBBike, "Toorx AppGate USB Bike");
DEFINE_DEVICE(ToorxAppGateUSBTreadmill, "Toorx AppGate USB Treadmill");
DEFINE_DEVICE(TrxAppGateUSBEllipticalIConsole, "Toorx AppGate USB Elliptical IConsole+");
DEFINE_DEVICE(ToorxTreadmill, "Toorx Treadmill");
DEFINE_DEVICE(TrueTreadmill, "True Treadmill");
DEFINE_DEVICE(TrueTreadmill2, "True Treadmill 2");
DEFINE_DEVICE(TrxAppGateUSBElliptical, "TrxAppGateUSB Elliptical");
DEFINE_DEVICE(UltrasportBike, "Ultrasport Bike");
DEFINE_DEVICE(WahooKickrSnapBike_KICKRCORE, "Wahoo KICKR CORE");
DEFINE_DEVICE(WahooKickrSarisTrainer, "Wahoo Kickr Saris Trainer");
DEFINE_DEVICE(WahooKickrSnapBike, "Wahoo Kickr Snap Bike");
DEFINE_DEVICE(YesoulBike, "Yesoul Bike");
DEFINE_DEVICE(YpooElliptical, "Ypoo Elliptical");
DEFINE_DEVICE(YpooElliptical2, "Ypoo Elliptical 2");
DEFINE_DEVICE(YpooElliptical3, "Ypoo Elliptical 3");
DEFINE_DEVICE(ZwiftRunpod, "Zwift Runpod");
DEFINE_DEVICE(iConceptBike, "iConcept Bike");
DEFINE_DEVICE(iConceptElliptical, "iConcept Elliptical");
DEFINE_DEVICE(SnodeBikeTF, "Snode Bike TF");
DEFINE_DEVICE(ZiproTreadmill, "Zipro Treadmill");



#undef DEFINE_DEVICE

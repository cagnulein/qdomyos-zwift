from pathlib import Path


def patch_bluetooth():
    path = Path("src/devices/bluetooth.cpp")
    text = path.read_text()
    if "fitPlusRower = new fitplusrower" in text:
        return

    ftms_anchor = '            } else if ((b.name().toUpper().startsWith(QStringLiteral("CR 00")) ||'
    dedicated = '''            } else if (b.name().toUpper().startsWith(QStringLiteral("MRK-R28-")) &&
                       !fitPlusRower && filter) {
                // Support case: matthieu.f.graveleau@gmail.com, debug-Wed_Sep_9_22_16_00_2026.log.txt.
                // The R28 advertises FTMS, but QZ 2.20.29 successfully used the proprietary FFF0 Merach protocol.
                this->setLastBluetoothDevice(b);
                this->stopDiscovery();
                fitPlusRower = new fitplusrower(noWriteResistance, noHeartService, bikeResistanceOffset,
                                                bikeResistanceGain);
                emit deviceConnected(b);
                connect(fitPlusRower, &bluetoothdevice::connectedAndDiscovered, this,
                        &bluetooth::connectedAndDiscovered);
                fitPlusRower->deviceDiscovered(b);
                this->signalBluetoothDeviceConnected(fitPlusRower);
'''
    if ftms_anchor not in text:
        raise RuntimeError("FTMS rower discovery anchor not found")
    text = text.replace(ftms_anchor, dedicated + ftms_anchor, 1)

    lines = text.splitlines(keepends=True)
    removed = False
    for index, line in enumerate(lines):
        if 'startsWith(QStringLiteral("MRK-R28-")) ||' in line:
            del lines[index]
            removed = True
            break
    if not removed:
        raise RuntimeError("MRK-R28 FTMS rower entry not found")
    text = "".join(lines)

    cleanup = '''    if (fitPlusBike) {

        delete fitPlusBike;
        fitPlusBike = nullptr;
    }
'''
    if cleanup not in text:
        raise RuntimeError("FitPlus bike cleanup anchor not found")
    text = text.replace(
        cleanup,
        cleanup + '''    if (fitPlusRower) {

        delete fitPlusRower;
        fitPlusRower = nullptr;
    }
''',
        1,
    )

    device_return = '''    } else if (fitPlusBike) {
        return fitPlusBike;
'''
    if device_return not in text:
        raise RuntimeError("FitPlus bike device() anchor not found")
    text = text.replace(
        device_return,
        device_return + '''    } else if (fitPlusRower) {
        return fitPlusRower;
''',
        1,
    )

    path.write_text(text)


def add_after_matching_line(path, needle, replacement):
    text = path.read_text()
    if replacement in text:
        return
    lines = text.splitlines(keepends=True)
    for index, line in enumerate(lines):
        if needle in line:
            lines.insert(index + 1, line.replace(needle, replacement))
            path.write_text("".join(lines))
            return
    raise RuntimeError(f"Build anchor not found: {needle}")


def patch_build():
    path = Path("src/qdomyos-zwift.pri")
    add_after_matching_line(path, "devices/fitplusbike/fitplusbike.cpp", "devices/fitplusrower/fitplusrower.cpp")
    add_after_matching_line(path, "devices/fitplusbike/fitplusbike.h", "devices/fitplusrower/fitplusrower.h")


def patch_tests():
    path = Path("tst/Devices/devicetestdataindex.cpp")
    text = path.read_text()
    if "DeviceIndex::FitPlusRower_MRK_R28" in text:
        return

    old = '''    // FitPlus MRK
    RegisterNewDeviceTestData(DeviceIndex::FitPlusBike_MRK_NoSettings)
        ->expectDevice<fitplusbike>()
        ->acceptDeviceName("MRK-", DeviceNameComparison::StartsWith)
        ->excluding<ftmsbike>()
        ->excluding<snodebike>();
'''
    new = '''    // FitPlus Rower - Merach R28
    // Reproduces the support case from matthieu.f.graveleau@gmail.com:
    // debug-Wed_Sep_9_22_16_00_2026.log.txt reported MRK-R28-6B51.
    RegisterNewDeviceTestData(DeviceIndex::FitPlusRower_MRK_R28)
        ->expectDevice<fitplusrower>()
        ->acceptDeviceName("MRK-R28-6B51", DeviceNameComparison::Exact);

    // FitPlus MRK
    RegisterNewDeviceTestData(DeviceIndex::FitPlusBike_MRK_NoSettings)
        ->expectDevice<fitplusbike>()
        ->acceptDeviceName("MRK-", DeviceNameComparison::StartsWith)
        ->rejectDeviceName("MRK-R28-", DeviceNameComparison::StartsWithIgnoreCase)
        ->excluding<ftmsbike>()
        ->excluding<fitplusrower>()
        ->excluding<snodebike>();
'''
    if old not in text:
        raise RuntimeError("FitPlus MRK test anchor not found")
    path.write_text(text.replace(old, new, 1))


patch_bluetooth()
patch_build()
patch_tests()

#include <QByteArray>

#include "m3ibiketestdata.h"

/**
 * @brief hex2bytes Converts a hexadecimal string to bytes, 2 characters at a time.
 * @param s A hexidecimal string e.g. "023F4A" to  { 0x02, 0x3F, 0x4A }
 */
static QByteArray hex2bytes(const std::string& s)
{
    QByteArray v;

    for (size_t i = 0; i < s.length(); i +=2)
    {
        std::string slice(s, i, 2);
        uint8_t value = std::stoul(slice, 0, 16);
        v.append(value);
    }
    return v;
}

QBluetoothDeviceInfo M3IBikeTestData::get_bluetoothDeviceInfo(const QBluetoothUuid &uuid, const QString &name, bool valid) {
    // The M3I bike detector looks into the manufacturer data.

    QBluetoothDeviceInfo result = BluetoothDeviceTestData::get_bluetoothDeviceInfo(uuid, name, true);

    if(!valid) {
        result.setManufacturerData(1, QByteArray("Invalid manufacturer data."));
        return result;
    }

    int key=0;
    result.setManufacturerData(key++, hex2bytes("02010639009F00000000000000000014008001"));

    /*
    // more data that has been supplied
    result.setManufacturerData(key++, hex2bytes("02010639009F00000000000000000014008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000014008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000013008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000013008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000012008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000020000000012008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBA000000000000000011008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC000000020000000010008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC000000020000000010008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000F008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000F008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000E008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FBC00000002000000000E008001"));
    result.setManufacturerData(key++, hex2bytes("02010639009F0000000003000000000C000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000C000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000C000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000B000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000B000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000B000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000A000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FDD00000003000000000A000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009F0000000000000000000A000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000009000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000009000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000009000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000008000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000008000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000008000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000007000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FE8000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    result.setManufacturerData(key++, hex2bytes("02010639009FD3000000030000000000000001"));
    */

    return result;
}



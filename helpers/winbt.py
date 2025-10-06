import sys
import logging
import asyncio
import threading
import random
import struct
import binascii

from typing import Any, Union

from bless import (
    BlessServer,
    BlessGATTCharacteristic,
    GATTCharacteristicProperties,
    GATTAttributePermissions,
)

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(name=__name__)

trigger: Union[asyncio.Event, threading.Event]
if sys.platform in ["darwin", "win32"]:
    trigger = threading.Event()
else:
    trigger = asyncio.Event()

def read_request(characteristic: BlessGATTCharacteristic, **kwargs) -> bytearray:
    logger.debug(f"Reading {characteristic.value}")
    return characteristic.value

def write_request(characteristic: BlessGATTCharacteristic, value: Any, **kwargs):
    characteristic.value = value
    logger.debug(f"Char value set to {characteristic.value}")
    if characteristic.value == b"\x0f":
        logger.debug("NICE")
        trigger.set()

def generate_indoor_bike_data():
    # Flags (16 bits)
    flags = (1 << 2) | (1 << 6)  # Instantaneous Cadence and Instantaneous Power present

    speed = random.randint(0, 20000)  # 0-20000

    # Instantaneous Cadence (uint16, 0.5 rpm resolution)
    cadence = random.randint(0, 400)  # 0-200 rpm

    # Instantaneous Power (sint16, watts)
    power = random.randint(10, 50)

    # Pack data into bytes
    data = struct.pack("<HHHh", flags, speed, cadence, power)
    
    return data

def generate_zwift_ride_data():
    data_str = "2308ffbfffff0f1a04080010001a04080110001a04080210001a0408031000"
    data = binascii.unhexlify(data_str)
    return data

async def update_indoor_bike_data(server, service_uuid, char_uuid):
    while True:
        c = server.get_characteristic(char_uuid)
        c.value = bytes(generate_indoor_bike_data())
        server.update_value(service_uuid, char_uuid)
        await asyncio.sleep(1)

async def update_zwift_ride_data(server, service_uuid, char_uuid):
    while True:
        c = server.get_characteristic(char_uuid)
        c.value = bytes(generate_zwift_ride_data())
        server.update_value(service_uuid, char_uuid)    
        await asyncio.sleep(1)


async def run(loop):
    trigger.clear()
    
    # Instantiate the server
    server = BlessServer(name="FTMS Indoor Bike", loop=loop)
    server.read_request_func = read_request
    server.write_request_func = write_request

    # Add Fitness Machine Service
    ftms_uuid = "00001826-0000-1000-8000-00805f9b34fb"
    await server.add_new_service(ftms_uuid)

    # Add Indoor Bike Data Characteristic
    indoor_bike_data_uuid = "00002ad2-0000-1000-8000-00805f9b34fb"
    char_flags = (
        GATTCharacteristicProperties.read
        | GATTCharacteristicProperties.notify
    )
    permissions = GATTAttributePermissions.readable
    await server.add_new_characteristic(
        ftms_uuid, indoor_bike_data_uuid, char_flags, generate_indoor_bike_data(), permissions
    )

    zwift_ride_uuid = "00000001-19ca-4651-86e5-fa29dcdd09d1"
    await server.add_new_service(zwift_ride_uuid)

    syncRxChar = "00000003-19CA-4651-86E5-FA29DCDD09D1"
    syncRx_flags = (
        GATTCharacteristicProperties.write
    )
    syncRx_permissions = GATTAttributePermissions.writeable

    syncTxChar = "00000004-19CA-4651-86E5-FA29DCDD09D1"
    syncTx_flags = (
        GATTCharacteristicProperties.read
        | GATTCharacteristicProperties.indicate
    )
    syncTx_permissions = GATTAttributePermissions.readable

    asyncChar = "00000002-19CA-4651-86E5-FA29DCDD09D1"
    async_flags = (
        GATTCharacteristicProperties.read
        | GATTCharacteristicProperties.notify
    )
    async_permissions = GATTAttributePermissions.readable
    await server.add_new_characteristic(
        zwift_ride_uuid, syncRxChar, syncRx_flags, generate_indoor_bike_data(), syncRx_permissions
    )
    await server.add_new_characteristic(
        zwift_ride_uuid, syncTxChar, syncTx_flags, generate_indoor_bike_data(), syncTx_permissions
    )
    await server.add_new_characteristic(
        zwift_ride_uuid, asyncChar, async_flags, generate_zwift_ride_data(), async_permissions
    )

    logger.debug(server.get_characteristic(indoor_bike_data_uuid))
    await server.start()
    logger.debug("Advertising")
    logger.info(f"FTMS Indoor Bike is now advertising")

    # Start updating the indoor bike data
    update_task = asyncio.create_task(update_indoor_bike_data(server, ftms_uuid, indoor_bike_data_uuid))
    update_task_zwift_ride = asyncio.create_task(update_zwift_ride_data(server, zwift_ride_uuid, asyncChar))

    await asyncio.sleep(99999999)
    await server.stop()

loop = asyncio.get_event_loop()
loop.run_until_complete(run(loop))
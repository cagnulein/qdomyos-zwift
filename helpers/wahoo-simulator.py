import sys
import logging
import asyncio
import threading
import random
import struct
import binascii
import time
from typing import Any, Union

# Verificare che siamo su macOS
if sys.platform != 'darwin':
    print("Questo script è progettato specificamente per macOS!")
    sys.exit(1)

# Importare bless
try:
    from bless import (
        BlessServer,
        BlessGATTCharacteristic,
        GATTCharacteristicProperties,
        GATTAttributePermissions,
    )
except ImportError:
    print("Errore: impossibile importare bless. Installarlo con: pip install bless")
    sys.exit(1)

# Configurazione logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Trigger per eventi
trigger = threading.Event()

# Informazioni sul dispositivo
DEVICE_NAME = "Wahoo KICKR 51A6"

# UUID dei servizi standard
CYCLING_POWER_SERVICE = "00001818-0000-1000-8000-00805f9b34fb"
USER_DATA_SERVICE = "0000181c-0000-1000-8000-00805f9b34fb"
FITNESS_MACHINE_SERVICE = "00001826-0000-1000-8000-00805f9b34fb"

# UUID dei servizi Wahoo personalizzati
WAHOO_SERVICE_1 = "a026ee01-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_SERVICE_3 = "a026ee03-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_SERVICE_6 = "a026ee06-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_SERVICE_B = "a026ee0b-0a7d-4ab3-97fa-f1500f9feb8b"

# UUID delle caratteristiche standard
CYCLING_POWER_MEASUREMENT = "00002a63-0000-1000-8000-00805f9b34fb"
CYCLING_POWER_FEATURE = "00002a65-0000-1000-8000-00805f9b34fb"
SENSOR_LOCATION = "00002a5d-0000-1000-8000-00805f9b34fb"
CYCLING_POWER_CONTROL_POINT = "00002a66-0000-1000-8000-00805f9b34fb"
WEIGHT = "00002a98-0000-1000-8000-00805f9b34fb"
FITNESS_MACHINE_FEATURE = "00002acc-0000-1000-8000-00805f9b34fb"
TRAINING_STATUS = "00002ad3-0000-1000-8000-00805f9b34fb"
FITNESS_MACHINE_CONTROL_POINT = "00002ad9-0000-1000-8000-00805f9b34fb"
FITNESS_MACHINE_STATUS = "00002ada-0000-1000-8000-00805f9b34fb"
INDOOR_BIKE_DATA = "00002ad2-0000-1000-8000-00805f9b34fb"

# UUID delle caratteristiche Wahoo personalizzate
WAHOO_CUSTOM_CP_CHAR = "a026e005-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_CHAR_1 = "a026e002-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_CHAR_2 = "a026e004-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_CHAR_3 = "a026e00a-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_CHAR_4 = "a026e023-0a7d-4ab3-97fa-f1500f9feb8b"
WAHOO_CHAR_5 = "a026e037-0a7d-4ab3-97fa-f1500f9feb8b"

# Stato dispositivo - variabili globali
current_power = 120
current_cadence = 85
current_speed = 25.0
current_resistance = 5

# Funzioni di callback
def read_request(characteristic, **kwargs):
    logger.debug(f"Lettura: {characteristic.value}")
    return characteristic.value

def write_request(characteristic, value, **kwargs):
    uuid_str = str(characteristic.uuid).lower()
    logger.info(f"Scrittura su caratteristica: {uuid_str}, valore: {binascii.hexlify(value)}")
    
    # Gestione delle richieste di scrittura
    if uuid_str == FITNESS_MACHINE_CONTROL_POINT.lower():
        handle_ftms_control_point(value)
    elif uuid_str == CYCLING_POWER_CONTROL_POINT.lower():
        handle_cp_control_point(value)
    elif uuid_str in [WAHOO_CHAR_1.lower(), WAHOO_CHAR_3.lower(), WAHOO_CHAR_4.lower(), WAHOO_CHAR_5.lower()]:
        handle_wahoo_char_write(uuid_str, value)
    
    characteristic.value = value

# Gestori di richieste di scrittura
def handle_ftms_control_point(data):
    global current_power, current_resistance
    
    if not data:
        return
        
    op_code = data[0]
    logger.info(f"Comando FTMS Control Point: {op_code:#x}")
    
    if op_code == 0x05:  # Set Target Power (ERG mode)
        if len(data) >= 3:
            target_power = int.from_bytes(data[1:3], byteorder='little')
            logger.info(f"Target power impostato: {target_power}W")
            current_power = target_power

def handle_cp_control_point(data):
    if not data:
        return
        
    op_code = data[0]
    logger.info(f"Comando CP Control Point: {op_code:#x}")

def handle_wahoo_char_write(uuid_str, data):
    logger.info(f"Scrittura su caratteristica Wahoo {uuid_str}: {binascii.hexlify(data)}")

# Funzioni per generare dati
def generate_cycling_power_data():
    global current_power, current_cadence
    
    # Varia leggermente i valori
    current_power += random.randint(-3, 3)
    current_power = max(0, min(2000, current_power))
    
    current_cadence += random.randint(-1, 1)
    current_cadence = max(0, min(200, current_cadence))
    
    # Crea Cycling Power Measurement
    power_data = bytearray(16)
    power_data[0:2] = (0x0034).to_bytes(2, byteorder='little')
    power_data[2:4] = (current_power).to_bytes(2, byteorder='little')
    power_data[4:8] = (int(current_power * 10)).to_bytes(4, byteorder='little')
    power_data[8:12] = (0).to_bytes(4, byteorder='little')
    power_data[12:14] = (current_cadence).to_bytes(2, byteorder='little')
    power_data[14:16] = (0xBAD8).to_bytes(2, byteorder='little')
    
    return bytes(power_data)

def generate_indoor_bike_data():
    global current_speed, current_cadence
    
    # Varia leggermente i valori
    current_speed += random.uniform(-0.2, 0.2)
    current_speed = max(0, min(60.0, current_speed))
    
    # Crea Indoor Bike Data
    bike_data = bytearray(8)
    bike_data[0:2] = (0x0044).to_bytes(2, byteorder='little')
    bike_data[2:4] = (int(current_speed * 100)).to_bytes(2, byteorder='little')
    bike_data[4:6] = (current_cadence).to_bytes(2, byteorder='little')
    bike_data[6:8] = (0).to_bytes(2, byteorder='little')
    
    return bytes(bike_data)

async def run():
    # Crea server con minimo di parametri
    server = BlessServer(name=DEVICE_NAME)
    server.read_request_func = read_request
    server.write_request_func = write_request
    
    logger.info(f"Configurazione del simulatore {DEVICE_NAME}...")
    
    # 1. Servizi standard
    # Aggiungi Cycling Power Service
    await server.add_new_service(CYCLING_POWER_SERVICE)
    await server.add_new_characteristic(
        CYCLING_POWER_SERVICE,
        CYCLING_POWER_MEASUREMENT,
        GATTCharacteristicProperties.read | GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable
    )
    await server.add_new_characteristic(
        CYCLING_POWER_SERVICE,
        CYCLING_POWER_FEATURE,
        GATTCharacteristicProperties.read,
        None,
        GATTAttributePermissions.readable
    )
    await server.add_new_characteristic(
        CYCLING_POWER_SERVICE,
        CYCLING_POWER_CONTROL_POINT,
        GATTCharacteristicProperties.write | GATTCharacteristicProperties.indicate,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    await server.add_new_characteristic(
        CYCLING_POWER_SERVICE,
        WAHOO_CUSTOM_CP_CHAR,
        GATTCharacteristicProperties.write | GATTCharacteristicProperties.indicate,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    
    # Aggiungi Fitness Machine Service
    await server.add_new_service(FITNESS_MACHINE_SERVICE)
    await server.add_new_characteristic(
        FITNESS_MACHINE_SERVICE,
        INDOOR_BIKE_DATA,
        GATTCharacteristicProperties.read | GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable
    )
    await server.add_new_characteristic(
        FITNESS_MACHINE_SERVICE,
        FITNESS_MACHINE_CONTROL_POINT,
        GATTCharacteristicProperties.write | GATTCharacteristicProperties.indicate,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    await server.add_new_characteristic(
        FITNESS_MACHINE_SERVICE,
        FITNESS_MACHINE_FEATURE,
        GATTCharacteristicProperties.read,
        None,
        GATTAttributePermissions.readable
    )
    
    # 2. Servizi Wahoo personalizzati
    # Wahoo Service 1
    await server.add_new_service(WAHOO_SERVICE_1)
    await server.add_new_characteristic(
        WAHOO_SERVICE_1,
        WAHOO_CHAR_1,
        GATTCharacteristicProperties.write_without_response | GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    await server.add_new_characteristic(
        WAHOO_SERVICE_1,
        WAHOO_CHAR_2,
        GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable
    )
    
    # Wahoo Service 3
    await server.add_new_service(WAHOO_SERVICE_3)
    await server.add_new_characteristic(
        WAHOO_SERVICE_3,
        WAHOO_CHAR_3,
        GATTCharacteristicProperties.write_without_response | GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    
    # Wahoo Service 6
    await server.add_new_service(WAHOO_SERVICE_6)
    await server.add_new_characteristic(
        WAHOO_SERVICE_6,
        WAHOO_CHAR_4,
        GATTCharacteristicProperties.write_without_response | GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    
    # Wahoo Service B
    await server.add_new_service(WAHOO_SERVICE_B)
    await server.add_new_characteristic(
        WAHOO_SERVICE_B,
        WAHOO_CHAR_5,
        GATTCharacteristicProperties.read | GATTCharacteristicProperties.write_without_response | GATTCharacteristicProperties.notify,
        None,
        GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    )
    
    logger.info("Configurazione dei servizi completata")
    
    # Avvia il server
    await server.start()
    logger.info(f"{DEVICE_NAME} è ora in fase di advertising")
    
    # Imposta i valori iniziali DOPO l'avvio del server
    # Valori per servizi standard
    server.get_characteristic(CYCLING_POWER_MEASUREMENT).value = generate_cycling_power_data()
    server.get_characteristic(CYCLING_POWER_FEATURE).value = (0x08).to_bytes(4, byteorder='little')
    server.get_characteristic(INDOOR_BIKE_DATA).value = generate_indoor_bike_data()
    server.get_characteristic(FITNESS_MACHINE_FEATURE).value = (0x02C6).to_bytes(4, byteorder='little')
    
    # Valori per caratteristiche Wahoo
    server.get_characteristic(WAHOO_CHAR_1).value = bytearray(1)
    server.get_characteristic(WAHOO_CHAR_2).value = bytearray(1)
    server.get_characteristic(WAHOO_CHAR_3).value = bytearray(1)
    server.get_characteristic(WAHOO_CHAR_4).value = bytearray(1)
    server.get_characteristic(WAHOO_CHAR_5).value = bytearray(1)
    
    # Loop di aggiornamento
    try:
        counter = 0
        while True:
            # Aggiorna i dati principali
            server.get_characteristic(INDOOR_BIKE_DATA).value = generate_indoor_bike_data()
            server.get_characteristic(CYCLING_POWER_MEASUREMENT).value = generate_cycling_power_data()
            
            # Invia notifiche
            server.update_value(FITNESS_MACHINE_SERVICE, INDOOR_BIKE_DATA)
            server.update_value(CYCLING_POWER_SERVICE, CYCLING_POWER_MEASUREMENT)
            
            if counter % 10 == 0:  # Log ogni 10 cicli
                logger.info(f"Potenza: {current_power}W, Cadenza: {current_cadence}rpm, Velocità: {current_speed:.1f}km/h")
            
            counter += 1
            await asyncio.sleep(0.1)
            
    except KeyboardInterrupt:
        logger.info("Arresto richiesto dall'utente")
    except Exception as e:
        logger.error(f"Errore durante l'esecuzione: {e}")
    finally:
        await server.stop()
        logger.info("Server arrestato")

if __name__ == "__main__":
    print("=" * 80)
    print(f"Wahoo KICKR 51A6 BLE Simulator per macOS (Versione completa)")
    print("=" * 80)
    print(f"Avvio della simulazione di {DEVICE_NAME}")
    print("Premi Ctrl+C per terminare il server")
    print("=" * 80)
    
    try:
        asyncio.run(run())
    except KeyboardInterrupt:
        print("\nSimulazione fermata dall'utente")
    except Exception as e:
        print(f"Errore: {e}")
        print("Potrebbe essere necessario eseguire questo script con sudo")
        sys.exit(1)
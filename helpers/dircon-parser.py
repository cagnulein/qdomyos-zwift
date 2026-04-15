from dataclasses import dataclass
from typing import List, Optional, Tuple
import re

@dataclass
class HubRidingData:
    power: Optional[int] = None
    cadence: Optional[int] = None
    speed_x100: Optional[int] = None
    hr: Optional[int] = None
    unknown1: Optional[int] = None
    unknown2: Optional[int] = None

    def __str__(self):
        return (f"Power={self.power}W Cadence={self.cadence}rpm "
                f"Speed={self.speed_x100/100 if self.speed_x100 else 0:.1f}km/h "
                f"HR={self.hr}bpm Unknown1={self.unknown1} Unknown2={self.unknown2}")

def parse_protobuf_varint(data: bytes, offset: int = 0) -> Tuple[int, int]:
    result = 0
    shift = 0
    while offset < len(data):
        byte = data[offset]
        result |= (byte & 0x7F) << shift
        offset += 1
        if not (byte & 0x80):
            break
        shift += 7
    return result, offset

def parse_hub_riding_data(data: bytes) -> Optional[HubRidingData]:
    try:
        riding_data = HubRidingData()
        offset = 0
        while offset < len(data):
            key, new_offset = parse_protobuf_varint(data, offset)
            wire_type = key & 0x07
            field_number = key >> 3
            offset = new_offset
            
            if wire_type == 0:
                value, offset = parse_protobuf_varint(data, offset)
                if field_number == 1:
                    riding_data.power = value
                elif field_number == 2:
                    riding_data.cadence = value
                elif field_number == 3:
                    riding_data.speed_x100 = value
                elif field_number == 4:
                    riding_data.hr = value
                elif field_number == 5:
                    riding_data.unknown1 = value
                elif field_number == 6:
                    riding_data.unknown2 = value
        return riding_data
    except Exception as e:
        print(f"Error parsing protobuf: {e}")
        return None

@dataclass
class DirconPacket:
    message_version: int = 1
    identifier: int = 0xFF
    sequence_number: int = 0
    response_code: int = 0
    length: int = 0
    uuid: int = 0
    uuids: List[int] = None
    additional_data: bytes = b''
    is_request: bool = False
    riding_data: Optional[HubRidingData] = None

    def __str__(self):
        uuids_str = ','.join(f'{u:04x}' for u in (self.uuids or []))
        base_str = (f"vers={self.message_version} Id={self.identifier} sn={self.sequence_number} "
                   f"resp={self.response_code} len={self.length} req?={self.is_request} "
                   f"uuid={self.uuid:04x} dat={self.additional_data.hex()} uuids=[{uuids_str}]")
        if self.riding_data:
            base_str += f"\nRiding Data: {self.riding_data}"
        return base_str

def parse_dircon_packet(data: bytes, offset: int = 0) -> Tuple[Optional[DirconPacket], int]:
    if len(data) - offset < 6:
        return None, 0
        
    packet = DirconPacket()
    packet.message_version = data[offset]
    packet.identifier = data[offset + 1]
    packet.sequence_number = data[offset + 2]
    packet.response_code = data[offset + 3]
    packet.length = (data[offset + 4] << 8) | data[offset + 5]
    
    total_length = 6 + packet.length
    if len(data) - offset < total_length:
        return None, 0
        
    curr_offset = offset + 6
    
    if packet.identifier == 0x01:  # DPKT_MSGID_DISCOVER_SERVICES
        if packet.length == 0:
            packet.is_request = True
        elif packet.length % 16 == 0:
            packet.uuids = []
            while curr_offset + 16 <= offset + total_length:
                uuid = (data[curr_offset + 2] << 8) | data[curr_offset + 3]
                packet.uuids.append(uuid)
                curr_offset += 16
                
    elif packet.identifier == 0x02:  # DPKT_MSGID_DISCOVER_CHARACTERISTICS
        if packet.length >= 16:
            packet.uuid = (data[curr_offset + 2] << 8) | data[curr_offset + 3]
            if packet.length == 16:
                packet.is_request = True
            elif (packet.length - 16) % 17 == 0:
                curr_offset += 16
                packet.uuids = []
                packet.additional_data = b''
                while curr_offset + 17 <= offset + total_length:
                    uuid = (data[curr_offset + 2] << 8) | data[curr_offset + 3]
                    packet.uuids.append(uuid)
                    packet.additional_data += bytes([data[curr_offset + 16]])
                    curr_offset += 17
                    
    elif packet.identifier in [0x03, 0x04, 0x05, 0x06]:  # READ/WRITE/NOTIFY characteristics
        if packet.length >= 16:
            packet.uuid = (data[curr_offset + 2] << 8) | data[curr_offset + 3]
            if packet.length > 16:
                packet.additional_data = data[curr_offset + 16:offset + total_length]
                if packet.uuid == 0x0002:
                    packet.riding_data = parse_hub_riding_data(packet.additional_data)
            if packet.identifier != 0x06:
                packet.is_request = True
                
    return packet, total_length

def extract_bytes_from_c_array(content: str) -> List[Tuple[str, bytes]]:
    packets = []
    pattern = r'static const unsigned char (\w+)\[\d+\] = \{([^}]+)\};'
    matches = re.finditer(pattern, content)
    
    for match in matches:
        name = match.group(1)
        hex_str = match.group(2)
        
        hex_values = []
        for line in hex_str.split('\n'):
            line = line.split('//')[0]
            values = re.findall(r'0x[0-9a-fA-F]{2}', line)
            hex_values.extend(values)
        
        byte_data = bytes([int(x, 16) for x in hex_values])
        packets.append((name, byte_data))
    
    return packets

def get_tcp_payload(data: bytes) -> bytes:
    ip_header_start = 14  # Skip Ethernet header
    ip_header_len = (data[ip_header_start] & 0x0F) * 4
    tcp_header_start = ip_header_start + ip_header_len
    tcp_header_len = ((data[tcp_header_start + 12] >> 4) & 0x0F) * 4
    payload_start = tcp_header_start + tcp_header_len
    return data[payload_start:]

def parse_file(filename: str):
    with open(filename, 'r') as f:
        content = f.read()
        packets = extract_bytes_from_c_array(content)
        
        for name, data in packets:
            print(f"\nPacket {name}:")
            payload = get_tcp_payload(data)
            print(f"Dircon payload: {payload.hex()}")
            
            offset = 0
            while offset < len(payload):
                packet, consumed = parse_dircon_packet(payload, offset)
                if packet is None or consumed == 0:
                    break
                print(f"Frame: {packet}")
                offset += consumed

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python script.py <input_file>")
        sys.exit(1)
        
    parse_file(sys.argv[1])

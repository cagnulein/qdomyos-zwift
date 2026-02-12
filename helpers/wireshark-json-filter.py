import json

def generate_code(hex_string, start_index):
    hex_pairs = [hex_string[i:i+2] for i in range(0, len(hex_string), 2)]
    output = ""
    array_name = f"initData{start_index}"
    array_elements = ', '.join([f"0x{hex_pair}" for hex_pair in hex_pairs])
    output += f"uint8_t {array_name}[] = {{{array_elements}}};\n"
    output += f'writeCharacteristic({array_name}, sizeof({array_name}), QStringLiteral("init"), false, false);\n'
    output += "QThread::msleep(sleepms);\n\n"
    return output

json_file_path = "C:\\Work\\qdomyos-zwift\\helpers\\tmp.json"

with open(json_file_path, 'r') as file:
    # Carica i dati JSON
    json_data = json.load(file)


line = 0

for item in json_data:
    try:        
        if(item['_source']['layers']['btatt']['btatt.value_raw'][0] != ''):
            line = line + 1
        print(generate_code(item['_source']['layers']['btatt']['btatt.value_raw'][0], line))
    except:
        pass
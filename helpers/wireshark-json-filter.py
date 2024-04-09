import json

json_file_path = "C:\\Work\\qdomyos-zwift\\helpers\\tmp.json"

with open(json_file_path, 'r') as file:
    # Carica i dati JSON
    json_data = json.load(file)

for item in json_data:
    try:
        print(item['_source']['layers']['btatt']['btatt.value_raw'][0])
    except:
        pass
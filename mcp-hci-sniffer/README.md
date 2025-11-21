# HCI Sniffer MCP Server

MCP server per analizzare i file HCI snoop di Android e generare codice C++ per l'implementazione di nuovi dispositivi ProForm in QDomyos-Zwift.

## Cosa fa questo MCP

Questo MCP server fornisce strumenti per:

1. **Leggere file HCI snoop** di Android (btsnoop_hci.log)
2. **Estrarre pacchetti BLE** (scrivere e notifiche)
3. **Rimuovere automaticamente** i primi 11 byte (header BLE)
4. **Identificare pattern** di inizializzazione vs sendPoll
5. **Generare codice C++** pronto per proformtreadmill.cpp

## Installazione

### 1. Installa le dipendenze

```bash
cd mcp-hci-sniffer
npm install
```

### 2. Configura Claude Code

Aggiungi questo MCP alla configurazione di Claude Code. Modifica il file di configurazione:

**Per Claude Desktop** (`~/Library/Application Support/Claude/claude_desktop_config.json` su macOS):

```json
{
  "mcpServers": {
    "hci-sniffer": {
      "command": "node",
      "args": ["/home/user/qdomyos-zwift/mcp-hci-sniffer/index.js"]
    }
  }
}
```

**Per Claude Code Web** (file `.claude/settings.json` nel progetto):

```json
{
  "mcp": {
    "servers": {
      "hci-sniffer": {
        "command": "node",
        "args": ["./mcp-hci-sniffer/index.js"]
      }
    }
  }
}
```

### 3. Riavvia Claude Code

Dopo aver modificato la configurazione, riavvia Claude Code perché il nuovo MCP sia disponibile.

## Come catturare i file HCI snoop su Android

### Passo 1: Abilita il logging Bluetooth
1. Vai in **Impostazioni** → **Opzioni sviluppatore**
2. Attiva **"Abilita log snoop Bluetooth HCI"**
3. Riavvia il dispositivo (su alcuni telefoni)

### Passo 2: Usa il dispositivo Bluetooth
1. Connetti il tuo ProForm treadmill
2. Esegui una sessione completa (inizializzazione + invio comandi)
3. Disconnetti il dispositivo

### Passo 3: Estrai il file di log
```bash
adb pull /sdcard/Android/data/btsnoop_hci.log ./my_device_capture.log
```

oppure

```bash
adb bugreport
# Il file sarà in FS/data/misc/bluetooth/logs/btsnoop_hci.log
```

## Strumenti disponibili

### 1. `parse_hci_snoop`

Analizza il file e mostra tutti i pacchetti.

**Parametri:**
- `file_path`: Path assoluto al file btsnoop_hci.log
- `filter`: `"writes"` (inviati al dispositivo), `"notifications"` (ricevuti), o `"all"`

**Esempio richiesta a Claude:**
```
Usa parse_hci_snoop per leggere /path/to/btsnoop_hci.log e filtra solo i write
```

### 2. `generate_cpp_code`

Genera codice C++ pronto per proformtreadmill.cpp

**Parametri:**
- `file_path`: Path assoluto al file
- `remove_ble_header`: `true` (rimuove i primi 11 byte) o `false`
- `auto_split`: `true` (separa automaticamente init/sendPoll) o `false`

**Esempio richiesta a Claude:**
```
Genera codice C++ dal file /path/to/btsnoop_hci.log, rimuovi i BLE header e separa init da sendPoll
```

### 3. `analyze_packet_sequence`

Analizza la sequenza per trovare il boundary tra init e sendPoll.

**Parametri:**
- `file_path`: Path assoluto al file

**Esempio richiesta a Claude:**
```
Analizza la sequenza di pacchetti in /path/to/btsnoop_hci.log
```

## Workflow completo per aggiungere un nuovo ProForm

### Passo 1: Cattura il file HCI snoop
```bash
adb pull /sdcard/Android/data/btsnoop_hci.log ./proform_newmodel.log
```

### Passo 2: Chiedi a Claude di analizzare
```
Analizza il file proform_newmodel.log con analyze_packet_sequence
```

### Passo 3: Genera il codice C++
```
Genera codice C++ dal file proform_newmodel.log
```

### Passo 4: Claude integrerà automaticamente
Claude userà il codice generato per:
1. Aggiungere il flag al .h
2. Aggiungere le impostazioni in qzsettings.h/cpp
3. Aggiornare settings.qml
4. Implementare btinit() e sendPoll() in proformtreadmill.cpp

## Usare questo MCP in altri progetti

✅ **SÌ!** Una volta installato, questo MCP è disponibile in **qualsiasi progetto** dove usi Claude Code.

Per usarlo in un altro progetto:
1. Il MCP è già configurato globalmente (se usato con Claude Desktop)
2. Basta chiedere a Claude di usare gli strumenti HCI sniffer
3. Fornisci il path al file .log del nuovo progetto

## Esempio di utilizzo

```
Claude, ho catturato un file HCI snoop per un nuovo treadmill ProForm.
Il file è /home/user/captures/proform_505.log

Puoi:
1. Analizzare il file per trovare il boundary init/sendPoll
2. Generare il codice C++
3. Integrarlo nel progetto seguendo il pattern del ProForm 995i
```

## Dettagli tecnici

### Formato btsnoop_hci.log
- **Header file**: 16 bytes (magic "btsnoop\0" + version + datalink)
- **Record header**: 24 bytes per pacchetto
- **Packet data**: variabile

### Parsing BLE
- Estrae pacchetti HCI ACL Data (type 0x02)
- Identifica canale L2CAP 0x0004 (ATT)
- Filtra ATT Write Command (0x52) e Write Request (0x12)
- Rimuove automaticamente i primi 11 byte come da CLAUDE.md

### Pattern detection
- Cerca sequenze ripetitive per identificare sendPoll
- Tutto prima del pattern ripetuto = initialization
- Pattern ripetuto = sendPoll frames

## Risoluzione problemi

### "Invalid btsnoop file format"
- Verifica che il file sia un vero btsnoop_hci.log di Android
- Alcuni file bugreport comprimono il file, decomprimi prima

### "No write packets found"
- Il dispositivo potrebbe usare un altro metodo di comunicazione
- Prova con filter="all" per vedere tutti i pacchetti

### MCP non disponibile in Claude
- Verifica la configurazione in claude_desktop_config.json
- Controlla che il path sia assoluto
- Riavvia Claude Desktop/Code

## Licenza

GPL-3.0 (come QDomyos-Zwift)

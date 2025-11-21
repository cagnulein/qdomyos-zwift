# Esempi di utilizzo HCI Sniffer MCP

## Scenario 1: Nuovo ProForm Treadmill da zero

### Passo 1: Cattura il file
```bash
# Sul tuo Android, abilita HCI snoop nelle opzioni sviluppatore
# Connetti il treadmill ProForm
# Esegui una sessione completa
# Estrai il file

adb pull /sdcard/Android/data/btsnoop_hci.log ./proform_505_cse.log
```

### Passo 2: Analisi preliminare
Chiedi a Claude:
```
Analizza il file /home/user/captures/proform_505_cse.log con analyze_packet_sequence
```

Risposta di Claude (esempio):
```json
{
  "totalPackets": 158,
  "uniquePayloads": 45,
  "repeatBoundaryIndex": 25,
  "initFramesCount": 25,
  "sendPollFramesCount": 133
}
```

### Passo 3: Visualizza i pacchetti
```
Usa parse_hci_snoop per leggere /home/user/captures/proform_505_cse.log
filtrando solo writes
```

### Passo 4: Genera codice C++
```
Genera codice C++ dal file /home/user/captures/proform_505_cse.log
con rimozione automatica BLE header e auto-split di init/sendPoll
```

### Passo 5: Integrazione
```
Aggiungi supporto per ProForm 505 CSE usando il codice generato,
seguendo il pattern del ProForm 995i nel file CLAUDE.md
```

## Scenario 2: Debug di pacchetti esistenti

Hai già implementato un dispositivo ma qualcosa non funziona?

```
Confronta i pacchetti nel file /path/to/new_capture.log
con l'implementazione attuale di proform_treadmill_8_0
```

Claude può:
1. Estrarre i pacchetti dal file
2. Confrontarli con il codice esistente
3. Identificare differenze
4. Suggerire correzioni

## Scenario 3: Reverse engineering di sequenze

Vuoi capire quale comando fa cosa?

```
Mostrami tutti i write commands dal file /path/to/capture.log
con timestamp e payload per capire la sequenza di inizializzazione
```

Claude mostrerà:
```
Packet 0 @ 1234567890: 0xfe, 0x02, 0x08, 0x02, ...
Packet 1 @ 1234567950: 0x00, 0x12, 0x01, 0x00, ...
...
```

## Scenario 4: Trovare comandi specifici

Hai catturato una sessione dove hai cambiato velocità/inclinazione?

```
Dal file /path/to/speed_change.log, mostrami i write commands
raggruppati per handle caratteristica, voglio capire quale handle
controlla la velocità
```

## Scenario 5: Validare implementazione

Prima di fare commit:

```
Genera il codice dal mio file di riferimento /path/to/reference.log
e confrontalo con l'implementazione in src/devices/proformtreadmill/proformtreadmill.cpp
per verificare che tutti i frame siano corretti
```

## Tips & Tricks

### Filtrare per timestamp
```
Mostrami solo i pacchetti tra il timestamp 1000000 e 2000000
dal file /path/to/long_capture.log
```

### Esportare in formati diversi
```
Genera il codice dal file /path/to/capture.log ma senza rimuovere
il BLE header, voglio vedere il payload completo
```

### Pattern analysis
```
Nel file /path/to/capture.log, identifica se ci sono pattern
di polling ogni N pacchetti
```

### Multi-device comparison
```
Ho due file: proform_model_a.log e proform_model_b.log
Confronta le sequenze di inizializzazione per trovare differenze
```

## Domande comuni

**Q: Il file è troppo grande, ci sono troppi pacchetti**
```
Mostrami solo i primi 50 write commands dal file /path/to/big_file.log
```

**Q: Non trovo il boundary tra init e sendPoll**
```
Analizza manualmente i pacchetti e mostrami dove iniziano
le ripetizioni nel file /path/to/unclear.log
```

**Q: Voglio solo le notifiche (risposte dal dispositivo)**
```
Usa parse_hci_snoop sul file /path/to/capture.log
filtrando solo notifications
```

**Q: Come trovo l'handle della caratteristica?**
```
Mostrami tutti gli handle usati nei write commands
dal file /path/to/capture.log
```

## Workflow avanzato

### 1. Cattura multipla per robustezza
Cattura 3 sessioni diverse:
- session1.log (solo inizializzazione)
- session2.log (cambio velocità)
- session3.log (cambio inclinazione)

```
Confronta i tre file e identifica:
1. Quali frame sono sempre uguali (init)
2. Quali cambiano in session2 (velocità)
3. Quali cambiano in session3 (inclinazione)
```

### 2. Incrementale implementation
```
Step 1: Genera solo i frame di init da /path/to/capture.log
Step 2: Implementa e testa solo l'inizializzazione
Step 3: Genera i frame sendPoll
Step 4: Implementa il polling
Step 5: Identifica i frame per forceSpeed/forceIncline
```

### 3. Documentation auto-generata
```
Dal file /path/to/complete_capture.log genera:
1. Codice C++ commentato
2. Tabella con tutti i frame e i loro scopi
3. Diagramma della sequenza di comunicazione
```

## Integration con il progetto

Tutti questi comandi funzionano direttamente in Claude Code mentre lavori su QDomyos-Zwift!

Esempio workflow completo:
```
1. "Analizza proform_newmodel.log"
2. "Genera il codice C++"
3. "Aggiungi il supporto seguendo CLAUDE.md"
4. "Aggiorna anche settings.qml"
5. "Compila e verifica"
```

Claude farà tutto automaticamente usando l'MCP! 🚀

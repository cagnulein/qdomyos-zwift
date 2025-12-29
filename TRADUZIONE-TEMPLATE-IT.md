# Template Traduzioni Italiane - QDomyos-Zwift

## Statistiche
- **Totale stringhe**: 845
- **Da tradurre**: 845 (100%)
- **File**: `src/qdomyos-zwift.ts`

---

## Come Tradurre

### Formato Base
```xml
<message>
    <source>TESTO INGLESE</source>
    <translation type="unfinished"></translation>
</message>
```

### Dopo la Traduzione
```xml
<message>
    <source>TESTO INGLESE</source>
    <translation>TESTO ITALIANO</translation>
</message>
```

**IMPORTANTE**: Rimuovi `type="unfinished"` quando aggiungi la traduzione!

---

## Esempi Pratici di Traduzione

### ✅ Interfaccia Principale

```xml
<!-- PRIMA -->
<message>
    <source>Connection Status</source>
    <translation type="unfinished"></translation>
</message>

<!-- DOPO -->
<message>
    <source>Connection Status</source>
    <translation>Stato Connessione</translation>
</message>
```

```xml
<!-- PRIMA -->
<message>
    <source>Treadmill Connection Status</source>
    <translation type="unfinished"></translation>
</message>

<!-- DOPO -->
<message>
    <source>Treadmill Connection Status</source>
    <translation>Stato Connessione Tapis Roulant</translation>
</message>
```

### ✅ Metriche di Allenamento

```xml
<!-- Speed -->
<message>
    <source>Speed:</source>
    <translation>Velocità:</translation>
</message>

<!-- Cadence -->
<message>
    <source>Cadence:</source>
    <translation>Cadenza:</translation>
</message>

<!-- Heart Rate -->
<message>
    <source>Heart rate (bpm)</source>
    <translation>Frequenza cardiaca (bpm)</translation>
</message>

<!-- Resistance -->
<message>
    <source>Resistance:</source>
    <translation>Resistenza:</translation>
</message>

<!-- Power -->
<message>
    <source>Watt:</source>
    <translation>Watt:</translation>
</message>

<!-- Calories -->
<message>
    <source>Calories (kcal):</source>
    <translation>Calorie (kcal):</translation>
</message>

<!-- Distance -->
<message>
    <source>Odometer (km):</source>
    <translation>Distanza (km):</translation>
</message>

<!-- Pace -->
<message>
    <source>Pace (min/km):</source>
    <translation>Passo (min/km):</translation>
</message>

<!-- Inclination -->
<message>
    <source>Inclination (degrees):</source>
    <translation>Inclinazione (gradi):</translation>
</message>

<!-- Elevation -->
<message>
    <source>Elevation Gain (meters):</source>
    <translation>Dislivello (metri):</translation>
</message>
```

### ✅ Pulsanti e Azioni

```xml
<!-- Train button -->
<message>
    <source>Train me!</source>
    <translation>Allenami!</translation>
</message>

<!-- Start -->
<message>
    <source>Start</source>
    <translation>Inizia</translation>
</message>

<!-- Stop -->
<message>
    <source>Stop</source>
    <translation>Stop</translation>
</message>

<!-- Pause -->
<message>
    <source>Pause</source>
    <translation>Pausa</translation>
</message>

<!-- Resume -->
<message>
    <source>Resume</source>
    <translation>Riprendi</translation>
</message>
```

### ✅ Messaggi e Notifiche

```xml
<!-- New Lap -->
<message>
    <source>New lap started!</source>
    <translation>Nuovo giro iniziato!</translation>
</message>

<!-- Connected -->
<message>
    <source>Connected</source>
    <translation>Connesso</translation>
</message>

<!-- Disconnected -->
<message>
    <source>Disconnected</source>
    <translation>Disconnesso</translation>
</message>

<!-- Connecting -->
<message>
    <source>Connecting...</source>
    <translation>Connessione in corso...</translation>
</message>
```

### ✅ Numeri e Simboli (NON tradurre!)

```xml
<!-- Mantieni i numeri UGUALI -->
<message>
    <source>0</source>
    <translation>0</translation>
</message>

<message>
    <source>0.0</source>
    <translation>0.0</translation>
</message>

<!-- Mantieni simboli UGUALI -->
<message>
    <source>+</source>
    <translation>+</translation>
</message>

<message>
    <source>-</source>
    <translation>-</translation>
</message>
```

---

## ⚠️ Regole Importanti

### 1. **Placeholder - NON MODIFICARE**
I placeholder come `%1`, `%2`, `%n` devono rimanere IDENTICI:

```xml
<!-- CORRETTO ✅ -->
<message>
    <source>Speed: %1 km/h</source>
    <translation>Velocità: %1 km/h</translation>
</message>

<!-- SBAGLIATO ❌ -->
<message>
    <source>Speed: %1 km/h</source>
    <translation>Velocità: km/h</translation>  <!-- Manca %1! -->
</message>
```

### 2. **Tag HTML - Mantieni Uguali**
```xml
<!-- CORRETTO ✅ -->
<message>
    <source>&lt;b&gt;Speed&lt;/b&gt;</source>
    <translation>&lt;b&gt;Velocità&lt;/b&gt;</translation>
</message>
```

### 3. **Punteggiatura - Mantieni Consistente**
```xml
<!-- Se l'originale ha ":" mantienilo -->
<message>
    <source>Speed:</source>
    <translation>Velocità:</translation>
</message>

<!-- Se l'originale NON ha ":" non aggiungerlo -->
<message>
    <source>Speed</source>
    <translation>Velocità</translation>
</message>
```

### 4. **Maiuscole/Minuscole - Rispetta lo Stile**
```xml
<!-- Titolo - Prima lettera maiuscola -->
<message>
    <source>Connection Status</source>
    <translation>Stato Connessione</translation>
</message>

<!-- Tutto minuscolo - mantieni minuscolo -->
<message>
    <source>speed</source>
    <translation>velocità</translation>
</message>

<!-- TUTTO MAIUSCOLO - mantieni maiuscolo -->
<message>
    <source>SPEED</source>
    <translation>VELOCITÀ</translation>
</message>
```

### 5. **Unità di Misura - NON Tradurre**
```xml
<!-- Mantieni km/h, bpm, kcal, meters, etc. -->
<message>
    <source>Speed (km/h)</source>
    <translation>Velocità (km/h)</translation>
</message>

<message>
    <source>Heart rate (bpm)</source>
    <translation>Frequenza cardiaca (bpm)</translation>
</message>
```

---

## 📝 Glossario Termini Comuni

| Inglese | Italiano | Note |
|---------|----------|------|
| Speed | Velocità | |
| Cadence | Cadenza | Giri pedale al minuto |
| Resistance | Resistenza | Livello resistenza bici/ellittica |
| Heart rate | Frequenza cardiaca | |
| Pace | Passo | Minuti per km |
| Watt | Watt | NON tradurre |
| Calories | Calorie | |
| Distance | Distanza | |
| Odometer | Distanza / Contachilometri | |
| Elevation | Dislivello | |
| Inclination | Inclinazione | |
| Treadmill | Tapis Roulant | |
| Bike | Bici | |
| Elliptical | Ellittica | |
| Rower | Vogatore | |
| Train | Allenarsi / Allenamento | Dipende dal contesto |
| Workout | Allenamento | |
| Lap | Giro | |
| Connected | Connesso | |
| Disconnected | Disconnesso | |
| Start | Inizia / Avvia | |
| Stop | Stop / Ferma | |
| Pause | Pausa | |
| Resume | Riprendi | |

---

## 🛠️ Strumenti per Tradurre

### Opzione 1: Editor di Testo (Manuale)
1. Apri `src/qdomyos-zwift.ts` in un editor
2. Cerca `<translation type="unfinished"></translation>`
3. Aggiungi traduzione: `<translation>Testo Italiano</translation>`
4. Salva il file

### Opzione 2: Qt Linguist (GUI - Consigliato)
```bash
linguist src/qdomyos-zwift.ts
```
- Interfaccia grafica intuitiva
- Mostra contesto (dove appare la stringa)
- Evidenzia placeholder e problemi
- Salvataggio automatico

### Opzione 3: Weblate (Online - Più Facile)
1. Setup Weblate (vedi `docs/WEBLATE-SETUP-IT.md`)
2. Traduci via browser
3. Controllo qualità automatico
4. Nessun software da installare

---

## 🎯 Priorità Traduzioni

### Alta Priorità (UI principale)
- ✅ Metriche: Speed, Cadence, Heart rate, Calories, Distance
- ✅ Pulsanti: Start, Stop, Pause, Train me
- ✅ Stati: Connected, Disconnected, Connecting
- ✅ Titoli finestre: Connection Status, Treadmill Status

### Media Priorità (Settings e dialoghi)
- Opzioni configurazione
- Messaggi di errore
- Dialoghi conferma

### Bassa Priorità (Avanzate)
- Log tecnici
- Messaggi debug
- Opzioni avanzate

---

## 📊 Progresso Traduzioni

Dopo aver tradotto, verifica il progresso:

```bash
# Conta stringhe tradotte
grep -c '<translation>' src/qdomyos-zwift.ts

# Conta stringhe da tradurre
grep -c 'translation type="unfinished"' src/qdomyos-zwift.ts
```

---

## ✅ Checklist Qualità

Prima di committare le traduzioni, verifica:

- [ ] Tutti i placeholder (`%1`, `%2`) sono presenti
- [ ] Tag HTML aperti e chiusi correttamente
- [ ] Punteggiatura consistente con originale
- [ ] Unità di misura NON tradotte
- [ ] Maiuscole/minuscole appropriate
- [ ] Lunghezza testo ragionevole (pensa allo spazio UI)
- [ ] Terminologia consistente (usa sempre la stessa parola)
- [ ] File compila senza errori: `lrelease src/qdomyos-zwift.ts`

---

## 🚀 Workflow Completo

```bash
# 1. Estrai nuove stringhe
./scripts/update-translations.sh

# 2. Traduci con Qt Linguist
linguist src/qdomyos-zwift.ts

# 3. Compila traduzioni
./scripts/compile-translations.sh

# 4. Testa l'applicazione
qmake && make
LC_ALL=it_IT.UTF-8 ./qdomyos-zwift

# 5. Verifica tutto appare in italiano
# ...

# 6. Commit
git add src/qdomyos-zwift.ts src/qdomyos-zwift_it.qm
git commit -m "i18n: Add Italian translations for main UI"
```

---

## 💡 Suggerimenti

1. **Traduci in sessioni**: 50-100 stringhe alla volta
2. **Mantieni consistenza**: Usa sempre gli stessi termini
3. **Pensa all'utente**: Usa linguaggio naturale, non letterale
4. **Testa nell'app**: Verifica che le traduzioni non taglino la UI
5. **Chiedi se incerto**: Meglio chiedere che tradurre male

---

## 📞 Supporto

**Dubbi su una traduzione?**
- Apri una issue su GitHub
- Chiedi nel PR delle traduzioni
- Consulta il glossario sopra

**Problemi tecnici?**
- Vedi `docs/TRADUZIONI.md` per la guida completa
- Controlla che il file .ts abbia sintassi XML corretta
- Verifica con `lrelease` che compili senza errori

---

## 🎉 Grazie!

Ogni traduzione aiuta a rendere QDomyos-Zwift accessibile a più persone!

**Totale stringhe**: 845
**Target**: 100% tradotto
**Tempo stimato**: ~4-6 ore per traduttore esperto

Buona traduzione! 🇮🇹

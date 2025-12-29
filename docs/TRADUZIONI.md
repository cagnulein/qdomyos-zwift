# Sistema di Traduzione QDomyos-Zwift

## Panoramica

Il sistema di traduzione è ora completamente automatizzato tramite GitHub Actions e integrato nell'applicazione.

## Come Funziona

### 🤖 Aggiornamento Automatico (GitHub Actions)

**Trigger**: Ogni domenica a mezzanotte (UTC) oppure manualmente

**Cosa fa**:
1. Estrae automaticamente tutte le stringhe traducibili dal codice sorgente
2. Aggiorna il file `src/qdomyos-zwift.ts` con nuove stringhe
3. Crea automaticamente una Pull Request con le modifiche
4. Etichetta la PR come "translations" e "automation"

**Dopo che la PR è creata**:
1. Rivedi le nuove stringhe nel file `.ts`
2. Aggiungi le traduzioni italiane per le stringhe `<translation type="unfinished">`
3. Fai il merge della PR quando le traduzioni sono complete

### 📱 Applicazione delle Traduzioni

Le traduzioni vengono applicate **automaticamente** su tutte le piattaforme:

#### iOS
- Le traduzioni sono **embedded nell'app bundle**
- Il locale di sistema viene rilevato automaticamente
- Se la traduzione non è disponibile, usa l'inglese

#### Android
- Le traduzioni sono **embedded nell'APK**
- Il locale di sistema viene rilevato automaticamente
- Se la traduzione non è disponibile, usa l'inglese

#### Windows
- Le traduzioni sono **embedded nell'eseguibile**
- Il locale di sistema viene rilevato automaticamente
- Se la traduzione non è disponibile, usa l'inglese

#### Linux/macOS
- Le traduzioni sono **embedded nell'eseguibile**
- Il locale di sistema viene rilevato automaticamente
- Se la traduzione non è disponibile, usa l'inglese

### 🔄 Processo Completo

```
┌─────────────────────┐
│  Codice Sorgente    │  (tr("Speed"), qsTr("Distance"))
└──────────┬──────────┘
           │
           │ lupdate (automatico via GitHub Actions)
           ▼
┌─────────────────────┐
│   qdomyos-zwift.ts  │  (File XML con stringhe da tradurre)
└──────────┬──────────┘
           │
           │ Traduttore aggiunge traduzioni
           ▼
┌─────────────────────┐
│   qdomyos-zwift.ts  │  (File XML con traduzioni complete)
└──────────┬──────────┘
           │
           │ lrelease (durante build)
           ▼
┌─────────────────────┐
│ qdomyos-zwift_it.qm │  (File binario compilato)
└──────────┬──────────┘
           │
           │ Embedded via translations.qrc
           ▼
┌─────────────────────┐
│   Applicazione      │  (Carica traduzioni all'avvio)
└─────────────────────┘
```

## Workflow Manuale (Opzionale)

Se preferisci lavorare manualmente:

### 1. Estrarre nuove stringhe
```bash
./scripts/update-translations.sh
```

### 2. Tradurre stringhe

**Opzione A - Qt Linguist (GUI, consigliato)**:
```bash
linguist src/qdomyos-zwift.ts
```

**Opzione B - Editor di testo**:
Apri `src/qdomyos-zwift.ts` e cerca:
```xml
<message>
    <source>Speed</source>
    <translation type="unfinished"></translation>
</message>
```

Aggiungi la traduzione:
```xml
<message>
    <source>Speed</source>
    <translation>Velocità</translation>
</message>
```

### 3. Compilare traduzioni
```bash
./scripts/compile-translations.sh
```

### 4. Ricompilare applicazione
```bash
qmake && make
```

### 5. Testare
```bash
# Linux/macOS
LC_ALL=it_IT.UTF-8 ./qdomyos-zwift

# Windows PowerShell
$env:LANG="it_IT"
.\qdomyos-zwift.exe
```

## Struttura File

```
src/
├── qdomyos-zwift.ts          # File sorgente traduzioni (XML)
├── qdomyos-zwift_it.qm       # File compilato (generato automaticamente)
├── translations.qrc          # File risorse Qt per le traduzioni
└── main.cpp                  # Carica le traduzioni all'avvio

.github/workflows/
└── update-translations.yml   # Workflow automatico GitHub Actions

scripts/
├── update-translations.sh    # Script per estrarre stringhe
└── compile-translations.sh   # Script per compilare traduzioni

docs/
├── TRANSLATIONS.md           # Documentazione completa (inglese)
└── TRADUZIONI.md            # Questa guida (italiano)
```

## Per Sviluppatori

### Rendere stringhe traducibili

**C++**:
```cpp
QString text = tr("Speed: %1 km/h").arg(speed);
```

**QML**:
```qml
Text { text: qsTr("Speed: %1 km/h").arg(speed) }
```

### Best Practices

1. **Sempre usare tr() o qsTr()**
   ```cpp
   // ✓ Corretto
   QString text = tr("Hello");

   // ✗ Sbagliato
   QString text = "Hello";
   ```

2. **Usa placeholder per contenuto dinamico**
   ```cpp
   // ✓ Corretto
   tr("Distance: %1 %2").arg(dist).arg(unit);

   // ✗ Sbagliato (l'ordine delle parole può variare)
   tr("Distance: ") + QString::number(dist) + " " + unit;
   ```

3. **Fornisci contesto quando necessario**
   ```cpp
   tr("Open", "verb - open file");      // Apri
   tr("Open", "adjective - door open"); // Aperto
   ```

## Aggiungere Nuove Lingue

Per aggiungere supporto per una nuova lingua (es. tedesco):

### 1. Modifica `src/qdomyos-zwift.pri`
```qmake
TRANSLATIONS += \
    $$PWD/qdomyos-zwift.ts \
    $$PWD/qdomyos-zwift_de.ts
```

### 2. Crea file traduzione
```bash
lupdate src/qdomyos-zwift.pri -ts src/qdomyos-zwift_de.ts
```

### 3. Modifica `src/translations.qrc`
```xml
<RCC>
    <qresource prefix="/translations">
        <file>qdomyos-zwift_it.qm</file>
        <file>qdomyos-zwift_de.qm</file>
    </qresource>
</RCC>
```

### 4. Traduci e ricompila
```bash
# Traduci il file .ts
linguist src/qdomyos-zwift_de.ts

# Ricompila
qmake && make
```

## Risoluzione Problemi

### Le traduzioni non si caricano
1. Verifica il locale: aggiungi `qDebug() << QLocale::system().name();` in `main.cpp`
2. Controlla che il file `.qm` esista in `translations.qrc`
3. Verifica i messaggi nella console all'avvio dell'app

### Le stringhe non si aggiornano
1. Esegui `lupdate` per estrarre nuove stringhe
2. Ricompila con `make clean && make`
3. Verifica che le stringhe usino `tr()` o `qsTr()`

### Errori di build
1. Assicurati che Qt Linguist tools sia installato
2. Controlla la sintassi XML dei file `.ts`
3. Verifica che `translations.qrc` elenchi tutti i file `.qm`

## FAQ

**Q: Le traduzioni funzionano offline?**
A: Sì! Le traduzioni sono embedded nell'applicazione, non serve connessione.

**Q: Come forzo una lingua specifica?**
A: Modifica `main.cpp` e sostituisci `QLocale::system().name()` con `"it_IT"` (o altra lingua).

**Q: Posso tradurre anche le immagini?**
A: No, ma puoi caricare immagini diverse in base al locale usando le Qt Resources.

**Q: Quanto spesso viene eseguito il workflow automatico?**
A: Ogni domenica a mezzanotte UTC, oppure puoi eseguirlo manualmente da GitHub Actions.

**Q: Devo fare rebuild per ogni modifica al file .ts?**
A: Sì, le traduzioni devono essere compilate e l'app ribuildata per vedere i cambiamenti.

## Risorse Utili

- [Documentazione Qt Linguist](https://doc.qt.io/qt-5/qtlinguist-index.html)
- [Internazionalizzazione Qt](https://doc.qt.io/qt-5/internationalization.html)
- [File .ts su questo progetto](../src/qdomyos-zwift.ts)
- [Workflow GitHub Actions](../.github/workflows/update-translations.yml)

## Supporto

Per problemi o domande:
1. Apri una issue su GitHub
2. Contatta il maintainer del progetto
3. Consulta la documentazione completa in `docs/TRANSLATIONS.md`

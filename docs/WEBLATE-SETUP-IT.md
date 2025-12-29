# Guida Setup Weblate per QDomyos-Zwift

## Panoramica

Weblate è uno strumento di traduzione web che permette anche ai non-programmatori di contribuire facilmente alle traduzioni. Questa guida spiega come configurarlo per QDomyos-Zwift.

## Perché Weblate?

- ✅ **Gratis per progetti open source**
- ✅ **Supporto nativo per file Qt `.ts`**
- ✅ **Integrazione diretta con GitHub**
- ✅ **Interfaccia semplice per i traduttori**
- ✅ **Controlli di qualità automatici**
- ✅ **Memoria di traduzione e suggerimenti**
- ✅ **Più traduttori possono lavorare insieme**
- ✅ **Interfaccia mobile-friendly**

## Opzioni di Setup

### Opzione 1: Weblate Cloud (Consigliata) ⭐

**Pro**: Nessuna manutenzione, setup istantaneo, gratis per open source
**Contro**: Dati ospitati da Weblate

**Passi**:

1. **Registrati su https://hosted.weblate.org**
   - Usa il tuo account GitHub per registrarti
   - È gratis per progetti open source

2. **Crea Nuovo Progetto**
   - Click su "Add new translation project"
   - Nome: "QDomyos-Zwift"
   - URL: https://github.com/cagnulein/qdomyos-zwift
   - Licenza: Seleziona la licenza del progetto
   - Marca come "Public" e "Open source"

3. **Aggiungi Componente**
   - Click su "Add new translation component"
   - Nome: "Application"
   - Repository: https://github.com/cagnulein/qdomyos-zwift.git
   - Branch: master (o main)
   - File mask: `src/qdomyos-zwift*.ts`
   - File format: "Qt Linguist Translation File"
   - Template file: Lascia vuoto (Qt usa le stringhe sorgente)

4. **Configura Integrazione GitHub**
   - Vai in Settings → Integrations
   - Abilita integrazione "GitHub"
   - Aggiungi Weblate come GitHub App al tuo repository
   - Configura webhook per sync automatica

5. **Imposta Lingue di Traduzione**
   - Aggiungi Italiano (it_IT) come lingua principale
   - Puoi aggiungere altre lingue dopo (Tedesco, Spagnolo, Francese, etc.)

6. **Configura Workflow**
   - Settings → Workflow
   - Abilita "Push changes from Weblate to repository"
   - Imposta template "Commit message":
     ```
     i18n: Aggiorna traduzione {{ language_name }}

     Tradotto da {{ author }} via Weblate
     ```

7. **Invita Traduttori**
   - Condividi l'URL del progetto Weblate con i traduttori
   - Possono registrarsi e iniziare a tradurre immediatamente
   - Non serve conoscenza di programmazione!

## Workflow per i Traduttori

Una volta che Weblate è configurato, i traduttori possono:

1. **Login su Weblate**
   - Visita l'URL del progetto Weblate
   - Registrati o accedi (GitHub, Google, email)

2. **Seleziona Lingua**
   - Scegli Italiano (o altra lingua)
   - Vedi progresso traduzione (es. "234 / 500 stringhe tradotte")

3. **Traduci Stringhe**
   - Vedi stringa sorgente (Inglese)
   - Scrivi traduzione (Italiano)
   - Vedi contesto (posizione file, stringhe vicine)
   - Usa suggerimenti dalla memoria di traduzione

4. **Controlli Qualità**
   - Weblate controlla automaticamente:
     - Placeholder mancanti (%1, %2)
     - Coerenza punteggiatura
     - Lunghezza traduzione (per elementi UI)
     - Coerenza tag HTML

5. **Invia Traduzione**
   - Click su "Save"
   - La traduzione viene automaticamente committata su GitHub
   - Oppure raggruppata e committata periodicamente

## Interfaccia di Traduzione

Ecco come appare l'interfaccia per i traduttori:

```
┌─────────────────────────────────────────────────┐
│ Sorgente: Speed: %1 km/h                        │
│                                                 │
│ Traduzione: Velocità: %1 km/h                   │
│                                                 │
│ [Salva]  [Salta]  [Da rivedere]                │
│                                                 │
│ Suggerimenti:                                   │
│ • Velocità: %1 km/h (100% corrispondenza)       │
│ • Rapidità: %1 km/h (85% corrispondenza)        │
│                                                 │
│ Contesto:                                       │
│ File: src/HomeForm.ui.qml                       │
│ Vicini: Distance, Cadence, Power                │
└─────────────────────────────────────────────────┘
```

## Funzionalità per Traduttori

### 1. Memoria di Traduzione
- Salva automaticamente tutte le traduzioni
- Suggerisce traduzioni per stringhe simili
- Impara dal tuo stile di traduzione

### 2. Glossario
- Definisci terminologia coerente
- Esempio: "bike" → "bici" (non "bicicletta")
- Assicura coerenza in tutte le traduzioni

### 3. Controlli Qualità
- ✅ Placeholder presenti: %1, %2, etc.
- ✅ Tag HTML corrispondenti: `<b>`, `</b>`
- ✅ Punteggiatura coerente
- ⚠️ Traduzione troppo lunga (potrebbe non stare nella UI)
- ⚠️ Traduzione contiene URL (dovrebbe corrispondere al sorgente)

### 4. Commenti & Discussione
- I traduttori possono lasciare note
- Chiedere contesto o chiarimenti
- Gli sviluppatori possono rispondere

### 5. App Mobile
- Weblate ha app per iOS e Android
- Traduci ovunque
- Notifiche push per nuove stringhe
- Modalità offline

## Per Non Programmatori

### Cosa Serve
1. **Account su Weblate** (gratis, registrazione in 2 minuti)
2. **Nessun software** (tutto via browser web)
3. **Conoscenza dell'italiano** (ovviamente!)
4. **Nessuna conoscenza di programmazione** richiesta

### Come Contribuire
1. Ricevi l'URL del progetto Weblate dal maintainer
2. Registrati su Weblate (puoi usare Google/GitHub)
3. Clicca su lingua "Italiano"
4. Inizia a tradurre!

### Esempio Pratico

**Vedi**: `Speed: %1 km/h`
**Traduci**: `Velocità: %1 km/h`

**Nota**: Il `%1` è un placeholder - DEVE rimanere uguale!

**Vedi**: `Distance traveled: %1 %2`
**Traduci**: `Distanza percorsa: %1 %2`

**Nota**: `%1` è il numero, `%2` è l'unità (km o mi)

## Confronto Strumenti

| Strumento | Gratis OS | Interfaccia | Complessità | Qt Support |
|-----------|-----------|-------------|-------------|------------|
| **Weblate** | ✅ Sì | ⭐⭐⭐⭐⭐ | Media | ⭐⭐⭐⭐⭐ |
| **Crowdin** | ✅ Sì | ⭐⭐⭐⭐⭐ | Bassa | ⭐⭐⭐⭐ |
| **POEditor** | ✅ Sì* | ⭐⭐⭐⭐ | Bassa | ⭐⭐⭐ |
| **Transifex** | ✅ Sì | ⭐⭐⭐⭐ | Alta | ⭐⭐⭐⭐ |

*Limite 1000 stringhe per progetti gratis

## Setup Consigliato per QDomyos-Zwift

```
1. Setup Weblate Cloud (15 minuti)
   ↓
2. Collega GitHub (automatico)
   ↓
3. Invita traduttori italiani
   ↓
4. I traduttori traducono via web
   ↓
5. Weblate crea PR su GitHub automaticamente
   ↓
6. Review e merge PR
   ↓
7. Build automatica compila le traduzioni
   ↓
8. Traduzioni nell'app! 🎉
```

## Costi

- **Weblate Cloud**: GRATIS per open source
- **Setup**: 15 minuti una volta sola
- **Manutenzione**: ZERO
- **Per traduttori**: GRATIS e FACILE

## Alternative Semplici

Se Weblate è troppo complesso:

### 1. Google Sheets
**Pro**: Tutti conoscono Excel/Sheets
**Contro**: Manuale, nessun controllo qualità

**Setup**:
1. Esporta file .ts in CSV
2. Importa in Google Sheets
3. I traduttori modificano online
4. Esporta e riconverti in .ts

### 2. Crowdin (Più Semplice)
- Interfaccia ancora più semplice di Weblate
- Gratis per open source
- Ottima app mobile

**Link**: https://crowdin.com

## Demo Live

Prova Weblate senza registrarti:
https://hosted.weblate.org/projects/

Esempio progetti open source:
- https://hosted.weblate.org/projects/f-droid/
- https://hosted.weblate.org/projects/godot-engine/

## Prossimi Passi

1. **Decidi quale strumento usare**
   - Weblate (consigliato) - completo e potente
   - Crowdin - più semplice
   - Google Sheets - semplicissimo ma limitato

2. **Setup Account**
   - Registrati su Weblate/Crowdin
   - Collega il repository GitHub

3. **Invita Traduttori**
   - Condividi link al progetto
   - I traduttori si registrano e iniziano

4. **Monitora Progresso**
   - Dashboard mostra % completamento
   - Notifiche per nuove traduzioni

## Video Tutorial

Cercare su YouTube:
- "Weblate tutorial"
- "Crowdin open source"
- "Qt Linguist online"

## Supporto

- **Weblate Docs**: https://docs.weblate.org
- **Crowdin Support**: https://support.crowdin.com
- **Community**: Forum Weblate su GitHub

## Domande Frequenti

**Q: Costa qualcosa?**
A: No, gratis al 100% per progetti open source su Weblate/Crowdin.

**Q: Serve installare qualcosa?**
A: No, tutto via browser web.

**Q: Posso tradurre da smartphone?**
A: Sì! Sia Weblate che Crowdin hanno app mobile.

**Q: Quanti traduttori possono lavorare insieme?**
A: Illimitati! Tutti simultaneamente.

**Q: Come evito conflitti?**
A: Weblate gestisce tutto automaticamente, nessun conflitto possibile.

**Q: Posso vedere il progresso?**
A: Sì, dashboard mostra "234/500 stringhe tradotte (47%)".

**Q: Posso lasciare note per altri traduttori?**
A: Sì, ogni stringa ha sezione commenti.

## Riepilogo

✅ **Scelta Migliore**: Weblate Cloud
✅ **Tempo Setup**: 15 minuti
✅ **Costo**: Gratis
✅ **Facilità d'Uso**: ⭐⭐⭐⭐⭐
✅ **Per Non Programmatori**: Perfetto!

Vuoi che ti aiuti a configurare Weblate per il progetto?

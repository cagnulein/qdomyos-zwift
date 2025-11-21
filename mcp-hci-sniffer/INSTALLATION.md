# Installazione HCI Sniffer MCP

## Cos'è un MCP?

MCP (Model Context Protocol) è un protocollo che permette a Claude di usare strumenti esterni. Una volta installato, questo MCP sarà disponibile in **tutti i tuoi progetti**, non solo in QDomyos-Zwift!

## Installazione rapida

### 1. Installa dipendenze
```bash
cd /home/user/qdomyos-zwift/mcp-hci-sniffer
npm install
```

### 2. Configura Claude

Scegli il metodo in base a come usi Claude:

#### Opzione A: Claude Desktop (macOS/Linux/Windows)

**macOS:**
```bash
# Crea o modifica il file di configurazione
nano ~/Library/Application\ Support/Claude/claude_desktop_config.json
```

**Linux:**
```bash
nano ~/.config/Claude/claude_desktop_config.json
```

**Windows:**
```powershell
notepad %APPDATA%\Claude\claude_desktop_config.json
```

Aggiungi questa configurazione:
```json
{
  "mcpServers": {
    "hci-sniffer": {
      "command": "node",
      "args": ["/percorso/assoluto/a/qdomyos-zwift/mcp-hci-sniffer/index.js"]
    }
  }
}
```

**IMPORTANTE**: Sostituisci `/percorso/assoluto/a/` con il path vero!

#### Opzione B: Claude Code (Web)

Crea o modifica `.claude/settings.json` nella root del progetto:

```bash
cd /home/user/qdomyos-zwift
mkdir -p .claude
nano .claude/settings.json
```

Aggiungi:
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

### 3. Riavvia Claude

- **Claude Desktop**: Chiudi completamente e riapri
- **Claude Code Web**: Ricarica la pagina

### 4. Verifica installazione

Chiedi a Claude:
```
Quali MCP server hai disponibili?
```

Dovresti vedere `hci-sniffer` nella lista con questi strumenti:
- `parse_hci_snoop`
- `generate_cpp_code`
- `analyze_packet_sequence`

## Percorsi per piattaforma

### macOS
```
Configurazione: ~/Library/Application Support/Claude/claude_desktop_config.json
MCP directory: ovunque tu voglia
```

### Linux
```
Configurazione: ~/.config/Claude/claude_desktop_config.json
MCP directory: ovunque tu voglia
```

### Windows
```
Configurazione: %APPDATA%\Claude\claude_desktop_config.json
MCP directory: ovunque tu voglia
```

### Claude Code Web
```
Configurazione: .claude/settings.json (nel progetto)
MCP directory: preferibilmente nel progetto stesso
```

## Installazione per uso globale

Vuoi usare questo MCP in tutti i tuoi progetti? Installalo globalmente:

### 1. Sposta l'MCP in una posizione permanente
```bash
# macOS/Linux
sudo mkdir -p /usr/local/lib/mcp-servers
sudo cp -r mcp-hci-sniffer /usr/local/lib/mcp-servers/

# Windows (come Amministratore)
mkdir C:\MCP-Servers
xcopy mcp-hci-sniffer C:\MCP-Servers\mcp-hci-sniffer /E /I
```

### 2. Configura Claude Desktop con il path globale
```json
{
  "mcpServers": {
    "hci-sniffer": {
      "command": "node",
      "args": ["/usr/local/lib/mcp-servers/mcp-hci-sniffer/index.js"]
    }
  }
}
```

Ora l'MCP sarà disponibile in qualsiasi progetto senza doverlo copiare!

## Troubleshooting

### MCP non appare in Claude

**Controlla il file di configurazione:**
```bash
# macOS
cat ~/Library/Application\ Support/Claude/claude_desktop_config.json

# Linux
cat ~/.config/Claude/claude_desktop_config.json
```

**Verifica il JSON sia valido:**
```bash
node -e "console.log(JSON.parse(require('fs').readFileSync('path/to/config.json')))"
```

### Errore "Cannot find module"

**Reinstalla dipendenze:**
```bash
cd mcp-hci-sniffer
rm -rf node_modules
npm install
```

### Errore "Permission denied"

**Rendi il file eseguibile:**
```bash
chmod +x /path/to/mcp-hci-sniffer/index.js
```

### Path non trovato

**Usa sempre path assoluti:**
```bash
# Trova il path assoluto
cd /home/user/qdomyos-zwift/mcp-hci-sniffer
pwd
# Copia l'output e usalo nella configurazione
```

### Node.js non trovato

**Installa Node.js:**
```bash
# Verifica versione
node --version  # deve essere >= 18.0.0

# Se non installato
# macOS: brew install node
# Linux: sudo apt install nodejs npm (o yum/dnf)
# Windows: scarica da nodejs.org
```

## Verifica completa

Test passo-passo:

```bash
# 1. Node.js OK?
node --version

# 2. MCP installato?
cd /home/user/qdomyos-zwift/mcp-hci-sniffer
ls -la index.js package.json

# 3. Dipendenze OK?
npm list @modelcontextprotocol/sdk

# 4. File eseguibile?
ls -la index.js | grep x

# 5. Server avvia?
node index.js
# Dovresti vedere: "HCI Sniffer MCP server running on stdio"
# Premi Ctrl+C per uscire

# 6. Claude vede il server?
# Apri Claude e chiedi: "Quali MCP hai disponibili?"
```

## Prossimi passi

✅ Installazione completata? Leggi:
- `README.md` - Panoramica e strumenti disponibili
- `EXAMPLES.md` - Esempi pratici di utilizzo
- `test-setup.md` - Come testare con file reali

🚀 Pronto per catturare il tuo primo file HCI snoop!

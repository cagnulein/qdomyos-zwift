# Test Setup per HCI Sniffer MCP

## Verifica installazione

### 1. Test dipendenze
```bash
cd /home/user/qdomyos-zwift/mcp-hci-sniffer
node -e "console.log('Node OK:', process.version)"
npm list @modelcontextprotocol/sdk
```

### 2. Test avvio server (manuale)
```bash
cd /home/user/qdomyos-zwift/mcp-hci-sniffer
node index.js
```

Dovresti vedere:
```
HCI Sniffer MCP server running on stdio
```

Premi Ctrl+C per uscire.

## Configurazione per Claude Code Web

Crea o modifica il file `.claude/settings.json` nella root del progetto:

```json
{
  "mcp": {
    "servers": {
      "hci-sniffer": {
        "command": "node",
        "args": ["/home/user/qdomyos-zwift/mcp-hci-sniffer/index.js"]
      }
    }
  }
}
```

## Configurazione per Claude Desktop

**macOS**: `~/Library/Application Support/Claude/claude_desktop_config.json`
**Linux**: `~/.config/Claude/claude_desktop_config.json`
**Windows**: `%APPDATA%\Claude\claude_desktop_config.json`

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

## Test con file HCI snoop reale

Se hai un file btsnoop_hci.log, puoi testare manualmente:

```javascript
// test.js
import { readFileSync } from 'fs';

const buffer = readFileSync('./your_file.log');
const magic = buffer.toString('ascii', 0, 8);
console.log('Magic:', magic);
console.log('Is valid btsnoop?', magic === 'btsnoop\0');
```

## Verifica che Claude veda l'MCP

Dopo aver configurato e riavviato Claude, chiedi:

```
Quali MCP server hai disponibili?
```

Dovresti vedere `hci-sniffer` nella lista.

## Primo test completo

1. Cattura un file HCI snoop dal tuo Android
2. Copia il file sul computer
3. Chiedi a Claude:

```
Usa parse_hci_snoop per analizzare il file /path/to/your/btsnoop_hci.log
filtrando solo i write commands
```

Claude dovrebbe usare il tool e mostrarti i pacchetti estratti!

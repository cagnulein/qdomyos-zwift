# Plano: suporte à bike "Megagym" (OEM YPOO) no QZ

Documento de continuidade. Escrito a partir de uma sessão de análise do repo +
uma captura BLE real do equipamento. Tudo abaixo foi verificado no código ou no
log — nada é suposição, exceto onde marcado como **hipótese**.

---

## TL;DR

A bike é uma OEM chinesa (quase certamente **YPOO** rebrandeada) que fala **FTMS
padrão**. Ela aceita **resistência-alvo** (opcode `0x04`), mas **não** tem ERG
(`0x05`) nem modo simulação (`0x11`).

O QZ **não lê** a característica de features (`0x2ACC`) — ele decide o perfil do
equipamento por uma **tabela hardcoded por nome BLE**. Como a bike não está nessa
tabela, o QZ vai assumir ERG (default em `ftmsbike.cpp:57`) e o controle vai
falhar em treinos de potência.

**A tarefa é adicionar a bike a essa tabela com o perfil correto.** Faltam 3
dados de hardware para isso (seção 4).

---

## 1. O equipamento

**MAC:** `B8:F8:62:6D:A8:D6`
**Nome BLE:** ❓ **desconhecido** — a captura só registrou o MAC (ver seção 4)

### Características no serviço FTMS (`0x1826`)

| UUID | Props | O que é |
|---|---|---|
| `0x2ACC` | read | Fitness Machine Feature |
| `0x2AD3` | read, notify | Training Status |
| `0x2AD4` | read | Supported Speed Range |
| `0x2AD6` | read | **Supported Resistance Level Range** (não lido ainda) |
| `0x2AD8` | read | Supported Power Range |
| `0x2AD9` | indicate, write | **Fitness Machine Control Point** |
| `0x2ADA` | notify | Fitness Machine Status |
| `0x2AD2` | notify | Indoor Bike Data |
| `d18d2c10-c44c-11e8-a355-529269fb1459` | write | **proprietária YPOO** |

### O `0x2ACC` decodificado

```
raw = 86-52-00-00-04-00-00-00
Fitness Machine Features (bytes 0-3) = 0x00005286
Target Setting Features  (bytes 4-7) = 0x00000004
```

**Machine Features `0x5286`** → bits 1, 2, 7, 9, 12, 14:
cadência, distância total, nível de resistência, energia, tempo decorrido,
medição de potência.

**Target Setting Features `0x00000004`** → bit 2:

| bit | recurso | estado |
|---|---|---|
| 2 | Resistance Target Setting | ✅ **SIM** |
| 3 | Power Target (ERG) | ❌ não |
| 13 | Indoor Bike Simulation | ❌ não |

> ⚠️ O `paddle_logger.py` imprimiu isso **errado** na captura — usou os índices
> 11/12 (que no campo *Target Setting Features* são "Targeted Time in Three/Five
> Heart Rate Zones"). Ver seção 5, Etapa A.

Isso é consistente com o comportamento observado na prática: **o Kinomap
controla a resistência da bike** — usando exatamente o opcode `0x04`.

### Perfil resultante

| Recurso | Status |
|---|---|
| Métricas (cadência, potência, distância, energia, tempo, velocidade) | ✅ completas |
| Resistência-alvo (`0x04`) | ✅ declarado |
| ERG / potência-alvo (`0x05`) | ❌ não |
| Simulação / grade (`0x11`) | ❌ não |
| Paddles físicos | ✅ funcionam |

### Por que YPOO

O UUID proprietário `d18d2c10-c44c-11e8-a355-529269fb1459` **já existe no QZ**:
é o `_YpooMiniProCharId` em `src/devices/horizontreadmill/horizontreadmill.cpp:2360`.

O QZ já detecta outros YPOO:
- `YPOO-U3-` → `ypooelliptical` (`src/devices/bluetooth.cpp:1249`)
- `YPOO-MINI PRO-` → (`src/devices/bluetooth.cpp:1680`)

Detalhe relevante: essa característica proprietária está **dentro** do serviço
FTMS `0x1826`, não num serviço separado — é uma extensão de fabricante embutida
no padrão.

---

## 2. O que a captura mostrou

Sessão de ~113 s: pedalada + acionamento dos paddles.

**Paddles funcionam e são reportados por dois caminhos:**
- `0x2ADA` (Machine Status) opcode `0x07` = "Target Resistance Level Changed"
- campo `resistance` do `0x2AD2` (Indoor Bike Data)

As duas fontes concordam, com pequeno jitter de ordem (em t=73.310, 84.350,
88.310 e 90.350 o `0x2AD2` chegou antes do `0x2ADA`). Inofensivo — o QZ lê a
resistência do `0x2AD2` (`ftmsbike.cpp:938`).

**Faixa de resistência observada: 1 → 21.** O máximo real ainda é desconhecido
(precisa do `0x2AD6`).

**A bike alterna dois pacotes IBD distintos**, ~1 s cada:

| flags | conteúdo |
|---|---|
| `0x01F5` | moreData=1 (sem velocidade), cadência, distância, resistência, potência, potência média, energia |
| `0x2A00` | moreData=0 (com velocidade), FC, tempo decorrido, tempo restante |

Não é problema: o QZ decodifica flags corretamente pela union em
`ftmsbike.cpp:825-861`.

**A bike reporta o nível como uint8 puro.** O spec FTMS define o parâmetro do
status `0x07` como `sint16` com resolução 0,1 — a bike manda 1 byte com o nível
inteiro. Desvio de firmware relevante (ver seção 4, dado nº 3).

---

## 3. Como o QZ trata isso hoje

### O QZ ignora o `0x2ACC`

`ergModeSupported`, `resistance_lvl_mode` e `max_resistance` **não** vêm da
característica de features. Vêm de uma cadeia `if/else if` por nome BLE em
`ftmsbike::deviceDiscovered()` — `src/devices/ftmsbike/ftmsbike.cpp:2033-2100`.

O default, em `src/devices/ftmsbike/ftmsbike.cpp:57`:

```cpp
ergModeSupported = true; // by default ftms devices SHOULD have ergMode supported
```

**Consequência:** sem entrada na tabela, o QZ presume ERG. Em treino de potência
ele manda `FTMS_SET_TARGET_POWER` (`0x05`), a bike não suporta, o controle falha
ou oscila. Métricas continuam funcionando.

### O perfil correto

O mesmo já usado para DOMYOS, JFBK5.0 e SPAX-BK:

```cpp
resistance_lvl_mode = true;
ergModeSupported    = false;
max_resistance      = <do 0x2AD6>;
```

Com isso o QZ **emula ERG**, convertendo potência→resistência:
- `ftmsbike.cpp:1835` — roteia comando de potência por `changePower()`
- `ftmsbike.cpp:297` — `_ergTable.resistanceFromPowerRequest(power, Cadence.value(), max_resistance)`

### Como o QZ escreve resistência

`src/devices/ftmsbike/ftmsbike.cpp:374-376`:

```cpp
uint8_t write[] = {FTMS_SET_TARGET_RESISTANCE_LEVEL, 0x00, 0x00};
write[1] = ((uint16_t)requestResistance * 10) & 0xFF;
write[2] = ((uint16_t)requestResistance * 10) >> 8;
```

Ou seja **nível × 10**, little-endian (resolução 0,1 do spec). Esse ramo só é
usado por uma lista específica de modelos (`JFBK5_0 || DIRETO_XR || YPBM ||
FIT_BK || ZIPRO_RAVE || SPEEDRACEX || MRK_S28 || USDC_D700 || FS_YK`).

Opcodes em `src/devices/ftmsbike/ftmsbike.h:39-44`:
`FTMS_REQUEST_CONTROL = 0x00`, …, `FTMS_SET_TARGET_RESISTANCE_LEVEL = 0x04`,
`FTMS_SET_TARGET_POWER = 0x05`.

Handshake de init em `ftmsbike.cpp:171-195`: `REQUEST_CONTROL` → `START_RESUME`.
Descoberta do control point em `ftmsbike.cpp:1735-1737`.

### Escape hatch: a setting `ftms_bike`

Antes de escrever código, dá para testar a bike **sem alterar nada**:

- `src/devices/bluetooth.cpp:2013` — `!b.name().compare(ftms_bike, Qt::CaseInsensitive)`
- Setting: `src/qzsettings.cpp:772-773` (default `"Disabled"`)
- UI: Settings → **FTMS Bike** (`src/settings.qml:4441-4470`), ComboBox com os
  dispositivos escaneados. **Exige reiniciar o app.**

Isso faz o QZ tratar a bike como FTMS genérica. Métricas devem funcionar
imediatamente; o controle de resistência é o que precisa do perfil da seção 3.

---

## 4. Os 3 dados que faltam

Sem eles não dá para escrever o driver.

### 1. Nome BLE
A captura só tem o MAC. Necessário para a entrada na tabela e para a detecção em
`bluetooth.cpp`.

```bash
python tools/paddle_logger.py --scan
```

*(o `--scan` foi corrigido no commit `8a09f38` — `return_adv=True`)*

### 2. `0x2AD6` — Supported Resistance Level Range
É `read` e nunca foi lido. Retorna 3 × `sint16`: mínimo, máximo, incremento.
O máximo vira o `max_resistance`. Observamos até 21; o real pode ser 24, 32…

### 3. Teste de escrita no `0x2AD9`
A captura prova que o control point **existe** e que a bike **declara** suporte —
não que a escrita funciona.

```
write 0x2AD9 <- 00              # Request Control
   esperar indicate: 80-00-01   # resposta, opcode 0x00, sucesso
write 0x2AD9 <- 04-64-00        # Set Target Resistance, nível 10 (×10, como o QZ faz)
   esperar indicate: 80-04-01
   esperar STAT 07-0a no 0x2ADA
```

**Testar também `04-0A-00`** (inteiro puro).

> **Hipótese:** como a bike *reporta* o nível como uint8 puro (seção 2), é
> provável que também *espere* inteiro puro. Se for o caso, o ramo `× 10` do
> `ftmsbike.cpp:374-376` **não** serve, e a entrada da tabela precisa ir no outro
> ramo. Esse teste decide.

---

## 5. Tarefas

### Etapa A — corrigir e estender `tools/paddle_logger.py`

O script está em `tools/paddle_logger.py` (301 linhas, commit `8a09f38`).

**A1. Corrigir os índices de bit do `0x2ACC`** (linhas 193-198):

```python
emit(f"bit2  (Resistance Target)      = {'SIM' if target & (1 << 2)  else 'NAO'}")
emit(f"bit3  (Power Target / ERG)     = {'SIM' if target & (1 << 3)  else 'NAO'}")
emit(f"bit13 (Indoor Bike Simulation) = {'SIM' if target & (1 << 13) else 'NAO'}")
```

**A2. Corrigir o rótulo do opcode `0xFF`** (linha 116): é
`"Control Permission Lost"`, não `"Control Point alterado"`. Importa porque esse
opcode aparece justamente ao perder controle durante o teste de escrita.

**A3. Ler e decodificar o `0x2AD6`** — 3 × `sint16` (min, máx, incremento),
logo após a leitura do `0x2ACC`.

**A4. Implementar `--test-write N`.** A constante `CHR_CONTROL_POINT` já existe
na linha 33 mas **nunca é usada**. Precisa de `start_notify` no `0x2AD9` (é
`indicate`) para capturar as respostas `80-xx-xx`, e escrever os dois formatos
com log do indicate + do `STAT 07` resultante.

**Resultado:** uma única rodada fecha os 3 dados da seção 4.

### Etapa B — implementar o driver

**B1.** Adicionar a entrada em `ftmsbike::deviceDiscovered()`
(`src/devices/ftmsbike/ftmsbike.cpp`, na cadeia da linha 2033), no padrão do
DOMYOS/JFBK5.0:

```cpp
} else if (bluetoothDevice.name().toUpper().startsWith("<NOME_BLE>")) {
    qDebug() << QStringLiteral("<NOME> found");
    resistance_lvl_mode = true;
    ergModeSupported = false;
    max_resistance = <do 0x2AD6>;
}
```

Se o teste A4 indicar inteiro puro, incluir também o flag no ramo de escrita
adequado (`ftmsbike.cpp:374-376`).

**B2.** Adicionar o padrão de detecção em `src/devices/bluetooth.cpp`, no bloco
do `ftmsbike` (cadeia que termina na linha ~2025).

> ⚠️ **Ler a seção "Adding Device Detection to bluetooth.cpp" do `AGENTS.md`
> antes.** A ordem dos padrões importa — o primeiro match vence. Verificar
> conflito com os padrões YPOO existentes (`bluetooth.cpp:1249` e `:1680`), que
> vêm **antes** e podem capturar o dispositivo como elíptica/esteira.

**B3.** Settings, se for preciso um flag dedicado:
- `src/qzsettings.h` + `src/qzsettings.cpp` (atualizar `allSettingsCount`, hoje
  `1001` em `qzsettings.cpp:1282`)
- `src/settings.qml` — property **no fim** da lista de properties
- `src/settings-catalog.json` — mantido **à mão**; manter `settingCount`
  sincronizado (hoje `965`) e adicionar no fim

**B4.** Teste em `tst/Devices/` no padrão do projeto — registrar
`BluetoothDeviceTestData` via `DeviceTestDataIndex`. Ver `docs/50_writing_tests.md`.

### Etapa C — validar

1. Testar primeiro via setting **`ftms_bike`** (seção 3) — sem alterar código
2. Confirmar métricas na tela do QZ
3. Confirmar controle de resistência num treino
4. Ativar `log_debug` e procurar `"FTMS service and Control Point found"`
5. Ajustar sensibilidade com `bike_resistance_offset` / `bike_resistance_gain_f`
   (`src/qzsettings.cpp:83-84`)

---

## 6. Armadilhas conhecidas

- **`AGENTS.md` é normativo.** Regra 1: perguntar, não presumir. Regra 3: não
  mexer em código não relacionado.
- **`src/settings-catalog.json` é manual** — precisa ser atualizado no mesmo
  commit que alterar qualquer property de settings.
- **Ordem dos padrões em `bluetooth.cpp`** — ver B2.
- **A bike aceita uma conexão por vez.** Fechar QZ / Kinomap / MyWhoosh antes de
  rodar o logger (já documentado no cabeçalho do script).
- **Build Android** tem várias armadilhas documentadas no `AGENTS.md`
  (androiddeployqt não empacota as `.so` do Qt de forma confiável, gradle exige
  Java 17, `.qm` são artefatos não versionados).

---

## 7. Estado do repositório

- **Repo:** `nickolas122/qz` (fork de `cagnulein/qdomyos-zwift`, GPL-3.0)
- **Base:** `master` em `8a09f38` — *"Add BLE/FTMS paddle logger tool…"*
- **Upstream anterior:** `ccb7f86`, um **revert** de
  *"Fix Android IP detection and multicast for OpenBikeControl (MyWhoosh Link) (#4864)"*

### Nota lateral: OpenBikeControl / MyWhoosh no Android

Se for usar o MyWhoosh: o commit revertido `845c1c3` adicionava o
`WifiManager.MulticastLock` no `ForegroundService.java` e as permissões
`CHANGE_WIFI_MULTICAST_STATE` / `ACCESS_NETWORK_STATE` no Manifest. **Nada disso
está no HEAD atual**, então a descoberta mDNS do QZ pelo MyWhoosh provavelmente
falha no Android. O commit está no histórico e é `cherry-pick`-ável — mas foi
revertido pelo mantenedor por motivo não explicado no commit.

Alternativa: informar o IP manualmente no MyWhoosh (porta **21587**), se o app
permitir. Notar que habilitar OpenBikeControl **desliga o Wahoo Dircon**
(`src/settings.qml:3474`).

---

## Referências rápidas de código

| O quê | Onde |
|---|---|
| Default `ergModeSupported = true` | `src/devices/ftmsbike/ftmsbike.cpp:57` |
| Tabela de perfis por nome BLE | `src/devices/ftmsbike/ftmsbike.cpp:2033-2100` |
| Escrita de resistência (× 10) | `src/devices/ftmsbike/ftmsbike.cpp:374-376` |
| Conversão potência→resistência | `src/devices/ftmsbike/ftmsbike.cpp:297`, `:1835` |
| Parsing de flags do `0x2AD2` | `src/devices/ftmsbike/ftmsbike.cpp:825-861`, `:938` |
| Descoberta do control point | `src/devices/ftmsbike/ftmsbike.cpp:1735-1737` |
| Handshake de init FTMS | `src/devices/ftmsbike/ftmsbike.cpp:171-195` |
| Enum de opcodes FTMS | `src/devices/ftmsbike/ftmsbike.h:39-44` |
| Detecção genérica `ftms_bike` | `src/devices/bluetooth.cpp:2013` |
| Padrões YPOO existentes | `src/devices/bluetooth.cpp:1249`, `:1680` |
| UUID proprietário YPOO | `src/devices/horizontreadmill/horizontreadmill.cpp:2360` |
| Setting `ftms_bike` | `src/qzsettings.cpp:772-773` |
| UI do `ftms_bike` | `src/settings.qml:4441-4470` |
| Logger BLE | `tools/paddle_logger.py` |

# Megagym — plano de execução

> Documento único. Substitui `PLANO-EXECUCAO.md` e `VIABILIDADE-RASPBERRY-BRIDGE.md`,
> ambos removidos. Herda os fatos de `PLANO-MEGAGYM.md` (branch `docs/plano-megagym`) e
> corrige o que a medição derrubou.
>
> **Bridge físico abandonado** — ver §2. Nada aqui envolve abrir a bike.

**Convenção:** ✅ medido · ⚙️ verificado no código · ❓ desconhecido.

Evidência: `.fit/` e `logs_qz/` neste repositório, sessão de 03/08/2026.

---

## 1. O que ficou estabelecido

Sete fatos, todos medidos ou verificados. É o que sobrou de valor da caracterização.

### 1.1 `W = f(R_alvo, cadência)` — provado por dois caminhos independentes ✅

**Pelo FTMS**, no treino estruturado (`logs_qz/debug-Mon_Aug_3_19_45_12_2026.log`):

```
t=329,99  CMD -> 16   (+8 níveis)      t=420,00  CMD -> 2   (-14 níveis)
t=330,67  R=16 W=136 cad=81            t=420,66  R=2 W=85 cad=81
          0,68 s depois                          0,66 s depois
```

**Pelo knob**, na bancada, sem nenhum comando do QZ
(`logs_qz/debug-Mon_Aug_3_19_34_06_2026.log`):

```
 t(s)     R      W    cad   W/rpm
 -1,00     4     68    66   1,030
 +0,00    22    140    68   2,059    <-- +18 níveis em UMA amostra de 1 s
 +1,01    22    152    72   2,111
```

+18 níveis num segundo, cadência parada, watt dobrando. A 2–3 níveis/s isso levaria 6 a 9 s.
Também no mesmo log: 22→1 em 1,00 s e 1→17 em 1,00 s.

**Nenhum dos dois caminhos de entrada expõe posição física.** O console publica o alvo.

### 1.2 Dispersão zero, reproduzida ✅

53 pares (R, cadência) com n ≥ 5 amostras: **53 com dispersão exatamente 0 (100%)**. Mesma
assinatura da `PLANO-MEGAGYM` §2.2, em dados novos. Não há medição em lugar nenhum.

### 1.3 O atuador, medido ✅

| Sentido | Taxa | 19 níveis | 5 níveis |
|---|---|---|---|
| Subida | 2–3 níveis/s | 6,3 – 9,5 s | 1,7 – 2,5 s |
| Descida | 4–5 níveis/s | 3,8 – 4,8 s | 1,0 – 1,3 s |

Movimento **contínuo**, não rajadas por nível — o console aceita alvo absoluto e executa
um movimento só ✅. Assimetria ~2× sugere que ele trabalha contra algo que ajuda no retorno.

### 1.4 Não existe zona morta ✅ — retratação de duas conclusões anteriores

Tabela densa, **a cadência fixa em 81 rpm**, preenchendo os buracos da `PLANO-MEGAGYM` §2.3:

| R | W | Δ | R | W | Δ |
|---|---|---|---|---|---|
| 2 | 85 | — | 10 | 109 | +3 |
| 3 | 87 | +2 | 11 | 113 | +4 |
| 4 | 90 | +3 | 12 | 117 | +4 |
| 5 | 92 | +2 | 15 | 131 | +4,7 |
| 6 | 96 | +4 | 16 | 136 | +5 |
| 7 | 99 | +3 | 17 | 142 | +6 |
| 8 | 103 | +4 | 18 | 148 | +6 |
| 9 | 106 | +3 | | | |

**O passo cresce suave de +2 para +6 W por nível. Sem platô, sem cotovelo** — a curva
convexa que frenagem magnética produz.

O "platô R 1–13" da `PLANO-MEGAGYM` §2.3 era artefato: aquela tabela não tem R 2–5 nem
9–12 e calcula W/rpm médio misturando cadências. E **W não é proporcional à cadência** —
R=16 dá 136 W a 81 rpm e 131 W a 79 rpm ✅. A tabela é genuinamente 2-D.

Duas afirmações anteriores minhas caem junto: nem "a zona morta é física", nem "é firmware
escondendo faixa". Ela não existe.

### 1.5 O ERG rastreia a cadência ✅ — a §5.2 não reproduziu

Na sessão real, com `zwift_erg=true`:

| Bloco | n | corr(cadência, R) |
|---|---|---|
| t=330–420 | 91 | **−0,567** |
| t=540–686 | 146 | **−0,465** |

Correlação negativa é a assinatura de ERG funcionando: cadência sobe, resistência desce
para segurar a potência. Intervalo mediano entre comandos: **1,01 s**.

MyWhoosh mandou **143 escritas "erg mode"** no `0x2AD9`, e o QZ chamou
`resistanceFromPowerRequest` **34.657 vezes** — o `update()` recalcula continuamente a
partir do último alvo e da cadência corrente, sem depender de o app reenviar.

**Consequência para o plano:** o sintoma previsto na `PLANO-MEGAGYM` §5.2 — *"o ERG ajusta
a resistência uma vez por bloco e congela"* — **não aparece nesta configuração**. Ver §4.

### 1.6 O log trunca ✅

`debug-Mon_Aug_3_19_45_12_2026.log` parou em **37.481.436 bytes, cortado no meio de uma
linha**, cobrindo 848 s de uma sessão de 2025 s — perdeu 66%.

Dois cortes conhecidos: 40,5 MB (§5.3 de lá) e 37,5 MB. **Não é limite fixo de bytes nem
de tempo** (1318 s contra 848 s), mas os dois na mesma faixa.

### 1.7 O caminho de escrita ⚙️

Para o perfil YPBM, `ftmsbike.cpp:374` escreve **um pacote FTMS absoluto**:

```cpp
uint8_t write[] = {FTMS_SET_TARGET_RESISTANCE_LEVEL, 0x00, 0x00};
write[1] = ((uint16_t)requestResistance * 10) & 0xFF;   // nível × 10, LE
write[2] = ((uint16_t)requestResistance * 10) >> 8;
```

---

## 2. O que foi abandonado, e por quê

**Bridge físico, em todas as variantes** — MITM no chicote, substituição do console,
SmartSpin2k no knob, tap só-leitura. Nada de abrir a bike.

A conta que levou a isso, ganho a ganho:

| Ganho alegado | Destino |
|---|---|
| 12 níveis físicos escondidos | **Não existem** (§1.4) |
| Faixa acima de R=32 | Sem evidência — o apoio era a análise refutada em §1.4 |
| Estado físico real da resistência | **Tem substituto em software** (§3) |
| Paddles limpos | Entregue por OpenBikeControl, sem hardware (§5.2) |
| Latência determinística | Irrelevante: 2–3 níveis/s contra milissegundos de BLE |
| Liberar o rádio BLE do Pi | Único que sobra — **não medido**, e com alternativa barata (§5.3) |

Contra isso: risco elétrico real (o chicote carrega AC de gerador ✅), console
provavelmente irreparável numa YPOO rebrandeada, e replicar um controlador de posição com
taxas assimétricas é trabalho de embarcado sério — não "acionar um motor".

**Decisão: não vale.** Registrada em 04/08/2026.

---

## 3. Frente A — compensar o atraso do ímã ★

**A principal, e a única que melhora o treino de imediato.**

### 3.1 O problema

O QZ comanda mais rápido do que o atuador anda:

- Demanda do MyWhoosh chega a **~1005 ms de mediana** (`PLANO-MEGAGYM` §4.2) ✅
- O atuador faz **2–3 níveis/s** subindo ✅
- A demanda percorre **15 níveis entre p5 e p95** com offset 18 ✅

Logo, em terreno ondulado o alvo corre à frente dos ímãs e **nunca é alcançado**. Como o
watt publicado é o alvo (§1.1), o `.fit` registra um esforço que as pernas não fizeram — e
a assimetria (sobe devagar, desce rápido) enviesa o erro **numa direção só**: mais leve nas
subidas, em dia nas descidas. O viés acumula ao longo da sessão.

### 3.2 A solução: limitar a taxa do comando

Se o QZ nunca pedir mais do que o atuador entrega, o alvo não corre à frente do físico — e
como o reportado **é** o alvo, o watt publicado volta a ser verdade.

Custa responsividade em terreno. Mas aquela responsividade era falsa: número instantâneo,
ímã lento. Troca-se um número errado e rápido por um número certo e lento.

**O QZ não tem isso** ⚙️: `zwift_erg_filter` é banda morta em watts (`bike.cpp:110`), não
taxa; o único rate limit em `ftmsbike.cpp:519` é específico de DOMYOS.

### 3.3 Onde entra

`ftmsbike::update()`, no bloco de `ftmsbike.cpp:503–531`, que já tem o padrão de deferral
do DOMYOS logo ao lado — mesmo lugar, mesma forma ⚙️:

```cpp
resistance_t rR = requestResistance + (gearsModifier() * gearMultiplier);
if (rR != currentResistance().value() || lastGearValue != gears()) {
    ...
    forceResistance(rR);           // ftmsbike.cpp:531
```

O `update()` roda a cada `poll_device_time` ⚙️, então serve de tick para a rampa: em vez de
escrever `rR` de uma vez, escrever no máximo `slew × Δt` níveis por tick na direção de `rR`,
guardando o alvo final para os ticks seguintes.

Ponto de atenção: o limitador precisa continuar rampando **mesmo sem nova demanda**, então
o alvo pendente tem que sobreviver entre ticks — não pode depender de `requestResistance`
ser reescrito.

### 3.4 Settings novas

| Chave | Default | Nota |
|---|---|---|
| `resistance_slew_up` | 0 = desligado | níveis/s. Medido: 2–3 → começar em **2** |
| `resistance_slew_down` | 0 = desligado | níveis/s. Medido: 4–5 → começar em **4** |

Plumbing conhecido: `qzsettings.h`/`.cpp` com `allSettingsCount`, `settings.qml`,
`settings-catalog.json` mantido à mão com `settingCount` sincronizado, e teste em `tst/`
(`PLANO-MEGAGYM` §7 Fase 6 e §11) ⚙️.

Default desligado é obrigatório: com `slew=0` o comportamento tem de ser **idêntico** ao
atual, byte a byte.

### 3.5 Teste e critério

Duas sessões de free ride de ~20 min no mesmo percurso, uma com `slew=0` e outra com
`2/4`, exportando log e `.fit`.

| Aceita se | |
|---|---|
| Com `slew=0`, comportamento idêntico ao de hoje — regressão limpa | ✅ |
| Com `2/4`, nenhuma transição comandada excede a taxa configurada no log | ✅ |
| Com `2/4`, `\|ΔR\|` por segundo no `0x2AD2` fica dentro do limite | ✅ |
| Potência média das duas sessões difere de forma **explicável** pelo viés da §3.1 — a com limitador deve ficar **mais baixa**, porque para de contar esforço que não existiu | ✅ |

> O último critério é o que importa e é confirmatório, não de aprovação: se a média **não**
> cair, a premissa do viés está errada e é preciso reexaminar.

**Artefatos:** dois logs · dois `.fit` · série de `|ΔR|/s` das duas.

### 3.6 O que foi implementado ⚙️

O limitador existe no código. Falta só rodar as duas sessões da §3.5 — que é a parte que
o computador não faz.

**`src/devices/ftmsbike/resistanceslewlimiter.h`** — o limitador, sem Qt objects e sem
relógio próprio (quem chama passa o timestamp), justamente para ser testável sem bike.
Guarda o alvo final e devolve, a cada chamada, o nível mais distante que o atuador
poderia ter alcançado desde o comando anterior.

Duas decisões que a implementação obrigou a tomar, e que a §3.3 não previa:

- **A fração de nível tem que ser carregada entre ticks.** O `poll_device_time` é 200 ms
  e a 2 níveis/s um nível leva 500 ms. Descartando o resto a cada tick, a rampa andaria a
  1,67 níveis/s em vez de 2 — mais lenta que o ímã, somando atraso em vez de tirar. O
  relógio interno avança exatamente `níveis/taxa`, não até `agora`.
- **Taxa instantânea entre dois comandos não é a métrica.** Níveis são inteiros: um nível
  sozinho sempre cai um pouco antes ou depois da hora exata, e entre dois comandos
  consecutivos a 200 ms de poll aparecem 2,5 níveis/s com a taxa em 2. O que vale — e o
  que o teste verifica — é que **a distância percorrida desde o início da rampa nunca
  passa de `taxa × tempo decorrido`**, que com o carry acima vale exatamente. **O critério
  da §3.5 tem de ser lido assim**: `|ΔR|` acumulado contra o tempo, não par a par.

**`ftmsbike`** — todo comando de resistência passa agora por `commandResistance()`, que é
o gate: `forcePower()` (bikes em `resistance_lvl_mode`), o bloco de `update()` e o ERG
contínuo. `forceResistance()` continua sendo o escritor cru. Com as duas taxas em 0 o gate
chama `forceResistance()` e mais nada — tráfego idêntico ao de hoje.

O tick da rampa fica em `update()`, depois do bloco de resistência: o alvo pendente vive
no limitador, não em `requestResistance`, então a rampa termina sozinha depois que a
demanda que a começou sumiu — o ponto de atenção da §3.3.

Três casos que só apareceram ao escrever o teste:

| Caso | Decisão |
|---|---|
| Primeiro comando da sessão, limitador sem histórico | Semeia com `currentResistance()` se `resistance_received`; senão passa direto — não há distância sobre a qual medir taxa |
| ERG reemitindo o mesmo alvo a cada ~1 s | Retarget **não** reinicia o relógio, senão a rampa nunca sairia do lugar |
| Poll parado (app suspenso, 30 s de buraco) | Crédito limitado a 2 s — o ímã também não andou nesse buraco, então o tempo parado não compra salto |

**Testes:** `tst/Devices/TestResistanceSlewLimiter.h`, 14 casos, incluindo a invariante
de distância acima e as durações medidas da §1.3 (19 níveis: 9,5 s subindo, 4,75 s
descendo).

### 3.7 Primeira volta com o limitador ligado ✅

7/8, `resistance_slew_up = 2` / `_down = 4`. Relato: a potência reportada passou a
acompanhar o esforço no pedal nas trocas de resistência. As rampas do log batem com a
configuração — 4→26 em 11,0 s (2,0 níveis/s) e 26→1 em 6,0 s (4,2 níveis/s).

Isso confirma a premissa da §3.1 **qualitativamente**. O par de sessões da §3.5, que é o
que mede a queda de potência média, continua por rodar — e a §4.5 mostra por que ele tem
de ser rodado com as correções do seletor já dentro, senão mede as duas coisas juntas.

---

## 4. Frente B — ERG: o que realmente falta

**Antes de gastar esforço: o defeito principal não reproduziu** (§1.5). O ERG rastreou a
cadência com `corr(cadência, R)` de −0,47 a −0,57 e mediana de 1,01 s entre ajustes.

### 4.1 Não mexer no keep-alive sem antes reproduzir o defeito

A `PLANO-MEGAGYM` §5.2 descreve `virtualbike.cpp:1499` com o watchdog gateado por
`lastFTMSFrameReceived`, que é atualizado por qualquer escrita no `0x2AD9` (`:664`) ⚙️. O
raciocínio continua correto **no código**. Mas o sintoma que ele prevê não apareceu, porque
o `update()` do QZ recalcula a resistência continuamente (34.657 chamadas a
`resistanceFromPowerRequest`) sem depender de o app reenviar o alvo.

**Ação: reclassificar de "corrigir" para "reproduzir primeiro".** Se em alguma sessão o ERG
de fato congelar — resistência parada enquanto a cadência varia, `corr(cadência, R)` ≈ 0 —
aí sim a correção se justifica. Corrigir um defeito que não se manifesta é risco sem
retorno.

### 4.2 O que sobrou de concreto: as lacunas longas ❓

14 intervalos acima de 10 s entre comandos, o maior de **25 s**, contra mediana de 1,01 s.

Suspeita: a banda morta de `zwift_erg_filter = 10` W. Com passo de 4–6 W por nível (§1.4),
10 W equivalem a ~2 níveis de histerese — a cadência precisa variar bastante para
disparar ajuste.

**Ação:** repetir um treino com `zwift_erg_filter` e `_down` em **5** e comparar o
histograma de intervalos entre comandos, mais `corr(cadência, R)`. Custo zero, é setting.

| Aceita se | |
|---|---|
| Intervalos > 10 s caem sensivelmente | ✅ |
| `corr(cadência, R)` fica mais negativa | ✅ |
| Sem caça — sem oscilação de ±1 nível a cada tick por longos períodos | ✅ |

> Interage com a Frente A: banda morta menor gera mais trocas, que o limitador vai suavizar.
> **Testar A e B separadamente antes de combinar**, senão não se sabe o que causou o quê.

### 4.3 Auto-ERG — continua fazendo sentido ❓ **(não implementado)**

A Fase 6 da `PLANO-MEGAGYM` (settings `zwift_erg_auto` e `zwift_erg_auto_hold`, engate no
primeiro `0x05` e soltura por timeout) evita ter de alternar `zwift_erg` entre free ride e
treino. Você faz os dois, então o incômodo é real.

Prioridade **abaixo** de A e de 4.2: é conveniência, não correção.

> **Estado verificado em 8/8:** `zwift_erg_auto` e `zwift_erg_auto_hold` **não existem em
> lugar nenhum** — 0 ocorrências em `src/` nos 7 branches locais e 8 remotos, incluindo
> `master`, e nenhuma chave correspondente em `qzsettings.h`. Não é "não testado", é **não
> construído**. Não há o que testar até alguém escrever.

#### 4.3.1 O tile de potência-alvo — o que dá e o que não dá para testar hoje

Pergunta separável e legítima: uma mudança de alvo chega mesmo ao tile da GUI?

O caminho é `homeform.h:899` (`DataObject *target_power`), alimentado em
`homeform.cpp:6868` a partir de `((bike *)device)->lastRequestedPower().value()`. Ou seja,
**o tile não tem lógica própria** — ele renderiza `lastRequestedPower()`. Isso parte o teste
em duas metades de custo muito diferente:

| Metade | Custo | O que cobre |
|---|---|---|
| Contrato do dispositivo: `changePower()` → `lastRequestedPower()` | Barato, dá para hoje, sem GUI | Que o valor que o tile lê é o que o app mandou. É onde um defeito real moraria |
| Fim a fim: `lastRequestedPower()` → pixel do tile | Caro | `homeform` é um QObject grande acoplado a QML; instanciá-lo num teste pede `QQmlApplicationEngine` e um `bluetoothManager`. **Não existe andaime para isso**: a única menção a `homeform` em `tst/` é `bt.homeformLoaded = true` em `bluetoothdevicetestsuite.cpp:22`, uma flag |

A segunda metade também é a que menos paga: como o tile só espelha o campo, um teste de GUI
falharia quase sempre por causa do andaime, não por causa do produto. **Recomendação:
cobrir a primeira metade e verificar a segunda com o olho**, que é como ela já foi
verificada em 7/8 — o alvo do MyWhoosh apareceu no tile.

### 4.4 Marcha em dobro — encerrado como contorno

`bike.cpp:73` (+1×) e `ftmsbike.cpp:509` (+5×), 6 níveis por marcha ⚙️. Contornado por
`gears_from_bike=false` e marcha 0, que é a configuração-alvo de qualquer forma. Com
OpenBikeControl cuidando das marchas (§5.2), deixa de ser tocado. **Não corrigir.**

### 4.5 O defeito real do ERG: a tabela, não o keep-alive ✅⚙️

Sessão de 7/8, `slew = 2/4`, 907 s pedalando com alvo vivo do MyWhoosh
(`0QZ-backup-sex. ago. 7 19_33_49.fit`). Erro médio `real − alvo` = **−13,2 W**; 22% do
tempo mais de 10 W abaixo.

| cadência | n | alvo méd. | potência méd. | **erro** | R méd. |
|---|---|---|---|---|---|
| < 60 | 71 | 140 | 56 | **−84 W** | 10,5 |
| 60–65 | 48 | 113 | 67 | **−46 W** | 5,0 |
| 65–70 | 17 | 93 | 73 | **−20 W** | 4,8 |
| 70–75 | 122 | 109 | 102 | −6,6 W | 11,7 |
| ≥ 75 | 649 | 176 | 171 | −4,2 W | 19,1 |

Por alvo: 74 W → **−19,4 W (−26%)** com R médio **1,1**; 93 W → −15,6 W; acima de 102 W,
−3 a −8 W. Os dois eixos da queixa — cai com a cadência, pior em alvo baixo.

**Não é a Frente A.** Em regime permanente (alvo parado ≥15 s *e* resistência parada ≥5 s,
o que exclui toda rampa): 5/8 **pré-slew** dava −32,0 W abaixo de 70 rpm contra +1,5 W acima;
7/8 **pós-slew**, −87,7 W contra −4,2 W. O déficit de cadência baixa existe dos dois lados,
com os ímãs parados. Acima de 70 rpm as três sessões concordam dentro de poucos watts.

A causa está em `ergtable.h`, em três camadas:

1. **`estimateWattage` não extrapolava para baixo na cadência.** Abaixo da menor cadência já
   aprendida para um nível, devolvia a potência medida numa cadência *maior*. O seletor
   achava que o nível rendia mais do que rende e escolhia um abaixo. Confirmado no próprio
   log: a 70 rpm cotou R=7 em 86 W — a menor cadência aprendida de R=7 é 73 — enquanto o
   console reportava 78 W.
2. **Nível sem amostra nenhuma copiava a linha inteira do vizinho mais próximo.** R=5 não
   tinha amostra, copiava R=6, aprendido só entre 92 e 100 rpm: **`estimate(5) = estimate(6)
   = 113 W em qualquer cadência de 55 a 85**. Era um muro — a varredura ascendente fechava o
   bracket em R=4 para todo alvo entre ~70 e ~113 W.
3. **O bracket devolvia o índice de baixo**, nunca o mais próximo: mais um nível de
   subestimação sistemática, 3–15 W conforme a faixa.

E como a curva não era monótona em R (cadência 60: `56, 85, 79, 68, 113, 113, 86, 104, 96,
73, 76, 107, 80, 84, 127, 89…`), "primeiro bracket subindo" não significava nada.

**O que foi corrigido**

| Onde | Mudança |
|---|---|
| `estimateWattage` / `wattageAtResistance` | Extrapola pela inclinação das duas amostras mais próximas, nas duas pontas da cadência; nunca abaixo de zero |
| `estimateWattage` | Nível sem amostra é **interpolado** entre o vizinho de baixo e o de cima, não copiado |
| `resistanceFromPowerRequest` | Curva forçada não-decrescente por *pool adjacent violators*, depois o nível **mais próximo** do alvo. PAVA e não máximo corrente: o máximo corrigiria os vales mas espalharia um pico por todos os níveis acima, que é o erro perigoso |
| `ftmsbike::resistanceFromPowerRequest` | Cadência 0 **segura** a resistência em vez de aceitar o `1` da tabela — senão cada parada derrubava R ao fundo, com 12 s de rampa para voltar |
| `ftmsbike::ergResistanceAccepted` | Mudança de **um** nível só passa se for pedida continuamente por 3 s. A 112 W a seleção alternava 13↔14 uma vez por segundo por 90 s |
| `ergTable::setResistanceReportsCommand` | Liga com o limitador: neste console a potência é função do nível *comandado* (§1.1), então a amostra vale durante a rampa. A janela de 1 s descartava 368 coletas na sessão de 7/8 contra 9 na de 3/8 — justamente os níveis que só são visitados de passagem, que são os buracos da camada 2 |

Simulando o seletor novo sobre a tabela real da sessão:

| alvo | cad | R antigo (W) | R novo (W) |
|---|---|---|---|
| 74 | 59 | 1 (55) | 12 (73) |
| 93 | 68 | 6 (72) | 12 (91) |
| 102 | 74 | 9 (94) | 12 (103) |
| 112 | 73 | 12 (101) | 14 (110) |
| 150 | 84 | 17 (149) | 17 (149) |
| 180 | 88 | 19 (174) | 20 (181) |

A ponta alta quase não muda — lá já estava certo. **Testes:** `tst/Erg/TestErgTableSelection.h`.

> **Atenção na primeira volta:** em cadência baixa o seletor agora pede muito mais
> resistência (74 W a 59 rpm sai de R=1 para R=12). É o que a própria tabela mede, mas muda
> o tato. O limitador entra na frente e rampa isso em ~5,5 s, e a proteção de espiral
> continua bloqueando aumentos abaixo de 50 rpm.

### 4.6 Volta de verificação ✅

8/8, 32 min, mesmo corte da §4.5 (`0QZ-backup-sáb. ago. 8 11_13_33.fit`, 1807 s pedalando
com alvo vivo — o dobro da amostra de 7/8). Erro médio **−13,2 → +2,7 W**. Tempo mais de
10 W abaixo do alvo: **22% → 3%**.

| cadência | 7/8 | 8/8 |
|---|---|---|
| < 60 | −84,1 W | **−12,5 W** |
| 60–65 | −45,8 W | **+1,1 W** |
| 65–70 | −20,4 W | **−2,4 W** |
| 70–75 | −6,6 W | +0,9 W |
| 75–80 | −3,7 W | +4,3 W |
| ≥ 80 | −4,2 W | +3,4 W |

Por alvo, de 83 W a 218 W, tudo dentro de ±2,4 W — contra −19,4 W a 74 W e −15,6 W a 93 W
antes. A ponta baixa, que era o pior caso, deixou de ser caso.

**O déficit que sobrou é inteiramente a proteção de espiral, não a tabela.** Cortando em
torno dos 50 rpm onde ela age:

| faixa | n | alvo méd. | erro | R méd. |
|---|---|---|---|---|
| < 50 (proteção ativa) | 49 | 83 | **−21,6 W** | 21,4 |
| 50–60 | 30 | 74 | **+2,3 W** | 13,6 |
| 60–70 | 59 | 77 | **+0,1 W** | 8,7 |
| ≥ 70 | 1669 | 115 | +3,6 W | 7,7 |

Abaixo de 50 rpm o seletor pede mais resistência (já estava em R=21,4) e a proteção recusa
o aumento — 110 bloqueios contra 62 em 7/8, mas concentrados entre 36 e 47 rpm, enquanto os
de 7/8 iam até 6 rpm porque o ciclista estava parando. É a proteção fazendo o que existe
para fazer. **Se vale rever o limiar de 50 rpm agora que o seletor acerta o alvo é uma
pergunta nova**, e não é a mesma pergunta que a §4.5 respondeu.

Os mecanismos, um a um:

| | Resultado |
|---|---|
| Aprendizado da tabela | **1 amostra descartada contra 2658 guardadas** (7/8: 368 contra 962). Tabela foi de 520 para 610 pontos em uma volta |
| Histerese de um nível | Comandos de 1 nível caíram de 22/32 (69%) para 15/74 (20%). Reversões imediatas por segundo: 0,0165 → 0,0155 — não pioraram, e a composição virou passo de 2 níveis, que é rastreamento real e não ruído |
| Retenção sem cadência | **0 ocorrências — não foi exercitada.** O ciclista não parou de pedalar com alvo vivo nesta volta. Continua sem verificação de campo |
| Limitador de taxa | 293 linhas de rampa, R máximo 31 de 32 — pediu muito em cadência baixa, mas não encostou no teto |

---

## 5. Frente C — plataforma

### 5.1 Raspberry Pi

Continua valendo, e o §1.6 reforça: no Pi o log é filesystem comum mais `journalctl`, e a
truncagem provavelmente desaparece.

**Material** — ~R$ 770–940:

| Item | ~R$ |
|---|---|
| Pi 4 Model B **1 GB** | 599 |
| Fonte USB-C **5,1 V / 3 A** de qualidade | 60–120 |
| MicroSD 32 GB A1 de marca | 40–70 |
| Case com cooler ou dissipador | 40–100 |
| RTC **DS3231** (I²C) | 15–25 |
| Adaptador **USB-TTL** (console serial) | 15–25 |

Binário: artefato `raspberry-pi-binary-aarch64` do run corrente do master
(`.github/workflows/main.yml:1525`) ⚙️ — dispensa as ~45 min de compilação.

**Sem GUI, e não faz falta:** calibra-se no tablet e sobe a config pronta. O `.qzs` é um
INI de `QSettings` com os mesmos nomes de chave do `qDomyos-Zwift.conf`
(`homeform.cpp:11029`, `:11053`) ⚙️, e a `ergTable` viaja numa linha só, na chave
`ergDataPoints`, no formato `cadência|watt|resistência` separado por `;`
(`src/ergtable.h:291`, `:316`) ⚙️. **Pular chaves com `password` ou `token`** — são cifradas
no `.qzs` e o valor cifrado não serve no `.conf` ⚙️.

**Relógio:** o Pi não tem RTC e você não vai ter rede. Sem hora correta, a correlação
log↔`.fit` por hora de parede — a ferramenta que produziu tudo em §1 — quebra. Daí o DS3231.

| Aceita se | |
|---|---|
| QZ sobe, conecta na bike, anuncia como **`QZPI`** (`virtualbike.cpp:84`, guardado por `Q_OS_LINUX`) ⚙️ | ✅ |
| Hora correta após reboot sem rede | ✅ |
| Médias de potência e cadência iguais às do tablet | ✅ |
| Log não trunca | ✅ |

### 5.2 Marchas por OpenBikeControl

MyWhoosh suporta OBC **só por mDNS**, nunca por BLE (`my_whoosh.dart`,
`connections => [myWhooshLink, obpMdns]`) ⚙️. O botão `0x03` **Gear Set** manda índice
absoluto de marcha, `0x02` = marcha 1 até 254 (`PROTOCOL.md`) ⚙️ — por isso não dessincroniza
como simular tecla faria. MyWhoosh declara 30 marchas ⚙️.

Como vai por rede, **não toca no rádio do Pi e não entra no caminho de dados do trainer**.

Dois pré-requisitos no Windows:

- **Desabilitar IPv6 no adaptador** (`ncpa.cpl`). O `INSTRUCTIONS_WINDOWS_IPV6.md` do
  BikeControl é escrito para exatamente "BikeControl e MyWhoosh no mesmo Windows" ⚙️.
- **Firewall:** o BikeControl anuncia e o MyWhoosh abre o TCP (`MDNS.md`) ⚙️ — liberar
  entrada e mDNS em UDP 5353, rede Privada.

❓ a confirmar: se o teto de 20 min/dia se aplica ao modo controlador OBC ou só ao virtual
shifting próprio do BikeControl. Se aplicar, um teclado numérico USB (~R$ 50) faz o mesmo
sem limite.

### 5.3 Coexistência de rádio no Pi ❓

Único item que restou do argumento do bridge. Medir: **WiFi desligado** (`rfkill block
wifi`), **nenhum vínculo BLE extra no Pi**, três sessões de ≥ 60 min.

| Aceita se | |
|---|---|
| Nenhuma desconexão da bike nem do MyWhoosh | ✅ |
| O virtualbike segue anunciando do início ao fim | ✅ |
| Sem lacunas em `0x2AD2` acima de ~5 s | ✅ |

Se reprovar, a alternativa **não é** um bridge: é um segundo dongle **RTL8761B** (não
"CSR 4.0", que no mercado é majoritariamente clone falsificado) mais uma mudança pequena
para prender o papel de central em `hci1` — hoje o QZ só checa `allDevices()` não-vazio
(`bluetooth.cpp:152`) e usa o default ⚙️.

---

## 6. Settings

| Chave | Atual ✅ | Alvo | Nota |
|---|---|---|---|
| `bike_resistance_offset` | **17** | 18 | lido só no arranque (`main.cpp:649`) ⚙️ |
| `bike_resistance_gain_f` | 1 | 1 | ok |
| `gears_from_bike` | false | false | ok |
| `gears_current_value_f` | 0 | 0 | ok |
| `zwift_erg` | true | true em treino, false em free ride | correto para o que foi rodado |
| `virtualbike_forceresistance` | true | true | ok |
| `zwift_erg_filter` / `_down` | 10 / 10 | testar **5 / 5** | §4.2 |
| `resistance_slew_up` / `_down` | 0 / 0 ⚙️ | **2 / 4** | implementado, §3.6 |

---

## 7. Referências de código

| O quê | Onde |
|---|---|
| Escrita FTMS absoluta (YPBM) | `src/devices/ftmsbike/ftmsbike.cpp:374` |
| Limitador de taxa | `src/devices/ftmsbike/resistanceslewlimiter.h`; gate em `ftmsbike::commandResistance()`, tick em `update()` |
| Teste do limitador | `tst/Devices/TestResistanceSlewLimiter.h` |
| Banda morta do ERG | `src/devices/bike.cpp:110`, `:112` |
| `resistanceFromPowerRequest` | `src/ergtable.h` |
| Estimativa e seleção do ERG (§4.5) | `ergTable::estimateWattage`, `::wattageAtResistance`, `::nonDecreasing` |
| Histerese e retenção sem cadência (§4.5) | `ftmsbike::ergResistanceAccepted()`, `ftmsbike::resistanceFromPowerRequest()` |
| Teste do seletor | `tst/Erg/TestErgTableSelection.h` |
| `ergTable`: 10 amostras por par · persistência · serialização | `src/ergtable.h:37` · `:86`, `:90` · `:291`, `:316` |
| Marcha em dobro | `bike.cpp:73` (+1×), `ftmsbike.cpp:509` (+5×) |
| Keep-alive do ERG | `src/virtualdevices/virtualbike.cpp:1499`, `:664` |
| Nome anunciado no Linux (`QZPI`) | `virtualbike.cpp:84` |
| Adaptador BLE não selecionável | `src/devices/bluetooth.cpp:152` |
| Import/export de settings (INI) | `src/homeform.cpp:11029`, `:11053` |
| Tiles de preset de resistência | `homeform.cpp:4692–4726`; chaves em `qzsettings.cpp:525–545` |
| Offset lido no arranque | `src/main.cpp:649` |
| Jobs de Pi no CI | `.github/workflows/main.yml:1462`, `:1525`, `:1585` |

---

## 8. Incógnitas remanescentes

Ordenadas por quanto travam decisão.

1. ~~**As correções do §4.5 fecham o déficit de cadência baixa?**~~ ✅ **Fecham** — §4.6,
   volta de 8/8: erro médio +2,7 W contra −13,2 W, e entre 50 e 70 rpm o déficit sumiu.
   Aberto no lugar dela: **o limiar de 50 rpm da proteção de espiral ainda faz sentido?**
   Todo o déficit restante (−21,6 W abaixo de 50 rpm) é ela recusando aumento. Ela existe
   para quebrar o laço cadência baixa → mais resistência → cadência mais baixa, e com o
   seletor corrigido ela dispara mais (110 contra 62). Medir antes de mexer: é a mesma
   armadilha da §4.1.
2. **O limitador de taxa realmente derruba a potência média?** (§3.5) Confirmação
   qualitativa na sessão de 7/8 — com `slew = 2/4` a potência reportada passou a
   acompanhar o esforço no pedal. Falta o par de sessões medido da §3.5 para quantificar.
3. **As lacunas de 10–25 s no ERG são a banda morta?** (§4.2) Parcialmente respondida pela
   §4.5: as maiores lacunas não eram a banda morta, eram o seletor devolvendo o mesmo nível
   por 100 s enquanto o alvo ficava 20 W acima. Custo zero, é setting — ainda vale medir.
4. **A coexistência central/peripheral no `hci0` do Pi degrada?** (§5.3) Custo: uma sessão
   longa depois que a placa chegar.
5. **O log truncado desaparece no Pi?** (§5.1)
6. **O teto de 20 min/dia do BikeControl vale para o modo OBC?** (§5.2)
7. **O defeito do keep-alive chega a se manifestar?** (§4.1) Só investigar se aparecer.

**Fora do plano, opcional em qualquer ponto:** medidor de potência de pedal ou pedivela,
R$ 2.000–4.500. Não é pré-requisito de nada. É a única coisa que tornaria falsificável
qualquer afirmação sobre watt absoluto — inclusive o critério da §3.5, que hoje se apoia
numa premissa em vez de numa referência.

---

## 9. Defeitos abertos — levantados em 8/8

Três relatos do uso real. Nenhum corrigido ainda; os dois primeiros têm causa localizada no
código, o terceiro está na §4.3.

### 9.1 Nada re-arma a busca quando o dispositivo cai ❗

**Sintoma relatado:** desligar a bike derruba o QZ em vez de fazê-lo voltar para
"procurando".

**Achado.** Em `src/devices/bluetooth.cpp` existem **105 linhas** da forma
`connect(<device>, SIGNAL(disconnected()), this, SLOT(restart()))` — **todas comentadas.
Nenhuma ativa**, para nenhum tipo de dispositivo. O `disconnected()` é emitido
(`ftmsbike.cpp:2376–2379`, no `QLowEnergyController::disconnected`) e **não tem quem
escute**. Não há caminho de volta para a descoberta.

Consequência direta: o objeto do dispositivo continua vivo com um `QLowEnergyController`
morto pendurado, e todo mundo que ainda segura `bluetoothManager->device()` — `homeform`,
o tick de `update()`, os virtual devices — segue chamando método em cima disso. É o
candidato natural ao crash, e explica por que o sintoma é "morre" em vez de "volta a
procurar".

**Duas correções de premissa, ambas verificadas:**

1. **`android_daemon_mode` é chave morta.** Não existe em `qzsettings.h` nem em
   `qzsettings.cpp`, e nada no código a lê — as únicas ocorrências de "daemon" em `src/`
   são os fontes do `adb` embarcado para iOS, sem relação. Ela aparece no log porque
   `main.cpp:710` despeja `settings.allKeys()`, isto é, o arquivo de settings inteiro,
   incluindo chaves legadas de versões antigas. **Não existe "modo daemon" neste build**;
   não há o que configurar nem o que consertar aí.
2. **Log terminando no meio não é prova de crash.** A §1.6 já estabeleceu que o log trunca.
   Os três logs de 7/8 terminam abruptamente numa escrita do virtualbike e têm **0**
   ocorrências de "disconnected" — o que é consistente tanto com crash quanto com a
   truncagem conhecida. **O log do QZ não vai conter o stack.**

**O que logar.** O stack está no Android, não no QZ. Agora que o adb funciona:

```bash
"/c/Android/sdk/platform-tools/adb.exe" logcat -b crash -d          # buffer de crash
"/c/Android/sdk/platform-tools/adb.exe" shell dumpsys dropbox --print | head -200
"/c/Android/sdk/platform-tools/adb.exe" shell ls /data/tombstones   # se acessível
```
Reproduzir com `adb logcat` aberto e desligar a bike. Somar a isso um `qDebug` no
`disconnected()` de `ftmsbike` com o estado do controller no momento — hoje não há nenhum.

**Teste.** O acoplamento de `bluetooth.cpp` ao BLE real torna caro um teste fim a fim. O
teste que paga é sobre a **máquina de estados**: dado um dispositivo em estado conectado,
emitir `disconnected()` e verificar que (a) a descoberta re-arma e (b) ninguém mais
desreferencia o dispositivo antigo. Isso pede um device falso — `fakebike` já existe e
serve de ponto de partida.

### 9.2 O handshake de START/RESUME não espera confirmação, e o resultado fica travado ❗

**Sintoma relatado:** às vezes o comando de "start" não sai, ou sai cedo demais; a console
nunca faz a contagem "3..2..1"; só resolve reiniciando o QZ.

**Achado.** `ftmsbike::init()` (`ftmsbike.cpp:171`) tem quatro problemas somados, e os
quatro batem com o sintoma:

| | |
|---|---|
| 1 | `ret` recebe o retorno do `REQUEST_CONTROL` (0x00) e é **sobrescrito** pelo do `START_RESUME` (0x07). O resultado do request-control é descartado sem ser olhado |
| 2 | `writeCharacteristic` devolve o retorno de `enqueueWrite`: **"foi enfileirado"**, não "a bike confirmou". `ret == true` não diz nada sobre o controle ter sido concedido |
| 3 | **Não há espera pela indicação do control point entre 0x00 e 0x07.** Pelo FTMS o servidor precisa responder ao request-control antes de aceitar as operações seguintes. Emitir 0x07 na sequência é corrida — se o controle ainda não foi concedido, a bike descarta o start. É literalmente "cedo demais" |
| 4 | `initDone = true` **trava**: `init()` retorna cedo para sempre. Um handshake que falhou **nunca é repetido** — daí "só resolve reiniciando" |

O workaround do D500V2 no mesmo arquivo (`:1976–1994`) mostra que a ordem 0x00 → 0x07 já é
conhecida como frágil, mas o tratamento existe do lado do **dispositivo virtual**, não do
lado da bike real.

**O que logar.** Cada resposta do control point com o **result code** (o FTMS devolve
`0x80 <opcode> <result>`; `0x01` é sucesso), e o intervalo em ms entre o 0x00 e o 0x07.
Hoje o log mostra a escrita saindo e nada sobre a resposta, que é justamente a metade que
decide.

**Teste.** É o defeito mais barato de testar dos três: o control point é um caminho de
bytes. Alimentando as respostas do dispositivo dá para verificar sem BLE que (a) o 0x07 só
sai depois do ack do 0x00, (b) `initDone` só trava quando o 0x07 é confirmado, e (c) um ack
que não vem provoca nova tentativa em vez de silêncio permanente.

**Correção candidata, não aplicada:** tratar a indicação do control point, marcar `initDone`
apenas com o 0x07 confirmado, e repetir o handshake se a confirmação não chegar.

### 9.3 Auto-ERG — ver §4.3

Não é "não testado", é **não construído**. A verificação e o que dá para testar do tile de
potência-alvo estão na §4.3.1.

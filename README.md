# IceTrack 🧊
### Estação de Monitoramento Polar — FIAP Global Solution 2026

Plataforma de monitoramento e projeção do degelo glacial que consome dados satelitais
públicos para gerar visualizações históricas e alertas científicos. Esta entrega
corresponde ao módulo de **Edge Computing & Computer Systems**, que simula o nó
físico de coleta de dados em campo — a estação polar que complementa os dados orbitais
com medições locais em tempo real.

---

## O Problema

As geleiras do planeta estão em retração acelerada. Desde 1979, a extensão do gelo
ártico diminuiu em média **13% por década** (fonte: NSIDC — National Snow and Ice
Data Center). O derretimento do gelo marinho e das calotas polares contribui diretamente
para a elevação do nível dos oceanos, afetando centenas de milhões de pessoas em regiões
costeiras.

Monitorar esse processo em tempo real exige dois tipos de dados:

- **Dados orbitais** — imagens satelitais de larga escala (NASA, ESA, INPE)
- **Dados de campo** — medições locais de temperatura superficial e reflectividade
  do gelo, coletadas por estações físicas instaladas nas zonas polares

Este protótipo representa a segunda camada: **a estação de campo**.

---

## A Solução — IceTrack

O IceTrack é composto por três camadas integradas:
[ Satélites (NASA / ESA / INPE) ]
↓ dados orbitais
[ Backend Python ]  ←→  [ Estação Arduino (este repositório) ]
↓ dados processados
[ Dashboard Web / App de consulta ]

A estação Arduino coleta temperatura superficial e índice de albedo local, calcula
o índice de degelo em tempo real e emite alertas quando as condições indicam degelo
ativo. Esses dados seriam transmitidos via protocolo IoT (MQTT) para o backend,
onde são cruzados com as imagens satelitais para gerar projeções de longo prazo.

---

## Componentes do Protótipo

| Componente | Pino | Função |
|---|---|---|
| DHT22 | D8 | Temperatura superficial do gelo (°C) |
| Fotoresistor LDR | A0 | Albedo simulado — reflectividade da superfície |
| LCD I2C 16x2 | SDA=A4 / SCL=A5 | Exibição dos dados em tempo real |
| LED vermelho | D13 | Alerta visual de degelo ativo |

---

## Como Interpretar o Protótipo

### Temperatura (DHT22)

O sensor DHT22 mede a temperatura do ambiente simulando a temperatura superficial
de uma geleira. Na realidade, estações polares usam termômetros de contato ou
termômetros infravermelhos apontados para a superfície do gelo.

**Referência de valores:**
- Abaixo de **-10°C** → superfície estável, degelo mínimo
- Entre **-10°C e -2°C** → zona de atenção
- Acima de **-2°C** → limiar crítico, degelo ativo → **LED acende**

O limiar de -2°C é baseado no ponto em que a taxa de fusão superficial do gelo
marinho se torna significativa, conforme documentado pelo
IPCC AR6 (2021), Capítulo 9.

### Albedo (LDR)

O fotoresistor simula um **piranômetro de reflexão** — instrumento real usado em
estações polares para medir o albedo superficial, ou seja, a fração da radiação
solar que a superfície reflete.

**Fórmula aplicada:**
albedo(%) = map(leitura_LDR, 0, 1023, 30, 90)
Equivalente a:
albedo(%) = 30 + (leitura_LDR / 1023) × 60

**Referência dos limites (NSIDC / IPCC AR6):**

| Tipo de superfície | Albedo |
|---|---|
| Neve fresca / gelo limpo | ~90% |
| Gelo compactado antigo | ~50–60% |
| Gelo sujo (algas, carbono) | ~30% |
| Água exposta (ex-geleira) | ~6% |

Quanto mais luz o LDR recebe, maior o albedo calculado — simulando uma superfície
mais reflexiva (gelo fresco). Pouca luz = albedo baixo = superfície degradada.

### Índice de Degelo (Dg)

Calculado exclusivamente a partir da temperatura, usando interpolação linear
entre os extremos da faixa polar monitorada:
Se temp ≤ -30°C  →  Dg = 0%
Se temp ≥  10°C  →  Dg = 100%
Caso contrário   →  Dg = (temp - (-30)) / (10 - (-30)) × 100

**Interpretação:**
- **0–20%** → condições polares normais, gelo estável
- **21–50%** → aquecimento moderado, monitoramento necessário
- **51–80%** → degelo significativo em curso
- **81–100%** → fusão acelerada, situação crítica

### Ciclo de Retroalimentação (Ice-Albedo Feedback)

O motivo pelo qual monitorar **ambos** os sensores juntos é cientificamente relevante:
Gelo derrete → expõe água escura → albedo cai (de 90% para ~6%)
↓
Superfície absorve mais calor solar
↓
Temperatura sobe → mais gelo derrete → ciclo se acelera

Uma queda progressiva no albedo lido pelo LDR, mesmo sem aumento imediato
de temperatura, é sinal de degradação da superfície glacial — exatamente o
que estações de campo reais monitoram para antecipar eventos de degelo.

### Display LCD
Linha 0:  [sinal][temp]°C        [status]
Linha 1:  Dg:[índice]% Al:[albedo]%
Exemplo normal:   -16.5°C          OK
Dg: 11% Al:67%
Exemplo de alerta: +2.3°C        ALERT
Dg: 80% Al:38%

### LED de Alerta

Permanece apagado enquanto `temp ≤ -2°C`. Passa a **piscar a cada 1 segundo**
quando a temperatura ultrapassa o limiar crítico, indicando degelo ativo.
O estado do LED é alternado por flag a cada ciclo de leitura, sem uso de
`delay()`, mantendo o loop responsivo.

### Monitor Serial

Todas as leituras são enviadas ao monitor serial (9600 baud) em formato de
tabela para registro e análise:
Timestamp(ms) | Temp(C) | Albedo(%) | IDegelo(%) | Alerta
1000 ms    | -16.5 C  | 67%       | 11%        | Normal
2000 ms    | -16.5 C  | 65%       | 11%        | Normal
3000 ms    | +2.3 C   | 38%       | 80%        | *** ALERTA ***

---

## Simulação

O protótipo foi desenvolvido e validado no simulador **Wokwi**.

🔗 [Link do projeto no Wokwi](#) ← substituir pelo link real antes da entrega

Para simular diferentes cenários:
- Altere o atributo `"temperature"` do DHT22 no `diagram.json` para testar
  valores abaixo e acima do limiar de -2°C
- Ajuste a intensidade de luz sobre o LDR no simulador para variar o albedo

---

## Estrutura do Repositório
icetrack-edge/
├── icetrack_edge.ino   — código principal Arduino
├── diagram.json        — circuito Wokwi
├── README.md           — este arquivo
└── integrantes.txt     — nomes e RMs da equipe

---

## Referências

- NSIDC — National Snow and Ice Data Center: https://nsidc.org/data/g02135
- IPCC Sixth Assessment Report (AR6), 2021 — Chapter 9: Ocean, Cryosphere and Sea Level Change
- NASA Earthdata — MODIS Snow and Ice Products: https://earthdata.nasa.gov
- ESA Copernicus — Sentinel-2 Surface Reflectance: https://browser.dataspace.copernicus.eu

---

## Integrantes

| Nome | RM |
|---|---|
| — | — |
| — | — |
| — | — |
| — | — |
| — | — |

---

*Global Solution 2026 · FIAP · Engenharia de Software 1º Ano · Período: 25/05 – 09/06/2026*

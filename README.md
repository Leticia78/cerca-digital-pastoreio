#Cerca Digital para Gestão de Pastoreio e Monitorização Remota de Animais

> Projeto Final de Curso — Universidade Politécnica de Castelo Branco (UPCB-2026)  
> Autora: Letícia Pontes Síndica  

##  Descrição

Sistema IoT completo de **cerca digital (virtual fencing)** para monitorização remota de animais em pastoreio extensivo, sem necessidade de cercas físicas.

O nó terminal  instalado no animal transmite continuamente a sua localização GPS via **LoRaWAN EU868**, sendo os dados processados em tempo real no **Node-RED**, armazenados no **InfluxDB** e visualizados no **Grafana** com alertas automáticos por email quando o animal ultrapassa os limites definidos.

## Arquitetura do Sistema

[ESP32-S3 + GPS + LoRa]-[Geofencing Ray Casting]
        ↓ LoRaWAN 868 MHz
[Gateway Advantech WISE-6610 V2]
        ↓ MQTT (HiveMQ) — tópico: uplink/xxxxxx
[Node-RED — Geofencing Ray Casting]
        ↓
[InfluxDB 2.x] → [Grafana Dashboard]


## Hardware Utilizado

| Componente | Descrição |
|---|---|
| LilyGO TTGO T-Beam Supreme | ESP32-S3 
| DHT22 | Sensor de temperatura e humidade (GPIO2) |
| LM35 | Sensor de temperatura analógico (GPIO3) |
| Buzzer | Alerta sonoro (GPIO39) |
| Advantech WISE-6610 V2 | Gateway LoRaWAN industrial (concentrador SX1302) | 
| Bateria FST 18650 | 2600 mAh, 3.6V/4.2V |

##  Comunicação LoRaWAN

- **Banda:** EU868 (868 MHz, ISM não licenciada)
- **Protocolo:** LoRaWAN 1.0.3
- **Activação:** OTAA (Over-The-Air Activation)
- **Payload:** 15 bytes binários
- **DevEUI:**Ocultar DevEUI**
- **Encriptação:** xxxxxxxxxx

### Estrutura do Payload (15 bytes)

| Bytes | Campo | Tipo |
|---|---|---|
| 0-3 | Latitude | int32 |
| 4-7 | Longitude | int32 |
| 8-9 | Distância à cerca | int16 |
| 10 | Velocidade | uint8 |
| 11 | Satélites | uint8 |
| 12 | GPS Fix | uint8 |
| 13 | Estado Alerta | uint8 |
| 14 | Temperatura | int8 |


## Geofencing — Ray Casting

O algoritmo **Ray Casting** verifica em tempo real se o animal está dentro ou fora do perímetro digital:

 **Estado 0 🟢** — Animal seguro, dentro da cerca → silêncio
 **Estado 1 🟡** — Animal a menos de 15m do limite → 10 beeps
 **Estado 2 🔴** — Animal fora da cerca → 20 beeps + email automático

O geofencing é executado em **dois níveis** (redundância):
1. Localmente no **ESP32** para activação imediata do buzzer
2. No **Node-RED** para alertas remotos e registo na base de dados

##  Software e Plataformas

### Node-RED
- Broker MQTT: HiveMQ público, tópico `xxxxxxxx`
- Nós principais: MQTT In → JSON → Geofencing Ray Casting → InfluxDB + Worldmap

### InfluxDB 2.x
- Organização: `CercaDigital`
- Bucket: `MonitorizacaoAnimais_V2`
- Measurement: `animal`

### Grafana
- Dashboard: `Cerca Digital_Coleira`
- Painéis: Geomap, Gauges (temp/humidade/satélites), Time Series, GPS Fix
- Alertas: SMTP Gmail porta 587, avaliação a cada 10 segundos

##  Autonomia da Bateria

| Modo | Autonomia |
| Contínuo (sem sleep) | ~11 horas |



## 📁 Estrutura do Repositório

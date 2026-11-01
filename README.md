Cerca Digital para Gestão de Pastoreio e Monitorização Remota de Animais

Final Course Project — Instituto Politécnico de Castelo Branco (IPCB-2026)
Author: Letícia Pontes Síndica

Description

Complete IoT system for digital fencing (virtual fencing) for remote monitoring of animals in extensive grazing, without the need for physical fences.

The terminal node installed on the animal continuously transmits its GPS location via LoRaWAN EU868, with data processed in real time in Node-RED, stored in InfluxDB and visualised in Grafana with automatic email alerts when the animal exceeds the defined boundaries.

System Architecture
[ESP32-S3 + GPS + LoRa] — [Geofencing Ray Casting]
        ↓ LoRaWAN 868 MHz
[Gateway Advantech WISE-6610 V2]
        ↓ MQTT (HiveMQ) — topic: uplink/xxxxxx
[Node-RED — Geofencing Ray Casting]
        ↓
[InfluxDB 2.x] → [Grafana Dashboard]
Hardware Used
Component	Description
LilyGO TTGO T-Beam Supreme	ESP32-S3
DHT22	Temperature and humidity sensor (GPIO2)
LM35	Analogue temperature sensor (GPIO3)
Buzzer	Audible alert (GPIO39)
Advantech WISE-6610 V2	Industrial LoRaWAN gateway (SX1302 concentrator)
FST 18650 Battery	2600 mAh, 3.6V/4.2V

LoRaWAN Communication
Band: EU868 (868 MHz, unlicensed ISM)
Protocol: LoRaWAN 1.0.3
Activation: OTAA (Over-The-Air Activation)
Payload: 15 binary bytes
DevEUI: Configure in firmware/config.h
Encryption: AES-128

Payload Structure (15 bytes)
Bytes	Field	Type
0–3	Latitude	int32
4–7	Longitude	int32
8–9	Distance to fence	int16
10	Speed	uint8
11	Satellites	uint8
12	GPS Fix	uint8
13	Alert State	uint8
14	Temperature	int8

Geofencing — Ray Casting

The Ray Casting algorithm verifies in real time whether the animal is inside or outside the digital perimeter:

🟢 State 0 — Animal safe, inside the fence → silence
🟡 State 1 — Animal within 15 m of the boundary → 10 beeps
🔴 State 2 — Animal outside the fence → 20 beeps + automatic email

Geofencing runs at two levels (redundancy):

Locally on the ESP32 for immediate buzzer activation
In Node-RED for remote alerts and database logging

Software and Platforms
Node-RED
MQTT Broker: Public HiveMQ, topic xxxxxxxx
Main nodes: MQTT In → JSON → Geofencing Ray Casting → InfluxDB + Worldmap

InfluxDB 2.x
Organisation: CercaDigital
Bucket: MonitorizacaoAnimais_V2
Measurement: animal

Grafana
Dashboard: Cerca Digital_Coleira
Panels: Geomap, Gauges (temp / humidity / satellites), Time Series, GPS Fix
Alerts: SMTP Gmail port 587, evaluation every 10 seconds

Battery Life
Mode	Battery Life
Continuous (no sleep)	~11 hours

Repository Structure

cerca-digital-pastoreio/
├── firmware/
│   ├── CODIGO_CERCADIGITAL.ino
│   └── config.h.example
├── flows.json
├── dashboard-1793494304853.json
├── docs/
│   └── images/
├── influxdb/
│   └── README.md
├── .gitignore
└── README.md

System Screenshots
 Digital Fence Map
![Cerca Digital](docs/images/cerca.jpeg)

###  Dispositivo ESP32-S3
![ESP32-S3](docs/images/ESP32.S3.jpeg)

###  Gateway Advantech WISE-6610
![Gateway](docs/images/Gateway.LORAWAN.jpeg)
![Gateway](docs/images/GATEWAY.L.jpeg)
![Gateway](docs/images/GATEWAY.jpeg)

### Fluxo Node-RED
![Fluxo Node-RED](docs/images/Fluxo.dADOS.PNG)

###  InfluxDB
![InfluxDB](docs/images/influxdbbb.jpeg)

###  Estado — Seguro
![Seguro](docs/images/seguro.jpeg)

###  Estado — Zona de Risco
![Zona de Risco](docs/images/zona.risco.jpeg)

###  Estado — Fuga
![Fuga](docs/images/Fuga.jpeg)

###  Dashboard — Alerta
![Alerta](docs/images/alertaaa.jpeg)

###  Alerta por Email
![Email de Alerta](docs/images/Email.alerta.PNG)

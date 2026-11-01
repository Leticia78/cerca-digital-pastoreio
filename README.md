##Cerca Digital para Gestão de Pastoreio e Monitorização Remota de Animais##

Final Course Project — Polytechnic University of Castelo Branco (UPCB-2026)

Author: Letícia Pontes Sindica

Description

Complete IoT system for digital fencing (virtual fencing) for remote monitoring of animals in extensive grazing, without the need for physical fences.

The terminal node installed on the animal continuously transmits its GPS location via LoRaWAN EU868, with data processed in real time in Node-RED, stored in InfluxDB and visualised in Grafana with automatic email alerts when the animal exceeds the defined boundaries.

## System Architecture — 4-Layer Overview

![4-Layer System Architecture](docs/images/camadas.png)


## Hardware Used

| Component | Description |
|-----------|-------------|
| LilyGO TTGO T-Beam Supreme | ESP32-S3 dual-core Xtensa LX7 240 MHz + LoRa SX1262 + GPS |
| DHT22 | Temperature and humidity sensor (GPIO2) |
| LM35 | Analogue temperature sensor (GPIO3) |
| Buzzer | Audible alert (GPIO39) |
| Advantech WISE-6610 V2 | Industrial LoRaWAN gateway (SX1302 concentrator) |
| FST 18650 Battery | 2600 mAh, 3.6V/4.2V |

## LoRaWAN Communication

| Parameter | Value |
|-----------|-------|
| Band | EU868 (868 MHz, unlicensed ISM) |
| Protocol | LoRaWAN 1.0.3 |
| Activation | OTAA (Over-The-Air Activation) |
| Payload | 15 binary bytes |
| DevEUI |xxxxxxxxxxx |
| Encryption | AES-128 |

## Payload Structure (15 bytes)

| Bytes | Field | Type |
|-------|-------|------|
| 0–3   | Latitude | int32 |
| 4–7   | Longitude | int32 |
| 8–9   | Distance to fence | int16 |
| 10    | Speed | uint8 |
| 11    | Satellites | uint8 |
| 12    | GPS Fix | uint8 |
| 13    | Alert State | uint8 |
| 14    | Temperature | int8 |

Geofencing Algorithm — Ray Casting

The detection of an animal’s position relative to the boundaries of the digital fence is performed using an algorithm called Ray Casting, widely used in geofencing systems for being computationally efficient and suitable for resource-constrained devices.

The operating principle of the algorithm is based on the current position of the animal (defined by the GPS coordinates of latitude and longitude). An imaginary horizontal line (ray) is cast to the right. The system counts how many times this ray intersects the segments that form the perimeter of the digital fence (polygon). If the number of intersections is odd, the animal is inside the fence; if it is even (including zero), the animal is outside.

In this project, the Ray Casting algorithm is implemented at two distinct and complementary levels: on the T-Beam Supreme ESP32-S3, the microcontroller locally executes the algorithm, comparing the animal’s GPS position with the coordinates of the four vertices of the digital fence.

The Ray Casting algorithm verifies in real time whether the animal is inside or outside the digital perimeter:

## Geofencing States

| State | Indicator | Description | Alert |
|-------|-----------|-------------|-------|
| 0 | 🟢 Safe | Animal inside the fence | Silence |
| 1 | 🟡 Warning | Animal within 15 m of the boundary | 10 beeps |
| 2 | 🔴 Breach | Animal outside the fence | 20 beeps + automatic email |

In Node-RED (cloud processing), after receiving data via MQTT, the Node-RED flow re-executes the Ray Casting algorithm with the coordinates received via LoRaWAN, determining the fence_state (0, 1 or 2). The result is stored in InfluxDB and visualised in Grafana, where the animal's marker changes colour according to the state (green — yellow — red).

Software and Platforms

Node-RED

MQTT Broker: Public HiveMQ, topic xxxxxxxx

Main nodes: MQTT In → JSON → Geofencing Ray Casting → InfluxDB + Worldmap

#InfluxDB 2.x

Organisation: CercaDigital

Bucket: MonitorizacaoAnimais_V2

Measurement: animal

#Grafana

Dashboard: Cerca Digital_Coleira

Panels: Geomap, Gauges (temp / humidity / satellites), Time Series, GPS Fix

Alerts: SMTP Gmail port 587, evaluation every 10 seconds

#Battery Life

Mode	Battery Life

Continuous (no sleep)	~11 hours

Repository Structure

## Repository Structure

| Path | Description |
|------|-------------|
| `firmware/CODIGO_CERCADIGITAL.ino` | Main ESP32-S3 firmware |
| `firmware/config.h.example` | Configuration template (keys not included) |
| `flows.json` | Node-RED flows export |
| `dashboard-1793494304853.json` | Grafana dashboard export |
| `docs/images/` | System screenshots |
| `influxdb/README.md` | InfluxDB configuration details |
| `.gitignore` | Excludes real config.h and binaries |
| `README.md` | Project documentation |


System Screenshots

 Digital Fence Map
![Cerca Digital](docs/images/cerca.jpeg)

###  ESP32-S3 device
![ESP32-S3](docs/images/ESP32.S3.jpeg)

###  Gateway Advantech WISE-6610
![Gateway](docs/images/Gateway.LORAWAN.jpeg)
![Gateway](docs/images/GATEWAY.L.jpeg)
![Gateway](docs/images/GATEWAY.jpeg)

### Node-RED Flow
![Fluxo Node-RED](docs/images/Fluxo.dADOS.PNG)

###  InfluxDB
![InfluxDB](docs/images/influxdbbb.jpeg)

###  State — Secure
![Seguro](docs/images/seguro.jpeg)

###  State — Risk Zone
![Zona de Risco](docs/images/zona.risco.jpeg)

### State — Escape
![Fuga](docs/images/Fuga.jpeg)

###  Dashboard — Alert
![Alerta](docs/images/alertaaa.jpeg)

###  Email Alert
![Email de Alerta](docs/images/Email.alerta.PNG)

#include "config.h"
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <XPowersLib.h>
#include "DHT.h"

#define DHTPIN  2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

XPowersAXP2101 PMU;
TwoWire I2C_PMU = TwoWire(1);

// =====================================================
// OLED
// =====================================================
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(
  U8G2_R0, U8X8_PIN_NONE, 18, 17
);

// =====================================================
// GPS
// =====================================================
#define GPS_RX         9
#define GPS_TX         8
#define GPS_BAUD       9600
#define GPS_ENABLE_PIN 7

TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

unsigned long contadorPacotes = 0;

// =====================================================
// TEMPERATURA — LM35
// =====================================================
const int sensorPin = 3;

float lerTemperatura() {
  int value = analogRead(3);
  float millivolts = (value / 4095.0) * 3300.0;
  float celsius = millivolts / 10.0;
  Serial.print("Temp: ");
  Serial.print(celsius);
  Serial.println(" C");
  return celsius;
}

// =====================================================
// FUNÇÃO — Mostra texto no OLED
// =====================================================
void mostra(const char* texto) {
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB14_tr);
  int x = (128 - display.getStrWidth(texto)) / 2;
  display.drawStr(x, 38, texto);
  display.sendBuffer();
}

// =====================================================
// CERCA DIGITAL
// =====================================================
const int NUM_PONTOS_CERCA = 4;

float cercaLat[] = {
    39.819490534286274,
    39.819115592729060,
    39.819276282218354,
    39.819581179190860
};

float cercaLon[] = {
    -7.512666285037995,
    -7.512607276439668,
    -7.512019872665406,
    -7.512221038341523
};
const float ZONA_AVISO_METROS = 15.0;

bool dentroDoPoligono(float lat, float lon) {
    bool dentro = false;
    int j = NUM_PONTOS_CERCA - 1;
    for (int i = 0; i < NUM_PONTOS_CERCA; i++) {
        if (((cercaLon[i] > lon) != (cercaLon[j] > lon)) &&
            (lat < (cercaLat[j] - cercaLat[i]) * (lon - cercaLon[i]) /
            (cercaLon[j] - cercaLon[i]) + cercaLat[i])) {
            dentro = !dentro;
        }
        j = i;
    }
    return dentro;
}

float distanciaAoLimite(float lat, float lon) {
    float R = 6371000.0;
    float distMin = 999999.0;
    for (int i = 0; i < NUM_PONTOS_CERCA; i++) {
        float dLat = (lat - cercaLat[i]) * PI / 180.0;
        float dLon = (lon - cercaLon[i]) * PI / 180.0;
        float a = sin(dLat/2) * sin(dLat/2) +
                  cos(lat * PI / 180.0) * cos(cercaLat[i] * PI / 180.0) *
                  sin(dLon/2) * sin(dLon/2);
        float dist = R * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
        if (dist < distMin) distMin = dist;
    }
    return distMin;
}

void verificarGeofencingLocal(float lat, float lon, int fix) {
    if (fix != 1) return;

    bool dentro = dentroDoPoligono(lat, lon);
    float distancia = distanciaAoLimite(lat, lon);

    Serial.print("Dentro: "); Serial.print(dentro ? "SIM" : "NAO");
    Serial.print(" | Distância: "); Serial.print(distancia); Serial.println("m");

    if (!dentro) {
        Serial.println("FUGA!");
        mostra("FUGA!");
        buzzerAlerta();      // ← 20 bipes
    } else if (distancia <= ZONA_AVISO_METROS) {
        Serial.println("AVISO!");
        mostra("AVISO!");
        buzzerAviso();       // ← 10 bipes
    } else {
        Serial.println("SEGURO");
        mostra("OK");
        buzzerSeguro();      // ← silêncio
    }
}

// =====================================================
// BUZZER ACTIVO — GPIO 39
// =====================================================
void buzzerSeguro() {
    digitalWrite(39, LOW);   // Silêncio
    Serial.println("Buzzer: SEGURO — silêncio");
}

void buzzerAviso() {
    Serial.println("Buzzer: AVISO — bipes intermitentes");
    for (int i = 0; i < 10; i++) {
        digitalWrite(39, HIGH);
        delay(150);
        digitalWrite(39, LOW);
        delay(150);
    }
}

void buzzerAlerta() {
    Serial.println("Buzzer: ALERTA — apito contínuo");
    for (int i = 0; i < 20; i++) {
        digitalWrite(39, HIGH);
        delay(100);
        digitalWrite(39, LOW);
        delay(100);
    }
}
// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);

  // LM35
  analogReadResolution(12);
  Serial.println("LM35 iniciado...");

  // DHT22
  dht.begin();
  Serial.println("DHT22 iniciado no pino 2!");

  // OLED
  Wire.begin(17, 18);
  display.begin();
  mostra("INICIANDO");

  // GPS PMU
  I2C_PMU.begin(42, 41);
  if (!PMU.begin(I2C_PMU, AXP2101_SLAVE_ADDRESS, 42, 41)) {
    Serial.println("ERRO: PMU nao encontrado!");
  } else {
    Serial.println("PMU OK!");
    PMU.setALDO3Voltage(3300);
    PMU.enableALDO4();
    Serial.println("GPS alimentado!");
  }

  // Activa GPS L76K via GPIO 7
  pinMode(GPS_ENABLE_PIN, OUTPUT);
  digitalWrite(GPS_ENABLE_PIN, HIGH);
  delay(500);

  // GPS Serial
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS iniciado!");

  // Buzzer activo
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Começa em silêncio
  Serial.println("Buzzer iniciado!");

  // Rádio
  ConfigLoRa_t config;
  config.frequency = 868;
  int16_t state = radio.begin(config);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Erro radio: ");
    Serial.println(stateDecode(state));
    mostra("ERRO RADIO");
    while (true);
  }

  Serial.println("Radio OK!");
  mostra("CONECTANDO");

  // LoRaWAN OTAA
  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  state = node.activateOTAA();
  if (state != RADIOLIB_LORAWAN_NEW_SESSION &&
      state != RADIOLIB_LORAWAN_SESSION_RESTORED) {
    Serial.print("Join falhou: ");
    Serial.println(stateDecode(state));
    mostra("ERRO JOIN");
    while (true);
  }

  // DEPOIS — Classe A (padrão)
  Serial.println("Classe A activa!");

  Serial.println("Ligado ao gateway!");
  mostra("CONECTADO");
  delay(2000);
}

// =====================================================
// LOOP
// =====================================================
void loop() {
  int raw = analogRead(LM35_PIN);
  Serial.print("ADC raw: ");
  Serial.println(raw);

  // Lê GPS
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }

  contadorPacotes++;

  float lat = 0.0, lon = 0.0, alt = 0.0, vel = 0.0;
  int sat = 0, fix = 0;

  // Lê temperatura
  float temperatura = lerTemperatura();

  if (gps.location.isValid()) {
    fix = 1;
    lat = gps.location.lat();
    lon = gps.location.lng();
    alt = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
    vel = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
    sat = gps.satellites.isValid() ? gps.satellites.value() : 0;
  }

  // Geofencing Local
  verificarGeofencingLocal(lat, lon, fix);

  float humidade = dht.readHumidity();
  float tempDHT  = dht.readTemperature();

  if (isnan(humidade) || isnan(tempDHT)) {
    Serial.println("Falha ao ler DHT22!");
    humidade = 0.0;
    tempDHT  = 0.0;
  } else {
    Serial.print("DHT22 Humidade: "); Serial.print(humidade); Serial.println("%");
    Serial.print("DHT22 Temp: ");     Serial.print(tempDHT);  Serial.println("°C");
  }

   // Payload binário — 49 bytes (15 dados + 32 cerca + 2 distância)
  int32_t latInt = (int32_t)(lat * 1e6);
  int32_t lonInt = (int32_t)(lon * 1e6);
  int16_t altInt = (int16_t)alt;
  uint8_t velInt = (uint8_t)min(vel, 255.0f);

  // Calcula distância local
  float distanciaLocal = 0.0;
  if (fix == 1) {
      distanciaLocal = distanciaAoLimite(lat, lon);
  }
  uint16_t distInt = (uint16_t)constrain(distanciaLocal, 0, 65535);

  uint8_t dados[49];

  // Bytes 0-14 — dados actuais (igual ao anterior)
  dados[0]  = (latInt >> 24) & 0xFF;
  dados[1]  = (latInt >> 16) & 0xFF;
  dados[2]  = (latInt >> 8)  & 0xFF;
  dados[3]  =  latInt        & 0xFF;
  dados[4]  = (lonInt >> 24) & 0xFF;
  dados[5]  = (lonInt >> 16) & 0xFF;
  dados[6]  = (lonInt >> 8)  & 0xFF;
  dados[7]  =  lonInt        & 0xFF;
  dados[8]  = (altInt >> 8)  & 0xFF;
  dados[9]  =  altInt        & 0xFF;
  dados[10] = velInt;
  dados[11] = (uint8_t)sat;
  dados[12] = (uint8_t)fix;
  dados[13] = (uint8_t)constrain(temperatura, 0, 255);
  dados[14] = (uint8_t)constrain(humidade, 0, 255);

  // Bytes 15-46 — coordenadas da cerca actualizadas
  float cercaLat[] = {
      39.819490534286274,
      39.819115592729060,
      39.819276282218354,
      39.819581179190860
  };
  float cercaLon[] = {
      -7.512666285037995,
      -7.512607276439668,
      -7.512019872665406,
      -7.512221038341523
  };

  for (int i = 0; i < NUM_PONTOS_CERCA; i++) {
      int32_t cLat = (int32_t)(cercaLat[i] * 1e6);
      int32_t cLon = (int32_t)(cercaLon[i] * 1e6);
      dados[15 + i*8 + 0] = (cLat >> 24) & 0xFF;
      dados[15 + i*8 + 1] = (cLat >> 16) & 0xFF;
      dados[15 + i*8 + 2] = (cLat >> 8)  & 0xFF;
      dados[15 + i*8 + 3] =  cLat        & 0xFF;
      dados[15 + i*8 + 4] = (cLon >> 24) & 0xFF;
      dados[15 + i*8 + 5] = (cLon >> 16) & 0xFF;
      dados[15 + i*8 + 6] = (cLon >> 8)  & 0xFF;
      dados[15 + i*8 + 7] =  cLon        & 0xFF;
  }

  // Bytes 47-48 — distância ao limite em metros
  dados[47] = (distInt >> 8) & 0xFF;
  dados[48] =  distInt       & 0xFF;

  Serial.println("--------------------------------");
  Serial.print("Pacote #");  Serial.println(contadorPacotes);
  Serial.print("Lat: ");     Serial.println(lat, 6);
  Serial.print("Lon: ");     Serial.println(lon, 6);
  Serial.print("Fix: ");     Serial.println(fix);
  Serial.print("Sat: ");     Serial.println(sat);
  Serial.print("Distância: "); Serial.print(distanciaLocal); Serial.println("m");
  mostra("ENVIANDO");

  int16_t state = node.sendReceive(dados, sizeof(dados), 10);

  if (state >= RADIOLIB_ERR_NONE) {
    Serial.println("Uplink enviado!");
    char texto[20];
    sprintf(texto, "PKT:%lu", contadorPacotes);
    mostra(texto);

  } else {
    Serial.print("Erro: ");
    Serial.println(state);
    mostra("ERRO ENVIO");
  }

  delay(uplinkIntervalSeconds * 1000UL);
}
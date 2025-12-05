#include <Wire.h>
#include "SensorDHT.h"

// -------------------- CONFIGURAÇÕES --------------------
#define DHTPIN 4            // Pino do DHT22
#define DHTTYPE DHT22       // Tipo do sensor
#define I2C_DEV_ADDR 0x55   // Endereço do ESP32 como SLAVE
#define LED_PIN 2           // Pino seguro para LED no ESP32

// -------------------- VARIÁVEIS --------------------
SensorDHT sensorDHT(DHTPIN, DHTTYPE);
char i2cBuffer[32]; // Buffer que será enviado ao Master

// -------------------- FUNÇÕES I2C --------------------

// Chamado quando o Master pede dados (request)
void onRequest() {
  Wire.slaveWrite((uint8_t*)i2cBuffer, strlen(i2cBuffer));
}

// Chamado quando o Master envia dados (receive)
void onReceive(int len) {
  Serial.printf("Master enviou %d bytes: ", len);
  while (Wire.available()) {
    int cmd = Wire.read();

    if(cmd == 1){
      digitalWrite(LED_PIN, HIGH); // Acende LED
    } else if(cmd == 0){
      digitalWrite(LED_PIN, LOW);  // Apaga LED
    }

    Serial.print(cmd);
    Serial.print(" ");
  }
  Serial.println();
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  // Inicializa sensor DHT
  sensorDHT.begin();

  // Configura LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Inicializa I2C como Slave
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin(I2C_DEV_ADDR);

  Serial.println("ESP32 pronto como I2C Slave!");
}

// -------------------- LOOP PRINCIPAL --------------------
void loop() {
  // Lê temperatura e umidade do DHT22
  float temperature = sensorDHT.readTemperature();
  float humidity = sensorDHT.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erro ao ler DHT22");
    delay(1000);
    return;
  }

  // Formata dados para enviar ao Master
  snprintf(i2cBuffer, sizeof(i2cBuffer), "%.2f;%.2f", temperature, humidity);

  Serial.print("Atualizado buffer I2C -> ");
  Serial.println(i2cBuffer);

  delay(2000); // Atualiza a cada 2 segundos
}

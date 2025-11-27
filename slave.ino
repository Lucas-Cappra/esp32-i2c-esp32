#include <Wire.h>
#include "SensorDHT.h"

#define DHTPIN 4
#define DHTTYPE DHT22

#define I2C_DEV_ADDR 0x55   // endereço do ESP32 como SLAVE

SensorDHT sensorDHT(DHTPIN, DHTTYPE);

// Buffer que enviaremos ao Master
char i2cBuffer[32];

// Quando o Master pedir dados (I2C request)
void onRequest() {
  Wire.slaveWrite((uint8_t*)i2cBuffer, strlen(i2cBuffer));
}

// Quando o Master enviar algo para o Slave (opcional)
void onReceive(int len) {
  Serial.printf("Master enviou %d bytes: ", len);
  while (Wire.available()) {
    Serial.write(Wire.read());
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  sensorDHT.begin();

  // Inicializa como I2C Slave
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin((uint8_t)I2C_DEV_ADDR);

  Serial.println("ESP32 pronto como I2C Slave!");
}

void loop() {
  float temperature = sensorDHT.readTemperature();
  float humidity = sensorDHT.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erro ao ler DHT22");
    delay(1000);
    return;
  }

  // Formata a resposta que será enviada ao Master
  snprintf(i2cBuffer, sizeof(i2cBuffer), "%.2f;%.2f", temperature, humidity);

  Serial.print("Atualizado buffer I2C -> ");
  Serial.println(i2cBuffer);

  delay(2000); // Atualiza a cada 2s
}

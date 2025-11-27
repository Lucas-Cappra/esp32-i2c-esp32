#include <Wire.h>

#define SLAVE_ADDR 0x55   // endereço do ESP32 Slave
#define BUFFER_SIZE 32

void setup() {
  Serial.begin(115200);

  Wire.begin();   // Master I2C (SDA/SCL padrão)

  Serial.println("Master I2C iniciado!");
}

void loop() {

  // --- Solicita dados ao Slave ---
  Wire.requestFrom(SLAVE_ADDR, BUFFER_SIZE);

  char buffer[BUFFER_SIZE + 1];
  int idx = 0;

  while (Wire.available() && idx < BUFFER_SIZE) {
    buffer[idx++] = Wire.read();
  }
  buffer[idx] = '\0'; // finaliza string

  if (idx > 0) {
    Serial.print("Recebido do SLAVE: ");
    Serial.println(buffer);

    // --- Separar temperatura e umidade ---
    float temperature = 0;
    float humidity = 0;

    sscanf(buffer, "%f;%f", &temperature, &humidity);

    Serial.print("Temperatura: ");
    Serial.println(temperature);

    Serial.print("Umidade: ");
    Serial.println(humidity);
  } else {
    Serial.println("Nenhum dado recebido do Slave.");
  }

  delay(1500); // tempo entre leituras
}

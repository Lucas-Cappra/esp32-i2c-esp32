#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define SLAVE_ADDR 0x55   // endereço do ESP32 Slave
#define BUFFER_SIZE 32

// --- WiFi ---
const char* ssid = "AG";
const char* wifi_password = "qwerty12345";

// --- MQTT (use EXACT credentials that worked!) ---
const char* mqtt_server   = "acfff78be59c4667b190a6126d3f2973.s1.eu.hivemq.cloud";
const int   mqtt_port     = 8883;
const char* mqtt_user     = "qwerty";
const char* mqtt_password = "@Qwerty1";
const char* topic         = "dht22";

// --- TLS Client ---
WiFiClientSecure espClient;
PubSubClient client(espClient);


unsigned long lastMsg = 0;

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi connected!");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT... ");

    // Use a UNIQUE client ID!
    if (client.connect("ESP32Client123", mqtt_user, mqtt_password)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin();   // Master I2C (SDA/SCL padrão)

  Serial.println("Master I2C iniciado!");

  connectWiFi();

  espClient.setInsecure();   // required for Wokwi
  client.setServer(mqtt_server, mqtt_port);

  connectMQTT();
}

void loop() {
  
    float temperature = 0;
    float humidity = 0;

  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  if (millis() - lastMsg > 2000) {  // publish every 2 seconds
    lastMsg = millis();
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


    sscanf(buffer, "%f;%f", &temperature, &humidity);

    Serial.print("Temperatura: ");
    Serial.println(temperature);

    Serial.print("Umidade: ");
    Serial.println(humidity);

    char msg[80];
    snprintf(msg, sizeof(msg), "{\"temp\": %.1f, \"humidity\": %.1f}", temperature, humidity);

    if (client.publish(topic, msg)) {
      Serial.print("Published: ");
      Serial.println(msg);
    } else {
      Serial.println("Publish failed");
    }

    } else {
    Serial.println("Nenhum dado recebido do Slave.");
    }

  delay(1500); // tempo entre leituras
  }

}

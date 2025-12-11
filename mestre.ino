#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define SLAVE_ADDR 0x55   // endereço do ESP32 Slave
#define BUFFER_SIZE 32


int info = 0;

// --- WiFi ---
const char* ssid = "AG";
const char* wifi_password = "qwerty12345";

// --- MQTT (use EXACT credentials that worked!) ---
const char* mqtt_server   = "acfff78be59c4667b190a6126d3f2973.s1.eu.hivemq.cloud";
const int   mqtt_port     = 8883;
const char* mqtt_user     = "qwerty";
const char* mqtt_password = "@Qwerty1";
const char* topic         = "dht22";
const char* topic1         = "slider";

// --- TLS Client ---
WiFiClientSecure espClient;
PubSubClient client(espClient);


void callback(char* topic1, byte* payload, unsigned int length) {
  // Convert payload to string for easier handling
  payload[length] = '\0';
  String message = (char*)payload;      

  // Handle messages based on topic
  if (strcmp(topic1, "slider") == 0) {
    if (message == "on") {
      Serial.println("slider on!");
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(1);
      Wire.endTransmission();
      // Turn on bedroom lights
    } else if (message == "off") {
      Serial.println("slider off!");
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(0);
      Wire.endTransmission();
      // Turn off bedroom lights
    }
  } else if (strcmp(topic1, "slider") == 0) {
    // Publish current temperature
  }
}



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
  //callback();
  Serial.println("Master I2C iniciado!");

  connectWiFi();

  espClient.setInsecure();   // required for Wokwi
  client.setServer(mqtt_server, mqtt_port);

  connectMQTT();

  client.setCallback(callback);
  client.subscribe("slider");
}

void loop() {
    float temperature = 34;
    float humidity = 15;
// ==================== MQTT ========================== //
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  if (millis() - lastMsg > 2000) {  // publish every 2 seconds
    lastMsg = millis();
    // ================= Solicita dados ao Slave ================= // 
    Wire.requestFrom(SLAVE_ADDR, BUFFER_SIZE);

    char buffer[BUFFER_SIZE + 1];
    int idx = 0;

    while (Wire.available() && idx < BUFFER_SIZE) {
    buffer[idx++] = Wire.read();
    }
  buffer[idx] = '\0'; // finaliza string

    // ================= Reconhece dados ao Slave ================= // 
    if (idx > 0) {
      Serial.print("Recebido do SLAVE: ");


      //sscanf(buffer, "%f;%f", &temperature, &humidity); // transforma dados em variaveis

      //Serial.print("Temperatura: ");
      //Serial.println(temperature);

      //Serial.print("Umidade: ");
      //Serial.println(humidity);

      char msg[80];
      snprintf(msg, sizeof(msg), "{\"temp\": %.1f, \"humidity\": %.1f}", temperature, humidity); // transforma variaveis na msg

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

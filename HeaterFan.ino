#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

const char* topic = "HeaterFan/Power";
const int fan_pin = D2; // D2 on Wemos D1 Mini

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";
  for (int i=0;i<length;i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  int power = message.toInt();
  if (power >= 0 && power <= 100) {
    int duty_cycle = power * 10.23; // Map power value to duty cycle (0-1023)
    analogWrite(fan_pin, duty_cycle);
    Serial.print("Fan power set to ");
    Serial.print(power);
    Serial.println("%");

    // Blink LED once
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("Invalid power value");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  analogWriteFreq(25000); // Set PWM frequency to 25kHz
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(fan_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Initialize built-in LED pin
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

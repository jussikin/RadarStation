#include <Arduino.h>
#include <MLEDScroll.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <wlanconfig.h>

#define SENSEPIN D0
// Update these with values suitable for your network.

const char* ssid = WLANSERVER;
const char* password = WLANPASSWORD;
const char* mqtt_server = MQTTSERVER;

MLEDScroll matrix;
WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
}

void callback(char* topic, byte* payload, unsigned int length) {
  matrix.message("msg");
  matrix.display();
}

void setup() {
  matrix.begin();
  pinMode(SENSEPIN,INPUT_PULLDOWN_16);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    String clientId = CLIENTID;
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


int lowDetected = 0;
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int val = digitalRead(SENSEPIN);
  if(val==0){
    matrix.dot(0, 0, 1);
    matrix.dot(0, 1, 0);
    matrix.dot(0, 2, 1);
  }else{
    lowDetected = 1;
    matrix.dot(0, 0, 0);
    matrix.dot(0, 1, 1);
   
  }
  matrix.display();
  delay(100);
}
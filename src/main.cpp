#include <Arduino.h>
#include <MLEDScroll.h>
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include <wlanconfig.h>

#define SENSEPIN D0
#define EYEPIN D2
#define STRIPPIN D1
// Update these with values suitable for your network.

const char* ssid = WLANSERVER;
const char* password = WLANPASSWORD;
const char* mqtt_server = MQTTSERVER;

MLEDScroll matrix;
WiFiClient espClient;
Adafruit_NeoPixel eyes(2, EYEPIN, NEO_RBG | NEO_KHZ400);
Adafruit_NeoPixel strip(32, STRIPPIN, NEO_BGR | NEO_KHZ800);
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
  pinMode(EYEPIN,OUTPUT);
  pinMode(STRIPPIN,OUTPUT);
  eyes.begin();
  strip.begin();
  
   eyes.setPixelColor(0, eyes.Color(10, 0,0));
   eyes.setPixelColor(1, eyes.Color(10,0 , 0));
   eyes.show();
  pinMode(SENSEPIN,INPUT_PULLDOWN_16);
  matrix.clear();
  matrix.character("S");
  matrix.display();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect() {
  matrix.clear();
  matrix.character("C");
  matrix.display();
  // Loop until we're reconnected
  while (!client.connected()) {
    String clientId = CLIENTID;
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.publish(MQTOPIC, "ready");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  matrix.clear();

}

int animation = 0;
int lowDetected = 0;
int strippos= 0;
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int val = digitalRead(SENSEPIN);
  if(val==0){
    eyes.setPixelColor(0, eyes.Color(0,255,0));
    eyes.setPixelColor(1, eyes.Color(0,255,0));
    matrix.dot(0, 0, 1);
    matrix.dot(0, 1, 0);
    matrix.dot(0, 2, 1);
  }else{
    eyes.setPixelColor(0, eyes.Color(0,0,255));
    eyes.setPixelColor(1, eyes.Color(0,0,255));
    if(animation==0)
      animation = 300;
  }
  if(lowDetected!=val){
    if(val==1)
      client.publish(MQTOPIC, "move");
    if(val==0)
      client.publish(MQTOPIC,"stop");
  }
  lowDetected=val;
  matrix.clear();
  if(animation>0){
    int phase = animation%4;
    if(phase==3){
      matrix.rect(3,3,1,1,1);
    }
    if(phase==2){
      matrix.rect(2,2,2,2,1);
    }
    if(phase==1){
      matrix.rect(1,1,4,4,1);
    }
    if(phase==0){
      matrix.rect(0,0,7,7,1);
    }
    animation--;
  }
  strippos++;
  if(strippos==32)
    strippos=0;
  strip.clear();
  if(val==1){
    strip.setPixelColor(strippos,strip.Color(0,255,0));
    strip.setPixelColor(strippos+1,strip.Color(0,255,0));
  } else {
    strip.setPixelColor(strippos,strip.Color(0,0,255));
    strip.setPixelColor(strippos+1,strip.Color(0,0,255));
  }   
  matrix.display();
  delay(100);
  eyes.show();
  strip.show();
}
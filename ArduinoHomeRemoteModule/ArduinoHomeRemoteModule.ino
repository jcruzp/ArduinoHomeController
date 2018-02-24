// Arduino Home Remote Module Wifi
// v1.0
//
// Copyright (C)2018 Jose Cruz. All right reserved
// web: https://sites.google.com/view/raeiot/home
//
#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <string.h>


//#define isDEBUG

// Your Amazon email linked skill
#define MyID "YOUR AMAZON EMAIL"

const char *ssid = "YOUR WIFI SSID"; // Wifi SSID
const char *pass = "YOUR WIFI PASSWORD";   // Wifi password
const char* clientId = "pub-c-e93def7f-95aa-475a-aa60-cc3cd32ee8a7/sub-c-ec04dbbc-0893-11e8-8e75-cea83f8405bb/e73766bf-3d70-4280-8395-001a08ebeb72";

//PubNub server MQTT
const int mqtt_port = 1883;
const char* mqtt_server = "mqtt.pndsn.com";
// Suscribe topic
const char* topic = "AHC_IOT_01";

WiFiClient wclient; 
PubSubClient client(wclient, mqtt_server, mqtt_port);

//Define gpio pins from WemosD1
#define room_light D6   

// Callback to process all msgs received
void callback(const MQTT::Publish &pub)
{
  String msg = "";   // All messages from MQTT
  msg = pub.payload_string();
  msg.trim();
#ifdef isDEBUG
  Serial.println(msg);
  Serial.println(pub.topic());
#endif
  // If is my ID and Topic_room detect command On or Off
  if ((msg.indexOf(MyID) > 0) && (msg.indexOf("\"topic\":\"Topic_room\"") > 0)) {
    digitalWrite(room_light, msg.indexOf("\"command\":\"on\"") > 0 ? HIGH : LOW);
  }
}

//Reconnect to MQTT broker if lost connection
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
#ifdef isDEBUG    
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
#endif    
    if (client.connect(clientId)) {
#ifdef isDEBUG       
      Serial.println("MQTT broker connected");
#endif      
      client.subscribe(topic);
#ifdef isDEBUG       
      Serial.println("Suscribe to topic at MQTT broker Ok ...");
#endif
    } else {
#ifdef isDEBUG            
      Serial.println(" Failed connect, try again in 5 seconds");
#endif      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup()
{
  delay(2000);
#ifdef isDEBUG      
  Serial.begin(115200);
  Serial.println("Init Arduino Home Remote Module v1...");
#endif

  pinMode(room_light, OUTPUT);
  digitalWrite(room_light, LOW);

  client.set_callback(callback);

  if (WiFi.status() != WL_CONNECTED)
  {
#ifdef isDEBUG
    Serial.print("Connecting to ");
    Serial.println(ssid);
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    //if (WiFi.waitForConnectResult() != WL_CONNECTED)
    //  return;
#ifdef isDEBUG
    Serial.println("WiFi connected");
#endif
  }
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


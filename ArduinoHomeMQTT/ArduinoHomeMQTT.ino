//
// ArduinoHomeMQTT
// v2.0
//
// Copyright (C)2018 Jose Cruz. All right reserved
// web: https://sites.google.com/view/raeiot/home
//
#include <YunClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Process.h>

// Your Amazon email linked skill
#define MyID "YOUR AMAZON EMAIL LINKED IN ALEXA APP"

// DHT11 sensor data pin
#define DHTPIN 2
// DHT 11 sensor type
#define DHTTYPE DHT11
// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Alarm control pin
#define alarm 8

// Light control pins
#define room_light 9
#define kitchen_light 10
#define garage_light 11
#define livingroom_light 12

// PubNub MQTT Server address and port
int mqtt_server_port = 1883;
const char* mqtt_server = "mqtt.pndsn.com";
// Suscribe topic
const char* topic = "AHC_IOT_01";
// Publish topic
const char* topic2 = "AHC_IOT_02";

// PubNub publisher ID
//  pub-c-e93def7f-95aa-475a-aa60-cc3cd32ee8a7/
// PubNub suscriber ID
//  sub-c-ec04dbbc-0893-11e8-8e75-cea83f8405bb/
// UUID generated onlyne https://www.uuidgenerator.net/
//  
// PubNub Client ID
//  clientId = pubID + subID + uuid;
const char* clientId = "pub-c-e93def7f-95aa-475a-aa60-cc3cd32ee8a7/sub-c-ec04dbbc-0893-11e8-8e75-cea83f8405bb/YOUR UUID GENERATED";


// Define de Yun ethernet client
YunClient ethClient;
// Define PubNub pub and sub client
PubSubClient client(ethClient);

// Picture process
Process picture;

// Filename
String filename;
// Path for image file
String path = "/mnt/sda1/";

// Create an image file and sent by email
void TakePhoto() {
  // Generate filename with timestamp
  filename = "";
  picture.runShellCommand("date +%s");
  while (picture.running());

  while (picture.available() > 0) {
    char c = picture.read();
    filename += c;
  }
  filename.trim();
  filename += ".jpg";

  // Take picture
  picture.runShellCommand("/usr/bin/fswebcam  -i 0 --jpeg 95 --no-banner --fps 1 -S 1 -r 352x288 --save " + path + filename);
  while (picture.running());

  //Send it by email
  picture.runShellCommand("python /mnt/sda1/AHC_SendEmail.py " + path + " " + filename);
  while (picture.running());
}

//***********************
// Send temperature
//***********************
void SendTemperature()
{
  char cstr[16];
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  //  Serial.print("Temperature: ");
  //  Serial.print(t);
  //  Serial.println(" *C");

  client.publish(topic2, itoa(t, cstr, 10));
  //Serial.print("Temperature sent...");
  //Serial.println(cstr);
}

//***********************
// Send humidity
//***********************
void SendHumidity()
{
  char cstr[16];
  float h = dht.readHumidity();
  //Serial.print("Humidity: ");
  //Serial.print((int)h);
  //Serial.print(" %\t");

  client.publish(topic2, itoa(h, cstr, 10));
  //Serial.print("Humidity sent...");
  //Serial.println(cstr);
}

//Callback function for msg receive handle
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((char[])payload);

  String alexa_topic  = root["topic"];
  String alexa_command = root["command"];
  String alexa_id = root["id"];

  //Serial.println(alexa_topic);
  //Serial.println(alexa_command);

  //if (alexa_id.equals(MyID)) {
  //Handle all received msgs topic from MQTT
  if (alexa_topic.endsWith("Topic_room")) {
    digitalWrite(room_light, (alexa_command.endsWith("on") ? HIGH : LOW));
  } else if (alexa_topic.endsWith("Topic_kitchen")) {
    digitalWrite(kitchen_light, (alexa_command.endsWith("on") ? HIGH : LOW));
  } else if (alexa_topic.endsWith("Topic_garage")) {
    digitalWrite(garage_light, (alexa_command.endsWith("on") ? HIGH : LOW));
  } else if (alexa_topic.endsWith("Topic_living room")) {
    digitalWrite(livingroom_light, (alexa_command.endsWith("on") ? HIGH : LOW));
  } else if (alexa_topic.endsWith("Topic_temperature")) {
    SendTemperature();
  } else if (alexa_topic.endsWith("Topic_humidity")) {
    SendHumidity();
  } else if (alexa_topic.endsWith("Topic_alarm")) {
    digitalWrite(alarm, (alexa_command.endsWith("on") ? HIGH : LOW));
  } else if (alexa_topic.endsWith("Topic_photo")) {
    TakePhoto();
  }
  //}
}

//Reconnect to MQTT broker if lost connection
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("MQTT broker connected");
      client.subscribe(topic);
    } else {
      Serial.print("Failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Bridge.begin();
  delay(2000);
  
  Serial.begin(115200);
  Serial.println("Init Arduino Home Controller v2.0...");
  pinMode(alarm, OUTPUT);
  pinMode(room_light, OUTPUT);
  pinMode(kitchen_light, OUTPUT);
  pinMode(garage_light, OUTPUT);
  pinMode(livingroom_light, OUTPUT);

  // Define PubNub MQTT broker
  client.setServer(mqtt_server, mqtt_server_port);
  client.setCallback(callback);
  client.connect(clientId);
  Serial.println("Connected to PubNub MQTT broker OK ...");
  client.subscribe(topic);
  Serial.println("Suscribe to topic at MQTT broker Ok ...");

  dht.begin();
  //Wait for sensor initialize
  delay(4000);
  Serial.println("DHT sensor Ok ...");
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


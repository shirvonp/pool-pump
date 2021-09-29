#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <TinyMPU6050.h>


#ifndef STASSID
#define STASSID " "              // Wifi SSID
#define STAPSK  " "              // Password
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* mqtt_server = "";    //MQTT broker address

WiFiClient espClient;
PubSubClient client(espClient);

MPU6050 mpu (Wire);


const char* data = "";
String dataString =""; // holds the data to be written to the SD card
String MQTTString =""; 

float AccX = 0.000; 
float AccY = 0.000; 
float AccZ = 0.000; 
float GyX = 0.000; 
float GyY = 0.000; 
float GyZ = 0.000; 

File sensorData;

String filename;
String suffix;

int num_samples = 50;  
int num_files = 1;
long start_time;
long end_time;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

if (strcmp(topic,"MPU6050/SendSample")==0){
   if ((char)payload[0] == '1') {
      sendSample(); 
      client.publish("MPU6050/SendSample","0");  
    } 
   }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "MPU6050";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("MPU6050", "Pool Pump MPU6050 Online");
      // ... and resubscribe
      client.subscribe("MPU6050/SendSample");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(8000);
  
  mpu.Initialize();
//  Serial.println("Starting calibration...");
//  mpu.Calibrate();
//  Serial.println("Calibration complete!");
 


  ArduinoOTA.setHostname("MPU6050 Logger");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

String get_data(){
  mpu.Execute();  
  AccX = mpu.GetAccX(); 
  AccY = mpu.GetAccY(); 
  AccZ = mpu.GetAccZ();
  GyX = mpu.GetGyroX();
  GyY = mpu.GetGyroY();
  GyZ = mpu.GetGyroZ(); 
  dataString = String(AccX) + "," + String(AccY) + "," + String(AccZ) + "," + String(GyX) + "," + String(GyY) + "," + String(GyZ);
  //Serial.println(dataString);
  return dataString;  
  }


void sendSample(){
    for (int i = 0; i < num_samples; i++){
      dataString = get_data();
      MQTTString += dataString + ";";
    }
  data = (char*) MQTTString.c_str();
  client.publish("MPU6050/Sample",data);
  }


void loop() {
  ArduinoOTA.handle();
  if (!client.connected()) {
  reconnect();
  }
  client.loop();
  
}

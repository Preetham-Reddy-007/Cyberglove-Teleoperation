#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

/* WiFi network name and password */
const char * ssid = "SSID";
const char * pwd = "Pasword";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.51.115";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "broker.emqx.io";

// Potentiometer is connected to GPIO 36,39,34,32 
int potPin[4] = {36,39,34,32};

// variable for storing the potentiometer value
int potValue[20][4]  = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

void setup() {
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  WiFi.mode(WIFI_STA);   
  
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pwd); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
// Reading potentiometer value
  int i = 0;      
  while (i<20){
    int j = 0;
    while (j<4) {
      potValue[i][j] = analogRead(potPin[j]);
      j++;
      }
    i++;
  }
      
  float avgX = 0, avgY = 0, avgZ = 0, avgG = 0;
  i = 0;
  float sumG = 0;
  while (i<20){
    sumG += potValue[i][0];
    i++;
  }
  avgG = round(sumG/20);

  i = 0;
  float sumX = 0;
  while (i<20){
    sumX += potValue[i][1];
    i++;
  }
  avgX = round(sumX/20);

  i = 0;
  float sumY = 0;
  while (i<20){
    sumY += potValue[i][2];
    i++;
  }
  avgY = round(sumY/20);

  i = 0;
  float sumZ = 0;
  while (i<20){
    sumZ += potValue[i][3];
    i++;
  }
  avgZ = round(sumZ/20);

int g=0;
  if (avgG < 975){ 
    g = 1;
  }
  else if (avgG > 2780){
    g = 127;
  }
  else{
  g = round((127 * (avgG - 975))/(2780 -975));
  }
  
  int x = 0;
  if (avgX < 912){ 
    x = 1;
  }
  else if (avgX > 2670){
    x = 127;
  }
  else{
  x = round((127 * (avgX - 912))/(2670 -912));
  }

  int y = 0;
  if (avgY < 844){ 
    y = 1;
  }
  else if (avgY > 2484){
    y = 127;
  }
  else{
  y = round((127 * (avgY - 844))/(2484 - 844));
  }

  int z = 0;
  if (avgZ < 1100){ 
    z = 1;
  }
  else if (avgZ > 2750){
    z = 127;
  }
  else{
  z = round((127 * (avgZ - 1100))/(2750 - 1100));
  }
  
 String p1 = ";";
 Serial.println();
 Serial.println(x+ p1 + y + p1 + z + p1 + g);

 char buffer[4] = {x,y,z,g};   //the ASCII of the integer will be stored in this char array   
 
 boolean rc = client.publish("ESP32/Potentiometerdataread", buffer);
 Serial.println();
 Serial.print("  ");
 Serial.print("rc = ");
 Serial.print(rc);
 Serial.println();
 Serial.print("  ");
 Serial.print("Character buffer= ");
 Serial.print(buffer[0] + p1 + buffer[1] + p1 + buffer[2] + p1 + buffer[3]);
    
 delay(5000); 
}
}

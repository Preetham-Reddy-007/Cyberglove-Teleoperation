#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdlib.h>
#include <math.h>
#include <PubSubClient.h>
#include <Wire.h>

const char * ssid = "Galaxy M01s3238";
const char * pwd = "boem0957";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char messageTemp[4];
int x = 0;
int y = 0;
int z = 0;
int g = 0;
int d = 0;

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.111.115";
const char* mqtt_server = "broker.mqtt-dashboard.com";

// IP address to send UDP data to.
// it can be ip address of the server or 
// a network broadcast address
// here is broadcast address
const char * udpAddress = "192.168.111.115";
const int udpPort = 44444;

//create UDP instance
WiFiUDP udp;

void setup() {
  Serial.begin(115200);      // Initialize serial 
  while (!Serial) {
    ;                       // wait for serial port to connect. 
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
    Serial.println("\nConnected");
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println();
  Serial.print("Length of Payload: ");
  Serial.print(length);
  
  for (int i = 0; i < length; i++){
    Serial.println();
    Serial.print("Message: ");
    Serial.print(message[i]);
  }
  x = message[0];
  y = message[1];
  z = message[2];
  g = message[3];
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32SubClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("ESP32/Potentiometerdataread");
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
  
  String p1 =";";
  Serial.println();
  Serial.print("(x,y,z,g): ");
  Serial.print(x + p1 + y + p1 + z + p1 + g);
  Serial.println();
   
  if (g<=63) { 
      d = 1;
    Serial.print("d: "); 
    Serial.print(d);
    } else {
      d = 0;
      Serial.print("d: ");
      Serial.print(d);
    }
  Serial.println();
  Serial.print("(x,y,z,d): ");
  Serial.print(x + p1 + y + p1 + z + p1 + d);
  Serial.println();
  
  //data will be sent to server
  uint8_t buffer[50]= {x,y,z,d};
  //This initializes udp and transfer buffer
  udp.beginPacket(udpAddress, udpPort);
  udp.write(buffer, 4);
  udp.endPacket();
  memset(buffer, 0, 50);
  //processing incoming packet, must be called before reading the buffer
  udp.parsePacket();
  //receive response from server, 
  if(udp.read(buffer, 50) > 0){
    Serial.print("Server to client: ");
    Serial.println((char *)buffer);
  }
  
  delay(3000);      
}
  
  
  

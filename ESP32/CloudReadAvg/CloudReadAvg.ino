#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdlib.h>
#include <math.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

const char * ssid = "Galaxy M01s3238";
const char * pwd = "boem0957";
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

// IP address to send UDP data to.
// it can be ip address of the server or 
// a network broadcast address
// here is broadcast address
const char * udpAddress = "192.168.116.115";
const int udpPort = 44444;

//Channel details
unsigned long ChannelNumber = 1757700;
const char * myReadAPIKey = "5GUQ1ZVQNBD50U7Y";

int statusCode = 0;

//create UDP instance
WiFiUDP udp;

void setup() {
  Serial.begin(115200);      // Initialize serial 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  WiFi.mode(WIFI_STA);
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

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
    
void loop() {
    int thumb = ThingSpeak.readIntField(ChannelNumber,1, myReadAPIKey);
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Thumb: " + String(thumb));
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  delay(500); // No need to read the counter too often.
  
  int xc = ThingSpeak.readIntField(ChannelNumber,2, myReadAPIKey); 
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("X: " + String(xc));
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  delay(500); // No need to read the counter too often.
  
  int yc = ThingSpeak.readIntField(ChannelNumber,3, myReadAPIKey); 
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Y: " + String(yc));
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  delay(500); // No need to read the counter too often.
  
  int zc = ThingSpeak.readIntField(ChannelNumber,4, myReadAPIKey); 
   // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Z: " + String(zc));
  }
  else{
    Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); 
  }
  
  delay(500); // No need to read the counter too often.      

  String p1=";";
  Serial.println(thumb + p1 + xc + p1 + yc + p1 + zc);
  
int g = 0;
  if (thumb < 975){ 
    g = 0;
  }
  else if (thumb > 2780){
    g = 127;
  }
  else{
  g = round((127 * (thumb - 975))/(2780 -975));
  }
  
  int x = 0;
  if (xc < 912){ 
    x = 0;
  }
  else if (xc > 2670){
    x = 127;
  }
  else{
  x = round((127 * (xc - 912))/(2670 -912));
  }

  int y = 0;
  if (yc < 844){ 
    y = 0;
  }
  else if (yc > 2484){
    y = 127;
  }
  else{
  y = round((127 * (yc - 844))/(2484 - 844));
  }

  int z = 0;
  if (zc < 1100){ 
    z = 0;
  }
  else if (zc > 2750){
    z = 127;
  }
  else{
  z = round((127 * (zc - 1100))/(2750 - 1100));
  }
 
  Serial.println(x+ p1 + y + p1 + z + p1 + g);

  int d = 0;
  if (g<=63) { 
      d = 1;
    Serial.println(d);
    } else {
      d = 0;
      Serial.println(d);
    }

  //data will be sent to server
  uint8_t buffer[50]= {x,y,z,d};
  //This initializes udp and transfer buffer
  udp.beginPacket(udpAddress, udpPort);
  udp.write(buffer, 4);
  udp.endPacket();
  memset(buffer, 0, 50);
  //processing incoming packet, must be called before reading the buffer
  udp.parsePacket();
  //receive response from server, it will be HELLO WORLD
  if(udp.read(buffer, 50) > 0){
    Serial.print("Server to client: ");
    Serial.println((char *)buffer);
  }
  //Wait for 1 second
  delay(1000);      
}
  
  
  

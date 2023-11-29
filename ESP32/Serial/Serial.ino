#include <WiFi.h>
#include <WiFiUdp.h>
#include <stdlib.h>
#include <math.h>

/* WiFi network name and password */
const char * ssid = "Galaxy M01s3238";
const char * pwd = "boem0957";

// IP address to send UDP data to.
// it can be ip address of the server or 
// a network broadcast address
// here is broadcast address
const char * udpAddress = "192.168.26.115";
const int udpPort = 44444;

// Potentiometer is connected to GPIO 36,39,34,32 
int potPin[4] = {36,39,34,32};

// variable for storing the potentiometer value
int potValue[20][4]  = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

//create UDP instance
WiFiUDP udp;

void setup(){
  Serial.begin(115200);
  delay(1000);
  
  //Connect to the WiFi network
   WiFi.begin(ssid, pwd);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(){

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

  int g = 0;
  if (avgG < 975){ 
    g = 0;
  }
  else if (avgG > 2780){
    g = 127;
  }
  else{
  g = round((127 * (avgG - 975))/(2780 -975));
  }

  int x = 0;
  if (avgX < 912){ 
    x = 0;
  }
  else if (avgX > 2670){
    x = 127;
  }
  else{
  x = round((127 * (avgX - 912))/(2670 -912));
  }

  int y = 0;
  if (avgY < 844){ 
    y = 0;
  }
  else if (avgY > 2484){
    y = 127;
  }
  else{
  y = round((127 * (avgY - 844))/(2484 - 844));
  }

  int z = 0;
  if (avgZ < 1100){ 
    z = 0;
  }
  else if (avgZ > 2750){
    z = 127;
  }
  else{
  z = round((127 * (avgZ - 1100))/(2750 - 1100));
  }

  String p1=";";
  int d = 0;
  
  Serial.println(avgX + p1 + avgY + p1 + avgZ + p1 + avgG);
  Serial.println(x + p1 + y + p1 + z + p1 + g);
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
  delay(5000);
}

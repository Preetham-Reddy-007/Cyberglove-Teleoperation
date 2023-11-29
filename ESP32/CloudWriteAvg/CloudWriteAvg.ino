
#include <WiFi.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

/* WiFi network name and password */
const char * ssid = "Galaxy M01s3238";
const char * pwd = "boem0957";
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = 1;

const char * myWriteAPIKey = "FS1XGM9WEKUVR66R";

// Potentiometer is connected to GPIO 36,39,34,32 
int potPin[4] = {36,39,34,32};

// variable for storing the potentiometer value
int potValue[20][4]  = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

void setup() {
  Serial.begin(115200);  //Initialize serial
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
    Serial.println("\nConnected.");
  }
}

void loop() {

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
  // set the fields with the values
  ThingSpeak.setField(1, avgG);
  ThingSpeak.setField(2, avgX);
  ThingSpeak.setField(3, avgY);
  ThingSpeak.setField(4, avgZ);


  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeFields(myChannelNumber,myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    Serial.println("Thumb :"+ String(avgG));
    Serial.println("X :"+ String(avgX));
    Serial.println("Y :"+ String(avgY));
    Serial.println("Z :"+ String(avgZ));
    
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  delay(1000); // Wait 20 seconds to update the channel again
}

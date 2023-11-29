#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>


/* WiFi network name and password */
const char * ssid = "zenlab";
const char * pwd = "zenlab123";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "192.168.227.115";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
//const char* mqtt_server = "broker.emqx.io";

// Potentiometer is connected to GPIO 36,39,34,32 
int potPin[4] = {36,39,34,32};

// variable for storing the potentiometer value
int potValue[20][4];
int charValue[20][4];
char char_array[80]; //the ASCII of the integer will be stored in this char array   

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

//Callback Function
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
    
// Reading potentiometer value
Serial.println("START");
  int i = 0;      
  while (i<20){
    int j = 0;
    while (j<4) {
      potValue[i][j] = analogRead(potPin[j]);
      j++;
      }
    delay(250);
    i++;
  }
//Converting potentiometer value to ASCII character  
i = 0;
while (i<20){
int  j = 0;
  while (j<4){
    if (j == 0){
       if (potValue[i][j] < 975){
         charValue[i][j] = 1;
         j++;
       }
       else if (potValue[i][j] > 2780){
         charValue[i][j] = 127;
         j++;
       }
       else{
         charValue[i][j] = round((127 * (potValue[i][j] - 975))/(2780 -975))+1;
         j++;
       }
     }
     
     else if (j == 1){
       if (potValue[i][j] < 912){
         charValue[i][j] = 1;
         j++;
       }
       else if (potValue[i][j] > 2670){
         charValue[i][j] = 127;
         j++;
       }
       else{
         charValue[i][j] = round((127 * (potValue[i][j] - 912))/(2670 -912))+1;
         j++;
      }
     }
     
      else if (j == 2){
       if (potValue[i][j] < 844){
         charValue[i][j] = 1;
         j++;
       }
       else if (potValue[i][j] > 2484){
         charValue[i][j] = 127;
         j++;
       }
       else{
         charValue[i][j] = round((127 * (potValue[i][j] - 844))/(2484 - 844))+1;
         j++;
       }
      }
      
       else if (j == 3){
       if (potValue[i][j] < 1100){
         charValue[i][j] = 1;
         j++;
       }
       else if (potValue[i][j] > 2750){
         charValue[i][j] = 127;
         j++;
       }
       else{
         charValue[i][j] = round((127 * (potValue[i][j] - 1100))/(2750 - 1100))+1;
         j++;
       } 
      }
     }
      i++;
    }
//Loading the buffer
 int k = 0;
 i = 0;
 while (i<20){
 int j = 0; 
  while (j<4 && k<80){
   char_array[k] = charValue[i][j];
   j++;
   k++;
  }
  i++;
 }
    
 String p1 = ";";
 Serial.println();

 boolean rc = client.publish("ESP32/Potentiometerdataread", char_array);
 Serial.println();
 Serial.print("  ");
 Serial.print("rc = ");
 Serial.print(rc);
 Serial.println();
 Serial.print("  ");
 Serial.print("Character buffer= ");
 
 i = 0;
 while (i<80){
  Serial.print(char_array[i]);
  Serial.print(p1);
  i++;
 }

 Serial.println();
 Serial.print("  ");

 i=0;
 while (i<20){
  int j = 0;
  while (j<4){
    Serial.print(potValue[i][j]);
    Serial.print("  ");
    j++;
  }
  Serial.println();
  i++;
}


 i=0;
 while (i<20){
  int j = 0;
  while (j<4){
    Serial.print(charValue[i][j]);
    Serial.print("  ");
    j++;
  }
  Serial.println();
  i++;
}
Serial.print("Delay of 2s");
 delay(2000); 
}

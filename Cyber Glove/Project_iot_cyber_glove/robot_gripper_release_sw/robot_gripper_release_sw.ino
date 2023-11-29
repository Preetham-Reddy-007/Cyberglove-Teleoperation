
#include <WiFi.h> // ESP32 WiFi Library
#include <WebServer.h> // WebServer Library for ESP32
#include <ESP32Servo.h>
#include <WebSocketsClient.h> // WebSocket Client Library for WebSocket
#include <ArduinoJson.h> // Arduino JSON Library

const uint8_t ind_sense_pin = 34;
const uint8_t thumb_sense_pin = 35;

const uint8_t ind_servo_pin = 26;
const uint8_t thb_servo_pin = 25;
const uint8_t knkl_pitch_servo_pin=32;
const uint8_t knkl_roll_servo_pin=33;

const uint8_t ind_open_ang=0;
const uint8_t ind_close_ang=70;
const uint8_t thb_open_ang=85;
const uint8_t thb_close_ang=0;
const uint8_t knkl_roll_min = 0;
const uint8_t knkl_roll_max = 180;
const uint8_t knkl_pitch_min = 0;
const uint8_t knkl_pitch_mid = 90;
const uint8_t knkl_pitch_max = 180;

float y_ind_sense[3];
float y_thb_sense[3];

int x_ind_sense[3];
int x_thb_sense[3];

const uint16_t tf_thld=1000;
const uint16_t ff_thld=2500;

const uint8_t glove_open_cal=20;
const uint8_t glove_pinch_cal=120;
const int16_t glove_roll_cal_low=0;
const int16_t glove_roll_cal_high=-180;
const int16_t glove_pitch_cal_low=-90;
const int16_t glove_pitch_cal_high=90;

const float a1 = 1.0;
const float a2 = 2.0;
const float a3 = 1.0;
const float b1 = 4.841;
const float b2 = 1.789;
const float b3 = -0.948;

uint8_t _indang;
uint8_t _midang;
uint8_t _rngang;
uint8_t _lilang;
uint8_t _thbang;
uint16_t _roll;
uint16_t _pitch;
uint16_t _yaw;
uint8_t ff_status;
uint8_t tf_status;

int interval = 20; // virtual delay
unsigned long previousMillis = 0; // Tracks the time since last event fired


// Wifi Credentials
const char* ssid = "NATURE"; // Wifi SSID
const char* password = "sut3omki"; //Wi-FI Password
WebSocketsClient webSocket; // websocket client class instance
StaticJsonDocument<200> doc; // Allocate a static JSON document

Servo thbservo;
Servo indservo;
Servo knklrollservo;
Servo knklpitchservo;

/*************************Pressure Sensors**************************/

void push_y(float *stack,float value)
{
  int i=2;
  for (i=2;i>0;i--)
    stack[i]=stack[i-1];
  stack[0]=value;
}
void filtered_values(float *y,int *x)
{
  float y_out;
  y_out = (a1*x[0]+a2*x[1]+a3*x[2]+b2*y[1]+b3*y[2])/b1;
  push_y(y,y_out);
}

void push_x(int *stack,int value)
{
  int i=2;
  for (i=2;i>0;i--)
    stack[i]=stack[i-1];
  stack[0]=value;
}

void update_fing_values()
{
  int raw_ind;
  int raw_thb;
  
  raw_ind = analogRead(ind_sense_pin);
  raw_thb = analogRead(thumb_sense_pin);
  
  push_x(x_ind_sense,raw_ind);
  push_x(x_thb_sense,raw_thb);
  
  filtered_values(y_ind_sense,x_ind_sense);
  filtered_values(y_thb_sense,x_thb_sense);
}
/*******************************************************************/

/************************servo drive*********************************/
void setup_servo()
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  thbservo.setPeriodHertz(50); 
  indservo.setPeriodHertz(50); 
  knklrollservo.setPeriodHertz(50);
  knklpitchservo.setPeriodHertz(50);
}

void servo_attach()
{ 
  thbservo.attach(thb_servo_pin, 400, 2200);
  indservo.attach(ind_servo_pin, 500, 2200); 
  knklrollservo.attach(knkl_roll_servo_pin, 900, 2400);
  knklpitchservo.attach(knkl_pitch_servo_pin, 700, 2400);  
}
/********************************************************************/

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT)
  {
    // deserialize incoming Json String
    DeserializationError error = deserializeJson(doc, payload); 
    if (error) 
    { // Print erro msg if incomig String is not JSON formated
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    _indang =(uint8_t) doc["indang"];
    _midang=_indang;
//    _midang =(uint8_t) doc["midang"];
//    _rngang =(uint8_t) doc["rngang"];
//    _lilang =(uint8_t) doc["lilang"];
    _thbang =(uint8_t) doc["thbang"];
    _indang=map((_indang+_thbang),15,90,0,80);
    _thbang=map((_midang+_thbang),15,90,85,0);
    //_midang=map(_thbang+_indang,glove_open_cal,glove_pinch_cal,90,0);
    //_indang=map(_midang,0,90,ind_close_ang,ind_open_ang);
    //_thbang=map(midang,0,90,thb_close_ang,thb_open_ang);
    _roll =(int16_t) doc["roll"]+180;//,glove_roll_cal_low,glove_roll_cal_high,knkl_roll_min,knkl_roll_max);
    _pitch =(int16_t) doc["pitch"]+90;//,glove_pitch_cal_low,glove_pitch_cal_high,knkl_pitch_min,knkl_pitch_max);
    if(_indang<81 && _indang>=0)
    thbservo.write(_thbang);
    if(_thbang<86 && _thbang>=0)
    indservo.write(_indang); 
    if(_roll<181 && _roll>=0)
    knklrollservo.write(_roll);
    if(_pitch<181 && _pitch>=0)
    knklpitchservo.write(_pitch);   
   // _yaw =(int16_t) doc["yaw"];
    ff_status =(uint8_t) doc["ff"];
    tf_status = (uint8_t) doc["tf"];
  }
}

void setup() {
  // Connect to local WiFi
  WiFi.begin(ssid, password);
  setup_servo();
  servo_attach();
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print local IP address
  //address, port, and URL path
  webSocket.begin("192.168.0.153", 81, "/");// server IP
  // WebSocket event handler
  webSocket.onEvent(webSocketEvent);
  // if connection failed retry every 5s
  webSocket.setReconnectInterval(5000);
}

void loop() 
{
  webSocket.loop(); // Keep the socket alive 
  unsigned long currentMillis = millis(); // call millis  and Get snapshot of time
  if ((unsigned long)(currentMillis - previousMillis) >= interval) 
  { // How much time has passed, accounting for rollover with subtraction!
    update_fing_values();
//    if(y_ind_sense[0]>tf_thld && y_ind_sense[0]<ff_thld)
//      webSocket.sendTXT('1');
    if(y_thb_sense[0]<ff_thld && tf_status==1)
     { webSocket.sendTXT('0');
     Serial.println("off");
     }
    if(y_thb_sense[0]>ff_thld && tf_status==0)
    {
      webSocket.sendTXT('3'); 
      Serial.println(tf_status);
    }                
    previousMillis = currentMillis;   // Use the snapshot to set track time until next event
  }
}

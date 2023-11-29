#include <WiFi.h> 
#include <WebServer.h> 
#include <ESP32Servo.h>
#include <ArduinoJson.h> 
#include <WebSocketsServer.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

/**************************imu******************************/
#define INTERRUPT_PIN 5
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

MPU6050 mpu;
/************************************************************/

/**************************angle sensor********************/
const uint8_t ind_fing_pin = 39;
const uint8_t mid_fing_pin = 34;
const uint8_t rng_fing_pin = 35;
const uint8_t lil_fing_pin = 32;
const uint8_t thumb_pin = 36;

float y_ind[3];
float y_mid[3];
float y_rng[3];
float y_lil[3];
float y_thb[3];

int x_ind[3];
int x_mid[3];
int x_rng[3];
int x_lil[3];
int x_thb[3];

float ang_ind;
float ang_mid;
float ang_rng;
float ang_lil;
float ang_thb;

/****************************2nd order BW fliter*****************/

const float a1 = 1.0;
const float a2 = 2.0;
const float a3 = 1.0;
const float b1 = 4.841;
const float b2 = 1.789;
const float b3 = -0.948;

/*******************************************************************/

/*************************calibration joint angle**************/
const float ind_off=878.0;
const float mid_off=785.0;
const float rng_off=1179.0;
const float lil_off=950.0;
const float thb_off=895.0;

const float ind_range=1794.0;
const float mid_range=1708.0;
const float rng_range=1684.0;
const float lil_range=1845.0;
const float thb_range=1958.0;

const float delta_ang=107.0;

/***************************************************************/

/**********************servo and vibration motors ***********/
const uint8_t vmot_thb_pin = 2;// options 19
const uint8_t vmot_ind_pin = 4;
const uint8_t ind_servo_pin = 33;
const uint8_t thb_servo_pin = 25;
const uint8_t led_pin=17;
/*****************************************************************/

/***************** calibation haptic and tactile ****************/
const uint8_t dutyCycle_max = 128;
const uint8_t dutyCycle_min = 0;
const uint8_t thb_servo_push = 140;
const uint8_t ind_servo_push = 80;
const uint8_t thb_servo_pull = 180;
const uint8_t ind_servo_pull = 180;
/*****************************************************************/

uint8_t ff_status = 0;
uint8_t tf_status = 0;

Servo thbservo;
Servo indservo;
Servo led;
/************************your wifi router credentials***********************************/

const char* ssid = "NATURE";  
const char* password = "sut3omki"; 
/***********************************************************************/

/***********************joint angle sampling***************************/
int interval = 20; // sample time
unsigned long previousMillis = 0; // time stamp for last sample
/***************************************************/

/*********************** webpage*********************************/
String web ="<!DOCTYPE html><html><head> <title>Arduino and ESP32 Websocket</title> <meta name='viewport' content='width=device-width, initial-scale=1.0' /> <meta charset='UTF-8'> <style> body { background-color: #E6D8D5; text-align: center; } </style></head><body> <h1>INDEX FINGER ANGLE: <span id='indang'>-</span></h1> <h1>MIDDLE FINGER ANGLE: <span id='midang'>-</span></h1> <h1>RING FINGER ANGLE: <span id='rngang'>-</span></h1> <h1>LITLE FINGER ANGLE: <span id='lilang'>-</span></h1> <h1>THUMB ANGLE: <span id='thbang'>-</span></h1> <h1>ROLL: <span id='roll'>-</span></h1> <h1>PITCH: <span id='pitch'>-</span></h1> <h1>YAW: <span id='yaw'>-</span></h1> <h1>FORCE FEEDBACK: <span id='ff'>-</span></h1> <button type='button' id='BTN_1'> <h1>ON</h1> </button><button type='button' id='BTN_2'> <h1>OFF</h1> </button> <h1>TACTILE FEEDBACK: <span id='tf'>-</span></h1> <button type='button' id='BTN_3'> <h1>ON</h1> </button><button type='button' id='BTN_4'> <h1>OFF</h1> </button></body><script> var Socket; document.getElementById('BTN_1').addEventListener('click', button_1_pressed); document.getElementById('BTN_2').addEventListener('click', button_2_pressed); document.getElementById('BTN_3').addEventListener('click', button_3_pressed); document.getElementById('BTN_4').addEventListener('click', button_4_pressed); function init() { Socket = new WebSocket('ws://' + window.location.hostname + ':81/'); Socket.onmessage = function(event) { processCommand(event); }; } function processCommand(event) { var obj = JSON.parse(event.data); document.getElementById('indang').innerHTML = obj.indang; document.getElementById('midang').innerHTML = obj.midang; document.getElementById('rngang').innerHTML = obj.rngang; document.getElementById('lilang').innerHTML = obj.lilang; document.getElementById('thbang').innerHTML = obj.thbang; document.getElementById('roll').innerHTML = obj.roll; document.getElementById('pitch').innerHTML = obj.pitch; document.getElementById('yaw').innerHTML = obj.yaw; document.getElementById('ff').innerHTML = obj.ff; document.getElementById('tf').innerHTML = obj.tf; console.log(obj.indang); console.log(obj.midang); console.log(obj.rngang); console.log(obj.lilang); console.log(obj.thbang); console.log(obj.roll); console.log(obj.pitch); console.log(obj.yaw); console.log(obj.ff); console.log(obj.tf); } function button_1_pressed() { Socket.send('0'); } function button_2_pressed() { Socket.send('1'); } function button_3_pressed() { Socket.send('2'); } function button_4_pressed() { Socket.send('3'); } window.onload = function(event) { init(); }</script></html>";
String jsonString; // Temporary storage for the JSON String
/************************************************************/

WebServer server(80);  // create instance for web server on port "80"
WebSocketsServer webSocket = WebSocketsServer(81);  //create instance for webSocket server on port"81"


/****************************************imu**********************************/
/*============================================================================*/
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

void setup_imu()
{
// join I2C bus (I2Cdev library doesn't do this automatically)
    
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    
    /************************ initialize device************************/
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);
    
    /************************ verify connection************************/
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    /*****************************imu calibration********************/
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); 


    if (devStatus == 0) {
        // Calibration Time: generate offsets and calibrate our MPU6050
        mpu.CalibrateAccel(6);
        mpu.CalibrateGyro(6);
        mpu.PrintActiveOffsets();
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
        Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
        Serial.println(F(")..."));
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;
        Serial.print("dmpready : ");
        Serial.print(dmpReady);
        Serial.println(" ");

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

}

void update_imu()
{
  if (!dmpReady) return;
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer))
  {
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
  }
}
/*============================================================================*/
/****************************************end imu********************************/

/************************************ADC for joint angles****************/
/*----------------------------------------------------------------------*/
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
  int raw_mid;
  int raw_rng;
  int raw_lil;
  int raw_thb;
  
  raw_ind = analogRead(ind_fing_pin);
  raw_mid = analogRead(mid_fing_pin);
  raw_rng = analogRead(rng_fing_pin);
  raw_lil = analogRead(lil_fing_pin);
  raw_thb = analogRead(thumb_pin);
  
  push_x(x_ind,raw_ind);
  push_x(x_mid,raw_mid);
  push_x(x_rng,raw_rng);
  push_x(x_lil,raw_lil);
  push_x(x_thb,raw_thb);
  
  filtered_values(y_ind,x_ind);
  filtered_values(y_mid,x_mid);
  filtered_values(y_rng,x_rng);
  filtered_values(y_lil,x_lil);
  filtered_values(y_thb,x_thb);

  ang_ind = (y_ind[0]-ind_off)*(delta_ang/ind_range);
  ang_mid = (y_mid[0]-mid_off)*(delta_ang/mid_range);
  ang_rng = (y_rng[0]-rng_off)*(delta_ang/rng_range);
  ang_lil = (y_lil[0]-lil_off)*(delta_ang/lil_range);
  ang_thb = (y_thb[0]-thb_off)*(delta_ang/thb_range);
}
/*---------------------------------------------------------------------*/
/*************************************END**********************************/

///**************************tactile and force feedback*******************/
///*-----------------------------------------------------------------------*/
void setup_feedback()
{
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  thbservo.setPeriodHertz(50); 
  indservo.setPeriodHertz(50); 
  pinMode(vmot_thb_pin,OUTPUT);
  pinMode(vmot_ind_pin,OUTPUT);
}
  void activate_foce_feedback()
  {
    thbservo.attach(thb_servo_pin, 1000, 2000);
    indservo.attach(ind_servo_pin, 1000, 2000);
    thbservo.write(thb_servo_push);
    indservo.write(ind_servo_push);
    ff_status=1;
  }
  
  void deactivate_foce_feedback()
  {
    thbservo.attach(thb_servo_pin, 1000, 2000);
    indservo.attach(ind_servo_pin, 1000, 2000);
    thbservo.write(thb_servo_pull);
    indservo.write(ind_servo_pull);
    ff_status=0;
  } 
  
void activate_tactile_feedback()
{ 
    pinMode(vmot_thb_pin,OUTPUT); 
    digitalWrite(vmot_thb_pin,HIGH);
    pinMode(vmot_ind_pin,OUTPUT);
    digitalWrite(vmot_ind_pin,HIGH);      
  tf_status=1;
}

void deactivate_tactile_feedback()
{
  digitalWrite(2,LOW);  
  digitalWrite(4,LOW);
  tf_status=0;
}   
/*-----------------------------------------------------------------------*/
/********************************************END*******************************/
 
/*********************************websocket*********************************/
void webSocketEvent(byte num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: // enum that read status this is used for debugging.
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": DISCONNECTED");
      break;
    case WStype_CONNECTED:  // Check if a WebSocket client is connected or not
      Serial.print("WS Type ");
      Serial.print(type);
      Serial.println(": CONNECTED");
      /********************************/
      /*****add code for led **********/
      break;
    case WStype_TEXT: // check responce from client
      Serial.println(); // the payload variable stores teh status internally
      Serial.println(payload[0]);
      if (payload[0] == '3') { 
        activate_foce_feedback(); 
        activate_tactile_feedback();             
      }
      if (payload[0] == '1') {
        deactivate_foce_feedback(); 
        activate_tactile_feedback();
      }
      if (payload[0] == '0') {        
        deactivate_foce_feedback();
        deactivate_tactile_feedback();             
      }
      break;
  }
}

void update_webpage()
{
  StaticJsonDocument<200> doc;
  // create an object
  JsonObject object = doc.to<JsonObject>();
  object["indang"]=(uint8_t)ang_ind;
  object["midang"]=(uint8_t)ang_mid;
  object["rngang"]=(uint8_t)ang_rng;
  object["lilang"]=(uint8_t)ang_lil;
  object["thbang"]=(uint8_t)ang_thb;
  object["roll"]=ypr[2]* 180/M_PI;
  object["pitch"]=ypr[1] * 180/M_PI;
  object["yaw"]=ypr[0] * 180/M_PI;
  object["ff"] = ff_status ;
  object["tf"]= tf_status;
  serializeJson(doc, jsonString); // serialize the object and save teh result to teh string variable.
  Serial.println( jsonString ); // print the string for debugging.
  webSocket.broadcastTXT(jsonString); // send the JSON object through the websocket
  jsonString = ""; // clear the String.
}

void setup() {
  // put your setup code here, to run once:
  led.setPeriodHertz(10);
  led.attach(led_pin,0,50000);
  led.write(180);
  Serial.begin(115200); // Init Serial for Debugging.
  WiFi.begin(ssid, password); // Connect to Wifi 
  while (WiFi.status() != WL_CONNECTED) { // Check if wifi is connected or not
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  // Print the IP address in the serial monitor windows.
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // Initialize a web server on the default IP address. and send the webpage as a response.
  server.on("/", []() {
    server.send(400, "text\html", web);
  });
  server.begin(); // init the server
  webSocket.begin();  // init the Websocketserver
  webSocket.onEvent(webSocketEvent);  // init the webSocketEvent function when a websocket event occurs 
  //setup_feedback();
  setup_imu();
  led.detach();
  pinMode(led_pin,OUTPUT);
  digitalWrite(led_pin,HIGH);
}

void loop() 
{
  server.handleClient();  // webserver methode that handles all Client
  webSocket.loop(); // websocket server methode that handles all Client
  unsigned long currentMillis = millis(); // call millis  and Get snapshot of time
  if ((unsigned long)(currentMillis - previousMillis) >= interval) 
  { // How much time has passed, accounting for rollover with subtraction!
    update_fing_values();
    update_imu();
    update_webpage(); 
    previousMillis = currentMillis;   // Use the snapshot to set track time until next event
  }
}

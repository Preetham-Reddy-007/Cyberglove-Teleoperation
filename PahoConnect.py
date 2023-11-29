import socket
import pydobot
import math

import paho.mqtt.client as mqtt #import the client1
import time

########################################


def on_message(client, userdata, message):
    global message_received
    global dx
    global dy
    global dz
    global dg
    global mes
    print("message received " ,str(message.payload.decode("utf-8")))
    mes = str(message.payload.decode("utf-8"))
    print(mes[:1],mes[1:2],mes[2:3],mes[3:4])
    print(ord(mes[:1]),ord(mes[1:2]),ord(mes[2:3]),ord(mes[3:4]))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
    dx = ord(mes[:1])
    dy = ord(mes[1:2])
    dz = ord(mes[2:3])
    dg = ord(mes[3:4])
    client.loop_stop()
    
    
########################################

port = "/dev/ttyUSB2"
device = pydobot.Dobot(port=port, verbose=True)

#Move to Home
device.move_to(234, 0, 75, 0, wait=True)
device.wait(1000)
         
(x, y, z, r, j1, j2, j3, j4) = device.pose()
print(device.pose())

device.grip(False)
g_up = False
device.wait(1000) 
device.suck(False)
device.wait(1000)

try:
 while True:
    
    print("creating new instance")
    client = mqtt.Client("P1") #create new instance
    client.on_message=on_message #attach function to callback
    print("connecting to broker")
    broker_address="192.168.51.115"
    #broker_address="broker.mqtt-dashboard.co"
    #broker_address="broker.emqx.io"
    client.connect(broker_address,1883) #connect to broker    
    print("Subscribing to topic","ESP32/Potentiometerdataread")
    client.subscribe("ESP32/Potentiometerdataread")
    client.loop_start()
    time.sleep(4) # wait
    
    if mes:
        if (dg <= 63):
           d = 1
        else:
           d = 0
        print("d = ", d)
        #print received data
        xa = round((dx * 270)/127)
        print(xa)
        ya = round((dy * 270)/127)
        print(ya)
        za = round((dz * 150)/127)
        print(za)
        sqr = (xa**2) + (ya**2)
        sqrt = math.sqrt(sqr)
        print(sqrt)
        if (sqrt <= 270):
           if (sqrt >= 195):
            x = xa
            y = ya
           else:
               print('Out of 195 Workspace')
        else:
           print('Out of 270 Workspace')
       
        if (za <= 150) and (za >= 0):
            z = za
        else:
           print('Out of Workspace')
       
        if  d == 1:
            S = True
        else:
            S = False
        
        for i in range(1):
          device.move_to(x, y, z, r, wait=True)
          print(device.pose())
          device.wait(1000)
          if S == g_up:
             device.suck(False)
             device.wait(1000)
          else:
              g_up = S
              device.grip(S)
              device.wait(1000) 
              device.suck(False)
              device.wait(1000)
    
      
except KeyboardInterrupt:
    device.suck(False)
    device.wait(3000)
    device.close() 
     
    
# Close connection
device.close()

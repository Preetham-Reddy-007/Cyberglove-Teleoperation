import socket
import pydobot
import math

import paho.mqtt.client as mqtt #import the client1
import time
   
########################################

port = "/dev/ttyUSB1"
device = pydobot.Dobot(port=port, verbose=True)

#Move to Home
device.move_to(234, 0, 75, 0, wait=True)
device.wait(1000)
         
(x, y, z, r, j1, j2, j3, j4) = device.pose()
print(device.pose())

device.speed(50.0,50.0)
device.grip(False)
g_up = False
device.wait(1000) 
device.suck(False)
device.wait(1000)

####################################

def on_message(client, userdata, message):
    rows, cols = (20, 4)
    global arr
    arr = [[0 for m in range(cols)] for n in range(rows)]
    print(arr)
    global dx
    global dy
    global dz
    global dg
    global A
    print("message received " ,str(message.payload.decode("utf-8")))
    mes = str(message.payload.decode("utf-8"))
    i = 0
    while i < 80:
        j = 0
        while j < 20:
         k = 0
         while k < 4:
                 print("Message: " , mes[i:(i+1)], "Value: " , ord(mes[i:(i+1)]) )
                 arr[j][k] = ord(mes[i:(i+1)])
                 k += 1
                 i += 1
         j += 1	
    
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
    A = ord(mes[:1])
    client.loop_stop()

####################################    

    
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
    print("loopstart")
    
    if A:
        i = 0
        while i < 20:
                j = 0
                while j < 3:
                       if (arr[i][3] <= 63):
                         d = 1
                       else:
                         d = 0   
                       print("d = ", d)
        		#Map values to range of Dobot
                       xa = round((arr[i][j]  * 270)/127)
                       print(xa)
                       j += 1
                       ya = round((arr[i][j] * 270)/127)
                       print(ya)
                       j += 1
                       print("i=",i)
                       print("j=",j)
                       za = round((arr[i][j] * 150)/127)
                       j += 1 
                       print("j=",j)
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

                       for r1 in range(1):
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
                i += 1
     	
      
except KeyboardInterrupt:
    device.suck(False)
    device.wait(3000)
    device.close() 
     
    
# Close connection
device.close()

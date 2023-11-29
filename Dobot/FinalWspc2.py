import socket
import pydobot
import math

port = "/dev/ttyUSB2"
device = pydobot.Dobot(port=port, verbose=True)

#Move to Home
device.move_to(234, 0, 75, 0, wait=True)
device.wait(1000)
         
(x, y, z, r, j1, j2, j3, j4) = device.pose()
print(device.pose())

# bind all IP
HOST = '192.168.170.115' 
# Listen on Port 
PORT = 44444 
#Size of receive buffer   
BUFFER_SIZE = 1024    
# Create a TCP/IP socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the socket to the host and port
s.bind((HOST, PORT))

device.grip(False)
g_up = False
device.wait(1000) 
device.suck(False)
device.wait(1000)

try:
 while True:
    # Receive BUFFER_SIZE bytes data
    # data is a list with 2 elements
    # first is data
    #second is client address
    data = s.recvfrom(BUFFER_SIZE)
    D = data[0].decode('utf-8')
    dx = ord(D[0])
    print(dx)
    dy = ord(D[1])
    print(dy)
    dz = ord(D[2])
    print(dz)
    dg = ord(D[3])
    print(dg)
    if data:
        #print received data
        print('Client to Server: ',dx)
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
       
    if dg == 1:
        g = True
    else:
        g = False
        
    for i in range(1):
      device.move_to(x, y, z, r, wait=True)
      print(device.pose())
      device.wait(1000)
      if g == g_up:
         device.suck(False)
         device.wait(1000)
      else:
          g_up = g
          device.grip(g)
          device.wait(1000) 
          device.suck(False)
          device.wait(1000)
      
except KeyboardInterrupt:
    device.suck(False)
    device.wait(3000)
    s.close() 
    device.close() 
     
    
# Close connection
s.close()
device.close()
#!/usr/bin/env python
import socket
import sys
import time
from threading import Thread
HOST = '127.0.0.1'
#PORT = 52321
PORT = 50000

class sender(Thread):

   def __init__ (self,filename, threadid):
      Thread.__init__(self)
      self.ip = HOST
      self.port = PORT
      self.filename = filename
      self.threadid = threadid
      
   def run(self):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except socket.error, msg:
            sys.stderr.write("[ERROR] %s\n" % msg[1])
            sys.exit(1)

        try:
            sock.connect((HOST, PORT))
            print("connected!")
        except socket.error, msg:
            sys.stderr.write("[ERROR] %s\n" % msg[1])
            sys.exit(2)
        
        totalsent = 0
        totalrecv = 0
        f = open(self.filename,'rb')
        recvFile = open('recv'+str(self.threadid)+'.bin', 'wb')
        
        while(1):
            bytes = f.read(1000)
            if bytes:
                totalsent += sock.send(bytes)
                #time.sleep(.001)
                recv_data = sock.recv(1000)
                recvFile.write(recv_data)
            else:
                break
        print totalsent
        f.close()
        sock.shutdown(socket.SHUT_WR)
        print("send done")
        
        input = True
        while input:
            input = sock.recv(1024)
            
            if len(input) > 0:
                totalrecv += len(input)
                print "thread %d received super secret message - %s" %(self.threadid, input)
            
        sock.shutdown(socket.SHUT_RD)
        sock.close()
        recvFile.close()
        
senderlist = []
for i in range(1,2):
   filename = ""
   current = sender("iq.bin", i)
   senderlist.append(current)
   current.start()   
   
for s in senderlist:
   s.join()

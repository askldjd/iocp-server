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
   
        for i in range (1,2):
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            except socket.error as msg:
                sys.stderr.write("[ERROR] %s\n" % msg[1])
                sys.exit(1)

            try:
                sock.connect((HOST, PORT))
                print("thread %d connected!" %(self.threadid))
            except socket.error as msg:
                sys.stderr.write("[ERROR] %s\n" % msg[1])
                sys.exit(2)
            
            #rwThreshold = 100000
            rwThreshold = 0
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
                    totalrecv += len(recv_data)
                    
                    if 0 != rwThreshold:
                        if totalsent > rwThreshold:
                            break;
                else:
                    break
            f.close()
            sock.shutdown(socket.SHUT_WR)
            
            input = True
            while input:
                input = sock.recv(1024)
                
                if len(input) > 0:
                    totalrecv += len(input)
                    print "thread %d received super secret message - %s" %(self.threadid, input)
                
            sock.shutdown(socket.SHUT_RD)
            sock.close()
            recvFile.close()
        
            print "thread %d closed connection - sent %d received %d" %(self.threadid, totalsent, totalrecv)

#main starts here

if len(sys.argv) < 3:
    print("echo_client.py <file to send> <num thread> ")
    sys.exit(0)
    
senderlist = []
for i in range(1, int(sys.argv[2])+1):
   filename = sys.argv[1]
   current = sender(filename, i)
   senderlist.append(current)
   current.start()   
   
for s in senderlist:
   s.join()

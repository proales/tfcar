"""Records training data and / or drives the car with tensorflow.

Usage:
	main_car.py record
	main_car.py tf
"""

import math
import os
import re
import sys
import time

import cv2
import numpy as np
import serial
import tensorflow as tf

#from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer

PORT_NUMBER = 8080

#This class will handles any incoming request from
#the browser 
class myHandler(BaseHTTPRequestHandler):
  
  #Handler for the GET requests
  def do_GET(self):
    self.send_response(200)
    self.send_header('Content-type','text/html')
    self.end_headers()
    # Send the html message
    self.wfile.write("Hello World !")
    return

try:
  #Create a web server and define the handler to manage the
  #incoming request
  server = HTTPServer(('', PORT_NUMBER), myHandler)
  print 'Started httpserver on port ' , PORT_NUMBER
  
  #Wait forever for incoming htto requests
  server.serve_forever()

except KeyboardInterrupt:
  print '^C received, shutting down the web server'
  server.socket.close()


def main():
  print("hello world")
  port_in = serial.Serial("/dev/cu.usbmodem1421", 9600, timeout=0.0)
  port_in.flush()
  buffer_in = ""
  l

  with open('test.txt', 'w') as f:
    
    f.write('start\n')

    while True:
      loop_start_time = time.time()

      try:
        buffer_in += port_in.read(port_in.in_waiting).decode('ascii')
      except UnicodeDecodeError:
        buffer_in = ''
        print("Mysterious serial port error. Let's pretend it didn't happen. :)")

      while '\n' in buffer_in:
        line, buffer_in = buffer_in.split('\n', 1)
        f.write(line + '\n')
        print line

      # port_in.write(('T045S045M4\n').encode('ascii'))

      seconds = time.time() - loop_start_time
      while seconds < 1 / 30.:
        time.sleep(0.001)
        seconds = time.time() - loop_start_time

  f.closed

if __name__ == '__main__':
	main()

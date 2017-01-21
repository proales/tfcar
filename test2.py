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

mode = 0
port_in
buffer_in

def serial_in():
  try:
    buffer_in += port_in.read(port_in.in_waiting).decode('ascii')
  except UnicodeDecodeError:
    buffer_in = ''
    print("Serial port error")

  while '\n' in buffer_in:
    line, buffer_in = buffer_in.split('\n', 1)
    mode_update(line)
    f.write(line + '\n')
    print line


def mode_update(line):
  parts = line.split(' ')
  mode = parts[0][1]
  print('Mode is now: ' + mode)


def fps(loop_start_time):
  seconds = time.time() - loop_start_time
  while seconds < 1 / 30.:
    time.sleep(0.001)
    seconds = time.time() - loop_start_time


def record_mode():
  with open('test.txt', 'w') as f:
    while mode == 1:
      loop_start_time = time.time()

      try:
        buffer_in += port_in.read(port_in.in_waiting).decode('ascii')
      except UnicodeDecodeError:
        buffer_in = ''
        print("Mysterious serial port error. Let's pretend it didn't happen. :)")

      while '\n' in buffer_in:
        line, buffer_in = buffer_in.split('\n', 1)
        mode_update(line)
        f.write(line + '\n')
        print line
      
      fps(loop_start_time)
  f.closed


def playback_mode():
  with open('test.txt', 'r') as f:
    while mode == 2:
      loop_start_time = time.time()
      line = f.readline()
      port_in.write((line + '\n').encode('ascii'))

      

      fps(loop_start_time)
  f.closed


def main():
  print("Starting TFCar")
  port_in = serial.Serial("/dev/cu.usbmodem1461", 9600, timeout=0.0)
  port_in.flush()
  buffer_in = ""

  mode = 1
  
  while True:
    # Record mode
    print('Main Loop')
    if mode == 1:
      record_mode()
    elif mode == 2:
      playback_mode()
    else:
      listen_mode()

if __name__ == '__main__':
	main()

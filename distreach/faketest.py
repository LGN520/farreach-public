
from multiprocessing import Process

import logging
import os
import random
import sys
import time
import unittest
import zlib

import socket
import struct

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.dirname(this_dir))
from spineswitch.common import *

fake_cachepop_client_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
fake_cachepop_client_udpsock.sendto(int(6).to_bytes(4,byteorder="big"),("192.168.1.1",10080))
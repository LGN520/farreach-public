
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
from common import *

fake_cachepop_client_udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
fake_cachepop_client_udpsock.sendto(struct.pack("=II",CACHE_FREQUENCYREQ,2),("192.168.1.102",10086))
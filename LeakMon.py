#!/usr/bin/env python3
# (c) 2022 Pekko Metsä
# License: GPLv3 or any later

import socket
import configparser
from datetime import datetime

configfile = 'leakmon.conf'
cfg = configparser.ConfigParser()
cfg.read(configfile)

HOST =     cfg.get('network', 'arduino')
PORT = int(cfg.get('network', 'port'))

with socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'\n')
    data = s.recv(512)

df      = data[0]   # One byte is read as an integer.  No need to convert.
id_b    = data[1:3] # Longer stuff is read as byte strings.
count_b = data[3:7]
ts_b    = data[7:11]
v1_b    = data[11:13]
v2_b    = data[13:15]

id    = int.from_bytes(id_b,    "little", signed=False)
count = int.from_bytes(count_b, "little", signed=False)
ts    = int.from_bytes(ts_b,    "little", signed=False)
v1    = int.from_bytes(v1_b,    "little", signed=False)
v2    = int.from_bytes(v2_b,    "little", signed=False)

timestamp = datetime.fromtimestamp(ts)

print(df, id, count, timestamp, v1, v2)

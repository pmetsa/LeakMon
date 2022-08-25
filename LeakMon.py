#!/usr/bin/env python3

import socket
from datetime import datetime

HOST='teensy41.poroinfra.metsa'
PORT=8888

with socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'\n')
    data = s.recv(512)

df      = data[0]
id_b    = data[1:3]
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

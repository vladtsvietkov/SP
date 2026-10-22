#!/usr/bin/env python3
"""Collect actual RFC2217 bytes; timestamps remain inside MCU messages."""
import argparse,datetime,time,json,hashlib
from pathlib import Path
import serial
p=argparse.ArgumentParser();p.add_argument('output');p.add_argument('--seconds',type=float,default=20);p.add_argument('--port',type=int,default=4417)
a=p.parse_args();dest=Path(a.output);dest.parent.mkdir(parents=True,exist_ok=True)
deadline=time.monotonic()+60
while True:
 try:
  s=serial.serial_for_url(f'rfc2217://localhost:{a.port}',baudrate=115200,timeout=.2);break
 except serial.SerialException:
  if time.monotonic()>deadline:raise
  time.sleep(.5)
start=datetime.datetime.now(datetime.timezone.utc).isoformat();end=time.monotonic()+a.seconds
with dest.open('wb') as f:
 while time.monotonic()<end:
  b=s.read(2048)
  if b:f.write(b);f.flush()
s.close()
dest.with_suffix('.manifest.json').write_text(json.dumps({'actual_utc':start,'duration_host_s':a.seconds,'port':a.port,'baud':115200,'sha256':hashlib.sha256(dest.read_bytes()).hexdigest()},indent=2)+'\n')
print(dest.read_text(errors='replace'))


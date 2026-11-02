#!/usr/bin/env python3
"""Drive the bring-up diagnostic commands against this project's real UART."""
import serial,time,json,datetime,hashlib,argparse
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--output',default='docs/metrics/demo-fault-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ'))
a=p.parse_args();out=Path(a.output);out.mkdir(parents=True,exist_ok=False)
s=serial.serial_for_url('rfc2217://localhost:4417',115200,timeout=.5)
steps=[];phase=0;good=0;bad=0;deadline=time.monotonic()+45;buf=b''
with (out/'fault-recovery.log').open('wb') as f:
 while time.monotonic()<deadline:
  b=s.read_until(b'\n')
  if not b:continue
  f.write(b);f.flush();line=b.decode(errors='replace')
  if line.startswith('BMP ') and 'valid=1' in line:good+=1
  if line.startswith('BMP ') and 'valid=0' in line:bad+=1
  cmd=None
  if phase==0 and good>=2:cmd='nack';phase=1;good=0
  elif phase==1 and bad>=2:cmd='recover';phase=2;good=0
  elif phase==2 and good>=2:break
  if cmd:
   s.write((cmd+'\n').encode());steps.append({'actual_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'after_mcu_line':line.strip(),'tx':cmd})
s.close()
(out/'fault-inputs.json').write_text(json.dumps({'steps':steps,'completed':phase==2 and good>=2,'elf_sha256':hashlib.sha256(Path('firmware/build/firmware.elf').read_bytes()).hexdigest()},indent=2)+'\n')
if not (phase==2 and good>=2):raise SystemExit('Fault/recovery incomplete')
print('Actual Wokwi fault/recovery verified; commands:',[x['tx'] for x in steps])


#!/usr/bin/env python3
"""Derive C vectors from actual sigrok annotations, preserving their VCD identity."""
from pathlib import Path
import re,json,hashlib
root=Path('docs/captures/lab2')
frames=[];current=None;register=None;command=None;wcount=0
for line in (root/'i2c-startup.txt').read_text().splitlines():
 event=line.split('i2c-1: ',1)[-1]
 if event in ['Start','Start repeat','Repeated start','Stop']:
  if current and current['bytes']:frames.append(current)
  current=None
 if event.startswith('Address write:'):wcount=0
 elif event.startswith('Address read:'):
  current={'register':register,'command':command,'bytes':[]}
 elif event.startswith('Data write:'):
  value=int(event.rsplit(' ',1)[-1],16)
  if wcount==0:register=value
  elif register==0xf4:command=value
  wcount+=1
 elif event.startswith('Data read:') and current is not None:
  current['bytes'].append(int(event.rsplit(' ',1)[-1],16))
if current and current['bytes']:frames.append(current)
dht=[int(x,16) for x in re.findall(r'Byte: 0x([0-9a-fA-F]+)',(root/'dht-startup.txt').read_text())][:5]
assert len(dht)==5
cal=next(f['bytes'] for f in frames if f['register']==0xaa and len(f['bytes'])==22)
coeff=[]
for i in range(11):
 v=(cal[2*i]<<8)|cal[2*i+1]
 coeff.append(v if i in [3,4,5] or v<32768 else v-65536)
utb=next(f['bytes'] for f in frames if f['register']==0xf6 and f['command']==0x2e and len(f['bytes'])==2)
upb=next(f['bytes'] for f in frames if f['register']==0xf6 and f['command']==0x34 and len(f['bytes'])==3)
ut=int.from_bytes(bytes(utb),'big');up=int.from_bytes(bytes(upb),'big')>>8
vcd=root/'sensors-fault-recovery.vcd'
manifest={'vcd':str(vcd),'sha256':hashlib.sha256(vcd.read_bytes()).hexdigest(),'dht_bytes':dht,'bmp_coefficients':coeff,'bmp_ut':ut,'bmp_up':up,'decoded_frames':frames}
(root/'fixture.json').write_text(json.dumps(manifest,indent=2)+'\n')
dest=Path('tests/fixtures/captured.h');dest.parent.mkdir(exist_ok=True)
dest.write_text('/* Derived by tools/capture_fixture.py from own Wokwi VCD SHA256 '+manifest['sha256']+' */\n'
 +'static const uint8_t captured_dht22[5]={'+','.join(map(str,dht))+'};\n'
 +'static const int32_t captured_bmp_cal[11]={'+','.join(map(str,coeff))+'};\n'
 +f'static const int32_t captured_bmp_ut={ut},captured_bmp_up={up};\n')
print(json.dumps({k:v for k,v in manifest.items() if k!='decoded_frames'},indent=2))


#!/usr/bin/env python3
"""Inspect native SPI mode-0 edges of diagram-spi.json (D0..D3)."""
import sys,json,hashlib,statistics
from pathlib import Path
p=Path(sys.argv[1]);state={'!':0,'"':1,'#':1,'$':1};blocks=[];bits=[];rises=[];t=0;begin=0
def finish(end):
 if not bits:return
 data=[]
 for i in range(0,len(bits)-7,8):
  data.append([sum(bits[i+j][k]<<(7-j) for j in range(8)) for k in [0,1]])
 blocks.append({'start_us':begin/1000,'end_us':end/1000,'bytes':data})
 bits.clear()
with p.open() as f:
 for line in f:
  l=line.strip()
  if l.startswith('#'):t=int(l[1:])
  elif l and l[0] in '01' and l[1:] in state:
   key=l[1:];v=int(l[0]);old=state[key];state[key]=v
   if key=='$' and old!=v:
    if v:finish(t)
    else:begin=t
   if key=='!' and not old and v and not state['$']:
    bits.append((state['"'],state['#']));rises.append(t)
periods=[b-a for a,b in zip(rises,rises[1:]) if 2000<b-a<3000]
out={'source':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'mapping':'D0 CLK, D1 MOSI, D2 MISO, D3 CS#; mode0 MSB first','SCK_median_period_ns':statistics.median(periods),'SCK_median_Hz':1e9/statistics.median(periods),'blocks':blocks}
Path(str(p)+'.spi.json').write_text(json.dumps(out,indent=2)+'\n')
for b in blocks:
 data=b['bytes'];c=next((i for i,x in enumerate(data) if 0x40<=x[0]<=0x7f),None)
 if c is not None:print(round(b['start_us'],3),round(b['end_us'],3),'CMD',data[c][0]&63,'bytes',len(data),'MOSI',bytes(x[0] for x in data[c:c+8]).hex(' '),'MISO',bytes(x[1] for x in data[c:c+12]).hex(' '))
print('SCK period ns',out['SCK_median_period_ns'],'Hz',out['SCK_median_Hz'])

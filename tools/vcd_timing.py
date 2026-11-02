#!/usr/bin/env python3
"""Native 1 ns Wokwi edges; D0 SCL, D2 UART, D3 DHT, D4 ADC marker."""
import sys,json,statistics,hashlib
from pathlib import Path
p=Path(sys.argv[1]);signals={};edges={};now=0
with p.open() as f:
 for line in f:
  line=line.strip()
  if line.startswith('$var'):
   a=line.split();signals[a[3]]=a[4];edges[a[4]]=[]
  elif line.startswith('#'):now=int(line[1:])
  elif line and line[0] in '01' and line[1:] in signals:edges[signals[line[1:]]].append((now,int(line[0])))
def stats(v):return {'n':len(v),'min_us':min(v)/1000,'median_us':statistics.median(v)/1000,'mean_us':statistics.mean(v)/1000,'max_us':max(v)/1000} if v else None
out={'file':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'timescale':'1ns','duration_ms':now/1e6}
for name,e in edges.items():
 r=[t for t,v in e if v];intervals=[b-a for a,b in zip(r,r[1:])]
 out[name]={'rising_intervals':stats(intervals),'edges':len(e)}
 if name=='D0':
  clocks=[v for v in intervals if 100000<v<400000]
  out['scl_active_period']=stats(clocks)
  if clocks:out['scl_median_Hz']=1e9/statistics.median(clocks)
 if name=='D4':
  out['adc_pulse_width']=stats([b[0]-a[0] for a,b in zip(e,e[1:]) if a[1]==1 and b[1]==0])
Path(str(p)+'.timing.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))

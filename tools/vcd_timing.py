#!/usr/bin/env python3
"""Measure native VCD edge intervals without resampling the idle gaps."""
import sys,re,json,statistics
from pathlib import Path
p=Path(sys.argv[1]);signals={};edges={};now=0;scale=1
for line in p.read_text().splitlines():
 if line.startswith('$var'):
  a=line.split();signals[a[3]]=a[4];edges[a[4]]=[]
 elif line.startswith('#'):now=int(line[1:])
 elif line and line[0] in '01' and line[1:] in signals:
  edges[signals[line[1:]]].append((now,int(line[0])))
def stats(v):return {'n':len(v),'min_us':min(v)/1000,'avg_us':statistics.mean(v)/1000,'max_us':max(v)/1000} if v else None
out={'file':str(p),'timescale':'1ns','duration_ms':now/1e6}
for name,e in edges.items():
 r=[t for t,v in e if v];fall=[t for t,v in e if not v]
 intervals=[b-a for a,b in zip(r,r[1:])]
 if name=='D0':intervals=[v for v in intervals if 1000<v<100000]
 out[name]={'rising_intervals':stats(intervals),'edges':len(e)}
 if name=='D0' and intervals:out['scl_median_Hz']=1e9/statistics.median(intervals)
Path(str(p)+'.timing.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))


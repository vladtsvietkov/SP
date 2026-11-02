#!/usr/bin/env python3
"""Decode 115200 8N1 UART from native Wokwi VCD, no idle resampling."""
import argparse,bisect,json,hashlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('vcd');p.add_argument('output');p.add_argument('--symbol',default='#');a=p.parse_args()
source=Path(a.vcd);e=[];now=0
with source.open() as f:
 for l in f:
  if l.startswith('#'):now=int(l[1:])
  elif l.strip() in ['0'+a.symbol,'1'+a.symbol]:e.append((now,int(l[0])))
ts=[t for t,v in e];end=0;out=[];bad=0;bit=1e9/115200;frames=[]
for t,v in e:
 if v or t<end or t<1e6 or t+10*bit>now:continue
 data=[e[bisect.bisect_right(ts,t+(i+1.5)*bit)-1][1] for i in range(8)]
 bad+=e[bisect.bisect_right(ts,t+9.5*bit)-1][1]!=1
 out.append(sum(v<<i for i,v in enumerate(data)));frames.append(t);end=t+9.8*bit
dest=Path(a.output);dest.write_bytes(bytes(out))
meta={'source':a.vcd,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'baud':115200,'bytes':len(out),'bad_stop_bits':bad,'partial_final_frame_excluded':True,'method':'RX values at native bit centers; inspect capture mapping for symbol'}
pos=0;meta['diagnostic_lines']=[]
for line in bytes(out).splitlines(keepends=True):
 if line.startswith(b'DIAG '):meta['diagnostic_lines'].append({'text':line.decode().strip(),'first_start_ms':frames[pos]/1e6,'last_stop_ms':(frames[pos+len(line)-1]+10*bit)/1e6})
 pos+=len(line)
dest.with_suffix('.decode.json').write_text(json.dumps(meta,indent=2)+'\n');print(json.dumps(meta))
if bad:raise SystemExit('Invalid UART stop bits')

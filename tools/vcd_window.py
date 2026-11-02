#!/usr/bin/env python3
"""Crop actual scalar edges, recording the original time origin and hash."""
import argparse,json,hashlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('output');p.add_argument('--start-ms',type=float,required=True);p.add_argument('--duration-ms',type=float,required=True);p.add_argument('--signal',action='append',required=True);a=p.parse_args()
source=Path(a.source);wanted=dict(s.split('=',1) for s in a.signal);start=round(a.start_ms*1e6);end=start+round(a.duration_ms*1e6)
symbols={};values={};events=[];now=0;started=False;mapping={}
for line in source.read_text().splitlines():
 if line.startswith('$var'):
  f=line.split()
  if f[4] in wanted:
   symbols[f[3]]=chr(65+len(symbols));mapping[f[4]]=symbols[f[3]]
 elif line.startswith('#'):
  now=int(line[1:])
  if now>end:break
 elif line and line[0] in '01' and line[1:] in symbols:
  symbol=symbols[line[1:]]
  if now<start:values[symbol]=line[0]
  else:
   if not started:
    events.append('#0');events.extend(v+s for s,v in sorted(values.items()));started=True
   events.extend(['#'+str(now-start),line[0]+symbol])
assert now>=end,'Source does not cover requested interval'
assert len(symbols)==len(wanted),'Unknown signal'
header=['$version Analysis window; original edges unchanged, see JSON origin $end','$timescale 1ns $end','$scope module logic $end']
header += [f'$var wire 1 {mapping[name]} {alias} $end' for name,alias in wanted.items()]
header += ['$upscope $end','$enddefinitions $end']
dest=Path(a.output);dest.write_text('\n'.join(header+events+['#'+str(end-start)])+'\n')
meta={'source':a.source,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'output_sha256':hashlib.sha256(dest.read_bytes()).hexdigest(),'origin_ms':a.start_ms,'duration_ms':a.duration_ms,'mapping':wanted,'method':'Native scalar edge window; timestamps rebased by origin_ms, initial levels from preceding source events. No synthetic transitions.'}
dest.with_suffix('.window.json').write_text(json.dumps(meta,indent=2)+'\n');print(dest)

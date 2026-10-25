from pathlib import Path
import re
bad=[]
for folder in ['firmware/App','firmware/Devices']:
 for p in Path(folder).glob('*.[ch]'):
  for n,line in enumerate(p.read_text().splitlines(),1):
   if re.search(r'#\s*include.*(stm32|main\.h|board\.h|cmsis|Platform)',line,re.I):bad.append(f'{p}:{n}: {line}')
if bad:raise SystemExit('\n'.join(bad))
print('DewWindow layer dependency check: PASS')

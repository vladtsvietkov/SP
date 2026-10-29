from pathlib import Path
p=Path('media/dew-demo.img');p.parent.mkdir(exist_ok=True)
if p.exists():print('Preserved existing SD image:',p)
else:
 with p.open('wb') as f:f.truncate(4*1024*1024)
 print('Created blank 4MiB raw SD image:',p)

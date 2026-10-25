import subprocess,platform,json,re
from pathlib import Path
cmd=['xcrun','llvm-cov','gcov'] if platform.system()=='Darwin' else ['gcov']
files=list(Path('build-tests').glob('test-*.gcno'))
out=subprocess.check_output(cmd+['-b','-c']+[str(p) for p in files],text=True)
Path('build-tests/coverage.txt').write_text(out);print(out)
rows=[]
for m in re.finditer(r"File '([^']+)'\nLines executed:([\d.]+)% of (\d+)",out):
 if m[1].startswith(('firmware/Devices/','firmware/App/')):rows.append({'file':m[1],'line_pct':float(m[2]),'lines':int(m[3])})
Path('build-tests/coverage.json').write_text(json.dumps(rows,indent=2)+'\n')
if not rows or any(x['line_pct']<85 for x in rows):raise SystemExit('Required line coverage >=85% for each Devices/App file')

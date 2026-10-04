#!/usr/bin/env python3
"""Export recursive submodules into a self-contained Arduino ZIP."""
from pathlib import Path
import subprocess, zipfile
root=Path(__file__).resolve().parents[1]
state=subprocess.check_output(['git','submodule','status','--recursive'],cwd=root,text=True)
if any(line.startswith(('-', '+', 'U')) for line in state.splitlines()):
    raise SystemExit('Initialize exact dependencies: git submodule update --init --recursive')
files=['library.properties','library.json','README.md','LICENSE','THIRD_PARTY.md','VALIDATION.md','DESIGN.md']
for folder in ['src','examples','third_party/acan2515/src','third_party/acan2517FD/src']:
    files.extend(str(p.relative_to(root)) for p in (root/folder).rglob('*') if p.is_file())
for lib in ['acan2515','acan2517FD']:
    files.extend(str(p.relative_to(root)) for p in (root/'third_party'/lib).glob('*') if p.is_file() and 'license' in p.name.lower())
output=root/'dist/KaruraCAN.zip';output.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED) as z:
    for f in files:z.write(root/f,'KaruraCAN/'+f)
print(output)

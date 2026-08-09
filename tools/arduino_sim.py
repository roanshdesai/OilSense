import re, sys
# Reproduce what arduino-builder does: collect every function definition's
# signature and insert prototypes immediately before the FIRST function
# definition - which is what breaks code whose structs come later.
src = open(sys.argv[1]).read()
sig = re.compile(r'^([A-Za-z_][\w:<>&*\s]*?[\s&*])([A-Za-z_]\w*)\s*\(([^;{)]*)\)\s*\{', re.M)
protos, first = [], None
for m in sig.finditer(src):
    ret, name, args = m.group(1).strip(), m.group(2), m.group(3)
    if name in ('if','for','while','switch','else','do','return','sizeof'): continue
    if ret.endswith(('=','+','-','return')): continue
    if first is None: first = m.start()
    protos.append(f"{ret} {name}({args});")
ins = src.rindex('\n', 0, first) + 1
out = src[:ins] + "// --- simulated Arduino auto-prototypes ---\n" + "\n".join(protos) + "\n\n" + src[ins:]
open(sys.argv[2], 'w').write(out)
print(f"hoisted {len(protos)} prototypes to offset {ins}")

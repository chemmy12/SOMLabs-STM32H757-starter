# pio_no_relax.py
from SCons.Script import Import
Import("env")

def strip_relax(linkflags):
    out = []
    for f in linkflags:
        s = str(f)
        if "--relax" in s:
            # remove only --relax, keep other parts of the -Wl, group
            s = s.replace("--relax", "").replace(",,", ",")
            if s.endswith(","):
                s = s[:-1]
            if s and s != "-Wl,":
                out.append(s)
        else:
            out.append(f)
    return out

lf = env.get("LINKFLAGS", [])
env.Replace(LINKFLAGS=strip_relax(lf))
print(">> LINKFLAGS after strip:", env.get("LINKFLAGS"))

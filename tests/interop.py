# Interop test: Graphic Viewer <-> Pillow, both directions.
#
# Direction 1: files written by the new GV build (out-new) are decoded by
#   Pillow and compared against GV's own decode (the .raw dumps, which equal
#   the source images for lossless formats).
# Direction 2: files written by Pillow (GIF and TIFF-LZW) are decoded by GV
#   (tests/rawdump.exe) and compared against Pillow's own decode.
import os, struct, subprocess, sys
from PIL import Image

def gvraw(path):
    out = path + '.gvraw'
    subprocess.run(['bin/rawdump.exe', path, out], check=True,
                   capture_output=True)
    d = open(out, 'rb').read()
    os.remove(out)
    w, h, pal = struct.unpack('<iii', d[:12])
    return w, h, pal, d[12:]

ok = fail = 0
fails = []

# ---- direction 1: GV output -> Pillow ----
for name in sorted(os.listdir('out-new')):
    if name.endswith('.raw'):
        continue
    path = 'out-new/' + name
    try:
        img = Image.open(path)
        img.load()
    except Exception as e:
        fails.append('%s: PIL could not open: %s' % (name, e)); fail += 1
        continue
    w, h, pal, body = gvraw(path)
    if img.size != (w, h):
        fails.append('%s: size %s vs %s' % (name, img.size, (w, h))); fail += 1
        continue
    if img.mode == 'P':
        p = img.getpalette() or []
        p = p + [0]*(768-len(p))
        idx = list(img.getdata())
        gv_idx = list(body[768:768+w*h])
        gv_pal = list(body[:768])
        if idx != gv_idx:
            d = next(i for i in range(len(idx)) if idx[i] != gv_idx[i])
            fails.append('%s: pixels differ at %d' % (name, d)); fail += 1
        elif p != gv_pal:
            fails.append('%s: palette differs' % name); fail += 1
        else:
            ok += 1
    elif img.mode == 'L':
        data = list(img.getdata())
        gv_idx = list(body[768:768+w*h])
        ramp = [i for i in range(256) for _ in range(3)]
        if data != gv_idx:
            fails.append('%s: grey pixels differ' % name); fail += 1
        elif list(body[:768]) != ramp:
            fails.append('%s: grey palette is not the ramp' % name); fail += 1
        else:
            ok += 1
    else:
        rgb = img.convert('RGB').tobytes()
        if list(body[:w*h*3]) != list(rgb):
            n = len(body[:w*h*3])
            d = next((i for i in range(min(n, len(rgb)))
                      if body[i] != rgb[i]), -1)
            fails.append('%s: rgb differs at %d' % (name, d)); fail += 1
        else:
            ok += 1

# ---- direction 2: Pillow output -> GV ----
def make_img(w, h, seed):
    img = Image.new('RGB', (w, h))
    px = img.load()
    s = seed
    def nxt():
        nonlocal s
        s = (s * 1103515245 + 12345) & 0x7FFFFFFF
        return s
    for y in range(h):
        for x in range(w):
            px[x, y] = (nxt() & 255, nxt() & 255, nxt() & 255)
    return img

os.makedirs('pil', exist_ok=True)
cases = []
base = make_img(61, 37, 42)
cases.append(('pil/p256.gif', base.convert('P', palette=Image.ADAPTIVE,
                                             colors=256)))
cases.append(('pil/p16.gif', base.convert('P', palette=Image.ADAPTIVE,
                                          colors=16)))
cases.append(('pil/p2.gif', base.convert('P', palette=Image.ADAPTIVE,
                                         colors=2)))
cases.append(('pil/p16i.gif', base.convert('P', palette=Image.ADAPTIVE,
                                           colors=16)))
cases.append(('pil/rgb.tif', base))
cases.append(('pil/grey.tif', base.convert('L')))
cases.append(('pil/pal.tif', base.convert('P', palette=Image.ADAPTIVE,
                                          colors=256)))
for name, img in cases:
    kw = {}
    if name.endswith('.gif'):
        kw = {'interlace': name.endswith('i.gif'), 'loop': 3}
    else:
        kw = {'compression': 'tiff_lzw'}
    try:
        img.save(name, **kw)
    except Exception as e:
        fails.append('%s: PIL could not save: %s' % (name, e)); fail += 1
        continue
    try:
        img2 = Image.open(name); img2.load()
        if img2.mode == 'P':
            truth = (list(img2.getdata()),)
        elif img2.mode == 'L':
            truth = (list(img2.getdata()),)
        else:
            truth = (list(img2.convert('RGB').tobytes()),)
    except Exception as e:
        fails.append('%s: PIL could not reopen: %s' % (name, e)); fail += 1
        continue
    w, h, pal, body = gvraw(name)
    if (w, h) != img2.size:
        fails.append('%s: GV size %s vs %s' % (name, (w, h), img2.size))
        fail += 1
        continue
    if img2.mode == 'P':
        p = img2.getpalette() or []
        p = p + [0]*(768-len(p))
        if list(body[768:768+w*h]) != truth[0] or list(body[:768]) != p:
            fails.append('%s: GV decode differs from PIL' % name); fail += 1
        elif pal != 1:
            fails.append('%s: GV did not report a palette image' % name)
            fail += 1
        else:
            ok += 1
    elif img2.mode == 'L':
        if list(body[768:768+w*h]) != truth[0]:
            fails.append('%s: GV grey decode differs' % name); fail += 1
        else:
            ok += 1
    else:
        if list(body[:w*h*3]) != truth[0]:
            fails.append('%s: GV rgb decode differs' % name); fail += 1
        elif pal != 0:
            fails.append('%s: GV reported palette for RGB' % name); fail += 1
        else:
            ok += 1

print('interop: %d ok, %d fail' % (ok, fail))
for f in fails[:20]:
    print('  ', f)
sys.exit(1 if fail else 0)

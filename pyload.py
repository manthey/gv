import io, sys, PIL.Image
try:
  from pillow_heif import register_heif_opener
  register_heif_opener()
  from pillow_heif import register_avif_opener
  register_avif_opener()
except Exception:
  pass
src = sys.stdin.buffer
dest = sys.stdout.buffer
inp = io.BytesIO(src.read())
buf = io.BytesIO()
try:
  import rawpy
  inp.seek(0)
  rgb = rawpy.imread(inp).postprocess()
  img = PIL.Image.fromarray(rgb.astype('uint8'), 'RGB')
except Exception:
  inp.seek(0)
  img = PIL.Image.open(inp).convert('RGB')
img.save(buf, format='PPM')
dest.write(buf.getvalue())

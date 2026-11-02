import io, re, sys

SRC = '/Users/evil/dev/c/EwokOS/machines/raspi5/system/libs/videocore/src/g2d_qpu_kernels.h'
OUT = 'tools/qpudis/extra_kernels.h'

txt = io.open(SRC, encoding='utf-8').read()

arrays = {}
for m in re.finditer(r'const uint64_t (g2d_qpu_\w+)\[\]\s*=\s*\{(.*?)\n\};', txt, re.S):
    name, body = m.group(1), m.group(2)
    words = re.findall(r'0x([0-9a-fA-F]{16})U?LL', body)
    if words:
        arrays[name] = words

want = ['g2d_qpu_gauss_v7', 'g2d_qpu_gauss_h5', 'g2d_qpu_argb_scale_pow2',
        'g2d_qpu_argb_rotate', 'g2d_qpu_argb_fill4', 'g2d_qpu_argb_rot90']
sel = [(n, arrays[n]) for n in want if n in arrays]
if not sel:
    sys.stderr.write('none matched\n')
    sys.exit(2)

out = ['/* generated from EwokOS g2d_qpu_kernels.h, for host-side analysis only */',
       '#ifndef QPUDIS_EXTRA_KERNELS_H',
       '#define QPUDIS_EXTRA_KERNELS_H',
       '#include <stdint.h>',
       '']
for name, words in sel:
    short = name.replace('g2d_qpu_', '')
    out.append('static const uint64_t k_%s[] = {' % short)
    for i in range(0, len(words), 4):
        out.append('    ' + ', '.join('0x%sULL' % w for w in words[i:i + 4]) + ',')
    out.append('};')
    out.append('static const unsigned k_%s_n = %d;' % (short, len(words)))
    out.append('')
out.append('#endif')

io.open(OUT, 'w', encoding='utf-8').write('\n'.join(out))
print('extracted: ' + ', '.join('%s(%d)' % (n.replace('g2d_qpu_', ''), len(w)) for n, w in sel))

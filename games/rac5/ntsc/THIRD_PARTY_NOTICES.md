# Third-party source notices

This project uses only public, compatibly licensed source as a research lead.
The RAC5 adaptations below are checked against the pinned retail program;
the source and linker gates determine matching credit. Sony SDK source is not
a permitted reference.

## Newlib 1.9.0 / fdlibm

The functions below adapt the public Newlib 1.9.0 files at commit
`039e8a3c16964d02838b204b72bfdd4b7529107c`. The corresponding local research copies were compared to
this public release; the original files are byte-identical. The per-file
Sun Microsystems permission notice is retained in each adapted C file.
The public [`newlib/libc/include/math.h`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libc/include/math.h)
and [`newlib/libm/common/fdlibm.h`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/common/fdlibm.h)
supplied the shared math types and accessors. Their relevant declarations were
compared with the public release; RAC5-specific addresses and scalar references were
verified against the retail boot, not imported from an SDK header.

| RAC5 source | Public upstream |
| --- | --- |
| `nonmatching/boot/__ieee754_acosf.c` | [`newlib/libm/math/ef_acos.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/ef_acos.c) |
| `nonmatching/boot/__ieee754_rem_pio2.c` | [`newlib/libm/math/e_rem_pio2.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/e_rem_pio2.c) |
| `nonmatching/boot/__ieee754_rem_pio2f.c` | [`newlib/libm/math/ef_rem_pio2.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/ef_rem_pio2.c) |
| `nonmatching/boot/__ieee754_sqrt.c` | [`newlib/libm/math/e_sqrt.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/e_sqrt.c) |
| `nonmatching/boot/__ieee754_sqrtf.c` | [`newlib/libm/math/ef_sqrt.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/ef_sqrt.c) |
| `nonmatching/boot/__kernel_cos.c` | [`newlib/libm/math/k_cos.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/k_cos.c) |
| `nonmatching/boot/__kernel_rem_pio2.c` | [`newlib/libm/math/k_rem_pio2.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/k_rem_pio2.c) |
| `nonmatching/boot/__kernel_rem_pio2f.c` | [`newlib/libm/math/kf_rem_pio2.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/kf_rem_pio2.c) |
| `nonmatching/boot/acosf.c` | [`newlib/libm/math/wf_acos.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_acos.c) |
| `nonmatching/boot/asinf.c` | [`newlib/libm/math/wf_asin.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_asin.c) |
| `nonmatching/boot/atan2f.c` | [`newlib/libm/math/wf_atan2.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_atan2.c) |
| `nonmatching/boot/atanf.c` | [`newlib/libm/math/sf_atan.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_atan.c) |
| `nonmatching/boot/ceilf.c` | [`newlib/libm/math/sf_ceil.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_ceil.c) |
| `nonmatching/boot/copysignf.c` | [`newlib/libm/common/sf_copysign.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/common/sf_copysign.c) |
| `nonmatching/boot/cosf.c` | [`newlib/libm/math/sf_cos.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_cos.c) |
| `nonmatching/boot/expf.c` | [`newlib/libm/math/wf_exp.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_exp.c) |
| `nonmatching/boot/fabs.c` | [`newlib/libm/math/s_fabs.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/s_fabs.c) |
| `nonmatching/boot/fabsf.c` | [`newlib/libm/math/sf_fabs.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_fabs.c) |
| `nonmatching/boot/finitef.c` | [`newlib/libm/common/sf_finite.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/common/sf_finite.c) |
| `nonmatching/boot/floorf.c` | [`newlib/libm/math/sf_floor.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_floor.c) |
| `nonmatching/boot/fmodf.c` | [`newlib/libm/math/wf_fmod.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_fmod.c) |
| `nonmatching/boot/isnan.c` | [`newlib/libm/math/s_isnan.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/s_isnan.c) |
| `nonmatching/boot/isnanf.c` | [`newlib/libm/math/sf_isnan.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_isnan.c) |
| `nonmatching/boot/logf.c` | [`newlib/libm/math/wf_log.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_log.c) |
| `nonmatching/boot/sin.c` | [`newlib/libm/math/s_sin.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/s_sin.c) |
| `nonmatching/boot/sinf.c` | [`newlib/libm/math/sf_sin.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_sin.c) |
| `nonmatching/boot/sqrt.c` | [`newlib/libm/math/w_sqrt.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/w_sqrt.c) |
| `nonmatching/boot/sqrtf.c` | [`newlib/libm/math/wf_sqrt.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/wf_sqrt.c) |
| `nonmatching/boot/tanf.c` | [`newlib/libm/math/sf_tan.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libm/math/sf_tan.c) |

The original source notice permits use, copying, modification and
distribution provided that the Sun Microsystems notice is preserved.
The files in this table are modified for the RAC5 ABI, symbols and pool
placement; they are not verbatim copies of the upstream translation units.

## Newlib mprec.c (one adapted helper)

`nonmatching/boot/_i2b.c` adapts the `i2b` helper from public
[Newlib 1.9.0 `newlib/libc/stdlib/mprec.c`](https://github.com/mirror/newlib-cygwin/blob/039e8a3c16964d02838b204b72bfdd4b7529107c/newlib/libc/stdlib/mprec.c).
The relevant helper body is unchanged between that public release
and the local research copy. The original David M. Gay / AT&T
permission notice is retained in the adapted source. RAC5-specific
structure offsets were checked against the pinned retail boot.

## Vendored newlib 1.8.2 headers

`third_party/newlib-1.8.2/` carries the public newlib 1.8.2 headers
(`newlib/libc/include/`, the `newlib/libc/stdio/` private headers `local.h`,
`floatio.h`, `fvwrite.h`) and the release's `COPYING.NEWLIB`, unmodified. The
files come from the public release tarball
`https://sourceware.org/pub/newlib/newlib-1.8.2.tar.gz`
(SHA-256 `e52667ed78595dd1919f4a303f8b658b1d57b926e085e8021851b279143cf3ef`).

`nonmatching/boot/__svfscanf.c` is the verbatim `newlib/libc/stdio/vfscanf.c`
lines 20-801 from that release (diff empty against the public tarball); its
file header records the equivalence and the MB_CAPABLE define the Sony EE libc
was built with. The vendored headers are used only at preprocess time by the
private boot gate through repository-relative `-I` flags.

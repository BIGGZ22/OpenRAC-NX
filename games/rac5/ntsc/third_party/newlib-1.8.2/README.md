# newlib 1.8.2 public headers (vendored, unmodified)

Source: the public newlib 1.8.2 release tarball,
`https://sourceware.org/pub/newlib/newlib-1.8.2.tar.gz`
(SHA-256 `e52667ed78595dd1919f4a303f8b658b1d57b926e085e8021851b279143cf3ef`,
dated 1999-11-20), extracted 2026-10-18.

This directory carries only the files needed to preprocess one reconstructed
boot body (`__svfscanf`, which is byte-identical to
`newlib/libc/stdio/vfscanf.c` lines 20-801 of this release):

- `newlib/libc/include/` — the release's public headers, unmodified (55 files);
- `newlib/libc/stdio/{local.h,floatio.h,fvwrite.h}` — the release's private
  stdio headers, unmodified;
- `COPYING.NEWLIB` — the release's notice, unmodified.

No source file is modified; the reconstruction's own file header records the
equivalence. These headers are used only at preprocess time by the private
boot gate, through repository-relative `-I` flags in `config/cflags.txt`.

#!/bin/sh
# Generate ctx.c: our recovered types, preprocessed into plain C that m2c's
# pycparser front end can read. GCC-only syntax (__attribute__, __asm__)
# is stripped because pycparser rejects it; nothing here is ever compiled.
set -e
. tools/toolchain.sh
CPP=toolchain/sn-prodg-24/local/sce/ee/gcc/bin/ee-gcc2953.exe
printf '#include "common.h"\n#include "structs.h"\n' > build-sn/ctx_in.c
sn $CPP -E -P -Iinclude \
    -D'__attribute__(x)=' -D'__asm__(x)=' \
    build-sn/ctx_in.c > ctx.c
rm -f build-sn/ctx_in.c
echo "wrote ctx.c ($(wc -l < ctx.c) lines)"

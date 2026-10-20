/* SDK strncpy (symbol_addrs/usa-v2/symbol_addrs.txt names 0x00115AC0, task
   1884 / RULING 9817). The body is newlib's hand-written
   libc/machine/r5900/strncpy.S: after the two `ori/dsll` mask builds
   (0x8080..0x8080808080808081) it copies 16 bytes at a time with `lq`/`sq`
   while `pcpyld`/`pcpyud`/`psubb`/`pand` find no zero byte, drops to 8-byte
   `ld`/`sd` and then to a scalar byte tail. It does not guarantee
   termination, so callers such as McGetDir store their own terminator.

   No C reaches this: 17 of the 115 words are MMI, which rabbitizer prints as
   `.word 0x........   # INVALID` (0x78A30000 `lq`, 0x70E74B89 `pcpyld`,
   0x70031CE9 `pcpyud`, 0x70691248 `psubb`, 0x70431489 `pand`, ...). Same
   shape as func_00115340 (SDK memcmp) and func_0011B458 (_sceIDC) already in
   this directory. The source exists as assembly and reproduces the ROM
   byte-exact through the SN Ps2EeAs, so this is an ASM_FUNC candidate rather
   than a near miss; the lead's call. Nothing was written as C. */

extern char *func_00115AC0(char *dst, const char *src, unsigned int n);

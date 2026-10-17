/* functional: high. Retail copies one 64-bit word (ld/sd) at offset 0x38 from the object that the global at D_0013E633 + 0x2E9D points to into offset 0x38 of the argument. Only the first argument is read, so the one-argument prototype is right. */
extern char D_0013E633[];

void func_L00_0025E210(void *dst) {
    char *src = *(char **)(D_0013E633 + 0x2E9D);
    *(s64 *)((char *)dst + 0x38) = *(s64 *)(src + 0x38);
}

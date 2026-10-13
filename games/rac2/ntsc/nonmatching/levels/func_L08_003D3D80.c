typedef unsigned int ti_word __attribute__((mode(TI)));
extern void *func_0029EFD0(int);
extern void func_002A0F48(void *);

void *func_L08_003D3D80(void *source) {
    char *dest = (char *)func_0029EFD0(0x987);
    char *src = (char *)source;
    ti_word copy;
    if (dest != 0) {
        dest[0x30] = src[0x30];
        dest[0x31] = 1;
        *(unsigned short *)(dest + 0x32) = *(unsigned short *)(src + 0x32);
        *(unsigned short *)(dest + 0x7C) = 0x7F;
        dest[0x20] = 0;
        *(void **)(*(char **)(dest + 0x68)) = source;
        copy = *(ti_word *)(src + 0x10);
        *(ti_word *)(dest + 0x10) = copy;
        func_002A0F48(dest);
    }
    return dest;
}

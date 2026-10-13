typedef unsigned int ti_word __attribute__((mode(TI)));
extern void func_00282CF0(void *, void *, void *);
extern void func_002AD6F0(void *, void *, void *, void *);

void func_L08_003D0508(void *raw) {
    char *object = raw;
    char *state = *(char **)(object + 0x68);
    ti_word copy;
    char scratch[0x10];
    if (*(unsigned char *)(object + 0x20)) return;
    copy = *(ti_word *)(object + 0x10);
    *(ti_word *)(state + 0x90) = copy;
    *(unsigned short *)(state + 0x3E) = 15;
    *(unsigned int *)(state + 0x8C) |= 1;
    *(void **)(state + 8) = state + 0x50;
    *(void **)(state + 0) = state + 0x20;
    object[0x20] = 1;
    *(unsigned short *)(object + 0x34) |= 0x20;
    func_00282CF0(scratch, object + 0x10, state + 0x90);
    func_002AD6F0(state + 0x50, scratch, object + 0xF0, object + 0xF0);
}

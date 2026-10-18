/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), src/core/0012AC80.c (games/rac1): func_0012C4E0, matched C there.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */

extern void func_00128450(void *);
extern void func_0012C328(void *, void *);
extern void func_0012C328(void *, void *);
extern int func_00128918(void *, int);
extern void func_0012C328(void *, void *);
extern char D_00153AE8[];
extern void func_00128420(void *, unsigned int);
extern void func_0012C850(void *, int, int);
extern char D_00132FC0[];
extern char D_00133000[];
extern void func_00128D28(void *);
extern void func_0012C4C8(void *);
void _sequenceHeader(void *arg0) {
    char *s = (char *)arg0;
    unsigned int v;
    int mid;
    unsigned int flag;
    *(unsigned int *)(s + 0xD4) = 0;
    v = func_00128918(s, 0x20);
    *(unsigned int *)(s + 0x124) = v >> 20;
    mid = (v >> 8) & 0xFFF;
    *(unsigned int *)(s + 0x128) = mid;
    if (mid >= 0xAF1) {
        func_0012C328(s, D_00153AE8);
    }
    v = func_00128918(s, 0x1E);
    *(unsigned int *)(s + 0x134) = v >> 12;
    *(unsigned int *)(s + 0x138) = (v >> 1) & 0x3FF;
    flag = func_00128918(s, 1);
    *(unsigned int *)(s + 0x840) = flag;
    if (flag != 0) {
        func_00128450(s);
        func_00128420(s, 0x50000000);
        func_00128450(s);
    } else {
        func_0012C850(s, 0x50000000, (int)D_00132FC0);
    }
    flag = func_00128918(s, 1);
    *(unsigned int *)(s + 0x844) = flag;
    if (flag != 0) {
        func_00128450(s);
        func_00128420(s, 0x58000000);
        func_00128450(s);
    } else {
        func_0012C850(s, 0x58000000, (int)D_00133000);
    }
    func_00128D28(s);
    func_0012C4C8(*(void **)(s + 0x858));
}
extern void func_0012C328(void *, void *);

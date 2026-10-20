/* 108-byte near miss; retail is 104 bytes. Retail computes object+0x60 into $v1 in the branch delay slot and stores the 16-byte vector through it; this cc1 folds the +0x60 into the store and copies the object pointer with one extra daddu. The constant destination offset of a 16-byte store is the wall in docs/COMPILER-NOTES.md: pointer, member and indexed spellings all fold it. */
typedef int V4_Fb6552688 __attribute__((mode(TI)));
extern int func_002ED688(unsigned char *owner);
extern unsigned int func_002EE6A8(unsigned char *owner);
extern void func_002EE988(char *);

void func_002EE920(unsigned char *owner, V4_Fb6552688 *src)
{
    char *object;
    V4_Fb6552688 *dst;
    unsigned int state;

    object = (char *)func_002ED688(owner);
    dst = (V4_Fb6552688 *)(object + 0x60);
    if (object == 0)
        return;
    *dst = *src;
    state = func_002EE6A8(owner);
    if (state == 2)
        return;
    if (state == 0xC)
        return;
    func_002EE988((char *)owner);
}

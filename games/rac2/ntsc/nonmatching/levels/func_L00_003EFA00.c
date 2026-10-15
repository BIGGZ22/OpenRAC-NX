/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L00_003EFA00(char *obj)
{
    char local[16];
    int r;

    r = func_002B4038(*(short *)(obj + 0xAA), *(int *)(*(int *)(obj + 0x68) + 0x28));
    if (r == 0) {
        func_L00_003EF550();
        func_0029F338(obj);
        return;
    }
    qcopy(local, obj + 0x10);
    *(float *)(local + 0xC) = 2.0f;
    r = func_00283DB8(local, 100.0f);
    if (r == -1)
        return;
    func_L00_003EFA98(obj);
    func_L00_003EFD00(obj);
    func_L00_003F0130(obj);
}

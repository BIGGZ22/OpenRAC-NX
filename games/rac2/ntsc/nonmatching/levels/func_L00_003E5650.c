/* Matched byte for byte, but not landable: this body needs `qcopy`/`qzero` from
   include/common.h, which are inline assembly, and AGENTS.md's "Plain C" rule (no inline
   assembly in a function) forbids that as a matching route - docs/COMPILER-NOTES.md says it
   in as many words. Kept here as a starting point: the shape is right, and only the 128-bit
   copies would need a plain-C spelling that neither `mode(TI)` nor an aggregate gives yet.
   Removed from src/ on 2026-10-13. */

void func_L00_003E5650(char *object)
{
    char *data;
    Vec16_Fc936841d local;
    int result;

    data = *(char **)(object + 0x68);
    if (func_002B4038(*(short *)(object + 0xAA), *(int *)(data + 0x28)) == 0) {
        func_L00_003E5330();
        func_0029F338(object);
    } else {
        qcopy(&local, object + 0x10);
        local.w = 2.0f;
        result = func_00283DB8(&local, 100.0f);
        if (result != -1) {
            if (*(unsigned char *)(object + 0x20) != 6) {
                func_L00_003E5710(object);
                func_L00_003E5978(object);
                func_L00_003E55B0(object);
            }
            func_L00_003E5BD8(object);
            func_L00_003E6030(object);
            func_L00_003E6710(object);
        }
    }
}

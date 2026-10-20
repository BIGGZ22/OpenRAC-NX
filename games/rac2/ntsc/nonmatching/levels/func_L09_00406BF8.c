/* func_L09_00406BF8, 108 bytes, 1 placement (level 9).  Best candidate 112 bytes against
   retail's 108 (frame 0x20 in both, this one saves $s2 as well).
   Retail keeps the outgoing kind in $a1 - it never spills it - and reaches the `kind = 2`
   arm with `beqz $v0` plus a `b`; this cc1 spills both locals ($s1 = kind, $s2 = ret) and
   turns the test into `beql` with `addiu $s1,$zero,0x2` in the annulled slot (the same
   annulled-slot idiom the README records for func_L12_00427C58 / func_L05_0037BB28).
   The body: */

typedef unsigned char u8;
typedef signed int s32;

extern char D_L09_00189EA0[];
extern s32 func_325E78(char *a0, s32 a1);
extern void func_L09_00406BA0(s32 a0, u8 a1);

s32 func_L09_00406BF8(s32 a0, s32 a1) {
    s32 ret = 0;
    s32 kind = 0;

    if (a0 != -1) {
        if (func_325E78(D_L09_00189EA0, a0) != 0) {
            ret = 1;
        } else {
            kind = 2;
        }
    }
    func_L09_00406BA0(a1, kind);
    return ret;
}

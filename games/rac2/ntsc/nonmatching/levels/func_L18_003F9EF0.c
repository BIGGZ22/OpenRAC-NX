/* func_L18_003F9EF0 (188 bytes, 3 places) -- NEAR MISS, not integrated.  Needs the
   -G8 unit: four small-data tables (int pairs at gp+0x14F0/0x14F8, 16-byte records
   at gp+0x14B0/0x14D0).
   Per-frame state machine on the short counter at +0xAC and index at +0xAE: when
   the two int tables sum below the counter it resets both, calls func_003355A8(0)
   and the local func_L18_002E4090(0) and sets the index to -1; otherwise, when the
   first table alone is below the counter, it calls func_L01_0042A6F0(1, 0); then it
   runs func_00335370/func_003353C8 on the indexed 16-byte records and bumps the
   counter.
   The candidate is 200 bytes against 188: every call, both table reads, the -1
   store and the reloaded counter increment are right, but keeping the index in a C
   local across the two tail calls costs a second callee-saved register (this cc1
   saves s1, retail keeps the single s0 and reloads the index `lh $a0,0xAE($s0)`).
   Two spellings with the reloads written inline instead come out at 204. */
#include "common.h"

typedef struct L18_003F9EF0_F16 {
    char pad[0x10];
} L18_003F9EF0_F16;

extern s32 D_L18_001B04E0[] __attribute__((sda));
extern s32 D_L18_001B04E8[] __attribute__((sda));
extern L18_003F9EF0_F16 D_L18_001B04A0[] __attribute__((sda));
extern L18_003F9EF0_F16 D_L18_001B04C0[] __attribute__((sda));
extern void func_003355A8(s32);
extern void func_L18_002E4090(s32);
extern void func_L01_0042A6F0(s32, s32);
extern void func_00335370(L18_003F9EF0_F16 *);
extern void func_003353C8(L18_003F9EF0_F16 *);

void func_L18_003F9EF0(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);
    s16 n = *(s16 *)(s0 + 0xAC);
    s16 idx = *(s16 *)(s0 + 0xAE);

    if (D_L18_001B04E0[idx] + D_L18_001B04E8[idx] < n) {
        *(s16 *)(s0 + 0xAC) = 0;
        func_003355A8(0);
        func_L18_002E4090(0);
        *(s16 *)(s0 + 0xAE) = -1;
    } else if (D_L18_001B04E0[idx] < n) {
        func_L01_0042A6F0(1, 0);
    }
    func_00335370(&D_L18_001B04A0[idx]);
    func_003353C8(&D_L18_001B04C0[idx]);
    *(u16 *)(s0 + 0xAC) = *(u16 *)(s0 + 0xAC) + 1;
}

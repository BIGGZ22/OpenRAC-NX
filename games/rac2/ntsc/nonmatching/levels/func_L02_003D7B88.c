/* func_L02_003D7B88 (148 bytes, 8 places) - best attempt, NOT MATCHED (register wall).
 *
 * Docker-free ruler (scratch/wslcmp.py): "02_maktar_nebula: 140 bytes, expected 148" with the
 * guard first, 136 with the context loaded first.  The sibling func_L02_003DBEC8's body plus
 * the two level lookups is understood and reproduces retail's instruction sequence and store
 * order:
 *   q = *(char **)(a0 + 0x68)                     (the live index rides at +4, the id at +0)
 *   *(u8 *)(a0 + 0x30) = 0x80 ; *(f32 *)(q + 0xC) = -1.0f
 *   e = D_001FEAC0[*(s32 *)(q + 4)]               (lui/addiu 0x20/-0xE40, lw, sll 2, addu)
 *   *(s32 *)(q + 0x8)  = *(s32 *)(D_001B359C + (*(s32 *)q << 8) + 0x68)   (absolute lui/lw)
 *   *(s32 *)(q + 0x14) = 0 ; *(char **)(q + 0x18) = e ; *(TI128 *)(a0 + 0x10) = *(TI128 *)(e + 0x10)
 *   func_002A7750(a0, 1, -1)
 * Residue, all allocation:
 *   (a) the parameter copy lands in $t2 (guard first) or the context in $t1 (load first);
 *       retail keeps `daddu $t1,$a0,$zero` and `lw $t0,0x68($t1)`;
 *   (b) with the guard first this cc1 sinks the epilogue's `ld $ra` into the bnez delay slot
 *       (retail puts the context load there), which is where the eight missing bytes go.
 * Both level symbols must stay UNBOUND (`D_001FEAC0[]`, `D_001B359C`) because the function has
 * eight places; retail loads 0x1B359C absolutely, so it must not carry the sda attribute.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern char *D_001FEAC0[];
extern char *D_001B359C;
extern void func_002A7750(void *, int, int);

void func_L02_003D7B88(char *a0)
{
    char *q;
    char *e;
    if (*(u8 *)(a0 + 0x20) != 0) {
        return;
    }
    q = *(char **)(a0 + 0x68);
    *(u8 *)(a0 + 0x30) = 0x80;
    *(f32 *)(q + 0xC) = -1.0f;
    e = D_001FEAC0[*(s32 *)(q + 4)];
    *(s32 *)(q + 0x8) = *(s32 *)(D_001B359C + (*(s32 *)q << 8) + 0x68);
    *(s32 *)(q + 0x14) = 0;
    *(char **)(q + 0x18) = e;
    *(TI128 *)(a0 + 0x10) = *(TI128 *)(e + 0x10);
    func_002A7750(a0, 1, -1);
}

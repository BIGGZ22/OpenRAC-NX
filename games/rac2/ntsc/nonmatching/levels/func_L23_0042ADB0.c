/* func_L23_0042ADB0 (880 bytes, 1 place: 23:0042ADB0) -- NEAR MISS, not integrated.
   This C compiles to 824 bytes against retail's 880 (-g8 unit, -O2 -G8): 56 bytes short, 77 of
   retail's 220 words match in place. The structure is retail's: the guard on D_L23_001B1484,
   the angle blend, the sine of (counter % period) / period * pi, the packed 64-bit word, the
   two engine calls, the vector setup, and the 40 x 4 loop (retail's inner loop shape: counted
   down, four pointer inductions, the two spilled $sp+0x50 / $sp+0x54 row pointers reloaded in
   the outer header, `i+1` copied early into $s6) all compile to the same instruction kinds.
   What it is: the per-frame callback of the orbit camera ship (func_L23_0042ACC0 hands it to
   func_L00_002E6CF0). It blends payload[0] toward the angle D_L23_001B1480 * pi/10800 (func_00283B98),
   stores D_L23_001B1470 * 1.1f into D_L23_001B39D8, derives an oscillating amount from
   obj[0x96] % D_L23_001B1474, packs the five measured words into one 64-bit register value,
   asks func_0027C878 for the second, builds the 0x90-byte `Quad` (four vectors, four ints,
   four uv pairs, four doublewords) on the stack and draws it 40 times through func_00280B90
   with the matrix assembled in m[3].
   What differs:
   1. the eight 16-byte copies (m[0..2] from obj, m[0..2] from the state, v3, v5): retail `addiu`
      for each source and destination pointer, then `lq $v0,0($p) ; sq $v0,0($q)`, in strict
      source order (the stack pointers reused across the two matrix copies);
      here `lq`/`sq` with folded offsets and no pointer words (eleven `addiu` missing).
   2. the zero of the stack vector: retail `por $v0,$zero,$zero ; sq $v0,0xE0($sp)` (+0x1E4) and a
      second zero through a materialised pointer, `addiu $v0,$sp,0xE0 ; sq $zero,0($v0)` (+0x228);
      this cc1 folds both into `sq $zero,0xE0($sp)` (rac2_fold_zero_ti_store: the register is dead
      after the store). Retail's two forms differ exactly by sp-based vs register-based address.
   3. `div $zero,$v1,$v0 ; beql $v0,$zero,.+8 ; break 0,7 ; nop ; nop ; div.s` in retail (the
      division first, the check after, two padding nops after the label); here the check comes
      first and one `nop` is emitted (the known division-padding wall).
   4. frame 0x170 against retail's 0x190: retail has two more 16-byte slots at $sp+0x100..0x11F
      (spills are at 0x120/0x124); the vectors are laid out as retail's (v3 0xC0, v5 0xD0, v4 0xE0,
      x 0xF0) only because each address is taken early through `TI128 *pv3 = &v3` etc.: slots are
      handed out in the order the address is first taken.
   Notes: D_L23_001B39D8 is stored through $gp (`swc1 $f1,0x49E8($gp)`) but loaded absolutely
   (`lui $at,0x1B ; lwc1 $f12,0x39D8($at)`), so it needs two names (`D_L23_001B39D8` sda and
   `D_L23_001B39D8_2` plain, the same address). The five sda words D_L23_001B1C68/6C/70/74/7C are
   `lw ..($gp)` (offsets 0x2C78, 0x2C7C, 0x2C80, 0x2C84, 0x2C8C), D_L23_001B1C80 and
   D_L23_001B1C84 likewise; D_L23_001B1470..147C are absolute. The two-name table pair
   D_L23_002A8460 / D_L23_002A8464 (four bytes apart) must stay two symbols: retail does not
   derive the second base from the first (`lui $a1,0x2B ; addiu $a1,$a1,-0x7B9C` inside the
   inner loop). */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef unsigned long long u64;
typedef int TI128 __attribute__((mode(TI)));

typedef struct {
    float v[4][4];          /* 0x00 */
    s32 c[4];               /* 0x40 */
    float uv[4][2];         /* 0x50 */
    u64 w[4];               /* 0x70 */
} Quad;

typedef struct { s16 a; s16 b; } Pair;

extern u8 *D_L23_001B1484_2;
extern s32 D_L23_001B1480;
extern float D_L23_001B1470;
extern s32 D_L23_001B1474;
extern float D_L23_001B1478;
extern s32 D_L23_001B147C;
extern float D_L23_001B39D8 __attribute__((sda));
extern float D_L23_001B39D8_2;
extern s32 D_L23_001B1C68 __attribute__((sda));
extern s32 D_L23_001B1C6C __attribute__((sda));
extern s32 D_L23_001B1C70 __attribute__((sda));
extern s32 D_L23_001B1C74 __attribute__((sda));
extern s32 D_L23_001B1C7C __attribute__((sda));
extern s32 D_L23_001B1C80 __attribute__((sda));
extern float D_L23_001B1C84 __attribute__((sda));
extern float D_L23_002A7F50[][4];
extern float D_L23_002A8460[][2];
extern float D_L23_002A8464[][2];
extern Pair D_L23_002A81E0[][4];

extern float func_00283B98(float, float);
extern float func_00283198(float);
extern u64 func_0027C878(s32);
extern void func_002FC560(u16, u64);
extern void func_L22_002F3B38(void *, void *);
extern void func_00282CC0(void *, void *, void *);
extern void func_002833F0(void *, void *);
extern void func_00283738(void *, void *, void *);
extern void func_00282D30(void *, void *, float);
extern void func_00280B90(void *, void *, s32);

void func_L23_0042ADB0(u8 *obj) {
    Quad q;
    TI128 m[3];
    TI128 v3;
    TI128 v5;
    TI128 v4;
    TI128 x;
    TI128 *pv3 = &v3;
    TI128 *pv5 = &v5;
    TI128 *pv4 = &v4;
    u8 *state = D_L23_001B1484_2;
    u8 *payload;
    s32 n;
    s32 a2;
    s32 i;
    s32 j;

    if (state != 0) {
        payload = *(u8 **)(obj + 0x68);
        D_L23_001B39D8 = D_L23_001B1470 * 1.1f;
        *(float *)payload = func_00283B98(*(float *)payload, (float)D_L23_001B1480 * 0.00029088825f);
        n = D_L23_001B1474;
        a2 = (s32)(func_00283198((float)(s32)(*(u16 *)(obj + 0x96) % n) * (1.0f / (float)n) * 3.1415927f) * D_L23_001B1478) + D_L23_001B147C;
        *(s32 *)(payload + 4) = a2;
        q.w[3] = (u64)D_L23_001B1C68 | ((u64)D_L23_001B1C6C << 2) | ((u64)D_L23_001B1C70 << 4)
                 | ((u64)D_L23_001B1C74 << 6) | ((u64)a2 << 32);
        q.w[1] = func_0027C878(D_L23_001B1C7C);
        q.w[2] = 0xFF9000000260ULL;
        q.w[0] = 0;
        func_002FC560(0x47, 0x53001);
        m[0] = *(TI128 *)(obj + 0xC0);
        m[1] = *(TI128 *)(obj + 0xD0);
        m[2] = *(TI128 *)(obj + 0xE0);
        m[0] = *(TI128 *)(D_L23_001B1484_2 + 0xC0);
        m[1] = *(TI128 *)(D_L23_001B1484_2 + 0xD0);
        m[2] = *(TI128 *)(D_L23_001B1484_2 + 0xE0);
        *pv3 = *(TI128 *)(D_L23_001B1484_2 + 0x10);
        *pv4 = 0;
        ((float *)pv4)[3] = 1.0f;
        ((float *)pv4)[2] = D_L23_001B1C84;
        *pv5 = *pv4;
        func_L22_002F3B38(pv5, pv5);
        func_00282CC0(pv3, D_L23_001B1484_2 + 0x10, pv5);
        *pv4 = 0;
        ((float *)pv4)[2] = *(float *)payload;
        func_002833F0(&x, pv4);
        func_00283738(m, m, &x);
        ((float *)pv3)[3] = 1.0f;
        q.c[0] = D_L23_001B1C80;
        q.c[1] = D_L23_001B1C80;
        q.c[2] = D_L23_001B1C80;
        q.c[3] = D_L23_001B1C80;
        for (i = 0; i < 40; i++) {
            for (j = 0; j < 4; j++) {
                func_00282D30(&q.v[j], D_L23_002A7F50[D_L23_002A81E0[i][j].a], D_L23_001B39D8_2);
                q.uv[j][0] = D_L23_002A8460[D_L23_002A81E0[i][j].b][0];
                q.uv[j][1] = D_L23_002A8464[D_L23_002A81E0[i][j].b][0];
            }
            func_00280B90(&q, m, 0);
        }
    }
}

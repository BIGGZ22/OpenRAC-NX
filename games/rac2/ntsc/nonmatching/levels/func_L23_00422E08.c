/* func_L23_00422E08 (level 23, 1380 bytes) -- NEAR MISS, not integrated.
   state: 1380 bytes against retail's 1380 (345 instructions each); tools/check.py: 0 of 1 places
   match (instruction at +0x24 differs); 63 of 345 words differ once the two bodies are aligned.
   built as part of L23_0041E418-g8.c (-G8): the six steering constants are $gp floats, and the
   function is the only use of each name below.
   matches: the 0x160 frame and the local layout (vectors at sp+0x20/0x30, the 3x4 matrix at sp+0x40,
   the quaternion at sp+0x70, vectors at sp+0x80..0xB0, the collision record at sp+0xC0), the
   prologue, every call with its arguments, the cross-jumped tail the two func_L00_00320B70 calls
   share, the collision and class tests, the record stores (order found by trying all 5040), and
   the func_002ABBB8 set-up. D_L23_001A8F20 has to be a bare symbol (sda, sized past the $gp limit):
   as a plain extern its %hi is kept by GCSE in a saved register across five calls, where retail
   rematerialises `lui $a1,0x1B; addiu $a1,$a1,-0x70E0` at both calls; the bare form gives
   retail's pair and puts &lA0/&l90/dir in retail's $s2/$s3/$s6.
   what still differs: the four 16-byte copies. Retail moves each through registers at offset 0:
   `lq $v0,0($s3); sq $v0,0($s6)` (path 3), `addiu $s0,$s5,0x10; lq $v0,0($s0); sq $v0,0($s2)`
   (the join), `lq $v0,0($s0); sq $v0,0($s3)`, and `addiu $a0,$a0,0x20; lq $v0,0($a0);
   sq $v0,0($s1)` (the hit position). This cc1 folds every offset (`lq 0x90($sp)`, `sq 0xC0($s5)`,
   `lq 0x10($s5)`, `lq 0x20($a0)`, `sq 0x10($s5)`): cse's find_best_addr prefers reg+offset on the
   address-cost tie whenever the pointer was set in the same extended block, which is the case for
   all four here. Knock-on effects of the fold: obj and &lB0 swap $s5/$s4 (the folded copies give
   obj four more references), the copy of obj+0x10 kept for the collision code lands in $s6 early
   instead of $s1 at the func_00282CC0 call, reorg steals the folded `lq` of the join into a beql
   at the `< 0x91` test (retail: beqz with `daddu $a0,$s3,$zero`), and two argument set-ups
   (path 2 of func_L00_00320B70, func_002C65A0) are scheduled one move apart.
   tried: pointer variables for every vector (1388 bytes), pointers only for the join copies'
   destinations (1400 bytes), the join copies through a pointer taken before the branches (107
   words off), a plain extern for D_L23_001A8F20 (74 off), all 210 int/float interleavings of the
   func_L00_00320B70 prototype and all 165 of func_002C65A0 (one word better at most), a second
   variable for the second hit read (gives retail's $v0 there; kept).
   what would unblock it: the 16-byte copy fold (compiler side); the shape fits copies whose
   addresses are register operands, such as an inline-asm qcopy helper, which AGENTS.md rules out. */

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

typedef int s32;

typedef int TI16 __attribute__((mode(TI)));

extern void func_002E5CF8(int, int, u8 *);

extern void func_0029F338(u8 *object);

/* the steering constants of the homing shot */
extern float D_L23_001B1914 __attribute__((sda));

extern float D_L23_001B1918 __attribute__((sda));

extern float D_L23_001B191C __attribute__((sda));

extern float D_L23_001B1920 __attribute__((sda));

extern float D_L23_001B192C __attribute__((sda));

extern float D_L23_001B1930 __attribute__((sda));

/* read as a bare symbol through $at: sized past the $gp limit */
extern float D_L23_001B3700[3] __attribute__((sda));

extern u8 D_L23_001A8F20[0x20] __attribute__((sda));

/* the last hit reported by the collision query */
typedef struct {
    u8 pad[0x18];
    u8 *f18;
    s32 f1C;
    TI16 f20;
} L23Hit1C0A80;

extern L23Hit1C0A80 D_L23_001C0A80;

/* the record the collision query and the damage call read */
typedef struct {
    float v[4];
    u8 *obj;
    s32 flags;
    u8 b18;
    u8 b19;
    u16 h1A;
    float f1C;
    s32 f20;
} L23Rec;

extern void func_002AFFD8(void *, void *, s32);

extern void func_002AA2E8(float *, float, float);

extern void func_00282F20(void *, void *, float);

extern void func_00282DD8(void *, void *, void *);

extern void func_00282D30(void *, void *, float);

extern float func_00282DF0(void *);

extern float func_00282C10(float);

extern void func_00283958(void *, void *);

extern void func_00283738(void *, void *, void *);

extern void func_00282CC0(void *, void *, void *);

extern void func_00282CF0(void *, void *, void *);

extern float func_00282DB0(void *, void *);

extern float func_002A7878(float, float);

extern s32 func_002A77E0(s32);

extern void func_002A7A00(void *, float, float);

extern s32 func_002761F8(void *, void *, s32, s32, void *);

extern s32 func_00282948(void *);

extern void func_002ABBB8(u8 *, void *, s32, s32, s32, s32, s32, s32, s32, s32, s32, s32,
                          float, float, float, float, float, float, float);

extern void func_00277F48(void *, s32, s32, void *, float);

extern void func_L00_00320B70(void *, void *, void *, void *, void *, void *, float, float, float, float);

extern void func_002C65A0(void *, void *, s32, s32, s32, s32, s32, s32, float, float, float);

void func_L23_00422E08(u8 *obj) {
    float up[4];
    float vel[4];
    float mtx[12];
    float quat[4];
    float axis[4];
    float a[4];
    float b[4];
    float c[4];
    L23Rec rec;
    u8 *vars;
    u8 *hit;
    u8 *other;
    float len;
    float r;
    s32 n;
    s32 flag;
    u16 cls;

    *(u16 *)(obj + 0x34) &= 0xFFBE;
    vars = *(u8 **)(obj + 0x68);
    func_002AFFD8(obj + 0x10, up, 0);
    func_002AA2E8((float *)(vars + 0x1C), 2.3333335f, 0.055555556f);
    func_00282F20(vel, obj + 0xC0, *(float *)(vars + 0x1C));
    func_00282F20(axis, up, 1.0f);
    func_00282DD8(quat, obj + 0xE0, axis);
    func_00282D30(quat, quat, D_L23_001B192C * 0.5f);
    len = func_00282DF0(quat);
    quat[3] = -func_00282C10(1.0f - len * len);
    func_00283958(quat, mtx);
    func_00283738(obj + 0xC0, mtx, obj + 0xC0);
    if (*(u8 **)(vars + 0x18) != 0) {
        if (vars[0x28] != 0) {
            func_00282D30(c, *(u8 **)(vars + 0x18) + 0xE0, D_L23_001B1930);
            func_00282CC0(b, *(u8 **)(vars + 0x18) + 0x10, c);
            func_L00_00320B70(obj + 0x10, obj + 0xC0, b, vars + 0x20, up, a,
                              D_L23_001B1914, D_L23_001B1918, 0.0f, D_L23_001B1920);
            func_00282F20(obj + 0xC0, obj + 0xC0, 1.0f);
            func_00282D30(vel, obj + 0xC0, *(float *)(vars + 0x1C));
        } else if (vars[0x29] != 0) {
            if (*(s32 *)(vars + 0x14) < 0x91) {
                func_00282CF0(a, obj + 0x10, *(u8 **)(vars + 0x2C));
                if (func_00282DB0(a, vel) < 0.0f) {
                    func_L00_00320B70(obj + 0x10, obj + 0xC0, *(u8 **)(vars + 0x2C), vars + 0x20, up, b,
                                      D_L23_001B1914, D_L23_001B1918, 0.0f, D_L23_001B191C);
                    func_00282F20(obj + 0xC0, obj + 0xC0, 1.0f);
                    func_00282D30(vel, obj + 0xC0, *(float *)(vars + 0x1C));
                }
            }
        } else {
            func_00282D30(vel, vars + 0x30, *(float *)(vars + 0x1C));
            func_00282F20(a, vel, 1.0f);
            *(TI16 *)(obj + 0xC0) = *(TI16 *)a;
        }
    }
    *(TI16 *)b = *(TI16 *)(obj + 0x10);
    func_00282D30(c, vel, 0.7f);
    r = func_002A7878(0.1f, 0.3f);
    n = func_002A77E0(0xFF);
    func_002A7A00(a, 0.0f, 0.1f);
    func_00282CC0(a, a, b);
    func_002C65A0(a, D_L23_001A8F20, 0xF, 0x808080, n, 0x20, 0, 0, r * 210000.0f, r * 42000.0f, 0.0f);
    func_00282CC0(b, b, c);
    *(TI16 *)a = *(TI16 *)(obj + 0x10);
    func_00282CC0(obj + 0x10, obj + 0x10, vel);
    if (*(float *)(obj + 0x10) < 0.0f || *(float *)(obj + 0x14) < 0.0f || *(float *)(obj + 0x18) < 0.0f) {
        func_0029F338(obj);
        return;
    }
    rec.obj = obj;
    rec.flags = 0x8B0003;
    rec.f1C = D_L23_001B3700[0];
    rec.f20 = 0;
    rec.b18 = 3;
    rec.b19 = 3;
    rec.h1A = *(u16 *)(obj + 0xAA);
    if (obj[0xBC] == 0 && func_002761F8(a, obj + 0x10, 0x10, *(s32 *)(vars + 0x10), &rec) != 0) {
        hit = D_L23_001C0A80.f18;
        if (hit != 0) {
            if (hit != *(u8 **)(obj + 0xB8)) {
                obj[0xBC] = 2;
                *(TI16 *)(obj + 0x10) = D_L23_001C0A80.f20;
            }
        } else if (D_L23_001C0A80.f1C > 0) {
            obj[0xBC] = 1;
            *(TI16 *)(obj + 0x10) = D_L23_001C0A80.f20;
        }
        flag = 0;
        other = D_L23_001C0A80.f18;
        if (other != 0) {
            cls = *(u16 *)(other + 0xAA);
            if (cls == 0xF7E || cls == 0xF7F || (s16)cls == 0x120F) {
                func_002E5CF8(2, 0, obj);
                flag = 1;
            }
        }
        if (flag == 0) {
            func_002E5CF8(0, 0, obj);
        }
    } else if (func_00282948(vars + 0x14) != 0) {
        func_0029F338(obj);
        return;
    }
    if (obj[0xBC] != 0) {
        func_002ABBB8(obj, D_L23_001A8F20, 0, 2, 1, 4, -1, 0, 0, -1, 0, 0x810001,
                      0.0f, 0.0f, 1.5f, 1.0f, 9.0f, 1.0f, 15.0f);
        func_00277F48(obj + 0x10, 0, *(s32 *)(vars + 0x10), &rec, 2.0f);
        func_0029F338(obj);
    }
}

/* func_L23_00422668 (level 23, 300 bytes) -- NEAR MISS, not integrated.
   state: 296 bytes against retail's 300 (74 instructions against 75); check.py: 0 of 1 places match.
   matches: the 0x70 frame and local layout (the vector at sp+0, a 0x30-byte record at sp+0x10 whose
   address stays in a saved register across the two calls that take it), every call and argument, the
   record's byte and half stores at sp+0x28, and the shared `arg0[0x20] = 2` tail.
   what still differs: the else branch. Retail tests `beqz $v0` with a nop in the slot and copies the
   vector with `lq $v0,0($s1)` through the saved arg0+0x10; this cc1 folds the source to
   `lq $v0,0x10($s0)` and reorg moves it into a `beql` slot (the 4-byte gap). With that use gone,
   arg0+0x10 takes $s4 instead of $s1, so the payload, &record and &D_L23_001C0A80 sit one register
   lower ($s1/$s2/$s3 against $s2/$s3/$s4) and the prologue saves in another order.
   tried: a pointer variable for arg0+0x10 used by the first call, the copy and the last call (CSE folds
   it back: same bytes); the vector as a TI16 local (ADDRESSOF puts it on the stack after the record:
   frame 0x60, the locals swapped, its address in a saved register).
   what would unblock it (a hypothesis, compiler side, not a source shape): three retail habits that this
   cc1 never shows - 16-byte copies through a materialised address register (here and the `sq $v0,0($a2)`
   of func_L23_00422540), a copy that reorg does not move into a delay slot, and `$sN = $sp+N` pointers
   kept across calls - all fit a BLKmode struct assignment that the retail compiler emitted as one lq/sq
   block move through register addresses. This cc1 caps block moves at 8-byte alignment: 16-, 32- and
   64-byte structs aligned to 16 all copy with ld/sd pairs at folded offsets, so only TI16 casts reach
   lq/sq, and TImode MEMs fold. A by-value struct argument (EABI: the callee copies it) does give retail's
   `addu $18,$sp,16` / `l.s $f13,4($18)` shape (func_L02_003CED70) apart from those ld/sd copies. About
   370 level functions keep a `$sp+N` address in a saved register and read it back with lq; none of them
   is matched yet.
*/

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;

typedef int TI16 __attribute__((mode(TI)));

extern void func_00282F20(float *, float *, float);

typedef struct {
    u8 pad[0x18];
    u8 *f18;
} L23Block1C0A80;

extern L23Block1C0A80 D_L23_001C0A80;

/* a 16-byte vector, aligned for the quadword copies */
typedef struct {
    float v[4];
} L23Vec __attribute__((aligned(16)));

/* the 48-byte record func_002A8C00 fills and func_002A8E20 reads */
typedef struct {
    u8 pad[0x18];
    u8 b18;
    u8 b19;
    u16 h1A;
    u8 pad1C[0x14];
} L23Hit;

extern u8 *D_0018C0B0;

extern s32 func_002761F8(void *, void *, s32, void *, s32);

extern void func_002A8C00(float, L23Hit *, u8 *, s32, L23Vec *);

extern void func_002A8E20(u8 *, L23Hit *);

extern void func_L00_002D6900(s32);

extern void func_L22_0040E2B0(void *, s32, s32);

/* the object's first record is set up against the table entry, then its marker byte is set */
void func_L23_00422668(u8 *arg0, void *arg1) {
    u8 *payload = *(u8 **)(arg0 + 0x68);
    L23Vec dir;
    L23Hit hit;
    u8 *obj;

    if (func_002761F8(arg1, arg0 + 0x10, 0, arg0, 0) == 0) {
        return;
    }
    obj = D_L23_001C0A80.f18;
    if (obj != 0) {
        if (*(s16 *)(obj + 0xAA) == 0x7E5) {
            return;
        }
        func_00282F20(dir.v, (float *)payload, 1.0f);
        func_002A8C00(*(float *)(payload + 0x1C), &hit, arg0, 0x8B0003, &dir);
        hit.b18 = 5;
        hit.b19 = 3;
        hit.h1A = *(u16 *)(arg0 + 0xAA);
        if (D_L23_001C0A80.f18 == D_0018C0B0) {
            func_002A8E20(D_L23_001C0A80.f18, &hit);
            func_L00_002D6900(0x2B);
        }
        func_L22_0040E2B0(arg0 + 0x10, 0xA, 1);
    } else {
        *(TI16 *)&dir = *(TI16 *)(arg0 + 0x10);
        func_L22_0040E2B0(&dir, 0xF, 0);
    }
    arg0[0x20] = 2;
}

/* func_L23_00394630 (776 bytes, level 23 only, -G0 unit L23_0035CE90.c) -- NEAR MISS, not integrated.
   state: 744 bytes against retail's 776, about 100 words differ (tools/check.py, -O2 -G0). It is the level-23 twin of
   nonmatching/levels/func_L00_0038DB68.c (776 bytes too; same frame struct, same call list), with D_L23_001C0A80,
   D_L23_00266A70 and D_L23_001AC790 for D_L00_001BF200, D_L00_00262570 and D_L00_001AC850. The control flow, the blocks, the
   call order and the constants match (0.0003055556f is the spelling for 0x39A032FA; 5627.925f gives 0x45AFDF66).
   Walls, all of one kind: retail's TImode accesses never carry an offset: every lq/sq is `0($reg)` and the pointer is
   materialised, here three sites that exist only for the quad copy (`addiu $a0,$s0,0x20 ; lq $v0,0($a0)`,
   `addiu $v1,$s1,0x10 ; sq $v0,0($v1)` at +0x1D4 and `addiu $v1,$s0,0x20 ; lq` at +0x22C) and several that also feed a call
   ($s5 = &frame.old and $s3 = obj+0x10 are kept across the first three calls only because the quad copies use them).
   This cc1 folds the offset (`lq $v0,0x10($s1)`, `sq $v0,0x20($sp)`, `lq $v1,0x20($s0)`), so no $s5, a 0xB0 frame instead of
   0xC0, and the register assignment of everything after it moves. The `por $v0,$zero,$zero` also lands in $v1 for the same
   reason (the old-position copy is scheduled before the second zero store). The cse tie is the cause: ADDRESS_COST of
   (reg) and (plus reg small_int) are both 1 and find_best_addr then prefers the dearer rtx, i.e. the folded sum; retail's
   compiler most likely rejects (plus reg const) as a TImode address at expansion (a compiler-side lever, like the quad-save patches).
   Also not reproduced: `$s0 = &D_L23_001C0A80` kept as a callee-saved base (retail re-derives it with `addiu $s0,$a0,0xA80`
   in three delay slots from the shared `lui`), and the `$a0/$a1` pair of the D_00139574 * 0xE0 index (this cc1 uses $v1/$a0). */
typedef unsigned int VecBits __attribute__((mode(TI)));

typedef union {
    VecBits bits;
    float f[4];
} LocalVec;

typedef struct {
    char pad[0x18];
    char *hit;
    int count;
    LocalVec point;
    char gap[0x10];
    LocalVec normal;
} Collision;

extern unsigned char D_00139574;

extern char *D_0018C0B0;

extern int D_L23_001AC790 __attribute__((sda));

extern Collision D_L23_001C0A80;

extern char D_L23_00266A70[];

extern int func_002761F8(void *, void *, int, void *, void *);

extern int func_00282948(char *);

extern void func_00282CC0(void *, void *, void *);

extern void func_00282D30(void *, void *, float);

extern void func_00282F20(void *, void *, float);

extern void func_00282F70(void *, void *, float);

extern void func_00283028(void *, void *, void *);

extern void func_002AFFD8(void *, void *, int);

extern void func_002FEE50(int);

extern void func_L00_0038DE70(void *, void *, void *);

extern void func_L00_0038E880(void *, void *, void *);

extern void func_L00_0038F3D8(void *);

void func_L23_00394630(unsigned char *obj) {
    struct {
        LocalVec velocity, normal, old, surface;
        struct {
            LocalVec vector;
            char *moby;
            int flags;
            unsigned char kind, active;
            unsigned short class_id;
            float material;
            int enabled;
        } packet;
    } frame;
    char *ctx = *(char **)(obj + 0x68);
    char *position = (char *)obj + 0x10;
    char *hit;

    frame.velocity.bits = 0;
    frame.velocity.f[2] = 0.033333335f;
    frame.normal.bits = 0;
    frame.normal.f[2] = 1.0f;
    frame.old.bits = *(VecBits *)position;
    func_00282CC0(position, position, ctx);
    func_002AFFD8(position, &frame.surface, 0);
    func_00282D30(&frame.surface, &frame.surface, 0.0003055556f);
    func_L00_0038F3D8(obj);
    frame.packet.moby = (char *)obj;
    frame.packet.flags = 0x830000;
    if (*(int *)(ctx + 0x18) < 0x122)
        frame.packet.flags = 0x830001;
    frame.packet.enabled = 1;
    frame.packet.material = *(float *)(D_L23_00266A70 + D_00139574 * 0xE0 + 0x70);
    frame.packet.vector.bits = *(VecBits *)ctx;
    func_00282F70(&frame.packet.vector, &frame.packet.vector, 1.0f);
    frame.packet.class_id = *(unsigned short *)(obj + 0xAA);
    frame.packet.kind = 2;
    frame.packet.vector.f[3] = 5627.925f;
    frame.packet.vector.f[2] = 1.0f;
    frame.packet.active = 1;
    (*(int *)(ctx + 0x14))++;
    if (func_002761F8(&frame.old, position, 0x10, obj, &frame.packet)) {
        hit = D_L23_001C0A80.hit;
        if (hit) {
            if ((hit == D_0018C0B0 || *(short *)(hit + 0xAA) == 0xC0) && *(int *)(ctx + 0x18) >= 0x122)
                goto finish;
            if (*(int *)(D_L23_001C0A80.hit + 0x64) == D_L23_001AC790 && *(short *)(D_L23_001C0A80.hit + 0xAA) != 0x336)
                goto finish;
            obj[0xBC] = 1;
            *(VecBits *)(obj + 0x10) = D_L23_001C0A80.point.bits;
            func_00283028(&frame.velocity, ctx, &D_L23_001C0A80.normal);
            func_00282F20(&frame.velocity, &frame.velocity, 0.033333335f);
        } else {
            if (D_L23_001C0A80.count <= 0)
                goto finish;
            obj[0xBC] = 1;
            *(VecBits *)position = D_L23_001C0A80.point.bits;
            func_00283028(&frame.velocity, ctx, &D_L23_001C0A80.normal);
            func_00282F20(&frame.velocity, &frame.velocity, 0.033333335f);
        }
        func_00282F20(&frame.normal, &D_L23_001C0A80.normal, 1.0f);
    } else if (func_00282948(ctx + 0x18)) {
        obj[0xBC] = 1;
        func_00282CC0(&frame.velocity, &frame.velocity, ctx);
    }
finish:
    if (obj[0xBC]) {
        func_002FEE50(*(int *)(ctx + 0x1C));
        if (*(int *)(ctx + 0x14) < 20)
            func_L00_0038DE70(obj, &frame.velocity, &frame.normal);
        else
            func_L00_0038E880(obj, &frame.velocity, &frame.normal);
    }
}

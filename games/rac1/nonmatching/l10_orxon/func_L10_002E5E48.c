/* NON_MATCHING func_L10_002E5E48 -- src/overlays/l10_orxon/vendor_002E30F8.c
 * Best so far: BYTES 58/3804 (98.5% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Left: case 8 scheduling only. Before func_L00_0025F4A8: retail sets $s1=m+0x10 after $f14 and
 *   $f13 before $f12 (both 0.0 from $f20); before the two func_L00_00265050 calls retail stores the
 *   stack arg (sw $s0,0($sp)) first and moves $a0 earlier, ours puts sw in the jal delay slot.
 *   Same values, same calls, same order.
 *   Prototype note: func_L10_002CD578 is declared with its float last (vel, pos, a, b, c, scale);
 *   the definition (shared/vendor_00299AF0.c) has scale first. Floats travel in $f12 apart from the
 *   integer registers, so the call is the same; float-last is what put mov.s $f12 after $a3/$t0.
 *   sbs.py here is a side-by-side helper (reads obj.o, no compile).
 */
typedef struct Ship_5E48 {
    char p00[0x10];
    V_2E52B0 pos;
    unsigned char state;
    char p21[3];
    char *p24;
    char p28[4];
    float f2C;
    char p30;
    unsigned char b31;
    short s32;
    unsigned short s34;
    char p36[0xE];
    float f44;
    float f48;
    char p4C[6];
    unsigned char b52;
    unsigned char b53;
    char p54[4];
    float f58;
    char p5C[0x14];
    unsigned char b70;
    char p71[7];
    struct ShipD_5E48 *d;
    char p7C[3];
    unsigned char b7F;
    char p80[0x14];
    int i94;
    char p98[0xE];
    short sA6;
    unsigned short hA8;
    char pAA[6];
    unsigned char bB0;
    char pB1[0xB];
    unsigned char bBC;
} Ship_5E48;
typedef struct ShipD_5E48 {
    char p0[0x20];
    float f20;
    short s24;
    char p26[2];
    unsigned char b28;
    unsigned char b29;
    char p2A[0x16];
    char a40[0xB0];
    V_2E52B0 vF0;
    char p100[0x10];
    Ship_5E48 *tgt;
    int i114;
    char p118[8];
    V_2E52B0 a120;
    char p130[0x10];
    int i140;
    int i144;
    int i148;
    char p14C[0xC];
    int i158;
    char p15C[4];
    float f160;
    float f164;
    int i168;
    char p16C[0x2A];
    short s196;
    float f198;
    float f19C;
    char p1A0[0x10];
    int i1B0;
    char p1B4[0x1C];
    unsigned char a1D0[4];
} ShipD_5E48;
extern char D_L10_001DD690[];
extern Ship_5E48 *D_L10_00174358;
extern short D_L10_00161F64;
extern short D_L10_00161F68;
extern short D_L10_00161F6C;
extern float D_0015EE60 MACRO_ADDR;
extern char D_L10_00179910_2E9978[] __asm__("D_L10_00179910");
extern unsigned char D_0013D5CA_2E9978[] __asm__("D_0013D5CA");
extern void func_001E9730_5E48(char *, ...) __asm__("func_001E9730");
extern void func_L10_002E6D28(char *m);
extern int func_00215570(void *, int);
extern int func_L00_00203F20_2E9978(int a, int b) __asm__("func_L00_00203F20");
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern float func_001F9CB8(void *a);
extern void func_001F9C30(void *, void *, float);
extern int func_00215B18(char *, float);
extern void func_00215C00(void *, float, float, float);
extern void func_L03_00251AC8(void *, void *, int);
extern char *func_L10_002CD578_w(void *vel, void *pos, int a, int b, int c, float scale) __asm__("func_L10_002CD578");
extern void func_L00_002607A8(void *, float);
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);

/* Update for the blarg ship (moby class 1229): rise, follow its path, hover and fire at the target, fly off or explode. */
void func_L10_002E5E48(Ship_5E48 *m) {
    ShipD_5E48 *d = m->d;
    Ship_5E48 *tgt;
    V_2E52B0 A;
    V_2E52B0 B;
    V_2E52B0 C;
    V_2E52B0 E;
    V_2E52B0 D;
    float a;
    float b;

    func_L00_00264B40(2.5f, (int)m, 2, d->a1D0);
    if (d->i140 == -1) {
        func_001E9730_5E48(D_L10_001DD690, m->sA6, m->hA8);
        goto del;
    }
    func_L10_002E6D28((char *)m);
    tgt = d->tgt;
    switch (m->state) {
    case 0:
        m->s34 |= 0x1000;
        d->f20 = 9.0f;
        d->s24 = func_001FA898(9.0f);
        d->b28 = 3;
        d->b29 = 0;
        d->f19C = 0.0f;
        m->s32 = 0x7F;
        qcopy(&m->pos, &D_L10_001B0C30_2E52B0[d->i140]->nodes[0]);
        d->s196 = func_001F9850(*(int *)&D_L10_00161F64);
        m->i94 = 0;
        m->s34 |= 0x41;
        if (m->b53 != 0) func_00213DE0(m, 0, 0, 0);
        m->state = 1;
        if ((D_0014171B_t + 0xAA35)[m->bB0 + (D_0015EE84_m << 4)] != 0xFF) break;
        goto del;
    case 1:
        if (!func_00215570(&tgt->pos, d->i148)) break;
        m->state = 2;
        if (m->b53 != 1) func_00213DE0(m, 1, 0, func_001F9850(20));
        m->s34 &= 0xFFBE;
        m->i94 = *(int *)(m->p24 + 0x10);
        if (*(int *)D_L10_00179910_2E9978 == 0 && *(int *)(D_L10_00179910_2E9978 + 0x24) == -1
            && D_0013E633[0x2EC1] == 0 && *(unsigned short *)(D_0014171B + 0x62D) == 0
            && D_0013D5CA_2E9978[9] == 0) {
            func_L00_00203F20_2E9978(0x2715, 0x5C);
        }
        break;
    case 2:
        a = func_L00_001FF860(tgt->pos.x - m->pos.x, tgt->pos.y - m->pos.y);
        func_L00_002592B0((char *)m, &d->f198, a, 0.02f, 0.3f, D_0015EE6C * 3.1415927f);
        b = func_L00_001FF860(func_001F9D48(&m->pos, &tgt->pos), tgt->pos.z - m->pos.z);
        m->f44 = func_L00_00259148(&d->f19C, m->f44, -b, 0.02f, 0.3f, D_0015EE6C * 3.1415927f);
        if ((m->b70 & 2) && m->b53 == 1) func_00213DE0(m, 2, 0, func_001F9850(20));
        m->pos.z += D_0015EE6C * 4.0f;
        if (func_001F9D10(&m->pos, &D_L10_001B0C30_2E52B0[d->i140]->nodes[0]) < 0.5f) {
            m->state = 3;
            if (m->b53 != 2) func_00213DE0(m, 2, 0, func_001F9850(10));
        }
        break;
    case 3:
        qcopy(&A, &D_L10_001B0C30_2E52B0[d->i140]->nodes[d->i158]);
        a = func_L00_001FF860(A.x - m->pos.x, A.y - m->pos.y);
        func_L00_002592B0((char *)m, &d->f198, a, 0.02f, 0.3f, D_0015EE6C * 12.566371f);
        b = func_L00_001FF860(func_001F9D48(&m->pos, &tgt->pos), tgt->pos.z - m->pos.z);
        m->f44 = func_L00_00259148(&d->f19C, m->f44, -b, 0.02f, 0.3f, D_0015EE6C * 3.1415927f);
        func_001F9BF0_2E52B0(&B, &A, &m->pos);
        func_L00_001FF4B0_2E52B0(&d->a120, &B, D_0015EE6C * 8.0f);
        func_001F9BD8(&m->pos, &m->pos, &d->a120);
        if (func_001F9CB8(&B) < 0.5f) {
            int n = d->i158 + 1;
            if (n < D_L10_001B0C30_2E52B0[d->i140]->count) {
                d->i158 = n;
            } else {
                d->i158 = 0;
                m->state = 4;
                d->i168 = func_001FA898(func_001F9878(d->f160 * 60.0f));
            }
        }
        break;
    case 4:
        if (func_00215570(&((struct Hero *)(D_0013E633 + 0xE1D))->motion.pos, d->i1B0)) {
            m->state = 6;
            break;
        }
        *(u128_2E52B0 *)&A = *(u128_2E52B0 *)&tgt->pos;
        if (((struct Hero *)(D_0013E633 + 0xE1D))->unk20A4 != 1) {
            *(u128_2E52B0 *)&A = *(u128_2E52B0 *)&d->vF0;
            func_L00_001EFFF0(&m->pos, &A, 0, m, 0);
        } else {
            if ((Ship_5E48 *)((struct Hero *)(D_0013E633 + 0xE1D))->moby == tgt) {
                func_001F9C30(&C, &((struct Hero *)(D_0013E633 + 0xE1D))->motion.unk100, (float)func_001F9850(20));
                *(u128_2E52B0 *)&B = *(u128_2E52B0 *)&C;
                func_L00_001FF240(&C, &A, &B);
                A.z = func_00214358(&A, 0, 0.5f);
            }
            A.z += 0.2f;
            D_L10_00174358 = tgt;
        }
        a = func_L00_001FF860(tgt->pos.x - m->pos.x, tgt->pos.y - m->pos.y);
        func_L00_002592B0((char *)m, &d->f198, a, 0.02f, 0.3f, D_0015EE6C * 12.566371f);
        b = func_L00_001FF860(func_001F9D48(&m->pos, &A), A.z - m->pos.z);
        m->f44 = func_L00_00259148(&d->f19C, m->f44, -b, 0.02f, 0.3f, D_0015EE6C * 3.1415927f);
        if (func_001FA850(m->f48, func_L00_001FF860(tgt->pos.x - m->pos.x, tgt->pos.y - m->pos.y)) < 0.17453292f) {
            if (m->b53 != 3) func_00213DE0(m, 3, 0, func_001F9850(6));
            m->f58 = *(float *)&D_L10_00161F6C;
        }
        if (m->b52 == 3 && d->i114 != 2 && D_L10_00174358 == tgt) {
            int near = func_00215B18((char *)m, 1.0f);
            int far = func_00215B18((char *)m, 16.0f);
            if (near || far) {
                func_L00_00250800(m, far == 0, &C);
                func_00215C00(&B, 1.0f, m->f48, -m->f44);
                func_L00_001FF240(&D, &C, &B);
                func_001F9BF0_2E52B0(&B, &A, &C);
                func_L00_001FF4B0_2E52B0(&B, &B, *(float *)&D_L10_00161F68 * D_0015EE6C);
                func_001F9C30(&E, &B, func_001F9878(3.0f));
                func_L00_001FF240(&D, &E, &C);
                func_L03_00251AC8(m, &E, 0);
                func_L10_002CD578_w(&B, &C, (int)m, (int)tgt, func_001F9850(300), 1.0f);
            }
        } else if (m->b52 == 3 && m->b53 != 2) {
            func_00213DE0(m, 2, 0, func_001F9850(10));
        }
        if (func_001F9908(&d->i168)) {
            if (m->b53 != 2) func_00213DE0(m, 2, 0, func_001F9850(10));
            m->state = 5;
            d->i168 = func_001FA898(func_001F9878(d->f164 * 60.0f));
        }
        qcopy(&B, &D_L10_001B0C30_2E52B0[d->i144]->nodes[d->i158]);
        func_001F9BF0_2E52B0(&C, &B, &m->pos);
        func_L00_002607A8(&C, D_0015EE6C * 8.0f);
        func_001F9BD8(&m->pos, &m->pos, &C);
        if (func_001F9CB8(&C) < D_0015EE6C) {
            int n = d->i158 + 1;
            if (n < D_L10_001B0C30_2E52B0[d->i144]->count) d->i158 = n;
        }
        break;
    case 5:
        if (func_00215570(&((struct Hero *)(D_0013E633 + 0xE1D))->motion.pos, d->i1B0)) {
            m->state = 6;
            break;
        }
        *(u128_2E52B0 *)&A = *(u128_2E52B0 *)&tgt->pos;
        if (((struct Hero *)(D_0013E633 + 0xE1D))->unk20A4 != 1) {
            *(u128_2E52B0 *)&A = *(u128_2E52B0 *)&d->vF0;
            func_L00_001EFFF0(&m->pos, &A, 0, m, 0);
        } else {
            if ((Ship_5E48 *)((struct Hero *)(D_0013E633 + 0xE1D))->moby == tgt) {
                func_001F9C30(&C, &((struct Hero *)(D_0013E633 + 0xE1D))->motion.unk100, (float)func_001F9850(20));
                *(u128_2E52B0 *)&B = *(u128_2E52B0 *)&C;
                func_L00_001FF240(&C, &A, &B);
                A.z = func_00214358(&A, 0, 0.5f);
            }
            A.z += 0.2f;
            D_L10_00174358 = tgt;
        }
        a = func_L00_001FF860(tgt->pos.x - m->pos.x, tgt->pos.y - m->pos.y);
        func_L00_002592B0((char *)m, &d->f198, a, 0.02f, 0.3f, D_0015EE6C * 12.566371f);
        b = func_L00_001FF860(func_001F9D48(&m->pos, &A), A.z - m->pos.z);
        m->f44 = func_L00_00259148(&d->f19C, m->f44, -b, 0.02f, 0.3f, D_0015EE6C * 3.1415927f);
        if (func_001F9908(&d->i168)) {
            func_00213DE0(m, 3, 0, func_001F9850(10));
            m->f58 = *(float *)&D_L10_00161F6C;
            m->state = 4;
            d->i168 = func_001FA898(func_001F9878(d->f160 * 60.0f));
        }
        qcopy(&B, &D_L10_001B0C30_2E52B0[d->i144]->nodes[d->i158]);
        func_001F9BF0_2E52B0(&C, &B, &m->pos);
        func_L00_002607A8(&C, D_0015EE6C * 8.0f);
        func_001F9BD8(&m->pos, &m->pos, &C);
        if (func_001F9CB8(&C) < D_0015EE6C) {
            int n = d->i158 + 1;
            if (n < D_L10_001B0C30_2E52B0[d->i144]->count) d->i158 = n;
        }
        break;
    case 6:
        m->pos.z += D_0015EE6C * 10.0f;
        m->f2C *= D_0015EE60 * -0.015299976f + 1.0f;
        if (250.0f < m->pos.z) goto del;
        if (!(m->f2C < 0.01f)) break;
        goto del;
    case 7: {
        unsigned char next;
        if (m->b52 != 4) break;
        if (!(m->b70 & 2)) break;
        next = m->bBC;
        if (next == 4) {
            func_00213DE0(m, 3, 0, func_001F9850(10));
            m->f58 = *(float *)&D_L10_00161F6C;
            m->state = next;
            d->i168 = func_001FA898(func_001F9878(d->f160 * 60.0f));
        } else if (next == 6) {
            func_00213DE0(m, 2, 0, func_001F9850(10));
            m->state = next;
        } else {
            func_00213DE0(m, 2, 0, func_001F9850(10));
            m->state = 5;
            d->i168 = func_001FA898(func_001F9878(d->f164 * 60.0f));
        }
        break;
    }
    case 8: {
        float z = 0.0f;
        func_L00_0025F4A8(m, d->a40, &m->pos, z, z, 5, 2, 4, 2.0f, 1.0f, 9.0f, 1.0f, 5, 15.0f, 1, 1, -1, 0);
        func_L00_002584A8(m, 0, -1);
        func_L00_00265050((char *)m, 0x5FD, (float *)&m->pos, (char *)m + 0x40, 0, 0, z, D_L10_0015F660, D_L10_0015F660, D_L10_0015F660);
        func_L00_00265050((char *)m, 0x5FE, (float *)&m->pos, (char *)m + 0x40, 0, 0, z, D_L10_0015F660, D_L10_0015F660, D_L10_0015F660);
    del:
        func_0020D678(m);
        return;
    }
    }
    if (m->b31 != 0) {
        if (func_001F9D10(&m->pos, D_L10_001672C0) < 28.0f) {
            func_L00_0025B178(m);
            m->b7F = 0x16;
        }
    }
}

/* functional: medium. Per-frame update of a pickup/homing moby (states 0 idle, 1 wait, 2 chase one of up to three target mobys, 3 collect: delete the targets and set the saved-collected bits); the meaning of func_L09_00308F58 and of the 0x4/0x8/0xC/0x14 data slots is guessed, the operations follow the assembly. */
extern float angwrap_9E70(float, float) __asm__("func_001FA748");
extern float atan_9E70(float, float) __asm__("func_L00_001FF860");
extern float approach_9E70(float *, float, float, float, float, float) __asm__("func_L00_00259148");
extern float angdiff_9E70(float, float) __asm__("func_001FA850");
extern float sin_9E70(float) __asm__("func_001F9F90");
extern float cos_9E70(float) __asm__("func_001F9FA8");
extern void add3_9E70(void *, void *, void *) __asm__("func_L00_001FF240");
extern float dist_9E70(void *, void *) __asm__("func_001F9D48");
extern void mk_9E70(void *, float, float, float) __asm__("func_00215C00");
extern int frames_9E70(int) __asm__("func_001F9850");
extern void fx_9E70(void *, void *, void *, int) __asm__("func_L09_00308F58");
extern void del_9E70(void *) __asm__("func_0020D678");
extern char *D_L10_00160058_9E70 __asm__("D_L10_00160058");
extern unsigned char D_L10_001BB9C0_9E70[] __asm__("D_L10_001BB9C0");
extern unsigned char D_0014171B_9E70[] __asm__("D_0014171B");
extern int D_0015EE84_9E70 __asm__("D_0015EE84");
extern int D_L10_001BAC60_9E70[] __asm__("D_L10_001BAC60");
extern float D_0015EE6C_9E70 __asm__("D_0015EE6C");
extern float D_0015EE70_9E70 __asm__("D_0015EE70");

void func_L10_002E9E70(char *m) {
    int *d = *(int **)(m + 0x78);
    int st = *(unsigned char *)(m + 0x20);
    unsigned short id;
    char *t;
    char *a;

    if (st == 1) return;
    if (st != 0 && st != 2 && st != 3) return;

    if (st == 3) {
        char *s0 = 0;
        char *c;
        if (d[1] != -1) {
            char *o = D_L10_00160058_9E70 + (d[1] << 8);
            if (o != 0 && *(unsigned char *)(o + 0x20) != 0xFE && *(unsigned char *)(o + 0x20) != 0xFD) s0 = o;
        }
        if (s0 != 0) {
            del_9E70(s0);
            c = s0;
        } else {
            c = s0;
            if (d[2] != -1) {
                char *o = D_L10_00160058_9E70 + (d[2] << 8);
                if (o != 0 && *(unsigned char *)(o + 0x20) != 0xFE && *(unsigned char *)(o + 0x20) != 0xFD) c = o;
            }
        }
        if (c != 0) del_9E70(c);
        id = *(unsigned short *)(m + 0xB2);
        *(int *)(D_0014171B_9E70 + 0xAB75 + (((short)id >> 5) * 4 + (D_0015EE84_9E70 << 8))) |= 1 << (id & 0x1F);
        id = *(unsigned short *)(m + 0xB2);
        D_L10_001BAC60_9E70[(short)id >> 5] |= 1 << (id & 0x1F);
        return;
    }

    id = *(unsigned short *)(m + 0xB2);
    if (D_L10_001BB9C0_9E70[(short)id + 0x454] != 0 ||
        ((*(int *)(D_0014171B_9E70 + 0xAB75 + (((short)id >> 5) * 4 + (D_0015EE84_9E70 << 8))) >> (id & 0x1F)) & 1)) {
        if (st == 0) {
            if (d[1] == -1) {
                del_9E70(m);
                return;
            }
            d[4] = 0;
            *(unsigned char *)(m + 0x20) = 1;
            return;
        }
        *(unsigned char *)(m + 0x20) = 3;
        return;
    }
    if (st == 0) {
        if (d[1] == -1) {
            del_9E70(m);
        } else {
            d[4] = 0;
            *(unsigned char *)(m + 0x20) = 1;
        }
        return;
    }

    /* state 2, not collected: pick the first live target among d[1], d[2], d[3] */
    t = 0;
    if (d[1] != -1) {
        char *o = D_L10_00160058_9E70 + (d[1] << 8);
        if (o != 0 && *(unsigned char *)(o + 0x20) != 0xFE && *(unsigned char *)(o + 0x20) != 0xFD) t = o;
    }
    a = t;
    if (a == 0) {
        if (d[2] != -1) {
            char *o = D_L10_00160058_9E70 + (d[2] << 8);
            if (o != 0 && *(unsigned char *)(o + 0x20) != 0xFE && *(unsigned char *)(o + 0x20) != 0xFD) a = o;
        }
    }
    t = a;
    if (t == 0) {
        if (d[3] != -1) {
            char *o = D_L10_00160058_9E70 + (d[3] << 8);
            if (o != 0 && *(unsigned char *)(o + 0x20) != 0xFE && *(unsigned char *)(o + 0x20) != 0xFD) t = o;
        }
    }

    if (t == 0) {
        *(unsigned char *)(m + 0x20) = 3;
        if (d[5] != -1) {
            *(unsigned char *)(D_L10_00160058_9E70 + (d[5] << 8) + 0x20) = 1;
            d[5] = -1;
        }
    } else {
        float rot, ang, df;
        float pi = 3.1415927f;
        float v[4];
        float w[4];
        float buf[4];
        *(float *)(m + 0x48) = angwrap_9E70(*(float *)(m + 0x48), pi);
        ang = atan_9E70(*(float *)(t + 0x10) - *(float *)(m + 0x10), *(float *)(t + 0x14) - *(float *)(m + 0x14));
        *(float *)(m + 0x48) = approach_9E70((float *)(d + 4), *(float *)(m + 0x48), ang,
                                              D_0015EE70_9E70 * 1.5707964f, D_0015EE70_9E70 * pi, D_0015EE6C_9E70 * 0.7853982f);
        ang = atan_9E70(*(float *)(t + 0x10) - *(float *)(m + 0x10), *(float *)(t + 0x14) - *(float *)(m + 0x14));
        df = angdiff_9E70(*(float *)(m + 0x48), ang);
        if (df < 0.04363323f) {
            float sc = 1.5f;
            float e20, e21, e4;
            v[0] = sin_9E70(*(float *)(m + 0x48)) * sc;
            v[1] = cos_9E70(*(float *)(m + 0x48)) * sc;
            v[2] = 0.0f;
            add3_9E70(w, v, m + 0x10);
            e20 = D_0015EE6C_9E70 * 40.0f;
            e21 = atan_9E70(*(float *)(t + 0x10) - v[0], *(float *)(t + 0x14) - v[1]);
            e4 = atan_9E70(dist_9E70(v, t + 0x10), *(float *)(t + 0x18) - v[2]);
            mk_9E70(buf, e20, e21, e4);
            fx_9E70(m, v, buf, frames_9E70(30));
            if (t == D_L10_00160058_9E70 + (d[1] << 8)) d[1] = -1;
            else if (t == D_L10_00160058_9E70 + (d[2] << 8)) d[2] = -1;
            else d[3] = -1;
        }
        *(float *)(m + 0x48) = angwrap_9E70(*(float *)(m + 0x48), pi);
    }
    if (d[0] != -1) {
        *(unsigned char *)(D_L10_00160058_9E70 + (d[0] << 8) + 0xBC) = 1;
        d[0] = -1;
    }
}

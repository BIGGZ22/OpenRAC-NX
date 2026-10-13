/* functional: medium. Update of a sliding gate (level 15), five states: 0 stores home, 1 waits for a trigger moby / volume / saved flag, 2 slides open along -rot.z (travel up to duration), 3 holds until the hero and the second position leave the volume, 4 slides closed. Not run against captured calls; retail quirk kept: closing with a voice slot owned by us but inactive stores the moby pointer in vars->sound. */
typedef struct Vars_C1D0 {
    float home[4];
    int vol_open;       /* 0x10 */
    int vol_close;      /* 0x14 */
    float travel;       /* 0x18 */
    int trigger;        /* 0x1C */
    int sound;          /* 0x20 */
    float duration;     /* 0x24 */
    int lock;           /* 0x28 */
} Vars_C1D0;

extern char *G_160058_C1D0 __asm__("D_L15_00160058");
extern char G_E633_C1D0[] __asm__("D_0013E633");
extern char G_4171B_C1D0[] __asm__("D_0014171B");
extern char G_1BBB40_C1D0[] __asm__("D_L15_001BBB40");
extern int G_1BADE0_C1D0[] __asm__("D_L15_001BADE0");
extern char G_167440_C1D0[] __asm__("D_L15_00167440");
extern int G_level_C1D0 __asm__("D_0015EE84");
extern float G_frame_C1D0 __asm__("D_0015EE6C");
extern int func_00215570_C1D0(void *, int) __asm__("func_00215570");
extern int func_0022ED80_C1D0(int, int, void *) __asm__("func_0022ED80");
extern void func_L00_0028EBF0_C1D0(int) __asm__("func_L00_0028EBF0");
extern float func_001F9F90_C1D0(float) __asm__("func_001F9F90");
extern float func_001F9FA8_C1D0(float) __asm__("func_001F9FA8");
extern void func_001F9BD8_C1D0(void *, void *, void *) __asm__("func_001F9BD8");

void func_L15_0029C1D0_impl(char *moby) __asm__("func_L15_0029C1D0");
void func_L15_0029C1D0_impl(char *moby) {
    Vars_C1D0 *d = *(Vars_C1D0 **)(moby + 0x78);
    float offset[4] __attribute__((aligned(16)));
    float pos[4] __attribute__((aligned(16)));
    int active;
    int slot;
    char *vol0 = G_E633_C1D0 + 0xE9D;     /* hero position (0x13F4D0) */
    char *voices = G_E633_C1D0 + 0x1D;    /* voice slots, 0x70 each */
    char *flags = G_4171B_C1D0 + 0xAB75;  /* saved flag words, 0x100 per level */
    unsigned char state = *(unsigned char *)(moby + 0x20);

    if (state > 4) {
        return;
    }
    switch (state) {
    case 0:
        qcopy(d->home, moby + 0x10);
        *(unsigned char *)(moby + 0x20) = 1;
        return;
    case 1: {
        short id;
        active = 0;
        d->travel = 0;
        if (d->trigger != -1) {
            char *other = G_160058_C1D0 + (d->trigger << 8);
            if (*(short *)(other + 0xA6) == 0x4B9 && *(unsigned char *)(other + 0x20) == 4) {
                active = 1;
            } else if (*(short *)(other + 0xA6) == 0x596) {
                active = (*(unsigned char *)(other + 0x20) == 4);
            }
        }
        if (d->lock != 0) {
            id = *(unsigned short *)(moby + 0xB2);
            if (((unsigned char *)G_1BBB40_C1D0)[id + 0x454] != 0) {
                active = 1;
            } else if ((*(int *)(flags + G_level_C1D0 * 0x100 + (id >> 5) * 4) >> (id & 31)) & 1) {
                active = 1;
            }
        }
        if (func_00215570_C1D0(vol0, d->vol_open) != 0) {
            *(unsigned char *)(moby + 0x20) = 2;
        } else if (active) {
            *(unsigned char *)(moby + 0x20) = 2;
        }
        return;
    }
    case 2:
        if (*(char **)(voices + d->sound * 0x70 + 0x88) != moby || *(unsigned char *)(voices + d->sound * 0x70 + 0x74) == 0) {
            d->sound = func_0022ED80_C1D0(0, 4, moby);
        }
        d->travel = d->travel + (d->duration / 0.6f) * G_frame_C1D0;
        if (d->duration < d->travel) {
            slot = d->sound;
            if (slot != -1) {
                if (*(char **)(voices + slot * 0x70 + 0x88) == moby && *(unsigned char *)(voices + slot * 0x70 + 0x74) != 0) {
                    func_L00_0028EBF0_C1D0(slot);
                }
            }
            d->travel = d->duration;
            d->sound = -1;
            if (d->trigger != -1 || d->lock != 0) {
                unsigned short id = *(unsigned short *)(moby + 0xB2);
                int *w = (int *)(flags + G_level_C1D0 * 0x100 + ((short)id >> 5) * 4);
                *w = *w | (1 << (id & 31));
                id = *(unsigned short *)(moby + 0xB2);
                G_1BADE0_C1D0[(short)id >> 5] |= 1 << (id & 31);
                *(unsigned char *)(moby + 0x20) = 5;
            } else {
                *(unsigned char *)(moby + 0x20) = 3;
            }
        }
        offset[0] = func_001F9F90_C1D0(*(float *)(moby + 0x48)) * -d->travel;
        offset[1] = func_001F9FA8_C1D0(*(float *)(moby + 0x48)) * -d->travel;
        offset[2] = 0.0f;
        func_001F9BD8_C1D0(pos, d->home, offset);
        qcopy(moby + 0x10, pos);
        return;
    case 3:
        if (func_00215570_C1D0(vol0, d->vol_close) != 0) {
            return;
        }
        if (func_00215570_C1D0(G_167440_C1D0, d->vol_close) != 0) {
            return;
        }
        *(unsigned char *)(moby + 0x20) = 4;
        return;
    case 4:
        if (*(char **)(voices + d->sound * 0x70 + 0x88) != moby || *(unsigned char *)(voices + d->sound * 0x70 + 0x74) == 0) {
            d->sound = func_0022ED80_C1D0(0, 4, moby);
        }
        d->travel = d->travel - (d->duration / 0.6f) * G_frame_C1D0;
        if (d->travel < 0.0f) {
            int snd = -1;
            slot = d->sound;
            if (slot != -1) {
                char *owner = *(char **)(voices + slot * 0x70 + 0x88);
                snd = (int)owner;
                if (owner == moby) {
                    if (*(unsigned char *)(voices + slot * 0x70 + 0x74) != 0) {
                        func_L00_0028EBF0_C1D0(slot);
                        snd = -1;
                    }
                } else {
                    snd = -1;
                }
            }
            d->travel = 0;
            d->sound = snd;
            *(unsigned char *)(moby + 0x20) = 1;
        }
        offset[0] = func_001F9F90_C1D0(*(float *)(moby + 0x48)) * -d->travel;
        offset[1] = func_001F9FA8_C1D0(*(float *)(moby + 0x48)) * -d->travel;
        offset[2] = 0.0f;
        func_001F9BD8_C1D0(pos, d->home, offset);
        qcopy(moby + 0x10, pos);
        if (func_00215570_C1D0(vol0, d->vol_open) != 0) {
            *(unsigned char *)(moby + 0x20) = 2;
        } else if (func_00215570_C1D0(G_167440_C1D0, d->vol_open) != 0) {
            *(unsigned char *)(moby + 0x20) = 2;
        }
        return;
    }
}

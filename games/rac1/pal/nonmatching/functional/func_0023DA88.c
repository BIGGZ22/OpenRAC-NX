/* functional: high. Same reads, writes and return value as retail, in the same order; the remaining difference is code shape (register choice, and the constant 0x18 reloaded at the loop bottom), not behaviour. */
typedef struct VbEntry { s64 unk0; s64 unk8; s32 unk10; s32 unk14; } VbEntry;
typedef struct VbWin { char pad0[0x10]; s32 unk10; s32 unk14; } VbWin;

// Advances the ring's entries past the window of ts (viBufModifyPts).
int func_0023DA88(ViBuf *vb, TimeStamp *ts) {
    VbWin *w = (VbWin *)ts;
    s32 t6 = 1;
    s32 t2, t8, a0, a1, a1_2, v0_2, v0_3, a1_3, v0_4, n, w14;
    VbEntry *e;

    t2 = (vb->f5C - vb->f58 + vb->ntbl) % vb->ntbl;
    t8 = vb->count << 11;
    if (vb->f58 > 0) {
        e = (VbEntry *)vb->tbl + t2;
        if (e->unk14 != 0) {
            w14 = w->unk14;
            if (w14 != 0) {
loop:
                a0 = e->unk10;
                a1 = w->unk10;
                if (((a0 + t8) - a1) % t8 < w14) {
                    a1_2 = e->unk14;
                    v0_2 = (a1 + w14) - a0;
                    v0_3 = (a1_2 < v0_2) ? a1_2 : v0_2;
                    a1_3 = a1_2 - v0_3;
                    e->unk14 = a1_3;
                    e->unk10 = (a0 + v0_3) % t8;
                    if (a1_3 == 0) {
                        if (e->unk0 >= 0) {
                            e->unk14 = 0;
                            e->unk0 = -1;
                            e->unk8 = -1;
                            e->unk10 = 0;
                        }
                        v0_4 = vb->f58 - 1;
                        vb->f58 = (v0_4 <= -1) ? 0 : v0_4;
                    }
                } else {
                    t6 = 0;
                }
                n = vb->ntbl;
                t2 = (t2 + 1) % n;
                if (t6 != 0) {
                    e = (VbEntry *)vb->tbl + t2;
                    if (e->unk14 != 0 && w->unk14 != 0) {
                        goto loop;
                    }
                }
            }
        }
    }
    return 0;
}

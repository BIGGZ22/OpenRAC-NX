/*
 * Moby list head (MobyUpdateLoop): builds the run list of this frame in
 * scratchpad, then returns the first list head.
 *
 * Clears the class-slot list heads/tails (0x70000000, 224 slots of 8 bytes),
 * the target list (0x70000800) and the group flags (0x70000C80).
 * Walks the moby array from D_00160018 (0x100 bytes an element) to the first
 * state byte 0xFF. Deleted mobys (state >= 0x80) and mode & 2 are skipped. A
 * moby is active when it was drawn last frame, its distance byte is 0xFF, or
 * ((d*d - dx*dx) - dy*dy) - dz*dz is not negative (d = distance byte,
 * dx.. = pos - camera). Every active moby sets the group flag at
 * 0x70000C80 + group (group -1 included, the store is in a delay slot) and
 * a group member is not added here. Other active mobys are appended to their
 * class slot's list (link at +0x28), and written to the target list
 * (mode & 0x1000 advances it).
 *
 * Then the group pass: each flagged group's member list (D_001C7AC0[g], u16
 * indices ending at bit 15) adds its live members the same way (the target
 * list advances for mode without 0x1000 here). The target list is copied to
 * the pointer table at D_001C7C80 (NULL-terminated). Finally the class-slot
 * lists are chained in slot order and the first head is returned.
 */
extern int D_00160018 MACRO_ADDR;
extern char D_00187040[];
extern char D_001C7AC0[];
extern char D_001C7C80[];

void *func_0020E6B8(void) {
    int *q;
    char *base;
    char *cur;
    char *m;
    char *tgt;
    char *cam;
    char *sl;
    char *tail;
    char *lp;
    char *rp;
    char *out;
    char *head;
    char *first;
    char *last;
    int idx;
    int st;
    int group;
    int gi;
    int r5;
    int r6;
    unsigned int slot;
    unsigned int mode;
    unsigned int dist;
    unsigned int drawn;
    unsigned int e;
    float dx;
    float dy;
    float dz;
    float d;
    float t;

    for (q = (int *)0x70000000; q != (int *)0x70000700; q++) {
        *q = 0;
    }
    for (q = (int *)0x70000800; q != (int *)0x70000C00; q++) {
        *q = 0;
    }
    for (q = (int *)0x70000C80; q != (int *)0x70000D00; q++) {
        *q = 0;
    }

    base = (char *)D_00160018;
    cam = D_00187040 + 0x140;
    tgt = (char *)0x70000800;
    cur = base - 0x100;
    idx = -1;

    for (;;) {
        st = *(signed char *)(cur + 0x120);
        cur += 0x100;
        idx++;
        if (st < 0) {
            if (st == -1) {
                break;
            }
            continue;
        }

        m = cur;
        mode = *(unsigned short *)(m + 0x34);
        if (mode & 2) {
            continue;
        }

        drawn = *(unsigned char *)(m + 0x31);
        dist = *(unsigned char *)(m + 0x30);
        if (drawn == 0 && dist != 0xFF) {
            dx = *(float *)(m + 0x10) - *(float *)(cam + 0x0);
            dy = *(float *)(m + 0x14) - *(float *)(cam + 0x4);
            dz = *(float *)(m + 0x18) - *(float *)(cam + 0x8);
            d = (float)dist;
            t = d * d;
            t = t - dx * dx;
            t = t - dy * dy;
            t = t - dz * dz;
            if (t < 0.0f) {
                continue;
            }
        }

        /* The flag store sits in the branch's delay slot, so it happens for
           every active moby, group or not (group -1 writes 0x70000C7F). */
        group = *(signed char *)(m + 0x21);
        *((char *)0x70000C80 + group) = 1;
        if (group >= 0) {
            continue;
        }

        slot = *(unsigned char *)(m + 0x22);
        *(short *)tgt = (short)idx;
        if (mode & 0x1000) {
            tgt += 2;
        }

        sl = (char *)0x70000000 + slot * 8;
        tail = *(char **)(sl + 4);
        *(char **)(sl + 4) = m;
        if (tail == 0) {
            *(char **)sl = m;
        } else {
            *(char **)(tail + 0x28) = m;
        }
    }

    /* Group pass: each flagged group's members. */
    for (gi = 0; gi != 0x70; gi++) {
        if (*((unsigned char *)0x70000C80 + gi) == 0) {
            continue;
        }
        lp = *(char **)(D_001C7AC0 + 4 * gi);
        r5 = 0;
        for (;;) {
            if (r5 < 0) {
                break;
            }
            r5 = *(short *)lp;
            lp += 2;
            r6 = r5 & 0x7FFF;
            *(short *)tgt = (short)r6;
            m = base + (r6 << 8);
            st = *(signed char *)(m + 0x20);
            mode = *(unsigned short *)(m + 0x34);
            if (st < 0) {
                continue;
            }
            slot = *(unsigned char *)(m + 0x22);
            if ((mode & 0x1000) == 0) {
                tgt += 2;
            }
            sl = (char *)0x70000000 + slot * 8;
            tail = *(char **)(sl + 4);
            *(char **)(sl + 4) = m;
            if (tail == 0) {
                *(char **)sl = m;
            } else {
                *(char **)(tail + 0x28) = m;
            }
        }
    }

    /* The target list as a NULL-terminated pointer table. */
    out = D_001C7C80;
    for (rp = (char *)0x70000800; rp != tgt; rp += 2) {
        e = *(unsigned short *)rp;
        *(char **)out = base + (e << 8);
        out += 4;
    }
    *(char **)out = 0;

    /* Chain the class-slot lists in slot order; the first list head is the result. */
    first = 0;
    last = 0;
    for (sl = (char *)0x70000000; sl != (char *)0x70000700; sl += 8) {
        head = *(char **)sl;
        if (head == 0) {
            continue;
        }
        tail = *(char **)(sl + 4);
        if (first == 0) {
            first = head;
        } else {
            *(char **)(last + 0x28) = head;
        }
        last = tail;
        *(char **)(tail + 0x28) = 0;
    }
    return first;
}

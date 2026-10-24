int collect_mission_ids(int *out, int *mask, int *nums, int arg3) {
    MissionNode *p = D_001A2FA0;
    int count = 0;
    int all = 1;

    *out = 0;
    if (mask != 0) {
        *mask = 0;
    }
    if (nums != 0) {
        *nums = -1;
    }
    if (p == 0) {
        return 0;
    }
    while (p->id != 0) {
        unsigned short f = p->flags;
        short t = p->status;

        if (t != 2 && !(f & 2)) {
            all = 0;
        }
        if (!(f & 2) && t != 0 && !((f & 1) && t == 2)) {
            out[count] = p->id;
            if (arg3 != 0) {
                out[count] = (p->status == 2) ? 0x5295 : p->ids[p->sel];
            }
            if (mask != 0 && p->status == 2) {
                *mask |= 1 << count;
            }
            if (nums != 0) {
                *nums++ = p->base + p->sel;
            }
            count++;
        }
        p++;
    }
    if (mask != 0 && all != 0) {
        *mask |= 0x80000000;
    }
    return count;
}

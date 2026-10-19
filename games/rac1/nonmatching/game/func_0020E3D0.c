/* fcheck: unchecked (the harness could not link it: ee_rate_0020E3D0 has no retail address; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* func_0020E3D0: MobyAnimAdvance, the per-moby animation step of MobyUpdateLoop. Moves the key
   pair (frame A = m->frame, frame B = m->prevFrame, t at +0x54) on by speed * rate (+0x58, +0x5C),
   stepping keys forward or back; each key takes its rate from the sequence's override or from its
   own frame header. Then the sequence's triggers for the key times passed play (PlayClassSound),
   and the loop sound is refreshed (func_0020D790) on the frame-spread schedule of D_0015F6F0. */
extern int func_0022ED80(int, int, int);

/* A float as the EE FPU reads it: it has no infinity or NaN, and takes an all-ones exponent for a
   number beyond FLT_MAX, which its arithmetic saturates to +-FLT_MAX. A key whose frame header
   holds +inf's bits (level 9's class 193) gives t = 0 * rate = 0 there, not NaN. */
static float ee_rate_0020E3D0(unsigned int bits) {
    if ((bits & 0x7F800000) == 0x7F800000) {
        bits = (bits & 0x80000000) | 0x7F7FFFFF;
    }
    return *(float *)&bits;
}

void func_0020E3D0(Moby *m) {
    unsigned char *mp = (unsigned char *)m;
    unsigned int speedBits = *(unsigned int *)(mp + 0x58);
    unsigned int rateBits = *(unsigned int *)(mp + 0x5C);
    float speed = *(float *)(mp + 0x58);
    float rate = *(float *)(mp + 0x5C);
    float tOld = *(float *)(mp + 0x54);
    float t;
    unsigned char seqA = m->seq;
    unsigned char seqB = m->prevSeq;
    unsigned int oldA = m->frame;
    unsigned int flags = 0;
    unsigned int loop = m->unk7C;
    unsigned int voice = m->unk7D;
    unsigned int trig = m->unk7E;
    MobySeq *seq;
    int ti;
    int snap;
    int newA;
    int newB;
    int fc;
    int ovr;
    int oldBptr;
    int newBptr;
    int newAptr;

    if (rateBits == 0 || speedBits == 0) {
        goto tail;
    }
    /* The EE FPU has no infinity: 1.0f / 0 (func_00213DE0's rate for a blend of 0 frames) is
       FLT_MAX there, and FLT_MAX / FLT_MAX = 1 ends the key loop below. A host infinity here would
       make it NaN and never end. */
    if ((rateBits & 0x7F800000) == 0x7F800000) {
        rateBits = (rateBits & 0x80000000) | 0x7F7FFFFF;
        rate = *(float *)&rateBits;
    }

    seq = m->pClass->seqs[seqB];
    if (seqA == seqB) {
        t = (0.0f + tOld) + speed * rate;
    } else {
        t = tOld + rate;
    }

    ti = *(int *)&t;
    snap = (ti >= 0x3F7F0000 && ti <= 0x3F808000);
    if (snap || ti > 0x3F800000) {
        /* forward: the key passes the end of its interval, possibly more than once */
        if (snap) {
            t = 1.0f;
        }
        do {
            t = t - 1.0f;
            newA = m->prevFrame;
            newB = newA + 1;
            oldBptr = m->prevFrameData;
            if (seqA != seqB) {
                m->unk7E = seq->unk12;
                seqA = seqB;
                loop = 0xFF;
                voice = 0xFF;
                trig = 0;
            }
            /* The EE FPU's divide saturates: x / 0 is +-FLT_MAX (and FLT_MAX * 0 = 0), which ends
               the loop; the host's infinity would not. */
            if (rate != 0.0f) {
                t = t / rate;
            } else {
                *(unsigned int *)&t = (*(unsigned int *)&t & 0x80000000) | 0x7F7FFFFF;
            }
            flags |= 1;
            fc = ((unsigned char *)seq)[0x10];
            ovr = *(int *)((char *)seq + 0x18);
            if (fc - newB <= 0) {
                newB = 0;
                flags |= 2;
            }
            newBptr = seq->frames[newB];
            rate = ee_rate_0020E3D0((unsigned int)ovr);
            if (ovr == 0) {
                rate = ee_rate_0020E3D0(*(unsigned int *)(oldBptr));
            }
            t = t * rate;
            mp[0x52] = seqA;
            m->frame = newA;
            m->prevFrame = newB;
            m->frameData = oldBptr;
            m->prevFrameData = newBptr;
            *(float *)(mp + 0x5C) = rate;
            *(float *)(mp + 0x54) = t;
        } while (*(int *)&t > 0x3F800000);
        mp[0x70] = flags;
        goto after;
    } else if (ti < 0) {
        /* backward: the key passes its start, possibly more than once */
        do {
            /* The EE FPU's divide saturates: x / 0 is +-FLT_MAX (and FLT_MAX * 0 = 0), which ends
               the loop; the host's infinity would not. */
            if (rate != 0.0f) {
                t = t / rate;
            } else {
                *(unsigned int *)&t = (*(unsigned int *)&t & 0x80000000) | 0x7F7FFFFF;
            }
            newB = m->frame;
            newA = newB - 1;
            flags |= 1;
            ovr = *(int *)((char *)seq + 0x18);
            if (newA < 0) {
                fc = ((unsigned char *)seq)[0x10];
                newA = fc - 1;
                flags |= 2;
            }
            oldBptr = m->frameData;
            newAptr = seq->frames[newA];
            rate = ee_rate_0020E3D0((unsigned int)ovr);
            if (ovr == 0) {
                rate = ee_rate_0020E3D0(*(unsigned int *)(newAptr));
            }
            t = t * rate;
            m->frameData = newAptr;
            m->frame = newA;
            m->prevFrame = newB;
            t = t + 1.0f;
            m->prevFrameData = oldBptr;
            *(float *)(mp + 0x5C) = rate;
            *(float *)(mp + 0x54) = t;
        } while (*(int *)&t < 0);
        mp[0x70] = flags;
        goto after;
    } else {
        *(float *)(mp + 0x54) = t;
    }

after:
    mp[0x70] = flags;
    if (loop == 0xFF && voice == 0xFF && trig == 0) {
        return;
    }
    if (seqA != seqB) {
        goto tail;
    }
    if (trig != 0) {
        /* triggers: the first whose time falls in (old position, new position] plays */
        int curA = m->frame;
        int newPos = (curA << 4) + (int)(t * 16.0f);
        int oldPos = ((int)oldA << 4) + (int)(tOld * 16.0f);
        if (newPos - oldPos > 0) {
            unsigned int *tw;
            int k;
            fc = ((unsigned char *)seq)[0x10];
            tw = (unsigned int *)((char *)seq + 0x1C + fc * 4);
            for (k = 0; k < (int)trig; k++) {
                int time = (int)(tw[k] >> 16);
                if (newPos - time >= 0 && time - (oldPos + 1) >= 0) {
                    func_0022ED80((int)(tw[k] & 0xFFFF), 0, (int)m);
                    return;
                }
            }
        }
    }

tail:
    mp[0x70] = flags;
    if (loop == 0xFF && voice == 0xFF) {
        return;
    }
    if (voice != 0xFF) {
        func_0020D790(mp);
        return;
    }
    if ((((unsigned int)mp >> 8) & 3) == ((unsigned int)D_0015F6F0 & 3)) {
        func_0020D790(mp);
    }
}

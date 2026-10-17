/* functional: medium. Moby skin-and-light driver (retail: EE code feeding VU0 microprogram 104691 vertex by vertex through registers, scratchpad double buffers and DMA channels 8 and 9). Rewritten as the per-vertex arithmetic the pair computes: a0 = job record in main memory (u16 packet count at 0, light constants 0x40..0xBF, packet list at 0xC0 of {u32 (vertex table addr >> 4) << 8 | qwc, u32 output address}, zero-terminated, then an optional metal record), a1 = the 512-entry vertex cache; returns the address after this job (the next job record). Per packet: the matrix transfers go to the VU0 slot memory (kept in a static array, as VU0 data memory is in retail), each vertex goes through the slot machine (2-way, 3-way, single; stores before loads) to a skin matrix, the position is M*p truncated and wrapped to s16, the normal is M*(cos a cos e, sin a cos e, sin e) unnormalised from the trig table at scratchpad 0x70003800, lit by the 3 light rows/colours with the back factor, c = ambient + floor(128 * (sum C_k f_k) * rsqrt(|n'|^2)), packed as min(255, (s16(c) * multiplier) >> 7) with the saturation of pmfhl.sh; colours then positions are written straight to the chain hole (retail builds them in scratchpad and DMAs them out), every result is also stored in the vertex cache under its 9-bit id (carried 7 vertices late, the last ones in the epilogue record), and the duplicates are copied from the cache afterwards without relighting. With a non-zero metal word (job+0x14: lo16 shine alpha, hi16 record qwc) the metal record follows: per metal vertex a 1-3 matrix blend straight from the scratchpad palette, s16 position, colour 0x70 + sum floor(32 C_k max(L_k n', 0)) with the shine alpha in byte 3, and the sphere-map ST (E.n' + 1)/2 in 4.12. Differences from retail: no DMA or scratchpad buffering (data is read from and written to main memory directly), VU0 float rounding is the host's, and a packet list with fewer than 3 vertices is handled where retail's software pipeline assumes more. The formulas and record layouts come from ReRAC's reading of VU0 program 104691 (ISC) and were checked against the assembly's byte offsets, loop counts and pack instructions; not run against retail output. */

static float vu0_slots_1EE9F8[64][16];

static int ftrunc_1EE9F8(float v) {
    if (v >= 2147483520.0f) {
        return 0x7FFFFFFF;
    }
    if (v <= -2147483520.0f) {
        return -2147483647 - 1;
    }
    return (int)v;
}

static int ffloor_1EE9F8(float v) {
    int i;
    if (v >= 2147483520.0f) {
        return 0x7FFFFFFF;
    }
    if (v <= -2147483520.0f) {
        return -2147483647 - 1;
    }
    i = (int)v;
    if ((float)i > v) {
        i--;
    }
    return i;
}

static void copy_matrix_1EE9F8(float *dst, const float *src) {
    int i;
    for (i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
}

/* dst = (a * wa + b * wb [+ c * wc]) / 256 with the weights as exact multiples of 1/256. */
static void blend_1EE9F8(float *dst, const float *a, int wa, const float *b, int wb, const float *c, int wc) {
    float fa = (float)wa * (1.0f / 256.0f);
    float fb = (float)wb * (1.0f / 256.0f);
    float fc = (float)wc * (1.0f / 256.0f);
    int i;
    for (i = 0; i < 16; i++) {
        float v = a[i] * fa;
        v += b[i] * fb;
        if (c != 0) {
            v += c[i] * fc;
        }
        dst[i] = v;
    }
}

/* ftoi0 of M*(x, y, z, 1), low 16 bits of each lane (ppach). */
static void skin_pos_1EE9F8(const float *m, short *out, int xi, int yi, int zi) {
    float x = (float)xi;
    float y = (float)yi;
    float z = (float)zi;
    int c;
    for (c = 0; c < 3; c++) {
        float v = m[c] * x;
        v += m[4 + c] * y;
        v += m[8 + c] * z;
        v += m[12 + c];
        out[c] = (short)ftrunc_1EE9F8(v);
    }
}

/* n' = M0*cos a*cos e + M1*sin a*cos e + M2*sin e (translation row not used); the unit normal comes from the (cos, sin) table at scratchpad 0x3800. */
static void skin_normal_1EE9F8(const float *m, float *np, int az, int el) {
    const float *ta = (const float *)(0x70003800 + az * 8);
    const float *te = (const float *)(0x70003800 + el * 8);
    float nx = ta[0] * te[0];
    float ny = ta[1] * te[0];
    float nz = te[1];
    int c;
    for (c = 0; c < 3; c++) {
        float v = m[c] * nx;
        v += m[4 + c] * ny;
        v += m[8 + c] * nz;
        np[c] = v;
    }
}

/* d_k = L_k . n' where row j of the constants holds lane k = component j of light k. */
static void light_dots_1EE9F8(float *d, const float rows[3][4], const float *np) {
    int k;
    for (k = 0; k < 3; k++) {
        float v = rows[0][k] * np[0];
        v += rows[1][k] * np[1];
        v += rows[2][k] * np[2];
        d[k] = v;
    }
}

unsigned int func_001EE9F8(unsigned char *job, unsigned char *cache) {
    unsigned int npk = *(unsigned short *)job;
    unsigned char *end = job + 0x40 + ((npk >> 1) + 9) * 16;
    unsigned int metal_word = *(unsigned int *)(job + 0x14);
    float rows[3][4];
    float col[3][4];
    float negk[3];
    int amb[4];
    unsigned int *pk;
    int i, j;

    for (j = 0; j < 3; j++) {
        const float *r = (const float *)(job + 0x40 + j * 0x10);
        const float *c = (const float *)(job + 0x70 + j * 0x10);
        for (i = 0; i < 4; i++) {
            rows[j][i] = r[i];
            col[j][i] = c[i];
        }
    }
    for (j = 0; j < 3; j++) {
        float k = ((const float *)(job + 0xA0))[j];
        negk[j] = -(k < 0.0f ? -k : k);
    }
    for (i = 0; i < 4; i++) {
        amb[i] = (int)(((const unsigned int *)(job + 0xB0))[i] & 0xFFFF);
    }

    for (pk = (unsigned int *)(job + 0xC0); pk[0] != 0; pk += 2) {
        unsigned char *t = (unsigned char *)((pk[0] >> 8) << 4);
        unsigned char *out = (unsigned char *)pk[1];
        const unsigned int *th = (const unsigned int *)t;
        unsigned int npre = th[0];
        unsigned int n2 = th[1];
        unsigned int n3 = th[2];
        unsigned int n1 = th[3];
        unsigned int ndup = th[4];
        unsigned int ntot = th[5];
        const unsigned char *vrec = t + th[6];
        const unsigned char *mult = t + th[7];
        const unsigned char *tr = t + 0x20;
        const unsigned short *dups = (const unsigned short *)(t + 0x20 + ((npre * 2 + 6) & 0xF8));
        unsigned int infile = n2 + n3 + n1;
        unsigned int nrec = (th[7] - th[6]) >> 4;
        unsigned int pos_off = (ntot * 4 + 15) & 0xFFF0;
        unsigned int pos_end = pos_off + ((ntot * 6 + 15) & 0xFFF0);
        unsigned int v;

        if (ntot != 0) {
            for (i = 0; i < 16; i++) {
                out[pos_off - 16 + i] = 0;
                out[pos_end - 16 + i] = 0;
            }
        }

        for (v = 0; v < npre; v++) {
            copy_matrix_1EE9F8(vu0_slots_1EE9F8[(tr[v * 2 + 1] >> 2) & 63], (const float *)(0x70000000 + tr[v * 2] * 64));
        }

        for (v = 0; v < infile; v++) {
            const unsigned char *r = vrec + v * 16;
            float mm[16];
            float np[3];
            float d[3];
            float f[3];
            float s[4];
            float q, len2;
            short p[3];
            unsigned int id;
            unsigned int colour = 0;
            int ch;

            if (v < n2) {
                copy_matrix_1EE9F8(vu0_slots_1EE9F8[(r[6] >> 2) & 63], (const float *)(0x70000000 + (r[1] >> 1) * 64));
                blend_1EE9F8(mm, vu0_slots_1EE9F8[(r[2] >> 2) & 63], r[4], vu0_slots_1EE9F8[(r[3] >> 2) & 63], r[5], 0, 0);
                copy_matrix_1EE9F8(vu0_slots_1EE9F8[(r[7] >> 2) & 63], mm);
            } else if (v < n2 + n3) {
                blend_1EE9F8(mm, vu0_slots_1EE9F8[(r[2] >> 2) & 63], r[4], vu0_slots_1EE9F8[(r[3] >> 2) & 63], r[5],
                             vu0_slots_1EE9F8[((r[1] & 0xFE) >> 2) & 63], r[6]);
                copy_matrix_1EE9F8(vu0_slots_1EE9F8[(r[7] >> 2) & 63], mm);
            } else {
                copy_matrix_1EE9F8(vu0_slots_1EE9F8[(r[3] >> 2) & 63], (const float *)(0x70000000 + (r[1] >> 1) * 64));
                copy_matrix_1EE9F8(mm, vu0_slots_1EE9F8[(r[2] >> 2) & 63]);
            }

            skin_pos_1EE9F8(mm, p, *(const short *)(r + 10), *(const short *)(r + 12), *(const short *)(r + 14));
            skin_normal_1EE9F8(mm, np, r[8], r[9]);
            light_dots_1EE9F8(d, rows, np);
            for (j = 0; j < 3; j++) {
                float dn = d[j] * negk[j];
                f[j] = d[j] > dn ? d[j] : dn;
            }
            len2 = np[0] * np[0] + np[1] * np[1] + np[2] * np[2];
            q = len2 > 0.0f ? 1.0f / __builtin_sqrtf(len2) : 0.0f;
            for (ch = 0; ch < 4; ch++) {
                int h, prod;
                float sv = col[0][ch] * f[0];
                sv += col[1][ch] * f[1];
                sv += col[2][ch] * f[2];
                s[ch] = sv;
                h = (short)(amb[ch] + ffloor_1EE9F8(sv * q * 128.0f));
                prod = h * mult[v * 4 + ch];
                if (prod > 32767) {
                    prod = 32767;
                } else if (prod < -32768) {
                    prod = -32768;
                }
                colour |= (unsigned int)((prod >> 7) & 0xFF) << (ch * 8);
            }

            ((short *)(out + pos_off + v * 6))[0] = p[0];
            ((short *)(out + pos_off + v * 6))[1] = p[1];
            ((short *)(out + pos_off + v * 6))[2] = p[2];
            *(unsigned int *)(out + v * 4) = colour;

            /* The id of vertex v travels in the record 7 later; the last ones in the last epilogue record (six u16 from byte 4). */
            if (v + 7 < nrec) {
                id = *(const unsigned short *)(vrec + (v + 7) * 16);
            } else if (nrec != 0 && v + 7 - nrec < 6) {
                id = *(const unsigned short *)(vrec + (nrec - 1) * 16 + 4 + (v + 7 - nrec) * 2);
            } else {
                id = 0;
            }
            id &= 0x1FF;
            {
                unsigned char *ce = cache + id * 16;
                ((short *)ce)[0] = p[0];
                ((short *)ce)[1] = p[1];
                ((short *)ce)[2] = p[2];
                ((short *)ce)[3] = 0;
                *(unsigned int *)(ce + 8) = colour;
            }
        }

        /* Duplicates: copy position and final colour from the cache (offset = entry >> 3 inside the cache's 8 KB). */
        for (v = 0; v < ndup; v++) {
            unsigned int src = ((((unsigned int)cache >> 13) & 0xFFFF) << 13) | (dups[v] >> 3);
            const unsigned char *ce = (const unsigned char *)(src & ~0xFu);
            unsigned int o = infile + v;
            ((short *)(out + pos_off + o * 6))[0] = ((const short *)ce)[0];
            ((short *)(out + pos_off + o * 6))[1] = ((const short *)ce)[1];
            ((short *)(out + pos_off + o * 6))[2] = ((const short *)ce)[2];
            *(unsigned int *)(out + o * 4) = *(const unsigned int *)(ce + 8);
        }
    }

    if (metal_word == 0 || (metal_word >> 16) == 0) {
        return (unsigned int)end;
    }

    {
        unsigned char *rec = end;
        float er[3][2];
        float cm[3][4];
        unsigned int shine = metal_word & 0xFF;
        unsigned int *mp;

        for (j = 0; j < 3; j++) {
            const float *e = (const float *)(rec + j * 0x10);
            er[j][0] = e[0];
            er[j][1] = e[1];
            for (i = 0; i < 4; i++) {
                cm[j][i] = col[j][i] * 0.25f;
            }
        }

        for (mp = (unsigned int *)(rec + 0x40); mp[0] != 0; mp += 2) {
            unsigned char *t = (unsigned char *)((mp[0] >> 8) << 4);
            unsigned char *out = (unsigned char *)mp[1];
            const unsigned int *th = (const unsigned int *)t;
            unsigned int cnt = th[0];
            unsigned int po = th[1];
            unsigned int co = th[2];
            unsigned int tot = th[3];
            unsigned int v;

            if (po >= 16) {
                for (i = 0; i < 16; i++) {
                    out[po - 16 + i] = 0;
                }
            }
            if (co >= 16) {
                for (i = 0; i < 16; i++) {
                    out[co - 16 + i] = 0;
                }
            }
            if (tot >= 16) {
                for (i = 0; i < 16; i++) {
                    out[tot - 16 + i] = 0;
                }
            }

            for (v = 0; v < cnt; v++) {
                const unsigned char *r = t + 0x10 + v * 16;
                float mm[16];
                float np[3];
                float d[3];
                float e0, e1;
                short p[3];
                short st[2];
                int cc[3];
                unsigned int colour;
                int count = r[11];

                if (count <= 1) {
                    copy_matrix_1EE9F8(mm, (const float *)(0x70000000 + r[8] * 64));
                } else if (count == 2) {
                    blend_1EE9F8(mm, (const float *)(0x70000000 + r[8] * 64), r[12], (const float *)(0x70000000 + r[9] * 64), r[13], 0, 0);
                } else {
                    blend_1EE9F8(mm, (const float *)(0x70000000 + r[8] * 64), r[12], (const float *)(0x70000000 + r[9] * 64), r[13],
                                 (const float *)(0x70000000 + r[10] * 64), r[14]);
                }

                skin_pos_1EE9F8(mm, p, *(const short *)(r + 0), *(const short *)(r + 2), *(const short *)(r + 4));
                skin_normal_1EE9F8(mm, np, r[6], r[7]);
                light_dots_1EE9F8(d, rows, np);
                for (j = 0; j < 3; j++) {
                    if (d[j] < 0.0f) {
                        d[j] = 0.0f;
                    }
                }
                for (i = 0; i < 3; i++) {
                    cc[i] = 112;
                    for (j = 0; j < 3; j++) {
                        cc[i] += ffloor_1EE9F8(cm[j][i] * d[j] * 128.0f);
                    }
                }
                colour = (unsigned int)(cc[0] & 0xFF) | ((unsigned int)(cc[1] & 0xFF) << 8) | ((unsigned int)(cc[2] & 0xFF) << 16) | (shine << 24);

                e0 = er[0][0] * np[0];
                e0 += er[1][0] * np[1];
                e0 += er[2][0] * np[2];
                e1 = er[0][1] * np[0];
                e1 += er[1][1] * np[1];
                e1 += er[2][1] * np[2];
                st[0] = (short)(ftrunc_1EE9F8((e0 + 1.0f) * 4096.0f) >> 1);
                st[1] = (short)(ftrunc_1EE9F8((e1 + 1.0f) * 4096.0f) >> 1);

                *(short *)(out + v * 4) = st[0];
                *(short *)(out + v * 4 + 2) = st[1];
                ((short *)(out + po + v * 6))[0] = p[0];
                ((short *)(out + po + v * 6))[1] = p[1];
                ((short *)(out + po + v * 6))[2] = p[2];
                *(unsigned int *)(out + co + v * 4) = colour;
            }
        }
    }

    return (unsigned int)(end + ((metal_word >> 16) << 4));
}

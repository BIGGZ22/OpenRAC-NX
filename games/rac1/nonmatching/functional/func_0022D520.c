/* functional: high. fcheck: unchecked. Textured sky tile to GIF packet. arg0 = face count (>= 1; the loop is post-tested), arg1 = faces (u8 v0, v1, v2, texture), arg2 = per-vertex ST (two u16, 4 bytes each, used as 4.12 fixed point), arg3 = vertex records written by func_0022D2AC (16 bytes: 12.4 x, y, z, then alpha byte at +0xC and clip flags at +0xF). A face is skipped when the clip flags of its three vertices share a bit. On a texture change it emits TEX0_1 (A+D); a texture-table entry (D_0016055C->0x10, 16 bytes per texture) whose TEX0 dword is still 0 is first allocated as retail's func_0022D6B4 does: TBP from the D_0015EF74 VRAM cursor (which advances by 0x400 + (1 << (tw + th))), CBP from the cursor + 0x400, and a texture-upload record appended at D_0018D140[D_0015F558++] (image and CLUT addresses D_0016055C->0x14 + 16 * the entry's halfwords at +0xA and +8). Each vertex gives ST, RGBAQ (0x808080, alpha, Q = 1.0) and XYZ2 with z = 0. The packet starts at the D_00161000 cursor with a DMA cnt / VIF DIRECT / GIF A+D (EOP) header; when no face is drawn nothing is written and the cursor is left alone. The retail return value (qwords - 2) is not used by callers and not returned. func_0022D6B4 is part of this routine (entered by jump with the texture index in $1, returns by jump to func_0022D59C); it has no C signature of its own. Differential test: 60 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
extern u8 *D_00161000;
extern u8 *D_0016055C;
extern u32 D_0015EF74;
extern s32 D_0015F558;
extern u8 D_0018D140[];

static u64 sky_tex0_alloc(u8 *entry) {
    s16 tw = *(s16 *)(entry + 0xC);
    s16 th = *(s16 *)(entry + 0xE);
    u32 vram = D_0015EF74;
    u32 tbp = vram >> 8;
    u32 cbp = (vram + 0x400) >> 8;
    s32 n = tw - 6;
    u32 tbw;
    u64 tex0;
    u8 *rec;
    u8 *base;

    D_0015EF74 = vram + 0x400 + (1u << ((tw + th) & 31));
    if (n < 0) {
        n = 0;
    }
    tbw = (u32)(1 << (n & 31)) << 14;
    tex0 = (u64)(s64)(s32)(tbw + cbp);
    tex0 += (u64)0x13 << 20;
    tex0 += (u64)(s64)tw << 26;
    tex0 += (u64)(s64)th << 30;
    tex0 += (u64)1 << 34;
    tex0 += (u64)tbp << 37;
    tex0 += (u64)4 << 61;
    *(u64 *)entry = tex0;

    /* texture upload record */
    rec = D_0018D140 + D_0015F558 * 16;
    D_0015F558++;
    base = *(u8 **)(D_0016055C + 0x14);
    *(u32 *)(rec + 0x0) = (u32)(base + *(s16 *)(entry + 0xA) * 16);
    *(u16 *)(rec + 0x4) = 0;
    *(u16 *)(rec + 0x6) = (u16)tbp;
    *(u32 *)(rec + 0x8) = (u32)(base + *(s16 *)(entry + 0x8) * 16);
    rec[0xC] = (u8)tw;
    rec[0xD] = (u8)th;
    *(u16 *)(rec + 0xE) = (u16)cbp;
    return tex0;
}

void func_0022D520(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    const u8 *face = (const u8 *)arg1;
    const u8 *st = (const u8 *)arg2;
    const u8 *verts = (const u8 *)arg3;
    u8 *start = D_00161000;
    u8 *p = start + 0x20;
    u32 cur_tex = 0xFFFF;
    u32 qwc;

    do {
        u32 tex = face[3];
        s32 k;

        arg0--;
        if ((verts[face[0] * 16 + 15] & verts[face[1] * 16 + 15] & verts[face[2] * 16 + 15]) == 0) {
            if (tex != cur_tex) {
                u8 *entry = *(u8 **)(D_0016055C + 0x10) + tex * 16;
                u64 tex0 = *(u64 *)entry;

                if (tex0 == 0) {
                    tex0 = sky_tex0_alloc(entry);
                }
                *(u64 *)(p + 0) = tex0;
                *(u64 *)(p + 8) = 6; /* TEX0_1 */
                p += 0x10;
                cur_tex = tex;
            }
            for (k = 0; k < 3; k++) {
                u32 idx = face[k];
                const u8 *uv = st + idx * 4;
                const u8 *v = verts + idx * 16;

                *(float *)(p + 0x00) = (float)*(const u16 *)(uv + 0) * (1.0f / 4096.0f);
                *(float *)(p + 0x04) = (float)*(const u16 *)(uv + 2) * (1.0f / 4096.0f);
                *(u64 *)(p + 0x08) = 2; /* ST */
                *(u32 *)(p + 0x10) = ((u32)v[0xC] << 24) | 0x808080;
                *(float *)(p + 0x14) = 1.0f;
                *(u64 *)(p + 0x18) = 1; /* RGBAQ */
                *(u32 *)(p + 0x20) = (*(const u32 *)(v + 0) & 0xFFFF) | ((*(const u32 *)(v + 4) & 0xFFFF) << 16);
                *(u32 *)(p + 0x24) = 0;
                *(u64 *)(p + 0x28) = 5; /* XYZ2 */
                p += 0x30;
            }
        }
        face += 4;
    } while (arg0 != 0);

    qwc = (u32)(p - start) >> 4;
    if (qwc - 2 == 0) {
        return;
    }
    *(u32 *)(start + 0x00) = 0x10000000 | (qwc - 1);
    *(u32 *)(start + 0x04) = 0;
    *(u32 *)(start + 0x08) = 0;
    *(u32 *)(start + 0x0C) = 0x50000000 | (qwc - 1);
    *(u32 *)(start + 0x10) = (qwc - 2) | 0x8000;
    *(u32 *)(start + 0x14) = 0x10000000;
    *(u64 *)(start + 0x18) = 0xE; /* A+D */
    D_00161000 = p;
}

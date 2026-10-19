/* functional: high. fcheck: unchecked. SkySpriteProc: builds the sky sprite GIF packet at the D_00161000 cursor from the sprite list of the sky (D_0016055C: u16 count at +8, texture table at +0x10, image base at +0x14, 32-byte sprites at +0x1C). Header: a DMA ref tag to D_00101080 (qwc from D_00101070), the D_001607C0 qword, camera rows D_00187040+0x40..0x60 times 1024 and row +0x70, D_0018CE00+0x210 and +0x1A0, the D_001607D0 qword. Per sprite (u8 visible flag +1, s8 texture +2, u8 +3, RGBA +4, angle +8, position xyz +0x10, size +0x1C): culled when the texture is negative or the rotated position fails the same frustum test as the sky tiles (D_0018CE00+0xB0 / +0x1E0); the flag at +1 is set to 1 when drawn (if it was 0) and to 0 when culled. A drawn sprite gives 4 qwords: xyz,0; R,G,B,A as words; Q*FastCos(angle), Q*FastSin(angle), byte +3, 0 with Q = size * D_0018CE00[0x200].w / z; TEX0, 0. A texture whose TEX0 dword is still >= 0 is first allocated as retail's func_0022D1DC does (D_00160520 template | CBP | TBW | TW | TH | TBP from the D_0015EF74 VRAM cursor, an upload record at D_0018D140[D_0015F558++]). Sprites go in batches of at most 30, each between a D_001607E0 header (qword count in the low half, count - 1 in byte 0xE, 4 * sprites at +0x3C) and a D_001607F0 trailer; the packet ends with the D_00160800 qword. Nothing is written when the count is 0. func_0022D1DC (inputs in $9/$13, TEX0 out in $8) is part of this routine and has no C signature of its own; retail's VU0 micro FastCos/FastSin calls are the C FastCos/FastSin (func_001F9F90/func_001F9FA8). A z of exactly 0 divides by zero here (VU0 gives +-FLT_MAX). Differential test: 60 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model, VU0 micro FastCos/FastSin) to 1e-5 relative (VU0 multiply-add vs FPU rounding). */
extern u8 *D_0016055C;
extern u8 *D_00161000;
extern float D_0018CE00[];
extern float D_00187040[];
extern u32 D_00101070;
extern u8 D_00101080[];
extern u32 D_001607C0[];
extern u32 D_001607D0[];
extern u32 D_001607E0[];
extern u32 D_001607F0[];
extern u32 D_00160800[];
extern u64 D_00160520;
extern u32 D_0015EF74;
extern s32 D_0015F558;
extern u8 D_0018D140[];
extern float func_001F9F90(float); /* FastCos */
extern float func_001F9FA8(float); /* FastSin */

static void sky_copy16(void *dst, const void *src) {
    u32 *d = (u32 *)dst;
    const u32 *s = (const u32 *)src;

    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
}

static int sky_sign_set(float f) {
    union { float f; u32 u; } v;
    v.f = f;
    return (v.u >> 31) != 0;
}

/* func_0022D1DC: allocate VRAM for a sprite texture, write its upload record, return TEX0 */
static u64 sky_sprite_tex0(u8 *entry, u8 *image_base) {
    u8 *rec = D_0018D140 + D_0015F558 * 16;
    u32 vram = D_0015EF74;
    u32 tbp = vram >> 8;
    u32 cbp = (vram + 0x400) >> 8;
    u32 tw = entry[0xC];
    u32 th = entry[0xE];
    s32 n = (s32)tw - 6;
    u64 tex0;

    D_0015F558++;
    *(u16 *)(rec + 0x6) = (u16)tbp;
    *(u16 *)(rec + 0xE) = (u16)cbp;
    rec[0xC] = (u8)tw;
    rec[0xD] = (u8)th;
    D_0015EF74 = vram + 0x400 + ((1u << (tw & 31)) << (th & 31));
    *(u16 *)(rec + 0x4) = 0;
    *(u32 *)(rec + 0x0) = (u32)(image_base + *(u16 *)(entry + 0xA) * 16);
    *(u32 *)(rec + 0x8) = (u32)(image_base + *(u16 *)(entry + 0x8) * 16);
    if (n < 0) {
        n = 0;
    }
    tex0 = D_00160520 | cbp;
    tex0 |= (u64)(s64)(s32)(1u << (n & 31)) << 14;
    tex0 |= (u64)tw << 26;
    tex0 |= (u64)th << 30;
    tex0 |= (u64)tbp << 37;
    *(u64 *)entry = tex0;
    return tex0;
}

/* close a batch: trailer at p, header at hdr */
static u8 *sky_sprite_flush(u8 *hdr, u8 *p, s32 n4) {
    u32 qwc;

    sky_copy16(p, D_001607F0);
    qwc = (u32)(p - hdr) >> 4;
    sky_copy16(hdr, D_001607E0);
    *(u16 *)(hdr + 0x0) = (u16)qwc;
    hdr[0xE] = (u8)(qwc - 1);
    *(s32 *)(hdr + 0x3C) = n4;
    return p + 0x10;
}

void func_0022CEB8(void) {
    u8 *sky = D_0016055C;
    u8 *sprite = *(u8 **)(sky + 0x1C);
    s32 count = *(u16 *)(sky + 0x8);
    u8 *image_base = *(u8 **)(sky + 0x14);
    u8 *textures = *(u8 **)(sky + 0x10);
    const float *cam = D_00187040;
    const float *fr = D_0018CE00 + 0xB0 / 4;  /* frustum slope */
    const float *fs = D_0018CE00 + 0x1E0 / 4; /* frustum size factor */
    float qscale = D_0018CE00[0x200 / 4 + 3];
    u8 *p;
    u8 *hdr;
    s32 n4;
    s32 i;

    if (count == 0) {
        return;
    }

    p = D_00161000;
    ((u32 *)p)[0] = 0x30000000 | (D_00101070 & 0xFFFF);
    ((u32 *)p)[1] = (u32)D_00101080;
    ((u32 *)p)[2] = 0;
    ((u32 *)p)[3] = 0;
    p += 0x10;
    sky_copy16(p + 0x00, D_001607C0);
    sky_copy16(p + 0x70, D_001607D0);
    sky_copy16(p + 0x50, D_0018CE00 + 0x210 / 4);
    sky_copy16(p + 0x60, D_0018CE00 + 0x1A0 / 4);
    for (i = 0; i < 12; i++) {
        ((float *)(p + 0x10))[i] = cam[0x10 + i] * 1024.0f;
    }
    sky_copy16(p + 0x40, cam + 0x1C);
    p += 0x80;
    n4 = 0;
    hdr = p;
    p += 0x10;

    for (; count > 0; count--, sprite += 0x20) {
        const float *pos = (const float *)(sprite + 0x10);
        float size = *(float *)(sprite + 0x1C);
        u8 flag = sprite[1];
        s8 tex = (s8)sprite[2];
        float vx, vy, vz, ex, ey, q, angle;
        u8 *entry;
        u64 tex0;
        u32 rgba;

        if (tex < 0) {
            goto hidden;
        }
        vx = cam[0] * pos[0] + cam[4] * pos[1] + cam[8] * pos[2];
        vy = cam[1] * pos[0] + cam[5] * pos[1] + cam[9] * pos[2];
        vz = cam[2] * pos[0] + cam[6] * pos[1] + cam[10] * pos[2];
        ex = fr[0] * vz - (__builtin_fabsf(vx) - fs[0] * size);
        ey = fr[1] * vz - (__builtin_fabsf(vy) - fs[1] * size);
        if (sky_sign_set(ey) || sky_sign_set(ex)) {
            goto hidden;
        }

        q = (size * qscale) / vz;
        angle = *(float *)(sprite + 0x8);
        rgba = *(u32 *)(sprite + 0x4);
        entry = textures + tex * 16;
        tex0 = *(u64 *)entry;

        sky_copy16(p, pos);
        if (flag == 0) {
            sprite[1] = 1;
        }
        ((u32 *)p)[3] = 0;
        ((u32 *)p)[4] = rgba & 0xFF;
        ((u32 *)p)[5] = (rgba >> 8) & 0xFF;
        ((u32 *)p)[6] = (rgba >> 16) & 0xFF;
        ((u32 *)p)[7] = rgba >> 24;
        ((float *)p)[8] = q * func_001F9F90(angle);
        ((float *)p)[9] = q * func_001F9FA8(angle);
        *(u64 *)(p + 0x28) = sprite[3];
        if ((s64)tex0 >= 0) {
            tex0 = sky_sprite_tex0(entry, image_base);
        }
        *(u64 *)(p + 0x30) = tex0;
        *(u64 *)(p + 0x38) = 0;
        n4 += 4;
        p += 0x40;
        if (n4 - 0x78 >= 0) {
            p = sky_sprite_flush(hdr, p, n4);
            n4 = 0;
            hdr = p;
            p += 0x10;
        }
        continue;
    hidden:
        if (flag != 0) {
            sprite[1] = 0;
        }
    }

    if (n4 != 0) {
        p = sky_sprite_flush(hdr, p, n4);
    } else {
        p -= 0x10;
    }
    sky_copy16(p, D_00160800);
    D_00161000 = p + 0x10;
}

/* functional: medium. fcheck: unchecked. ComputeTfragTextureUsage: clears the 192-byte per-texture flag table at scratchpad 0x70003C00, then walks the zero-terminated list of {record pointer, count} pairs that TfragProc (func_002352C8) left at scratchpad 0x70003400; for every 16-byte texture record (float x, y, z; u16 radius at +0xC; u8 mip band at +0xE; u8 texture index at +0xF) whose flag is not already negative, it puts the point into view space and marks the texture 1 or -1 (near / far mip) when the sphere is in the depth window and inside the side planes. Retail takes its view constants in VU0 registers that TfragProc loaded and leaves alone (vf28 = gCamera+0x140 times 1024, vf29..vf31 = gCamera rows 0..2, vf24/vf26 = D_0018CE00+0xB0/+0x1E0); this C reloads them from the same globals, which nothing changes between the two calls in DrawTfrag/DmaTfragTextures. Sign tests are made on the float sign bits as retail does. Differential test: 60 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model), the VU0 registers preset to the values loaded from those globals. */
extern char D_00187040[];
extern char D_0018CE00[];

static unsigned int fbits_235EF0(float f)
{
    union { float f; unsigned int u; } c;
    c.f = f;
    return c.u;
}

void func_00235EF0(void)
{
    unsigned int *list = (unsigned int *)0x70003400;
    signed char *flags = (signed char *)0x70003C00;
    unsigned int *z = (unsigned int *)0x70003C00;
    float *cam = (float *)D_00187040;
    float *cv = (float *)D_0018CE00;
    float px, py, pz;
    float s24x, s24y, s26x, s26y;
    int i;

    px = cam[0x50] * 1024.0f;
    py = cam[0x51] * 1024.0f;
    pz = cam[0x52] * 1024.0f;
    s24x = cv[0xB0 / 4];
    s24y = cv[0xB0 / 4 + 1];
    s26x = cv[0x1E0 / 4];
    s26y = cv[0x1E0 / 4 + 1];

    for (i = 0; i < 0xC0 / 4; i++) {
        z[i] = 0;
    }

    for (;;) {
        unsigned char *p = (unsigned char *)list[0];
        unsigned int n = list[1];
        unsigned char *end;

        list += 2;
        if (p == 0) {
            return;
        }
        end = p + (n << 4);
        for (; p != end; p += 16) {
            float *v = (float *)p;
            float dist = (float)*(unsigned short *)(p + 0xC);
            float band = (float)((unsigned int)p[0xE] << 7);
            signed char *flag = &flags[p[0xF]];
            float dx, dy, dz, tx, ty, tz;
            float nearz, farz, farh, ex, ey;
            unsigned int fz, fh;

            if (*flag < 0) {
                continue;
            }
            dx = v[0] - px;
            dy = v[1] - py;
            dz = v[2] - pz;
            tx = cam[0] * dx;
            tx = tx + cam[4] * dy;
            tx = tx + cam[8] * dz;
            ty = cam[1] * dx;
            ty = ty + cam[5] * dy;
            ty = ty + cam[9] * dz;
            tz = cam[2] * dx;
            tz = tz + cam[6] * dy;
            tz = tz + cam[10] * dz;

            nearz = dist + tz;
            farz = (0.0f - dist) + tz;
            farh = farz - band * 0.5f;
            farz = farz - band;
            if (tx < 0.0f) tx = -tx;
            if (ty < 0.0f) ty = -ty;
            ex = s24x * tz - (tx - s26x * dist);
            ey = s24y * tz - (ty - s26y * dist);

            /* in front of the near edge and behind the far edge of the band */
            if (!(fbits_235EF0(farz) >> 31) || (fbits_235EF0(nearz) >> 31)) {
                continue;
            }
            if ((fbits_235EF0(ey) >> 31) || (fbits_235EF0(ex) >> 31)) {
                continue;
            }
            /* retail tests the 64-bit pair {nearz, farh} > 0 */
            fz = fbits_235EF0(nearz);
            fh = fbits_235EF0(farh);
            if (!(fh >> 31) && (fh | fz) != 0) {
                *flag = 1;
            } else {
                *flag = -1;
            }
        }
    }
}

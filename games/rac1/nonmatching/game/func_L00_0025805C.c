/* HeroEnvLighting: light colour of a moby from the environment zones around its position
   (a0 + 0x10). If func_L00_00257E18 finds a zone (record of 0x80 bytes in D_L00_0017EFC0,
   bit 0 of +0x50 set) at weight t, the colour at +0x80 becomes the per-channel blend of the
   zone's two colours (+0x40 at 255-k, +0x44 at k, k = (int)(255 t)) and +0x38 the zone's
   +0x48 | +0x4C << 8 | k << 16. Then, if func_L00_00257F4C finds a point light record r at
   weight t2, +0x3C = +0x80 plus r's colour (+0x10) scaled by 255 - (int)(t2 / r->0xC * 255),
   else +0x3C = +0x80. The packed-byte (MMI) arithmetic is spelled out per channel: each
   product's bits 8..15 are kept, and the two terms are added as bytes saturating at 255
   (paddub after psrlh/ppach/ppacb), not as a 16-bit sum. */
extern int func_L00_00257E18_m(void *, float *, int *) __asm__("func_L00_00257E18");
extern char *func_L00_00257F4C_m(void *, float *) __asm__("func_L00_00257F4C");
extern unsigned char D_L00_0017EFC0[];

void func_L00_0025805C(char *m) {
    int out[2];
    char *r;
    int i;

    if (func_L00_00257E18_m(m + 0x10, (float *)&out[0], &out[1]) != 0) {
        unsigned char *rec = D_L00_0017EFC0 + (out[1] << 7);
        if ((*(int *)(rec + 0x50) & 1) != 0) {
            int k = (int)(255.0f * *(float *)&out[0]);
            int w = 255 - k;
            unsigned int col = 0;
            for (i = 0; i < 4; i++) {
                int pc = (short)w * (short)rec[0x40 + i];
                int pd = (short)k * (short)rec[0x44 + i];
                unsigned int a = ((unsigned int)pd >> 8) & 0xFF;
                unsigned int b = ((unsigned int)pc >> 8) & 0xFF;
                unsigned int s = a + b;
                if (s > 0xFF) {
                    s = 0xFF;
                }
                col |= s << (i * 8);
            }
            *(unsigned int *)(m + 0x80) = col;
            *(int *)(m + 0x38) = *(int *)(rec + 0x48) | (*(int *)(rec + 0x4C) << 8) | (k << 16);
        }
    }

    r = func_L00_00257F4C_m(m + 0x10, (float *)&out[0]);
    if (r == 0) {
        *(int *)(m + 0x3C) = *(int *)(m + 0x80);
        return;
    }
    {
        int k = (int)(*(float *)&out[0] / *(float *)(r + 0xC) * 255.0f);
        int w = 255 - k;
        unsigned int base = *(unsigned int *)(m + 0x80);
        unsigned int col = 0;
        for (i = 0; i < 4; i++) {
            int pe = (short)((unsigned char *)r)[0x10 + i] * (short)w;
            unsigned int a = ((unsigned int)pe >> 8) & 0xFF;
            unsigned int s = a + ((base >> (i * 8)) & 0xFF);
            if (s > 0xFF) {
                s = 0xFF;
            }
            col |= s << (i * 8);
        }
        *(unsigned int *)(m + 0x3C) = col;
    }
}

/* func_00204918: RelocateTfrags, the level-load init of a tfrags block s (its table at s + w0,
   count w4, LOD base f8 at +8). Sets the LOD distances 6, 4 and 2 times the base in D_00160FA0 and
   calls func_00234380 with them; makes each tfrag's data pointer (+0x10) absolute; then for every
   ad-gif of every tfrag (0x50 bytes at data + tex_ofs + 0x50 * j, five quadwords) rewrites the data
   halves as GS registers: TEX0, TEX1, CLAMP, MIPTBP1, and MIPTBP2 = 0. The texture entry comes from
   table (a1), 16 bytes per texture index, and the GS base is D_0015EF8C >> 8.
   Checked against ReRAC's TfragAdGifs::gs_registers (bit layout). */
extern void func_00234380(void *);
extern int func_001F9968(int);
extern int D_0015EF8C MACRO_ADDR;
extern unsigned char *D_00160F8C_r __asm__("D_00160F8C") MACRO_ADDR;
extern int D_00160F90 MACRO_ADDR;
extern float D_00160FA0_r[3] __asm__("D_00160FA0") MACRO_ADDR;

void func_00204918(void *arg0, void *arg1) {
    unsigned char *s = (unsigned char *)arg0;
    unsigned char *table = (unsigned char *)arg1;
    float f;
    unsigned char *data;
    int n;
    int i;
    int j;
    int k;
    unsigned char *rec;
    unsigned char *ag;

    D_00160F90 = *(int *)(s + 4);
    f = *(float *)(s + 8);
    D_00160FA0_r[0] = f * 6.0f;
    D_00160FA0_r[1] = f * 4.0f;
    D_00160FA0_r[2] = f + f;
    func_00234380(D_00160FA0_r);

    data = s + *(int *)s;
    D_00160F8C_r = data;
    n = D_00160F90;

    for (k = 0; k < n; k++) {
        int *p = (int *)(data + 0x40 * k + 0x10);
        *p = *p + (int)data;
    }

    for (i = 0; i < n; i++) {
        rec = D_00160F8C_r + (i << 6);
        for (j = 0; j < rec[0x28]; j++) {
            int tex;
            int w;
            int h;
            int ty;
            int palette;
            int mipmap;
            int pad;
            int tw;
            int th;
            int base;
            int tbw;
            int tbw1;
            int wms;
            int wmt;
            int mmin;
            int kk;
            unsigned long tex0;
            unsigned long tex1;
            unsigned long clamp;
            unsigned long mip1;
            unsigned char *e;

            ag = (unsigned char *)(*(int *)(rec + 0x10) + *(unsigned short *)(rec + 0x1C) + j * 0x50);
            tex = *(int *)(ag + 0x00);
            e = table + (tex << 4);
            w = *(short *)(e + 4);
            h = *(short *)(e + 6);
            tw = func_001F9968(w);
            th = func_001F9968(h);
            base = D_0015EF8C >> 8;
            ty = *(short *)(e + 8);
            palette = *(short *)(e + 0xA);
            mipmap = *(short *)(e + 0xC);
            pad = *(short *)(e + 0xE);
            tbw = w >> 6;
            if (tbw <= 0) {
                tbw = 1;
            }
            tbw1 = w >> 7;
            if (tbw1 <= 0) {
                tbw1 = 1;
            }
            wms = *(int *)(ag + 0x20);
            wmt = *(int *)(ag + 0x24);
            kk = *(int *)(ag + 0x10);
            mmin = *(int *)(ag + 0x14);

            tex0 = (unsigned long)(long)tbw << 14;
            tex0 |= (unsigned long)(long)tw << 26;
            tex0 |= 0x01300000UL;
            tex0 |= (unsigned long)(long)th << 30;
            tex0 |= (unsigned long)1 << 34;
            tex0 |= (unsigned long)(long)(palette + base) << 37;
            tex0 |= (unsigned long)0x8000 << 48;
            tex1 = (unsigned long)(long)((ty - 1) << 2);
            tex1 |= (unsigned long)(long)mmin << 6;
            tex1 |= 0x20UL;
            tex1 |= (unsigned long)(unsigned int)kk << 32;
            clamp = (unsigned long)(long)wms;
            clamp |= (unsigned long)(long)wmt << 2;
            clamp |= (unsigned long)(long)tex << 24;
            mip1 = (unsigned long)(long)tbw1 << 14;
            mip1 |= (unsigned long)(long)(mipmap + base) << 20;
            mip1 |= (unsigned long)(long)(pad + base) << 40;
            mip1 |= (unsigned long)1 << 34;
            mip1 |= (unsigned long)1 << 54;

            *(unsigned long *)(ag + 0x00) = tex0;
            *(unsigned long *)(ag + 0x10) = tex1;
            *(unsigned long *)(ag + 0x20) = clamp;
            *(unsigned long *)(ag + 0x30) = mip1;
            *(unsigned long *)(ag + 0x40) = 0;
        }
    }
}

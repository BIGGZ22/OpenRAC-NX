/* SetPalMode: chooses the display mode (the D_0015EE80 flag picks the taller frame), lays out the
 * view rectangle around the GS's 2048 centre from the frame size, builds the GS register words for
 * the frame (scissor, offsets, frame and z buffers), sets the drawing environment, then clears the
 * texture pages a 32 x 32 block at a time.
 * Functional C for the port, written from the retail code: the same operations, not the same bytes. */
extern int D_0015EE80_3890 __asm__("D_0015EE80");
extern int D_0015EF74_3890 __asm__("D_0015EF74");
extern int D_0015EF78_3890 __asm__("D_0015EF78");
extern int D_0015EF80_3890 __asm__("D_0015EF80");
extern int D_0015EF84_3890 __asm__("D_0015EF84");
extern int D_0015EF88_3890 __asm__("D_0015EF88");
extern int D_0015EF8C_3890 __asm__("D_0015EF8C");
extern unsigned char D_00151880_3890[] __asm__("D_00151880");
extern int D_0013E600_3890[8] __asm__("D_0013E600");
extern unsigned long D_0013D010_3890[18] __asm__("D_0013D010");
extern unsigned long D_0013D200_3890 __asm__("D_0013D200");
extern unsigned long D_0013D270_3890 __asm__("D_0013D270");
extern unsigned char D_001942C0_3890[] __asm__("D_001942C0");
extern void flush_3890(int) __asm__("func_00118D80");
extern void sync_path_3890(int, int) __asm__("func_00120858");
extern void set_mode_3890(int, int, int, int, int, int) __asm__("func_001FAB40");
extern void put_draw_env_3890(void) __asm__("func_001FB498");
extern void put_disp_env_3890(void) __asm__("func_001FB530");
extern void frame_3890(void) __asm__("func_001FB470");
extern void fill_3890(void *, int, int) __asm__("func_001F99B0");
extern void def_load_image_3890(void *, int, int, int, int, int, int, int) __asm__("func_00122630");
extern void exec_load_image_3890(void *, void *) __asm__("func_00122958");

void func_001F3890(void) {
    unsigned char image[0x60];
    int *view = D_0013E600_3890;
    unsigned long *gs = D_0013D010_3890;
    int width, height, half_w, half_h, count, i;
    long scissor, offset, xy, frame;
    unsigned int zbuf;

    flush_3890(0);
    if (D_0015EE80_3890 != 0) {
        D_0015EF8C_3890 = 0x2C0000;
        D_0015EF84_3890 = 0x100000;
        D_0015EF88_3890 = 0x1E0000;
        D_0015EF80_3890 = 0;
        set_mode_3890(0x200, 0x1C0, 0x200, 0x200, 4, 0);
    } else {
        D_0015EF8C_3890 = 0x280000;
        D_0015EF84_3890 = 0xE0000;
        D_0015EF88_3890 = 0x1B0000;
        D_0015EF80_3890 = 0;
        set_mode_3890(0x200, 0x1A0, 0x200, 0x1C0, 0, 0);
    }
    width = *(short *)(D_00151880_3890 + 0x150);
    height = *(short *)(D_00151880_3890 + 0x152);
    half_w = width >> 1;
    half_h = height >> 1;
    view[0] = width;
    view[1] = height;
    view[2] = half_w;
    view[3] = half_h;
    view[4] = (0x800 - half_w) << 4;
    view[5] = (0x800 - half_h) << 4;
    view[6] = (half_w + 0x800) << 4;
    view[7] = (half_h + 0x800) << 4;
    flush_3890(0);
    sync_path_3890(0, 0);

    scissor = ((long)(view[0] - 1) << 16) | ((long)(view[1] - 1) << 48);
    zbuf = (unsigned int)(D_0015EF88_3890 >> 13) | 0x1000000;
    frame = (long)(D_0015EF84_3890 >> 13) | ((long)(view[0] >> 6) << 16);
    xy = (long)view[4] | ((long)view[5] << 32);
    offset = xy;
    D_0015EF78_3890 = D_0015EF8C_3890;
    D_0013D270_3890 = (long)(int)zbuf | 0x100000000L;
    gs[16] = scissor;
    gs[4] = frame;
    gs[10] = xy;
    gs[12] = offset;
    D_0013D200_3890 = (long)(int)zbuf;
    D_0015EF74_3890 = D_0015EF8C_3890;
    gs[6] = (long)(int)zbuf;
    gs[8] = (long)(int)zbuf;
    gs[2] = frame;
    gs[14] = scissor;
    flush_3890(0);

    put_draw_env_3890();
    put_disp_env_3890();
    flush_3890(0);
    sync_path_3890(0, 0);
    frame_3890();
    fill_3890(D_001942C0_3890, 0, 0x1000);
    count = (*(short *)(D_00151880_3890 + 0x158) * *(short *)(D_00151880_3890 + 0x15A)) >> 10;
    for (i = 0; i < count; i++) {
        def_load_image_3890(image, (short)(i << 4), 1, 0, 0, 0, 0x20, 0x20);
        flush_3890(0);
        exec_load_image_3890(image, D_001942C0_3890);
        sync_path_3890(0, 0);
    }
}

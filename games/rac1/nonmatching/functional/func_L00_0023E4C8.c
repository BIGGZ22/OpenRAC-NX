/* functional: medium. Draws four textured quads (a small cross/ring pattern of 4 quads built from the object's direction: texture 14, colours 0x3FF08080/0x3FF08030) in two rounds each via func_L00_001FD1D8; checked statically from the assembly only. */
typedef struct { float f[4]; } __attribute__((aligned(16))) Vec_3E4C8;
typedef struct {
    Vec_3E4C8 points[4];
    unsigned int colors[4];
    float texcoords[8];
    unsigned long texture[4];
} __attribute__((aligned(16))) Quad_3E4C8;
extern unsigned long tex_3E4C8(int) __asm__("func_001F4868");
extern void getpos_3E4C8(void *, int, void *) __asm__("func_0020DAF8");
extern void rotate_3E4C8(float, void *, void *, void *) __asm__("func_002156E0");
extern void cross_3E4C8(void *, void *, void *) __asm__("func_001F9CA0");
extern void scale_3E4C8(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void scale_to_3E4C8(float, void *, void *) __asm__("func_001F9C30");
extern void add_3E4C8(void *, void *, void *) __asm__("func_001F9BD8");
extern void clear_3E4C8(void *) __asm__("func_001F9BC0");
extern void draw_3E4C8(void *, void *, int) __asm__("func_L00_001FD1D8");
typedef int ti_3E4C8 __attribute__((mode(TI)));
extern float D_0015FBC0_3E4C8 __asm__("D_0015FBC0") __attribute__((section(".data")));
void func_L00_0023E4C8(void *m) {
    float four, quarter, minus_quarter, copied_x, other_x;
    int i;
    Vec_3E4C8 *p;
    Quad_3E4C8 quad;
    Vec_3E4C8 pos, rot, normal, base, scaled, offset, copied, other;

    quad.texture[1] = tex_3E4C8(14);
    quad.colors[0] = 0x3FF08080;
    quad.colors[1] = 0x3FF08080;
    quad.colors[2] = 0x3FF08030;
    quad.colors[3] = 0x3FF08030;
    quad.texture[2] = 0xFF9000000260UL;
    quad.texture[0] = 5;
    quad.texture[3] = 0x8000000048UL;
    quad.texcoords[0] = 0.0f;
    quad.texcoords[1] = 0.0f;
    quad.texcoords[2] = 1.0f;
    quad.texcoords[3] = 0.0f;
    quad.texcoords[4] = 0.0f;
    quad.texcoords[5] = 1.0f;
    quad.texcoords[6] = 1.0f;
    quad.texcoords[7] = 1.0f;
    getpos_3E4C8(m, 0, &pos);
    four = 4.0f;
    base.f[3] = 1.0f;
    rotate_3E4C8(-0.34906584f, &pos, &pos, &rot);
    cross_3E4C8(&rot, &pos, &normal);
    scale_3E4C8(&rot, &rot, 1.0f);
    cross_3E4C8(&normal, &rot, &pos);
    scale_to_3E4C8(four, &scaled, &pos);
    scaled.f[3] = 1.0f;
    add_3E4C8(&offset, &scaled, &base);
    *(ti_3E4C8 *)&copied = *(ti_3E4C8 *)&pos;
    add_3E4C8(&other, &copied, &pos);
    quarter = 0.25f;
    minus_quarter = -0.25f;
    copied.f[3] = quarter;
    other.f[3] = 0.5f;

    clear_3E4C8(&quad.points[0]);
    quad.points[0].f[2] = 0.0f;
    quad.points[0].f[3] = 1.0f;
    *(ti_3E4C8 *)&quad.points[1] = *(ti_3E4C8 *)&copied;
    quad.points[1].f[3] = 1.0f;
    quad.points[1].f[0] = four * copied.f[3];
    quad.points[1].f[2] += minus_quarter;
    clear_3E4C8(&quad.points[2]);
    quad.points[2].f[2] = 0.0f;
    quad.points[2].f[3] = 1.0f;
    *(ti_3E4C8 *)&quad.points[3] = *(ti_3E4C8 *)&copied;
    quad.points[3].f[2] += quarter;
    quad.points[3].f[3] = 1.0f;
    quad.points[3].f[0] = four * copied.f[3];
    draw_3E4C8(&quad, &pos, 1);
    i = 1;
    p = &quad.points[1];
    do {
        if (i < 2) {
            p->f[1] += minus_quarter;
            p->f[2] += quarter;
        } else {
            p->f[1] += quarter;
            p->f[2] += minus_quarter;
        }
        i += 2;
        p += 2;
    } while (i < 4);
    draw_3E4C8(&quad, &pos, 1);

    *(ti_3E4C8 *)&quad.points[0] = *(ti_3E4C8 *)&copied;
    copied_x = four * copied.f[3];
    quad.points[0].f[2] += minus_quarter;
    quad.points[0].f[3] = 1.0f;
    quad.points[0].f[0] = copied_x;
    *(ti_3E4C8 *)&quad.points[1] = *(ti_3E4C8 *)&other;
    other_x = four * other.f[3];
    quad.points[1].f[2] += minus_quarter;
    quad.points[1].f[3] = 1.0f;
    quad.points[1].f[0] = other_x;
    *(ti_3E4C8 *)&quad.points[2] = *(ti_3E4C8 *)&copied;
    quad.points[2].f[2] += quarter;
    quad.points[2].f[3] = 1.0f;
    quad.points[2].f[0] = copied_x;
    *(ti_3E4C8 *)&quad.points[3] = *(ti_3E4C8 *)&other;
    quad.points[3].f[2] += quarter;
    quad.points[3].f[3] = 1.0f;
    quad.points[3].f[0] = other_x;
    draw_3E4C8(&quad, &pos, 1);
    i = 0;
    p = &quad.points[0];
    do {
        if (i < 2) {
            p->f[1] += minus_quarter;
            p->f[2] += quarter;
        } else {
            p->f[1] += quarter;
            p->f[2] += minus_quarter;
        }
        i += 1;
        p += 1;
    } while (i < 4);
    draw_3E4C8(&quad, &pos, 1);

    *(ti_3E4C8 *)&quad.points[0] = *(ti_3E4C8 *)&other;
    quad.points[0].f[2] += minus_quarter;
    quad.points[0].f[3] = 1.0f;
    quad.points[0].f[0] = four * other.f[3];
    clear_3E4C8(&quad.points[1]);
    quad.points[1].f[2] += D_0015FBC0_3E4C8;
    quad.points[1].f[3] = 1.0f;
    quad.points[1].f[0] = four * copied.f[3] * 3.0f;
    *(ti_3E4C8 *)&quad.points[2] = *(ti_3E4C8 *)&other;
    quad.points[2].f[2] += quarter;
    quad.points[2].f[3] = 1.0f;
    quad.points[2].f[0] = four * other.f[3];
    clear_3E4C8(&quad.points[3]);
    quad.points[3].f[0] = four * copied.f[3] * 3.0f;
    quad.points[3].f[2] = 0.0f;
    quad.points[3].f[3] = 1.0f;
    draw_3E4C8(&quad, &pos, 1);
    i = 0;
    p = &quad.points[0];
    do {
        if (i < 2) {
            p->f[1] += minus_quarter;
            p->f[2] += quarter;
        } else {
            p->f[1] += quarter;
            p->f[2] += minus_quarter;
        }
        i += 2;
        p += 2;
    } while (i < 4);
    draw_3E4C8(&quad, &pos, 1);
}

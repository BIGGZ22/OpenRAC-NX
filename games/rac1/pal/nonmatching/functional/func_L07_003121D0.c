/* functional: high. Exact logic of retail func_L07_003121D0 (shared with level 13).
 * Ported from NTSC FUN_L07_00310df0 (src/overlays/shared/gameplay/entities/002b1e48.c) and verified against PAL assembly.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short s16;
typedef int s32;
typedef float f32;
typedef int openrac_ti __attribute__((mode(TI)));

extern u8 D_0014C150_3121D0[] __asm__("D_0014C150");
extern s32 D_0015EE84_3121D0 __asm__("D_0015EE84");
extern f32 D_0015EE6C_3121D0 __asm__("D_0015EE6C");
extern u8 *D_L07_00160058_3121D0 __asm__("D_L07_00160058");
extern s32 D_L07_001845A8_3121D0[] __asm__("D_L07_001845A8");

extern void func_001F9BD8_3121D0(void *, void *, void *) __asm__("func_001F9BD8");
extern f32 func_001F9CB8_3121D0(void *) __asm__("func_001F9CB8");
extern void func_L00_001FF4B0_3121D0(void *, void *, f32) __asm__("func_L00_001FF4B0");
extern void func_001F9EC0_3121D0(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_0022ED80_3121D0(s32, s32, void *) __asm__("func_0022ED80");

static inline void qcopy_3121D0(void *dst, const void *src) {
    *(openrac_ti *)dst = *(const openrac_ti *)src;
}

void func_L07_003121D0(u8 *moby) {
    u8 *data = *(u8 **)(moby + 0x78);
    u8 *other;
    u8 *position = moby + 0x10;
    f32 vectors[8];
    f32 distance;
    u8 state;

    if (moby[0xB0] != 0xFF &&
        D_0014C150_3121D0[moby[0xB0] + (D_0015EE84_3121D0 << 4)] == 0xFF) {
        *(s32 *)(data + 0x10) = -1;
    }

    state = moby[0x20];
    switch (state) {
    case 0:
        qcopy_3121D0(data, position);
        if (*(s32 *)(data + 0x38) != -1) {
            D_L07_001845A8_3121D0[*(s32 *)(data + 0x38)] = 0;
        }
        moby[0x20] = 1;
        *(s32 *)(data + 0x0C) = 0;
        return;
    case 1:
        if (*(s32 *)(data + 0x38) != -1) {
            D_L07_001845A8_3121D0[*(s32 *)(data + 0x38)] = 0;
        }
        if (*(s32 *)(data + 0x10) != -1) {
            other = D_L07_00160058_3121D0 + (*(s32 *)(data + 0x10) << 8);
            if (other != 0 && other[0x20] != 0xFE && other[0x20] != 0xFD &&
                other[0x20] != *(u32 *)(data + 0x14)) {
                return;
            }
        }
        *(openrac_ti *)(vectors + 4) = 0;
        vectors[4] = *(f32 *)(data + 0x1C);
        vectors[5] = *(f32 *)(data + 0x20);
        vectors[6] = *(f32 *)(data + 0x24);
        *(openrac_ti *)vectors = *(openrac_ti *)(vectors + 4);
        *(f32 *)(data + 0x0C) = func_001F9CB8_3121D0(vectors);
        if (*(s32 *)(data + 0x30) != -1) {
            func_0022ED80_3121D0(*(s32 *)(data + 0x30), 0, moby);
        }
        moby[0x20] = 2;
        return;
    case 2:
        if (*(s32 *)(data + 0x38) != -1) {
            D_L07_001845A8_3121D0[*(s32 *)(data + 0x38)] = 1;
        }
        if (*(s32 *)(data + 0x10) != -1) {
            other = D_L07_00160058_3121D0 + (*(s32 *)(data + 0x10) << 8);
            if (other != 0 && other[0x20] != 0xFE && other[0x20] != 0xFD &&
                other[0x20] == *(u32 *)(data + 0x18)) {
                if (*(s32 *)(data + 0x30) != -1) {
                    func_0022ED80_3121D0(*(s32 *)(data + 0x30), 0, moby);
                }
                moby[0x20] = 4;
                return;
            }
        }
        *(openrac_ti *)(vectors + 4) = 0;
        vectors[4] = *(f32 *)(data + 0x1C);
        vectors[5] = *(f32 *)(data + 0x20);
        vectors[6] = *(f32 *)(data + 0x24);
        *(openrac_ti *)vectors = *(openrac_ti *)(vectors + 4);
        func_001F9EC0_3121D0(vectors, vectors, moby + 0xC0);
        func_L00_001FF4B0_3121D0(vectors + 4, vectors, *(f32 *)(data + 0x28) * D_0015EE6C_3121D0);
        func_001F9BD8_3121D0(position, position, vectors + 4);
        *(f32 *)(data + 0x0C) -= *(f32 *)(data + 0x28) * D_0015EE6C_3121D0;
        if (!(*(f32 *)(data + 0x0C) < 0.0f)) {
            return;
        }
        func_001F9BD8_3121D0(position, data, vectors);
        *(f32 *)(data + 0x0C) = 0.0f;
        moby[0x20] = 3;
        if (*(s32 *)(data + 0x34) != -1) {
            func_0022ED80_3121D0(*(s32 *)(data + 0x30), 0, moby);
        }
        return;
    case 3:
        if (*(s32 *)(data + 0x38) != -1) {
            D_L07_001845A8_3121D0[*(s32 *)(data + 0x38)] = 1;
        }
        if (*(s32 *)(data + 0x10) == -1) {
            return;
        }
        other = D_L07_00160058_3121D0 + (*(s32 *)(data + 0x10) << 8);
        if (other == 0 || other[0x20] == 0xFE || other[0x20] == 0xFD ||
            other[0x20] != *(u32 *)(data + 0x18)) {
            return;
        }
        *(s32 *)(data + 0x0C) = 0;
        if (*(s32 *)(data + 0x30) != -1) {
            func_0022ED80_3121D0(*(s32 *)(data + 0x30), 0, moby);
        }
        moby[0x20] = 4;
        return;
    case 4:
        if (*(s32 *)(data + 0x38) != -1) {
            D_L07_001845A8_3121D0[*(s32 *)(data + 0x38)] = 0;
        }
        if (*(s32 *)(data + 0x10) == -1 ||
            (other = D_L07_00160058_3121D0 + (*(s32 *)(data + 0x10) << 8)) == 0 ||
            other[0x20] == 0xFE || other[0x20] == 0xFD ||
            other[0x20] == *(u32 *)(data + 0x14)) {
            if (*(s32 *)(data + 0x30) != -1) {
                func_0022ED80_3121D0(*(s32 *)(data + 0x30), 0, moby);
            }
            moby[0x20] = 2;
        } else {
            *(openrac_ti *)(vectors + 4) = 0;
            vectors[4] = *(f32 *)(data + 0x1C);
            vectors[5] = *(f32 *)(data + 0x20);
            vectors[6] = *(f32 *)(data + 0x24);
            *(openrac_ti *)vectors = *(openrac_ti *)(vectors + 4);
            distance = func_001F9CB8_3121D0(vectors);
            func_L00_001FF4B0_3121D0(vectors, vectors, -(*(f32 *)(data + 0x2C) * D_0015EE6C_3121D0));
            func_001F9EC0_3121D0(vectors + 4, vectors, moby + 0xC0);
            func_001F9BD8_3121D0(position, position, vectors + 4);
            *(f32 *)(data + 0x0C) += *(f32 *)(data + 0x2C) * D_0015EE6C_3121D0;
            if (!(distance < *(f32 *)(data + 0x0C))) {
                return;
            }
            qcopy_3121D0(position, data);
            if (*(s32 *)(data + 0x30) != -1) {
                func_0022ED80_3121D0(*(s32 *)(data + 0x34), 0, moby);
            }
            if (*(s32 *)(data + 0x38) != -1) {
                D_L07_001845A8_3121D0[*(s32 *)(data + 0x38)] = 0;
            }
            moby[0x20] = 1;
        }
        break;
    case 5:
    default:
        return;
    }
}

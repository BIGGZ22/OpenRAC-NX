extern int vram_offset_x;
extern int vram_offset_y;

void VRAM_GetOffset__FPiT0(int *x, int *y) {
    *x = vram_offset_x;
    *y = vram_offset_y;
}

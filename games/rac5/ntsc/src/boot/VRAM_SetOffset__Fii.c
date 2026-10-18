extern int vram_offset_x;
extern int vram_offset_y;

void VRAM_SetOffset__Fii(int x, int y) {
    vram_offset_x = x;
    vram_offset_y = y;
}

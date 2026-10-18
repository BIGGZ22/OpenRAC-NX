typedef struct { float u; float v; float ou; float ov; } tTEX_STATE;

extern tTEX_STATE tex_state;
extern char tex_offset_dirty;

void TexOffset__Q34scee3Psp2Guff(float u, float v) {
    tex_offset_dirty = 1;
    tex_state.ou = u;
    tex_state.ov = v;
}

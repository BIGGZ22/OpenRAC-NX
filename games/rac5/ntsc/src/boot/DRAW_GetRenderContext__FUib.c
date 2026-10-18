extern char draw_render_contexts[];

void *DRAW_GetRenderContext__FUib(unsigned int i, int b) {
    char *p = draw_render_contexts + i * 0x170C;
    return p + b * 0xE4;
}

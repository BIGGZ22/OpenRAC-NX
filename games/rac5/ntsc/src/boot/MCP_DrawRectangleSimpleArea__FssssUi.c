extern void UTIL_DrawFlatShadedRect__FssssUiUi(short, short, short, short,
                                                unsigned int, unsigned int);

void MCP_DrawRectangleSimpleArea__FssssUi(short x1, short y1, short x2, short y2,
                                          unsigned int colour) {
    UTIL_DrawFlatShadedRect__FssssUiUi(x1, y1, x2 - x1, y2 - y1, colour, 0xFFFFFF);
}

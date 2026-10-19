int MCP_GetFontPage__FUc(unsigned char page) {
    if ((unsigned char)(page + 0x6E) < 2) {
        return page - 0x92;
    }
    return 0;
}

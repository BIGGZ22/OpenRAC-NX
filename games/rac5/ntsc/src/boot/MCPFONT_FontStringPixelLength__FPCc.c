extern int MCPFONT_FontStringLength__FPCc(const char *);
extern int MCPFONT_FontStringPixelLength__FPCcUi(const char *, unsigned int);

int MCPFONT_FontStringPixelLength__FPCc(const char *s) {
    if (s == 0)
        return 0;
    return MCPFONT_FontStringPixelLength__FPCcUi(s, MCPFONT_FontStringLength__FPCc(s));
}

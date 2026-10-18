int MCP_FontStringLength__FPCc(const char *text) {
    int count;
    if (!text) return 0;
    count = 0;
    if (*text) {
        do {
            ++count;
        } while (text[count]);
    }
    return count;
}

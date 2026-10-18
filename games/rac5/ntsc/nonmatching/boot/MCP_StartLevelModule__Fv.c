extern unsigned int g_level_index;
extern char *g_level_names[];
extern unsigned int g_mcp_flag;
extern char *g_level_names_alt[];
extern char g_McpGlobals[];
extern int MCPI_GetTheirMsgBoxId__Fv(void);
extern int MCPI_GetMyMsgBoxId__Fv(void);
extern int GetThreadId(void);
extern int MCPI_GetGameId__Fv(void);
extern void MODULE_StartModule__FiiPCvi(int gameId, int length, char *args, int size);
extern char *strcpy(char *dst, const char *src);
extern unsigned int strlen(const char *s);
extern int sprintf(char *dst, const char *fmt, ...);
extern char str_mm[];
extern char str_mt[];
extern char str_im[];
extern char str_g[];

void MCP_StartLevelModule__Fv(void) {
    char buffer[0x100];
    char *name;
    char *p;

    name = g_level_names[g_level_index];
    if (g_mcp_flag != 0)
        name = g_level_names_alt[g_level_index];
    strcpy(buffer, name);
    p = buffer + strlen(name);
    *p++ = 0;
    p += sprintf(p, str_mm, 0, MCPI_GetTheirMsgBoxId__Fv());
    *p++ = 0;
    p += sprintf(p, str_mt, 0, MCPI_GetMyMsgBoxId__Fv());
    *p++ = 0;
    p += sprintf(p, str_im, 0, GetThreadId());
    *p++ = 0;
    p += sprintf(p, str_g, 0, (int)g_McpGlobals);
    *p++ = 0;
    MODULE_StartModule__FiiPCvi(MCPI_GetGameId__Fv(), p - buffer, buffer, 0x2C);
}

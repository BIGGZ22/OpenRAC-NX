extern void MCPI_HandleMessages__Fb(int on);
extern int MCP_GetState__Fv(void);
extern void MCP_StateIdle__Fv(void);
extern void MCP_StateLoading__Fv(void);
extern void MCP_StatePlaying__Fv(void);
extern void MCP_UpdatePost__Fv(void);
extern void DelayThread(unsigned int usec);

void MCP_Update__Fv(void) {
    MCPI_HandleMessages__Fb(0);
    switch (MCP_GetState__Fv()) {
    case 1:
        MCP_StateIdle__Fv();
        break;
    case 2:
        MCP_StateLoading__Fv();
        break;
    case 3:
        MCP_StatePlaying__Fv();
        break;
    case 4:
        DelayThread(0x13880);
        break;
    }
    MCP_UpdatePost__Fv();
}

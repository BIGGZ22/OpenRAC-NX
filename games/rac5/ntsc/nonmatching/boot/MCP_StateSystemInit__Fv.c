extern int mcp_state_1EEDAB0;
extern int mcp_state;

extern void mcp_state_system_init_callback_1E66AC8(void);

void MCP_StateSystemInit__Fv(void) {
    mcp_state_1EEDAB0 = 0;
    mcp_state = 1;
    mcp_state_system_init_callback_1E66AC8();
}

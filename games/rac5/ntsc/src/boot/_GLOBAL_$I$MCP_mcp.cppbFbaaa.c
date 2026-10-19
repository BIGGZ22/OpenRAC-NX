extern void mcp_init_1E65F20(int initialize, unsigned int priority);

void GLOBAL_I_MCP_mcp(void) __asm__("_GLOBAL_$I$MCP_mcp.cppbFbaaa");

void GLOBAL_I_MCP_mcp(void) {
    mcp_init_1E65F20(1, 0xFFFF);
}

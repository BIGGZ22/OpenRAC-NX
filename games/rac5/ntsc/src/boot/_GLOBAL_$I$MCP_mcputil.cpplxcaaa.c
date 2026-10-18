extern void mcputil_init_1E67648(int initialize, unsigned int priority);

void GLOBAL_I_MCP_mcputil(void) __asm__("_GLOBAL_$I$MCP_mcputil.cpplxcaaa");

void GLOBAL_I_MCP_mcputil(void) {
    mcputil_init_1E67648(1, 0xFFFF);
}

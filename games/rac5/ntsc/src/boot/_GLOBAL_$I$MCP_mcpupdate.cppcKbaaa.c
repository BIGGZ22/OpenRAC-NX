extern void mcpupdate_init_1E67288(int initialize, unsigned int priority);

void GLOBAL_I_MCP_mcpupdate(void) __asm__("_GLOBAL_$I$MCP_mcpupdate.cppcKbaaa");

void GLOBAL_I_MCP_mcpupdate(void) {
    mcpupdate_init_1E67288(1, 0xFFFF);
}

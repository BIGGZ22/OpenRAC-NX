extern unsigned char g_McpGlobals[]; /* 0x01F4A740 */

int MCP_HasLevelBeenSeen__FUi(unsigned int level) {
    if (g_McpGlobals) return (g_McpGlobals + 0x1F20)[level] & 1;
    return 0;
}

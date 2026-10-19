/* Per-asset-type byte counters, an array of 19 ints at g_GlobalContext + 0x20. */
struct GlobalContextUsage {
    char pad00[0x20];
    unsigned int asset_usage[19]; /* 0x20 */
};
extern struct GlobalContextUsage g_GlobalContext;

void MEM_AddAssetUsage__F12MEMAssetTypeUi(int type, unsigned int amount) {
    if (type < 19) {
        g_GlobalContext.asset_usage[type] = g_GlobalContext.asset_usage[type] + amount;
    }
}

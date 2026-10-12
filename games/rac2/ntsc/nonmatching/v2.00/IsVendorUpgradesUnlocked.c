extern unsigned char g_vendorUpgradesUnlocked;

int IsVendorUpgradesUnlocked(void)
{
    return g_vendorUpgradesUnlocked != 0;
}

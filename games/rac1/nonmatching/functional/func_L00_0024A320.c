/* functional: medium. If a >= 190 returns (byte at D_0013D4E0 != 0); otherwise (joined tail) returns 1 if f >= 58.5f (0x426A0000) else 0, f being the first float argument. */
extern unsigned char D_0013D4DF_4A320[] __asm__("D_0013D4DF") NOT_SDA;
int func_L00_0024A320(int a, float f) {
    if (a < 0xBE) {
        return f >= 58.5f;
    }
    return D_0013D4DF_4A320[1] != 0;
}

/* sceDmaGetChan 0x01E9CDE8..0x01E9CE10 */
extern void *dch[];
void *sceDmaGetChan(unsigned int ch) {
    if (ch >= 10) return 0;
    return dch[ch];
}

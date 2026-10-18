/* popmalloctotal 0x01EAB498..0x01EAB4A8 */
extern int popmalloc_grand_total;
int popmalloctotal(void) {
    return popmalloc_grand_total;
}

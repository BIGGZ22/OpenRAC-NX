/* The point light nearest a position (HeroEnvLighting): the light grid *D_L00_0015FC80 has a word per
   16-unit square (64 per row, x = (int)pos.x >> 4, y = (int)pos.y >> 4, both truncated); a non-zero word
   is the offset of a list {count; light indices} from the grid start. The first listed light (records of
   32 bytes at *D_L00_0015FC40: xyz, w = radius squared) whose (dx*dx + dy*dy) - w is negative is
   returned, with dx*dx + dy*dy stored at *dist (x and y only); else 0. The tail is the separate
   catalogue units func_L00_00257FB4 and func_L00_0025804C. The hand-written loop runs at least once
   (with a count of 0 it would not end; one pass here). */
extern unsigned int D_L00_0015FC80_w[] __asm__("D_L00_0015FC80");
extern char *D_L00_0015FC40_p[] __asm__("D_L00_0015FC40");

char *func_L00_00257F4C(float *pos, float *dist) {
    int x = (int)pos[0];
    int y = (int)pos[1];
    char *grid = (char *)D_L00_0015FC80_w[0];
    unsigned int word = *(unsigned int *)(grid + (((unsigned int)y >> 4) * 64 + ((unsigned int)x >> 4)) * 4);
    int *list;
    int n;
    int i;
    float *l;
    float dx, dy, d2;

    if (word == 0) {
        return 0;
    }
    list = (int *)(grid + word);
    n = list[0];
    i = 0;
    do {
        l = (float *)(D_L00_0015FC40_p[0] + list[1 + i] * 32);
        dx = pos[0] - l[0];
        dy = pos[1] - l[1];
        d2 = dx * dx + dy * dy;
        if (d2 - l[3] < 0.0f) {
            *dist = d2;
            return (char *)l;
        }
        i++;
    } while (i < n);
    return 0;
}

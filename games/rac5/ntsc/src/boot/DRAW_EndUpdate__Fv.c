extern int g_draw_currentMaterial[];

void DRAW_EndUpdate__Fv(void) {
    int i = 0;
    do {
        g_draw_currentMaterial[i] = 0;
        i++;
    } while (i == 0);
}

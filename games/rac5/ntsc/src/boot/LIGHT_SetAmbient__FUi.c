extern unsigned int light_ambient_color; /* 0x01EF5864 */

void LIGHT_SetAmbient__FUi(unsigned int color) {
    light_ambient_color = color;
}

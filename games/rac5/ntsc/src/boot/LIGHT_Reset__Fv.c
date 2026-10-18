extern int light_ambient;  /* 0x01EF5870 */
extern int light_diffuse;  /* 0x01EF5874 */
extern int light_specular; /* 0x01EF5878 */

void LIGHT_Reset__Fv(void) {
    light_ambient = 0;
    light_diffuse = 0;
    light_specular = 0;
}

extern unsigned char system_blend_enabled; /* 0x01EF5701 */

void SYSTEM_SetBlendEnabled__Fb(unsigned char enabled) {
    system_blend_enabled = enabled;
}

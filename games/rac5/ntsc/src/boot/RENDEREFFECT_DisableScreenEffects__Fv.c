extern char gScreenEffectA; /* 0x01EF1054 */
extern char gScreenEffectB; /* 0x01EF104C */
extern char gScreenEffectC; /* 0x01EF1058 */

void RENDEREFFECT_DisableScreenEffects__Fv(void) {
    gScreenEffectA = 0;
    gScreenEffectB = 0;
    gScreenEffectC = 0;
}

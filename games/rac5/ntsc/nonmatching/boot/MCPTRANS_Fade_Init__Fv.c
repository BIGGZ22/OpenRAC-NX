extern float sinf(float);
extern float sqrtf(float);

extern unsigned char gMcpTransFadeReady;
extern float gMcpTransFadeTable[128];

void MCPTRANS_Fade_Init__Fv(void)
{
    unsigned int k;
    unsigned int i;
    unsigned int j;
    float phase;

    if (gMcpTransFadeReady) {
        return;
    }
    phase = 0.0f;
    for (k = 0; k < 0x80; k++) {
        gMcpTransFadeTable[k] = sinf(phase);
        phase += 0.0490873866f;
    }
    for (i = 0; i < 0x31; i++) {
        for (j = 0; j < 0x23; j++) {
            float d;
            float dy;
            float s;

            s = 1.7777778f;
            d = (float)i - 24.0f;
            d = d * d;
            dy = (float)j - 17.0f;
            dy = dy * dy;
            dy = dy * s;
            d = d + dy;
            if (d > 0.1f) {
                sqrtf(d);
            }
        }
    }
    gMcpTransFadeReady = 1;
}

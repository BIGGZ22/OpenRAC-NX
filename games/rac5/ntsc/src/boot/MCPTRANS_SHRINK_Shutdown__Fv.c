extern unsigned char mcptrans_shrink_active;
extern unsigned char mcptrans_shrink_flag6F4;
extern int mcptrans_shrink_w70C;
extern int mcptrans_shrink_w710;
extern void *mcptrans_shrink_sound;
extern char mcptrans_spiral1[];
extern char mcptrans_spiral2[];
extern void Spiral_Stop__FP12Spiral_Group(void *);
extern void SND_StopPositionalSound__FP14SND_Positionalb(void *, int);

void MCPTRANS_SHRINK_Shutdown__Fv(void) {
    if (mcptrans_shrink_active != 0) {
        Spiral_Stop__FP12Spiral_Group(mcptrans_spiral1);
        Spiral_Stop__FP12Spiral_Group(mcptrans_spiral2);
        mcptrans_shrink_active = 0;
        mcptrans_shrink_flag6F4 = 0;
        mcptrans_shrink_w70C = 0;
        mcptrans_shrink_w710 = 0;
    }
    if (mcptrans_shrink_sound != 0) {
        SND_StopPositionalSound__FP14SND_Positionalb(mcptrans_shrink_sound, 1);
        mcptrans_shrink_sound = 0;
    }
}

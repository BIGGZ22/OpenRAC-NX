typedef struct { char b[16]; } SND_Positional;

extern SND_Positional *mcptrans_shutdown_sound;
extern void SND_StopPositionalSound__FP14SND_Positionalb(SND_Positional *s, int flag);
extern void SND_MCPShutdown__Fv(void);

void MCPTRANS_Shutdown__Fv(void) {
    SND_Positional *snd = mcptrans_shutdown_sound;
    if (snd) {
        SND_StopPositionalSound__FP14SND_Positionalb(snd, 1);
        mcptrans_shutdown_sound = 0;
        SND_MCPShutdown__Fv();
    }
}

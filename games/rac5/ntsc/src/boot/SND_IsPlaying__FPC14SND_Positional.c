typedef struct {
    int state;
} SND_Positional;

int SND_IsPlaying__FPC14SND_Positional(const SND_Positional *sound) {
    if (sound == 0) return 0;
    return (unsigned int)(sound->state - 2) < 2;
}

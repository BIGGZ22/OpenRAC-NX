typedef struct {
    char pad00[0x58];
    unsigned int seed;
} RandState;

extern RandState *g_randState;

void srand(unsigned int seed)
{
    g_randState->seed = seed;
}

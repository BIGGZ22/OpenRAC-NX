/* NON_MATCHING func_L05_00254838 -- src/overlays/shared/help_00237B00.c
 * Best so far: SIZE ours 764 / retail 768, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-07 mini30 main-only fresh reconstruction (8 runs). Updates challenge statistics while D_0013D355[13B] 
 *   p0 SIZE756/768; p1 load statistic after elapsed call and branch local756; p2 inline angle addresses and pre-lo
 */
extern unsigned char D_0013D355[];
extern unsigned char D_0013E633[] NOT_SDA;
extern char D_0014171B[] NOT_SDA;
extern int D_0015EFA4 MACRO_ADDR;
extern int D_0015EE84 MACRO_ADDR;
extern int func_001F9850(int);
extern float func_001F9B88(float);
extern float func_001F9878(float);
extern int func_001FA898(float);
typedef struct { float value[5]; } ScoreWeights_254838;
extern ScoreWeights_254838 D_L05_00216D68;
typedef struct {
    char pad0[0x6D0]; float angle[3]; char pad6DC[4];
    int actions[4]; int actionCount; float multiplier;
    int elapsed, actionScore; int turns[3]; int maxTurns;
    int turnScore, total; char pad718[0x190]; int active;
} ScoreState_254838;
typedef struct { char pad0[0xF0]; unsigned short attempts, best; unsigned int levels; } ScoreStats_254838;

/* Updates action and rotation scores while the challenge is active. */
void func_L05_00254838(void) {
    ScoreState_254838 *g;
    int i;
    if (D_0013D355[0x13B] == 0) return;
    g = (ScoreState_254838 *)(D_0013E633 + 0xE1D);
    if (g->active != 0) {
        ScoreStats_254838 *stats = (ScoreStats_254838 *)(D_0014171B + 0x22D);
        int elapsed;
        if (stats->attempts < 0xFFFF) stats->attempts++;
        elapsed = func_001F9850(D_0015EFA4);
        if (stats->best < elapsed / 600)
            stats->best = func_001F9850(D_0015EFA4) / 600;
        stats->levels |= (1 << D_0015EE84) | 0x80000000;
    }
    {
        int *turn = (int *)(D_0013E633 + 0x14ED + 0x30);
        i = 2;
        do {
            if (func_001F9B88(((float *)turn)[-12]) > 3.3161256f) {
                float value = ((float *)turn)[-12];
                int count = *turn;
                if (value > 0.0f) value -= 6.2831855f;
                else value += 6.2831855f;
                ((float *)turn)[-12] = value;
                *turn = count + 1;
            }
            turn++;
        } while (--i >= 0);
    }
    g = (ScoreState_254838 *)(D_0013E633 + 0xE1D);
    g->maxTurns = 0;
    {
        int *turn = g->turns;
        i = 2;
        do {
            if (g->maxTurns < *turn) g->maxTurns = *turn;
            turn++;
        } while (--i >= 0);
    }
    g = (ScoreState_254838 *)(D_0013E633 + 0xE1D);
    {
    int step = 100;
    g->turnScore = 0;
    if (g->maxTurns > 0) {
        int score = 0;
        i = g->maxTurns;
        do { score += step; step += 50; } while (--i != 0);
        g->turnScore = score;
    }
    }
    g = (ScoreState_254838 *)(D_0013E633 + 0xE1D);
    g->actionCount = 0;
    {
        int *action = g->actions;
        i = 3;
        do { if (*action != 0) g->actionCount++; action++; } while (--i >= 0);
    }
    {
        ScoreWeights_254838 weights = D_L05_00216D68;
        g = (ScoreState_254838 *)(D_0013E633 + 0xE1D);
        g->multiplier = weights.value[g->actionCount];
        g->actionScore = func_001FA898((float)((g->elapsed * 100) / (int)(func_001F9878(1.0f) * 60.0f)));
        g->actionScore += g->actionCount * 25;
        g->actionScore = func_001FA898((float)g->actionScore * g->multiplier);
        g->total = g->turnScore + g->actionScore;
    }
}

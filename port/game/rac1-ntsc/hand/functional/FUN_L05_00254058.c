/* Carried from the PAL decompilation's nonmatching/shared/func_L05_00254838.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* NON_MATCHING FUN_L05_00254058 -- src/overlays/shared/help_00237B00.c
 * Best so far: SIZE ours 764 / retail 768, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-07 mini30 main-only fresh reconstruction (8 runs). Updates challenge statistics while D_0013D255[13B] 
 *   p0 SIZE756/768; p1 load statistic after elapsed call and branch local756; p2 inline angle addresses and pre-lo
 */
extern unsigned char D_0013D255[];
extern unsigned char D_0013E533[] NOT_SDA;
extern char D_0014161B[] NOT_SDA;
extern int D_0015EEA4 MACRO_ADDR;
extern int D_0015ED84 MACRO_ADDR;
extern int func_001F9688(int);
extern float func_001F99C0(float);
extern float func_001F96B0(float);
extern int func_001FA6D0(float);
typedef struct { float value[5]; } ScoreWeights_254838_c254058;
extern ScoreWeights_254838_c254058 D_L05_002169F8;
typedef struct {
    char pad0[0x6D0]; float angle[3]; char pad6DC[4];
    int actions[4]; int actionCount; float multiplier;
    int elapsed, actionScore; int turns[3]; int maxTurns;
    int turnScore, total; char pad718[0x190]; int active;
} ScoreState_254838_c254058;
typedef struct { char pad0[0xF0]; unsigned short attempts, best; unsigned int levels; } ScoreStats_254838_c254058;

/* Updates action and rotation scores while the challenge is active. */
void FUN_L05_00254058(void) {
    ScoreState_254838_c254058 *g;
    int i;
    if (D_0013D255[0x13B] == 0) return;
    g = (ScoreState_254838_c254058 *)(D_0013E533 + 0xE1D);
    if (g->active != 0) {
        ScoreStats_254838_c254058 *stats = (ScoreStats_254838_c254058 *)(D_0014161B + 0x22D);
        int elapsed;
        if (stats->attempts < 0xFFFF) stats->attempts++;
        elapsed = func_001F9688(D_0015EEA4);
        if (stats->best < elapsed / 600)
            stats->best = func_001F9688(D_0015EEA4) / 600;
        stats->levels |= (1 << D_0015ED84) | 0x80000000;
    }
    {
        int *turn = (int *)(D_0013E533 + 0x14ED + 0x30);
        i = 2;
        do {
            if (func_001F99C0(((float *)turn)[-12]) > 3.3161256f) {
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
    g = (ScoreState_254838_c254058 *)(D_0013E533 + 0xE1D);
    g->maxTurns = 0;
    {
        int *turn = g->turns;
        i = 2;
        do {
            if (g->maxTurns < *turn) g->maxTurns = *turn;
            turn++;
        } while (--i >= 0);
    }
    g = (ScoreState_254838_c254058 *)(D_0013E533 + 0xE1D);
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
    g = (ScoreState_254838_c254058 *)(D_0013E533 + 0xE1D);
    g->actionCount = 0;
    {
        int *action = g->actions;
        i = 3;
        do { if (*action != 0) g->actionCount++; action++; } while (--i >= 0);
    }
    {
        ScoreWeights_254838_c254058 weights = D_L05_002169F8;
        g = (ScoreState_254838_c254058 *)(D_0013E533 + 0xE1D);
        g->multiplier = weights.value[g->actionCount];
        g->actionScore = func_001FA6D0((float)((g->elapsed * 100) / (int)(func_001F96B0(1.0f) * 60.0f)));
        g->actionScore += g->actionCount * 25;
        g->actionScore = func_001FA6D0((float)g->actionScore * g->multiplier);
        g->total = g->turnScore + g->actionScore;
    }
}

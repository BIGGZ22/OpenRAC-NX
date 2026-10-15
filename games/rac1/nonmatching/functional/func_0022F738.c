/* func_0022F738 -- src/game/space.c (functional C for the port, not a match)
 * The ship's two engine trails in the space scenes (DrawSpaceScene, func_002305A0): each trail is
 * the ring of 32 positions at D_0013E1F0 + 0x200 * trail (the newest at D_0013E130 +0x50, the count
 * drawn +0x54), drawn as a ribbon of two quads a segment. One quad spans the cross of the trail's
 * direction with the line between the two engines, the other that line itself, so the ribbon is seen
 * from any side. Each quad's corners sit on two consecutive positions, pushed out by
 * (1 - t^2) * width and tinted from the colour pair to the fade colour by t = age / 32. The colour
 * pair (D_001D9DC0) and width (D_001605D0) are picked by the ship (D_0013E130 +0x26). Effect
 * texture 0x13 (0 on the loading flight, mode 6 with +0x20 == 4); the texture coordinates come from
 * D_001D9DA0. Written from the retail routine. */
typedef struct {
    float pos[4][4];
    unsigned int col[4];
    float st[4][2];
    unsigned long long clamp;
    unsigned long long tex0;
    unsigned long long tex1;
    unsigned long long alpha;
} TrailQuad_22F738 __attribute__((aligned(16)));

extern int D_0015F6E8;
extern char D_0013E130[];
extern float D_0013E1F0[64][4];
extern float D_001D9DA0[4][2];
extern unsigned int D_001D9DC0[][2];
extern float D_001605D0[];
extern unsigned long long func_001F4868(int);
extern float func_001FA888(int);
extern unsigned int func_001FA8A8(unsigned int, unsigned int, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F9DC0(void *, void *, float);
extern void func_001F7EF8(void *, void *, int);

static void trail_quad_22F738(TrailQuad_22F738 *q, float (*dirs)[4], int age, int k, float *trail, int tint) {
    int ship = *(short *)(D_0013E130 + 0x26);
    int m;
    for (m = 0; m < 4; m++) {
        float t = func_001FA888(age - (m >> 1)) * 0.03125f;
        if (tint) {
            q->col[m] = func_001FA8A8(D_001D9DC0[ship][0], D_001D9DC0[ship][1], t);
        }
        func_001F9DC0(q->pos[m], dirs[m], (1.0f - t * t) * D_001605D0[ship]);
        func_001F9BD8(q->pos[m], q->pos[m], trail + ((k + (m >> 1)) & 0x1F) * 4);
    }
    func_001F7EF8(q, 0, 0);
}

void func_0022F738(int ship) {
    TrailQuad_22F738 q;
    float across[4][4]; /* engine 1 - engine 0 here, its negation, the same one position on */
    float side[4][4];   /* across x the trail's direction, for each corner */
    float along[2][4];
    int i;
    int j;
    int n;
    int tex = 0x13;

    (void)ship;
    if (D_0015F6E8 == 6 && *(int *)(D_0013E130 + 0x20) == 4) {
        tex = 0;
    }
    q.tex0 = func_001F4868(tex);
    q.tex1 = 0xFF9000000260ULL;
    q.alpha = 0x8000000048ULL;
    q.clamp = 0;
    for (n = 0; n < 4; n++) {
        q.st[n][0] = D_001D9DA0[n][0];
        q.st[n][1] = D_001D9DA0[n][1];
    }
    for (i = 0; i < *(int *)(D_0013E130 + 0x54) - 1; i++) {
        int k = (*(int *)(D_0013E130 + 0x50) - i + 0x1F) & 0x1F;
        int k1 = (k + 1) & 0x1F;
        int k2 = (k + 2) & 0x1F;
        for (j = 0; j < 2; j++) {
            float *trail = D_0013E1F0[j * 32];
            func_001F9BF0(across[0], D_0013E1F0[32 + k], D_0013E1F0[k]);
            func_001F9BF0(across[1], D_0013E1F0[k], D_0013E1F0[32 + k]);
            func_001F9BF0(across[2], D_0013E1F0[32 + k1], D_0013E1F0[k1]);
            func_001F9BF0(across[3], D_0013E1F0[k1], D_0013E1F0[32 + k1]);
            func_001F9BF0(along[0], trail + k1 * 4, trail + k * 4);
            func_001F9BF0(along[1], trail + k2 * 4, trail + k1 * 4);
            if (i == 0) {
                along[1][0] = along[0][0];
                along[1][1] = along[0][1];
                along[1][2] = along[0][2];
                along[1][3] = along[0][3];
            }
            func_001F9CA0(side[0], across[0], along[0]);
            func_001F9CA0(side[1], across[1], along[0]);
            func_001F9CA0(side[2], across[2], along[1]);
            func_001F9CA0(side[3], across[3], along[1]);
            trail_quad_22F738(&q, side, i + 1, k, trail, 1);
            trail_quad_22F738(&q, across, i + 1, k, trail, 0);
        }
    }
}

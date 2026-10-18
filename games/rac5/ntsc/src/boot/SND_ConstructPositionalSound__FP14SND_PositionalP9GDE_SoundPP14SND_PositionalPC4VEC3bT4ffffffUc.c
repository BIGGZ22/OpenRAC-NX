/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    float x;
    float y;
    float z;
} VEC3;

typedef struct {
    int f00;             /* 0x00 */
    int f04;             /* 0x04 */
    int f08;             /* 0x08 */
    char _pad0C[4];
    int f10;             /* 0x10 */
    float f14;           /* 0x14 */
    float f18;           /* 0x18 */
    unsigned char f1C;   /* 0x1C */
    unsigned char f1D;
    unsigned char f1E;
    unsigned char f1F;
    unsigned char f20;
    unsigned char f21;
    unsigned char f22;
    char _pad23;
    VEC3 v24;            /* 0x24 */
    char _pad30[0x0C];
    float f3C;
    float f40;
    float f44;
    float f48;
} SND_Positional;

void SND_ConstructPositionalSound__FP14SND_PositionalP9GDE_SoundPP14SND_PositionalPC4VEC3bT4ffffffUc(
    SND_Positional *self, void *snd, SND_Positional **prev, const VEC3 *pos,
    int flag, float f1, float f2, float f3, float f4, float f5, float f6,
    int uc) {
    self->f04 = (int)snd;
    self->f10 = (int)prev;
    self->f14 = f5;
    self->f18 = f6;
    self->f08 = 0;
    self->f1F = 0;
    self->f1C = 0;
    if (pos) {
        self->f20 = 1;
        self->v24 = *pos;
    } else {
        self->f20 = 0;
    }
    self->f21 = flag;
    self->f22 = uc;
    self->f3C = f3;
    self->f40 = f4;
    self->f44 = f1;
    self->f48 = f2;
}

typedef struct { char b[12]; } Spiral_Group;

extern void PAR_CreateGroup__FP9PAR_Groupi13PAR_BlockMode(void *group, int count, int mode);

void Spiral_CreateGroup__FP12Spiral_Groupi(Spiral_Group *g, int count) {
    PAR_CreateGroup__FP9PAR_Groupi13PAR_BlockMode(g, count, 1);
}

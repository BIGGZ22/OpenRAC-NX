typedef struct {
    char pad_00[0x08];
    int  field_08;
    int  field_0C;
    unsigned short field_10;
} tPAR_GROUP;

extern void PAR_ClearGroup__FP9PAR_Group(tPAR_GROUP *g);

void PAR_DestroyGroup__FP9PAR_Group(tPAR_GROUP *g) {
    PAR_ClearGroup__FP9PAR_Group(g);
    g->field_0C = 0;
    g->field_10 = 0x20;
    g->field_08 = 0;
}

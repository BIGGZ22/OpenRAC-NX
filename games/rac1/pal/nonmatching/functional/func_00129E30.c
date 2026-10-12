/* functional: high. Same calls, arguments, reads and writes as the assembly (q1.c form): records the picture fields into arg0, looks up D_00132F98 by the 4-bit index, then outputs the picture through func_0012A7E8 or func_001299E8 and tail-calls func_00129C78 when the picture fits. Only the register choice for the copies differs. */
extern int func_00129CA0(void *, void *, void *, void *, void *);
extern int func_00129948(void *, void *);
extern void func_0012A7E8(void *, void *);
extern int func_00129C78(void *);
extern unsigned int D_00132F98[];

struct DrArg0 {
    char pad0[0x80];
    int unk80;
    char pad84[4];
    unsigned long unk88;
    char pad90[0x20];
    int unkB0;
    int unkB4;
    int unkB8;
    int unkBC;
    int unkC0;
    int unkC4;
    int unkC8;
    int unkCC;
    int unkD0;
    char padD4[0x858 - 0xD4];
    void *unk858;
};

struct DrTbl {
    char pad0[0x10];
    int unk10;
    char pad14[0xC];
    long unk20;
};

struct DrArg1 {
    char pad0[0x28];
    int unk28;
    char pad2C[0x44 - 0x2C];
    int unk44;
    int unk48;
    int unk4C;
    int unk50;
    int unk54;
    int unk58;
    int unk5C;
    int unk60;
};

void func_00129E30(struct DrArg0 *arg0, struct DrArg1 *arg1) {
    unsigned long temp_6_28;
    struct DrTbl *temp_7_14;
    char *temp_7_9;

    temp_7_9 = arg0->unk858;
    func_00129CA0(arg0, arg1, temp_7_9 + 0x10, temp_7_9 + 0x18, temp_7_9 + 0x20);
    temp_7_14 = arg0->unk858;
    arg0->unk80 = temp_7_14->unk10;
    temp_6_28 = D_00132F98[(int)((temp_7_14->unk20 << 27) >> 32) & 0xF];
    arg0->unkCC = arg1->unk5C;
    arg0->unk88 = temp_6_28;
    arg0->unkD0 = arg1->unk60;
    arg0->unkB4 = arg1->unk44;
    arg0->unkB8 = arg1->unk48;
    arg0->unkBC = arg1->unk4C;
    arg0->unkC0 = arg1->unk50;
    arg0->unkC4 = arg1->unk54;
    arg0->unkC8 = arg1->unk58;
    if (func_00129948(arg0, arg1) != 0) {
        if (arg1->unk28 == 1) {
            if (arg0->unkB0 != 0) {
                func_0012A7E8(arg0, arg1);
            } else {
                func_001299E8((struct Cpr8Mpeg *)arg0, (struct Cpr8Pic *)arg1);
            }
            func_00129C78(arg0);
        }
    }
}

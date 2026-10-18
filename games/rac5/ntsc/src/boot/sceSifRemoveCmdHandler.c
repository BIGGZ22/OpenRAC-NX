typedef struct {
    int field0;
    int field1;
    int field2;
} SifCmdHandler;

extern SifCmdHandler *D_1FE3EA4;
extern SifCmdHandler *D_1FE3EAC;

void sceSifRemoveCmdHandler(int idx) {
    if (idx < 0) {
        D_1FE3EA4[idx & 0x7FFFFFFF].field0 = 0;
    } else {
        D_1FE3EAC[idx].field0 = 0;
    }
}

typedef struct tMEMCARD_STATE_NODE {
    int field0;
    int field4;
    int field8;
    int fieldC;
} tMEMCARD_STATE_NODE;

void MEMCARD_InitStateNode__FP19tMEMCARD_STATE_NODE(tMEMCARD_STATE_NODE *node) {
    node->field8 = 1;
    node->fieldC = 0;
    node->field0 = 0;
    node->field4 = 0;
}

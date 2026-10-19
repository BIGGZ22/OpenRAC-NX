/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Moby state machine on m[0x20]: state 0 sets the 0x8000 flag on m->0x34 when data[0] is set, stores data->0xC = m->0x48 and turns m->0x48 by 90 degrees (FastSubRots if data[0] else FastAddRots), then (states 0 and 1) if the object data[2] names has byte 0xBC set plays class sound 0 (func_0022ED80) and goes to state 2; state 2 turns data->0xC by 0.698 rad the same way and eases m->0x48 toward it with func_L00_0025CE58. */
extern float func_001FA790_17B68(float, float) __asm__("func_001FA790");
extern float func_001FA748_17B68(float, float) __asm__("func_001FA748");
extern int func_0022ED80_17B68(int, int, int) __asm__("func_0022ED80");
extern float func_L00_0025CE58_17B68(void *, float, void *, float, float, float) __asm__("func_L00_0025CE58");
extern int D_L05_00160098_17B68 __asm__("D_L05_00160098");
extern float D_0015EE70_17B68 __asm__("D_0015EE70");
extern float D_0015EE6C_17B68 __asm__("D_0015EE6C");

void func_L05_00317B68(char *m) {
    int *data = *(int **)(m + 0x78);
    unsigned char state = *(unsigned char *)(m + 0x20);

    if (state != 1) {
        if (state == 0) {
            if (data[0] != 0) {
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x8000;
            }
            *(unsigned char *)(m + 0x20) = 1;
            ((float *)data)[3] = *(float *)(m + 0x48);
            if (data[0] != 0) {
                *(float *)(m + 0x48) = func_001FA790_17B68(*(float *)(m + 0x48), 1.5707964f);
            } else {
                *(float *)(m + 0x48) = func_001FA748_17B68(*(float *)(m + 0x48), 1.5707964f);
            }
        } else if (state == 2) {
            float r;
            if (data[0] != 0) {
                r = func_001FA790_17B68(((float *)data)[3], 0.6981317f);
            } else {
                r = func_001FA748_17B68(((float *)data)[3], 0.6981317f);
            }
            func_L00_0025CE58_17B68(m + 0x48, r, data + 1,
                                    D_0015EE70_17B68 * 12.566371f,
                                    D_0015EE70_17B68 * 12.566371f,
                                    D_0015EE6C_17B68 * 12.566371f);
            return;
        } else {
            return;
        }
    }
    if (*(unsigned char *)(((char *)D_L05_00160098_17B68) + (data[2] << 8) + 0xBC) != 0) {
        func_0022ED80_17B68(0, 0, (int)m);
        *(unsigned char *)(m + 0x20) = 2;
    }
}

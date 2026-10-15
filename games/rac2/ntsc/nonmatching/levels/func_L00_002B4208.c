/* Best plain-C attempt from worker w08. Last check: 216 bytes vs 208; 0/27. */
typedef unsigned char u8;
typedef float f32;
typedef int CandidateV4 __attribute__((mode(TI)));

typedef struct {
    u8 pad0[0x98];
    f32 f98;
    u8 pad1[0x1B0 - 0x9C];
    f32 f1B0;
    f32 f1B4;
    u8 pad2[0x22C3 - 0x1B8];
    u8 flag;
} CandidateOwner;

extern CandidateOwner D_00189E20;
extern char D_00189F10[];
extern float func_002EE6E0(float, float);
extern float func_002EE6F8(float, float);
extern CandidateV4 func_002EE5D8(CandidateV4 *, CandidateV4 *, char *);
extern CandidateV4 func_00323E88(CandidateV4 *, float, float);

void func_L00_002B4208(float angle)
{
    float a;
    float b;
    float values[4];
    CandidateV4 result;

    if (angle <= 3.1415927f)
        a = angle;
    else
        a = D_00189E20.f98;
    if (D_00189E20.flag != 0) {
        a = func_002EE6E0(0.0f, a) * D_00189E20.f1B4;
        b = func_002EE6F8(0.0f, a) * D_00189E20.f1B4;
        values[0] = a;
        values[1] = b;
        values[2] = 0.0f;
        values[3] = 0.0f;
        result = func_002EE5D8((CandidateV4 *)values, (CandidateV4 *)values, (char *)&D_00189E20);
    } else {
        result = func_00323E88((CandidateV4 *)values, D_00189E20.f1B4, *(float *)((char *)&D_00189E20 + 0x324));
    }
    *(CandidateV4 *)D_00189F10 = result;
}

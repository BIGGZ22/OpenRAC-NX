extern int gAnimFrameCount; /* 0x01F7E290 */
extern int gAnimStateA;     /* 0x01EF58FC */
extern int gAnimSkeletonList;     /* 0x01F7E294 */
extern int gAnimStateC;     /* 0x01F7E2A8 */
extern int gAnimStateD;     /* 0x01F7E2AC */
extern int gAnimStateE;     /* 0x01F7E2B0 */
extern int gAnimStateF;     /* 0x01F7E2B4 */
extern int gAnimStateG;     /* 0x01F7E2B8 */

void ANIM_Begin__Fv(void) {
    gAnimFrameCount++;
    gAnimStateA = 0;
    gAnimSkeletonList = 0;
    gAnimStateC = 0;
    gAnimStateD = 0;
    gAnimStateE = 0;
    gAnimStateF = 0;
    gAnimStateG = 0;
}

/* fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* Near miss func_L12_00304038 with the variable-length array replaced by a fixed buffer for the port (hostgen cannot take a VLA). */
/* NON_MATCHING func_L12_00304038 -- src/overlays/l12_hoven/vendor_002EDAA0.c
 * Best so far: BYTES 4/724 (99.5% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   mini16 a03: updates four point/tangent curves using a VLA and queues a nearby draw callback. Best p5.c BYTES4/
 *   Scoped per-loop indices, pre-store spacing local, and MACRO_ADDR revision scalar align all instructions. Expli
 */
#include "common.h"
extern int changed(int *) __asm__("func_001F9908");
extern void func_001F9BC0(float *);
extern void func_L00_00258DB0(float *, float, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern float func_001F9D10(void *, void *);
extern void func_L12_00303CA8(Moby *);
extern float D_0015EE6C MACRO_ADDR;
extern int D_L12_00161E84 MACRO_ADDR;
extern float D_L12_00161E7C SDATA(D_L12_00161E7C);
extern float D_L12_00161E78 SDATA(D_L12_00161E78);
extern float D_L12_00161E80 SDATA(D_L12_00161E80);
extern char D_L12_001672C0[];
typedef struct CurveVec { float x,y,z,w; } CurveVec;
typedef struct CurveData { CurveVec points[4][70]; CurveVec tangents[4][70]; int flags[4]; char pad[0x14]; float length; char pad2[4]; int count; } CurveData;
/* Updates four curves and registers a nearby object's draw callback. */
void func_L12_00304038(char *m) {
 CurveData *d = *(CurveData **)(m+0x78);
 int i;
 D_L12_00161E78 += D_L12_00161E7C * D_0015EE6C;
 if (D_L12_00161E78 > 1.0f) D_L12_00161E78 -= 1.0f;
 for(i=0;i<4;i++) {
  if(changed(d->flags+i)) {
   float pts[70][4]; /* N=70: points[4][70] bounds count */
   { int j; for(j=0;j<d->count;j++) {
    CurveVec *p=&d->points[i][j];
    func_001F9BC0((float *)p);
    { float step=d->length / d->count;
    p->w=1.0f;
    p->x=j * step; }
    func_L00_00258DB0(pts[j],0.0f,D_L12_00161E80*D_0015EE6C);
   } }
   { int j; for(j=1;j<d->count-1;j++) {
    CurveVec *p=&d->tangents[i][j];
    func_001F9BD8(p,pts[j-1],pts[j]);
    func_001F9BD8(p,p,pts[j+1]);
    func_001F9C30(p,p,0.333f);
   } }
   d->flags[i]=D_L12_00161E84;
  } else {
   { int j; for(j=1;j<d->count-1;j++) {
    CurveVec *p=&d->points[i][j];
    func_001F9BD8(p,p,&d->tangents[i][j]);
   } }
  }
 }
 if(func_001F9D10(m+0x10,D_L12_001672C0)<48.0f) func_001F49B0(func_L12_00303CA8,m);
}

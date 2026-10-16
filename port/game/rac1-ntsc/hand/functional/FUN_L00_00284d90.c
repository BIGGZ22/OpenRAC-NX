/* Carried from the PAL decompilation's src/overlays/shared/pause_00277208.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
typedef struct { short uid; short bolts; } BoltSource_286068_c284d90;
extern BoltSource_286068_c284d90 D_0014D590[][64];



/* the bolt tally slot of moby uid in level lvl: scanning down from slot 63, the first slot that
   holds uid (stored as uid + 1) or is free, which it claims; -1 when there is none or uid < 0 */
int FUN_L00_00284d90(int lvl, int uid) {
    int k = -1;

    if (uid >= 0) {
        uid++;
        k = 63;
        while (k >= 0 && D_0014D590[lvl][k].uid != uid && D_0014D590[lvl][k].uid != 0) {
            k--;
        }
        if (k >= 0) {
            D_0014D590[lvl][k].uid = uid;
        }
    }
    return k;
}

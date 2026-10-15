# func_001EC2B8 (camera.c, switch to camera)

Near-miss: 118/768, right size. What is left is the saved-register
choice for three pointers (retail src $s3, old camera $s4, the tail's
D_00187040 base $s5; ours old $s3, tail base $s4, src $s5), the
temporaries that follow from it, and `src`'s place in the t == 2 block
(ours before the +0x270 toggle's load, retail just before its store).

Levers that got it to size:
- D_00187040 is reached through block-local `char *c = D_00187040`
  pointers (one outer `g` for the +0x180 read and the blend-5 block), so
  its %hi stays in $s1 and each block re-adds %lo like retail.
- `src = cam + 0x30` is assigned in every branch, before the branch's
  last store where retail has it, so cross-jumping does not merge them.
- The case-6 toggle is an if/else with two stores through its own local.
- Float store pairs are written in the order that schedules into
  retail's (0x294 before 0x288 in source where retail stores 0x288 first
  and fills the delay slot with 0x294).
- D_0015EE84 is MACRO_ADDR (retail's `lui $2 / lw $2` reuse).

Tried without effect: declaration orders, reading +0x7E before the
blend test (grows), no `t` local, the tail through the symbol (grows).

Best candidate (118/768):

```c
typedef struct {
    char unk_00[0x1C];
    char *unk1C;
} CamRec20s;
extern CamRec20s *D_0015F090_s __asm__("D_0015F090") MACRO_ADDR;
extern char D_00187040[];
extern char D_00189750[];
extern int D_0018C42C;
extern int D_0015EE84 MACRO_ADDR;
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_001EC270(void *);

/* Camera_switchToCam(UpdateCam *): makes cam the current camera. How it
   takes over depends on the old camera's exit mode (+0x7E) and the new
   one's blend type (its record's +0x1D): cut, timed blend (+0x78 seconds,
   or a default), or inheriting the old camera's matrix and position.
   Then the old camera is reset and becomes the previous one, the
   position history buffers are swapped, and the new camera's setup
   hook and backup run. */
void func_001EC2B8(void *arg0) {
    char *cam = arg0;
    int blend;
    char *g;
    char *old;
    char *rec;
    int t;
    char *src;

    blend = 0;
    g = D_00187040;
    rec = D_0015F090_s[*(short *)(cam + 0x84)].unk1C;
    old = *(char **)(g + 0x180);

    if (rec != 0) {
        blend = (unsigned char)rec[0x1D];
    }
    t = *(short *)(old + 0x7E);
    if (t == 4) {
        src = cam + 0x30;
        *(short *)(cam + 0x8E) = 1;
    } else if (t == 2 || blend == 1 || blend == 5) {
        if (blend == 1) {
            char *c = D_00187040;
            float s = *(float *)(cam + 0x78);

            c[0x273] = 0;
            if (s > 0.0f) {
                *(float *)(c + 0x294) = s;
                *(float *)(c + 0x288) = s;
            } else {
                *(float *)(c + 0x294) = 0.018f;
                *(float *)(c + 0x288) = 0.018f;
            }
        } else if (blend == 5) {
            float s = *(float *)(cam + 0x78);

            g[0x273] = 2;
            if (s > 0.0f) {
                *(int *)(g + 0x2F4) = func_001FA898_r(s);
            } else {
                *(int *)(g + 0x2F4) = 0x28;
            }
        }
        src = cam + 0x30;
        {
            char *c = D_00187040;

            *(short *)(c + 0x270) = *(short *)(c + 0x270) == 0 ? 1 : 2;
        }
    } else if (t == 3 || t == 5 || blend == 3 || blend == 6) {
        qcopy(cam + 0x30, old + 0x30);
        qcopy(cam, old);
        qcopy(cam + 0x10, old + 0x10);
        qcopy(cam + 0x20, old + 0x20);
        cam[0x7D] = 2;
        src = cam + 0x30;
        if (*(short *)(old + 0x7E) == 5 || blend == 6) {
            {
                char *c = D_00187040;
                float s = *(float *)(cam + 0x78);

                c[0x273] = 0;
                if (s > 0.0f) {
                    *(float *)(c + 0x294) = s;
                    *(float *)(c + 0x288) = s;
                } else {
                    *(float *)(c + 0x294) = 0.018f;
                    *(float *)(c + 0x288) = 0.018f;
                    if (D_0015EE84 == 1) {
                        *(float *)(c + 0x294) = 0.01f;
                        *(float *)(c + 0x288) = 0.01f;
                    }
                }
            }
            {
                char *c = D_00187040;

                if (*(short *)(c + 0x270) == 0) {
                    *(short *)(c + 0x270) = 1;
                } else {
                    *(short *)(c + 0x270) = 2;
                }
            }
        }
    } else {
        *(short *)(cam + 0x8E) = 1;
        src = cam + 0x30;
    }
    *(short *)(old + 0x7E) = 0;
    old[0x7D] = 0;
    *(short *)(old + 0x8E) = 0;
    {
        char *c = D_00187040;
        char *dst;

        *(char **)(c + 0x184) = old;
        func_001F9A98(D_00189750, D_00189750 - 0x280, 0x280);
        *(char **)(*(char **)(c + 0x184) + 0x70) = D_00189750;
        *(char **)(c + 0x180) = cam;
        *(char **)(cam + 0x70) = D_00189750 - 0x280;
        *(int *)(c + 0x398) = 0;
        func_001EC270(cam);
        func_001EC038();
        if (D_0018C42C == 0) {
            qcopy(c + 0x140, src);
        }
        dst = cam + 0x64;
        *(float *)(cam + 0x64) = *(float *)(cam + 0x30);
        *(float *)(dst + 4) = *(float *)(src + 4);
        *(float *)(dst + 8) = *(float *)(src + 8);
    }
}
```

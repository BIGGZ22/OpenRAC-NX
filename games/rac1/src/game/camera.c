#include "common.h"
#include "vector.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * camera.cpp in the original source; text 0x1EC038-0x1EDFF8.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern int func_001E97C8(void *arg0);

/* Same signature as the declaration further down this file; duplicate
   identical declarations are legal and avoid a signature clash. */
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;

/* BackupCurrentCam */
void BackupCurrentCam(void) {
    FastMemCopy(D_00189310, D_001871C0, 0xA0);
    FastMemCopy(D_001899D0, D_001899D0 - 0x500, 0x280);
    *(void **)(D_00189310 + 0x70) = D_001899D0;
}

extern int D_0015F08C MACRO_ADDR;
extern void (*D_001893B0[])(void);

/* ExecuteCamPostUpdFuncs: runs the D_0015F08C queued post-update
   callbacks, then empties the queue. */
/* D_0015F08C as MACRO_ADDR removes the hoisted address register (frame
   0x30); a plain indexed loop. */
void ExecuteCamPostUpdFuncs(void) {
    int i;

    for (i = 0; i < D_0015F08C; i++) {
        D_001893B0[i]();
    }
    D_0015F08C = 0;
}

/* Not a standalone function: no `jr $31` -- dead-value computation
   (`$v0 = 0` twice with intervening nops) then a store, falling through
   to whatever follows. Same fallthrough-fragment category as
   func_00113AD8 in core_text. */
LINKER_REMNANT("asm/remnants/text", func_001EC108);

extern float func_001F9B88(float);

/* Cam_InterpValues(a, b, p, c, d, e): steps *p toward b - a, clamps it
   to +-e and to +-func_001F9B88(b - a), and returns a + *p. The nop
   between the first compare and its bc1f is ps2eeas's
   (tools/ps2eeas_nops.py). */
float Cam_InterpValues(float a, float b, float *p, float c, float d, float e) {
    float diff = b - a;
    float v = *p;

    v = v + (c * diff - d * v);
    *p = v;
    if (e != 0.0f) {
        if (e < v) {
            *p = e;
        } else if (v < -e) {
            *p = -e;
        }
    }
    if (FastAbsF(diff) < *p) {
        *p = FastAbsF(diff);
    } else if (-FastAbsF(diff) > *p) {
        *p = -FastAbsF(diff);
    }
    return a + *p;
}

/* Not a standalone function: single `addiu $sp,$sp,0x50`, no `jr $31` --
   fallthrough fragment, same category as func_00113AD8 in core_text. */
LINKER_REMNANT("asm/remnants/text", func_001EC208);

extern char D_001871D0[];
extern void func_0020D678(void *); /* DeleteMoby */

/* Camera_handleCollWithHero: spawns (via func_001E97C8) or deletes the
   moby kept at D_001871D0+0xC4, depending on the flag at arg0+0x86. */
/* A `char *c` base, the `== 0` arm first, and DeleteMoby takes the slot
   as its argument. */
void Camera_handleCollWithHero(void *arg0) {
    char *c = D_001871D0;

    if (*(short *)((char *)arg0 + 0x86) == 0) {
        if (*(void **)(c + 0xC4) == 0) {
            *(int *)(c + 0xC4) = func_001E97C8(c - 0x50);
        }
    } else {
        if (*(void **)(c + 0xC4) != 0) {
            DeleteMoby(*(void **)(c + 0xC4));
            *(void **)(c + 0xC4) = 0;
        }
    }
}

/* 0x14-byte dispatch records, indexed by the type id at +0x8C.
   Declared as a real struct array, not `char[]` + byte offset: the two
   forms are not codegen-equivalent here. Retail emits `addu $2,$2,$3`
   (base, index); a char-pointer form emits `addu $2,$3,$2` (index,
   base) and no amount of reordering the C addition changes it, because
   GCC canonicalises the PLUS before operand order is chosen. Indexing
   a typed array puts the base first. See func_001EC270/func_001EC780. */
extern DispatchRec D_001E8F80[];

/*
 * 1/68, and the residual is one commutative-operand-order byte: retail
 * emits `addu $2,$2,$3` (base + index), this compiler `addu $2,$3,$2`
 * (index + base). Same instruction, same destination, same size.
 *
 * Getting here took two real fixes worth reusing. Writing the field read
 * as `... * 0x14 + 8` folds the +8 into the %lo address constant instead
 * of leaving it as a `lw` offset (7/68); computing the record pointer
 * first and reading `rec + 8` separately fixes that. And building the
 * pointer with `rec += idx` rather than in the initialiser makes the sum
 * land in the base's register as retail does, rather than the index's
 * (3/68 -> 1/68) -- the documented in-place-accumulate lever.
 *
 * The last byte resisted an explicit index local and both `rec += idx`
 * and `rec = rec + idx`, which is the known scratch-register/operand
 * choice question. Kept per the same-size-tiny-diff precedent.
 */
/* Camera_runSetupToNewCam(UpdateCam *) */
void Camera_runSetupToNewCam(void *arg0) {
    void (*fn)(void *) = D_001E8F80[*(short *)((char *)arg0 + 0x8C)].fn_08;
    if (fn != 0) {
        fn(arg0);
    }
}

typedef u32 u128 __attribute__((mode(TI), aligned(16)));
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Vec3;
struct UpdateCam {
    u128 m0;
    u128 m1;
    u128 m2;
    Vec4f pos;
    u8 pad40[0x24];
    Vec3 prev_pos;
    void *saved_state;
    u8 state; /* activation rule, see camera_activation_check_priority */
    u8 pad75[3];
    f32 transition_duration;
    u8 active;
    u8 handoff_state;
    s16 transition_state;
    u8 pad80[4];
    s16 descriptor_index;
    s16 unk86; /* 0 lets the hero collision moby spawn; 6 disables shake */
    u8 pad88[4];
    s16 type; /* index into camera_types (D_001E8C00) */
    s16 activation_blocked;
    u8 pad90[0x10];
};
typedef struct {
    u8 pad0[0x1D];
    u8 descriptor_kind;
} CameraDescriptorInfo;
typedef struct {
    u8 pad0[0x1C];
    CameraDescriptorInfo *descriptor;
} CameraDescriptor;
struct CameraTransitionState {
    u8 pad0[0x140];
    u128 published_position;
    u8 pad150[0x30];
    struct UpdateCam *current;
    struct UpdateCam *previous;
    u8 pad188[0xE8];
    s16 transition_phase;
    u8 pad272;
    u8 transition_mode;
    u8 pad274[0x14];
    f32 configured_rotation_rate;
    u8 pad28C[8];
    f32 configured_position_rate;
    u8 pad298[0x5C];
    s32 configured_frames;
    u8 pad2F8[0xA0];
    s32 snapshot_pending;
};
extern struct CameraTransitionState D_00187040_EC2B8 __asm__("D_00187040");
extern CameraDescriptor * D_0015F090_EC2B8 __asm__("D_0015F090") MACRO_ADDR;
extern s32 D_0015EE84 MACRO_ADDR;
extern u8 D_00189750[];
extern s32 D_0018C42C_EC2B8[] __asm__("D_0018C42C");
extern void func_001EC038(void);
extern void func_001EC270_EC2B8(struct UpdateCam *next_camera) __asm__("func_001EC270");
extern void func_001F9A98(void *dst, void *src, s32 size);
extern s32 func_001FA898(f32 transition_duration);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/textbin/fun_001ebf10.c, switch_active_camera_record. */
void Camera_SwitchActive(struct UpdateCam *next_camera) {

    struct UpdateCam *previous_camera;
    CameraDescriptorInfo *descriptor;
    f32 transition_duration;
    f32 mode_one_duration;
    f32 handoff_duration;
    f32 *previous_position;
    Vec4f *position;
    s32 descriptor_kind = 0;
    s32 previous_transition_state;

    descriptor = D_0015F090_EC2B8[next_camera->descriptor_index].descriptor;
    previous_camera = D_00187040_EC2B8.current;
    previous_transition_state = previous_camera->transition_state;
    if (descriptor != 0) {
        descriptor_kind = descriptor->descriptor_kind;
    }
    if (previous_transition_state == 4) {
        next_camera->activation_blocked = 1;
    } else if (previous_transition_state == 2 || descriptor_kind == 1 || descriptor_kind == 5) {
        if (descriptor_kind == 1) {
            mode_one_duration = next_camera->transition_duration;
            D_00187040_EC2B8.transition_mode = 0;
            if (mode_one_duration > 0.0f) {
                D_00187040_EC2B8.configured_position_rate = mode_one_duration;
                D_00187040_EC2B8.configured_rotation_rate = mode_one_duration;
            } else {
                D_00187040_EC2B8.configured_position_rate = 0.018f;
                D_00187040_EC2B8.configured_rotation_rate = 0.018f;
            }
        } else if (descriptor_kind == 5) {
            transition_duration = next_camera->transition_duration;
            D_00187040_EC2B8.transition_mode = 2;
            if (transition_duration > 0.0f) {
                D_00187040_EC2B8.configured_frames =
                    truncate_float_to_s32(transition_duration);
            } else {
                D_00187040_EC2B8.configured_frames = 40;
            }
        }
        if (D_00187040_EC2B8.transition_phase == 0) {
            D_00187040_EC2B8.transition_phase = 1;
        } else {
            D_00187040_EC2B8.transition_phase = 2;
        }
    } else if (previous_transition_state == 3 || previous_transition_state == 5 ||
               descriptor_kind == 3 || descriptor_kind == 6) {
        qcopy(&next_camera->pos, &previous_camera->pos);
        qcopy(&next_camera->m0, &previous_camera->m0);
        qcopy(&next_camera->m1, &previous_camera->m1);
        qcopy(&next_camera->m2, &previous_camera->m2);
        next_camera->handoff_state = 2;
        if (previous_camera->transition_state == 5 || descriptor_kind == 6) {
            handoff_duration = next_camera->transition_duration;
            D_00187040_EC2B8.transition_mode = 0;
            if (handoff_duration > 0.0f) {
                D_00187040_EC2B8.configured_position_rate = handoff_duration;
                D_00187040_EC2B8.configured_rotation_rate = handoff_duration;
            } else {
                D_00187040_EC2B8.configured_position_rate = 0.018f;
                D_00187040_EC2B8.configured_rotation_rate = 0.018f;
                if (D_0015EE84 == 1) {
                    D_00187040_EC2B8.configured_position_rate = 0.01f;
                    D_00187040_EC2B8.configured_rotation_rate = 0.01f;
                }
            }
            if (D_00187040_EC2B8.transition_phase == 0) {
                D_00187040_EC2B8.transition_phase = 1;
            } else {
                D_00187040_EC2B8.transition_phase = 2;
            }
        }
    } else {
        next_camera->activation_blocked = 1;
    }

    position = &next_camera->pos;
    previous_camera->transition_state = 0;
    previous_camera->handoff_state = 0;
    previous_camera->activation_blocked = 0;
    D_00187040_EC2B8.previous = previous_camera;
    FastMemCopy(D_00189750, D_00189750 - 0x280,
                           0x280);
    D_00187040_EC2B8.previous->saved_state = D_00189750;
    D_00187040_EC2B8.current = next_camera;
    next_camera->saved_state = D_00189750 - 0x280;
    D_00187040_EC2B8.snapshot_pending = 0;
    func_001EC270_EC2B8(next_camera);
    BackupCurrentCam();
    /* The callbacks run before this flag is read; previous_position is updated either way. */
    if (D_0018C42C_EC2B8[0] == 0) {
        qcopy(&D_00187040_EC2B8.published_position, &next_camera->pos);
    }
    previous_position = &next_camera->prev_pos.x;
    previous_position[0] = next_camera->pos.x;
    previous_position[1] = position->y;
    previous_position[2] = position->z;
}

typedef struct {
    char unk_00[0x10];
    int unk10;
    char unk14[8];
    int unk1C;
} CamRec20;
extern CamRec20 *D_0015F090 MACRO_ADDR;
extern CamRec20 *D_0015F040 MACRO_ADDR;
extern char D_0013F450[];
extern char D_0013F4D0[];
extern int func_00215570(void *arg0, int arg1);
typedef struct {
    char unk_00[4];
    int (*fn_04)(void *, void *);
    char unk_08[0xC];
} CamPrioHook;
extern CamPrioHook D_001E8F80_prio[] __asm__("D_001E8F80");

/* Camera_ActivationCheckPriority(cur, other): whether camera `cur` should
   take over from `other`. An inactive camera (+0x7C) never does; the camera
   type's +4 hook (D_001E8F80) may decide first (-1 no, 1 yes); otherwise
   the mode at +0x74 decides: 0 (and 1/2 once +0x7D is set) by priority
   byte, 4 through func_00215570 on the target record, 7 by the level's
   mode and target. Each case ends in `if (x) return 1;` falling out to the
   one shared `return 0;`, which is what lets cross-jumping and reorg give
   retail's branches; case 7's final test shares its `return 1` with the
   g < 0 exit so its `$v0 = 1` is not hoisted above the load. */
int Camera_ActivationCheckPriority(void *cur, void *other) {
    char *c = (char *)cur;
    char *o = (char *)other;
    unsigned char *state = (unsigned char *)(c + 0x74);

    if (*(unsigned char *)(c + 0x7C) == 0) {
        return 0;
    }
    {
        int (*fn)(void *, void *) = D_001E8F80_prio[*(short *)(c + 0x8C)].fn_04;
        if (fn != 0) {
            switch (fn(cur, other)) {
            case -1:
                return 0;
            case 1:
                return 1;
            }
        }
    }
    switch (*(int *)state) {
    case 1:
    case 2:
        if (state[9] == 0) {
            return 0;
        }
        /* fallthrough */
    case 0:
        if (o == 0) {
            return 1;
        }
        if (*(short *)(o + 0x7E) != 0) {
            return 1;
        }
        if (state[8] > *(unsigned char *)(o + 0x7C)) {
            return 1;
        }
        break;
    case 4: {
        char *p = (char *)D_0015F090[*(short *)(c + 0x84)].unk1C;

        if (o != 0 && *(short *)(o + 0x7E) == 0 && !(state[8] > *(unsigned char *)(o + 0x7C))) {
            return 0;
        }
        if (is_point_inside_clip_volume(gHeroPos, *(int *)(p + 0xC))) {
            return 1;
        }
        break;
    }
    case 7: {
        char *base = D_0013F450;
        short e = *(short *)(c + 0x86);

        if (e != *(int *)(base + 0x2284)) {
            return 0;
        }
        if (*(short *)(o + 0x7E) == 0 && !(state[8] > *(unsigned char *)(o + 0x7C))) {
            return 0;
        }
        if (e != 3) {
            return 1;
        }
        {
            CamRec20 *rec = &D_0015F090[*(short *)(c + 0x84)];
            int g = *(int *)((char *)rec->unk1C + 0x24);

            if (g < 0 || (*(int *)(base + 0x560) == D_0015F040[g].unk10
                          && *(int *)(base + 0x570) == 0)) {
                return 1;
            }
        }
        break;
    }
    }
    return 0;
}
__asm__(".section .text\n\tnop\n");

/* Same shape/blocker as func_001EC270: indirect call via a function
   pointer loaded from a per-type dispatch table, wrapped in an
   sq-for-lone-$ra save this compiler doesn't reproduce (see
   func_001E9E70's comment). Not attempted. */
/* Same vtable dispatch as func_001EC270, on the +0x10 slot instead of
   +8; identical 1/68 operand-order residual, same cause. */
/* Camera_Exit(UpdateCam *) */
void Camera_Exit(void *arg0) {
    void (*fn)(void *) = D_001E8F80[*(short *)((char *)arg0 + 0x8C)].fn_10;
    if (fn != 0) {
        fn(arg0);
    }
}

extern int D_00189C50[];
typedef struct { char unk_00[0xA0]; } CamSlot;
extern CamSlot D_00187510[];
extern int func_001EC5B8(void *cur, void *other);
extern void func_001EC2B8(struct UpdateCam *next_camera);
/* The camera-type table (D_001E8F80) with its +0xC hook typed: DispatchRec
   above keeps that slot as bytes. */
typedef struct {
    char unk_00[0xC];
    void (*fn_0C)(void *);
    char unk_10[4];
} CamTypeHooks;
extern CamTypeHooks D_001E8F80_hooks[] __asm__("D_001E8F80");

/* Camera_ActivationCheck: exits the current camera (func_001EC780, the
   type table's +0x10 hook), then scans the 48 camera slots (D_00189C50[i]
   != 0 = enabled, D_00187510[i] = the 0xA0-byte record) and keeps
   whichever func_001EC5B8 prefers over the current one. If that changed
   the camera, func_001EC2B8 switches to it. Then func_001EC210, the new
   camera's +0xC hook (read before that call, as retail does), a copy of
   its fields 0x30-0x38 to 0x64-0x6C, and the post-update queue
   (func_001EC098). Returns -1, which the one caller ignores.

   The indexed loop is what gives retail's preheader: strength reduction
   builds the two slot pointers in its order, and loop reversal makes the
   count run down 47..0. Indexing the table directly by a typed +0xC
   member puts the base first in the addu. */
int UpdateAllCameras(void) {
    void *cur = D_001871C0;
    int changed = 0;
    CamSlot *rec;
    int i;

    Camera_Exit(cur);

    for (i = 0; i < 0x30; i++) {
        if (D_00189C50[i] != 0) {
            rec = &D_00187510[i];
            if (rec != cur) {
                if (Camera_ActivationCheckPriority(rec, cur)) {
                    cur = rec;
                    changed = 1;
                }
            }
        }
    }

    if (changed) {
        Camera_SwitchActive(cur);
    }
    {
        void (*fn0C)(void *) = D_001E8F80_hooks[*(short *)((char *)cur + 0x8C)].fn_0C;
        char *src;
        char *dst;
        float v;

        Camera_handleCollWithHero(cur);
        if (fn0C != 0) {
            fn0C(cur);
        }
        src = (char *)cur + 0x30;
        dst = (char *)cur + 0x64;
        v = *(float *)src;
        *(float *)((char *)cur + 0x64) = v;
        *(float *)(dst + 4) = *(float *)(src + 4);
        *(float *)(dst + 8) = *(float *)(src + 8);
    }
    ExecuteCamPostUpdFuncs();
    return -1;
}

extern void func_001F9BF0(void *dst, void *a, void *b);      /* dst = a - b (vector) */
extern float func_001F9C78(void *a, void *b);                 /* dot(a, b) */
extern float func_001F9CB8(void *a);                           /* |a| */
extern void func_001F9DC0(void *dst, void *src, float len);    /* dst = normalize(src) * len */
extern float func_001F9FC0(float x);                            /* approx acos(x) */
extern void func_002156E0(void *dst, void *vec, void *axis, float angle); /* dst = vec rotated `angle` around axis */

/* The camera's angles to a target: out[0] = signed yaw between dir0 and
   (p0 - p1) off the axis, out[1] = signed pitch after rotating dir0 by
   that yaw, out[2] = |p0 - p1|. A zero length becomes 0.0001 before the
   acos. The two sign fixups are shaped differently, as in retail. */
void Camera_Pos2Polar3d(float *out, void *p0, void *p1, void *dir0, void *dir1,
                    void *axis) {
    char diff[16];
    char proj[16];
    char perp[16];
    char unit[16];
    char rotated[16];
    float d1, d2, d3, d4, d5;
    float lenPerp, lenDiff;
    float angle1, angle2;
    float a0, a1;

    FastVecSub(diff, p0, p1);
    d1 = FastVecDot(diff, axis);
    func_001F9DC0(proj, axis, d1);
    FastVecSub(perp, diff, proj);

    d2 = FastVecDot(dir0, perp);
    lenPerp = FastVecLength(perp);
    if (lenPerp == 0.0f) {
        lenPerp = 0.0001f;
    }
    angle1 = FastArcSin(d2 / lenPerp);
    a0 = 1.57079637f - angle1;

    func_001F9DC0(unit, perp, 1.0f);
    d3 = FastVecDot(dir1, unit);
    if (d3 < 0.0f) {
        a0 = -a0;
    }
    out[0] = a0;

    build_look_at_matrix(rotated, dir0, axis, a0);
    d4 = FastVecDot(rotated, diff);
    lenDiff = FastVecLength(diff);
    if (lenDiff == 0.0f) {
        lenDiff = 0.0001f;
    }
    angle2 = FastArcSin(d4 / lenDiff);
    a1 = -(1.57079637f - angle2);

    func_001F9DC0(unit, diff, 1.0f);
    d5 = FastVecDot(axis, unit);
    if (d5 < 0.0f) {
        a1 = 1.57079637f - angle2;
    }
    out[1] = a1;

    out[2] = FastVecLength(diff);
}

extern void func_001EC8D8(float *out, void *p0, void *p1, void *dir0, void *dir1,
                           void *axis);
extern char D_001872B0[];

/* Builds three unit vectors from D_0013F450's +0x2080 pointer table
   (+0xC0/+0xD0/+0xE0 offsets, re-read at each call as retail does),
   stashes two of them into D_001872B0's record (+0x90, +0xA0), calls
   func_001EC8D8 to compute the camera's yaw/pitch/dist into +0x70,
   then copies +0xD0 back over +0xB0 (retail's qcopy, see common.h). */
void Camera_CaptureOrientation(void) {
    char *g = D_0013F450;
    char *r = D_001872B0;
    char local0[16];
    char local1[16];
    char local2[16];

    func_001F9DC0(local0, *(char **)(g + 0x2080) + 0xC0, 1.0f);
    func_001F9DC0(local1, *(char **)(g + 0x2080) + 0xD0, 1.0f);
    func_001F9DC0(local2, *(char **)(g + 0x2080) + 0xE0, 1.0f);

    qcopy(r + 0x90, local0);
    qcopy(r + 0xA0, local2);

    Camera_Pos2Polar3d((float *)(r + 0x70), r + 0xC0, g + 0x80, local0, local1, local2);

    qcopy(r + 0xB0, r + 0xD0);
}

extern void func_001F9BD8(void *, void *, void *);
extern char D_0013F590[];

/* When the flag at +2 is clear, copies the 16-byte vector at +0x50 to
   +0xC0 with retail's qcopy, optionally runs func_001F9BD8 on +0x60,
   then copies +0x60 to +0xD0. Sibling of func_001ECC10 just above it. */
void Camera_stagePendingTransform(void) {
    char *base = D_001872B0;
    if (*(unsigned char *)(base + 2) == 0) {
        qcopy(base + 0xC0, base + 0x50);
        if (*(unsigned char *)(base + 3) == 2) {
            FastVecAdd(base + 0xC0, D_0013F590, base + 0xC0);
        }
        qcopy(base + 0xD0, base + 0x60);
    }
}

extern char D_001872B0[];

/* When the flag at +2 is set, copies the 16-byte vectors at +0xC0 and
   +0xD0 back to +0x50 and +0x60 with retail's qcopy (see common.h). */
void Camera_commitPendingTransform(void) {
    char *base = D_001872B0;
    if (*(unsigned char *)(base + 2) != 0) {
        qcopy(base + 0x50, base + 0xC0);
        qcopy(base + 0x60, base + 0xD0);
    }
}

/* The final mode store goes through this alias: written through
   D_001872B0 itself, gcse keeps the entry's %hi alive in a saved register
   across the calls, where retail rebuilds it. */
extern short D_001872B0_h __asm__("D_001872B0") NOT_SDA;
extern void func_00215328(void *, void *);
extern int func_001F98C0(int);
extern float func_001FA888(int);

/* Camera mode update from arg (its +0x30 vector is copied). In mode 1
   the sub-mode at +3 picks what is captured from it: 0 the +0x50/+0x60
   pair, 2 the +0xC0/+0xD0 pair and func_001ECAB8's angles, otherwise
   D_0013F450's axes and func_001EC8D8's yaw/pitch/dist into +0x70. Other
   modes restore through func_001ECB98/func_001ECC10. Then the mode
   becomes 3, +2 takes the sub-mode, and either the +0x10 blend resets or
   the +0x70 timer advances. */
void Camera_StartBlend(void *arg) {
    char *arg0 = arg;
    char *r = D_001872B0;
    char local0[16];
    char local1[16];
    char local2[16];
    unsigned char sub;

    if (*(short *)r == 1) {
        sub = r[3];
        if (sub == 0) {
            qcopy(r + 0x50, arg0 + 0x30);
            func_00215328(r + 0x60, arg0);
        } else if (sub == 2) {
            qcopy(r + 0xC0, arg0 + 0x30);
            func_00215328(r + 0xD0, arg0);
            Camera_CaptureOrientation();
        } else {
            char *g = D_0013F450;

            func_001F9DC0(local0, *(char **)(g + 0x2080) + 0xC0, 1.0f);
            func_001F9DC0(local1, *(char **)(g + 0x2080) + 0xD0, 1.0f);
            func_001F9DC0(local2, *(char **)(g + 0x2080) + 0xE0, 1.0f);
            Camera_Pos2Polar3d((float *)(r + 0x70), arg0 + 0x30, *(char **)(r - 0xF0) + 0x30,
                          local0, local1, local2);
            func_00215328(r + 0xB0, arg0);
            qcopy(r + 0xD0, r + 0xB0);
        }
    } else {
        sub = r[3];
        if (sub == 2) {
            Camera_stagePendingTransform();
            Camera_CaptureOrientation();
        } else if (sub == 1) {
            Camera_stagePendingTransform();
            qcopy(r + 0xB0, r + 0xD0);
        } else if (sub == 0) {
            Camera_commitPendingTransform();
        }
    }
    D_001872B0_h = 3;
    *(unsigned char *)(r + 2) = r[3];
    if (*(unsigned char *)(r + 2) == 0) {
        char *a = r + 0x10;

        *(float *)(a + 0x10) = *(float *)(a + 0x14);
        *(int *)(a + 0xC) = 0;
        qcopy(r + 0x40, r + 0x50);
        *(float *)(a + 0x0) = 0;
        *(float *)(a + 0x4) = *(float *)(a + 0x8);
        qcopy(r + 0x30, r + 0x60);
    } else {
        char *b = r + 0x70;
        int n = ++*(int *)(b + 0x14);

        *(int *)(b + 0xC) = func_001F98C0(n);
        *(float *)(b + 0x10) = 1.0f / IntToFloat(*(int *)(b + 0xC));
    }
}

extern char D_00187180[];
extern char D_0018C418[];
extern char D_00187390[];
extern float D_0015EE60 MACRO_ADDR;
extern float func_00214220(float, float, float);
extern void func_001FA6C0(void *, void *);
extern void func_001FA480(void *, void *);

/* Camera blend step toward to: while the position (cam[3]) or rotation
   (cam[0]) blend hasn't reached 1, move cam+0x40 from cam+0x30 toward
   to's position by the eased factor (func_00214220), copy it to
   D_00187180 unless the D_0018C418 flag is set, slerp the rotation
   (func_001FA5C8) into cam+0x50 and load it as the view matrix, then
   advance both blends by their rates times D_0015EE60, capped at 1.
   Returns 1 once both are complete. */
int Camera_StepBlend(void *arg0, void *arg1) {
    float *to = arg0;
    float *cam = arg1;
    float m[4];
    float q[16];
    char *st;
    float t;
    float *rot;

    if (cam[3] == 1.0f && cam[0] == 1.0f) {
        return 1;
    }
    t = CosineInterpolate(0.0f, 1.0f, cam[3]);
    FastVecAdd(cam + 12, D_0013F590, cam + 12);
    st = D_0018C418;
    cam[16] = cam[12] + (to[12] - cam[12]) * t;
    cam[17] = cam[13] + (to[13] - cam[13]) * t;
    cam[18] = cam[14] + (to[14] - cam[14]) * t;
    if (*(int *)(st + 0x14) == 0) {
        qcopy(D_00187180, cam + 16);
    }
    func_00215328(m, to);
    rot = cam + 20;
    t = CosineInterpolate(0.0f, 1.0f, cam[0]);
    func_001FA5C8(rot, cam + 8, m, t);
    func_001FA6C0(rot, q);
    if (*(int *)(st + 0x14) == 0) {
        func_001FA480(D_00187390, q);
    }
    cam[3] += cam[4] * D_0015EE60;
    if (1.0f < cam[3]) {
        cam[3] = 1.0f;
    }
    cam[0] += cam[1] * D_0015EE60;
    if (1.0f < cam[0]) {
        cam[0] = 1.0f;
    }
    return 0;
}

extern void func_001F9CA0(void *, void *, void *);
extern void func_001EC8D8(float *out, void *p0, void *p1, void *dir0, void *dir1, void *axis);
extern void func_001F9DC0(void *dst, void *src, float len);
extern float func_001FA790(float, float);
extern float func_001FA748(float, float);
extern void func_002156E0(void *dst, void *vec, void *axis, float angle);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BF0(void *dst, void *a, void *b);
extern float func_001F9C78(void *a, void *b);
extern float func_001F9CB8(void *a);
extern float func_001F9FC0(float x);
extern void func_00215380(void *arg0, void *axis, float angle);
extern void func_00215650(void *arg0, void *arg1, void *arg2);
extern void func_001F9908(int *arg0);
extern unsigned char D_001872B2 NOT_SDA;
extern int D_0018C42C;
extern char D_0013F6E0[];

typedef struct {
    float v[4];
} Vec;

/* Camera update toward a target orientation: blends yaw/pitch/distance and rebuilds the camera basis.
   Adapted from Lombyte (MIT) for PAL: src/gameplay/camera/fun_001eccd8.c, FUN_001eccd8. */
int Camera_UpdateTowardTarget(void *arg0, void *arg1) {
    char *cam = arg0;
    float *b = arg1;
    Vec ang;
    Vec fwd;
    Vec side;
    Vec up;
    Vec target;
    Vec pos;
    Vec diff;
    Vec proj;
    Vec m[3];
    Vec r0;
    Vec r1;
    Vec r2;
    Vec q;
    float step;
    float dyaw;
    float dpitch;
    float yaw;
    float turn;
    float sign;
    float s2;
    float d;
    char *g;

    if (*(int *)(b + 3) <= 0) {
        return 1;
    }
    step = 1.0f / CosineInterpolate(1.0f, (float)*(int *)(b + 5), (float)*(int *)(b + 3) * b[4]);
    if (D_001872B2 == 2) {
        qcopy(&target, gHeroPos);
        qcopy(&fwd, b + 8);
        qcopy(&up, b + 12);
        FastVecCross(&side, &fwd, &up);
        Camera_Pos2Polar3d(ang.v, cam + 0x30, &target, &fwd, &side, &up);
    } else {
        qcopy(&target, cam + 0x30);
        g = D_0013F450;
        func_001F9DC0(&fwd, *(char **)(g + 0x2080) + 0xC0, 1.0f);
        func_001F9DC0(&up, *(char **)(g + 0x2080) + 0xE0, 1.0f);
        ang.v[2] = 0.0f;
        ang.v[1] = 0.0f;
        ang.v[0] = 3.1415927f;
    }
    dyaw = FastSubRots(ang.v[0], b[0]);
    b[0] = FastAddRots(b[0], dyaw * step);
    dpitch = FastSubRots(ang.v[1], b[1]);
    b[1] = FastAddRots(b[1], dpitch * step);
    b[2] = b[2] + (ang.v[2] - b[2]) * step;
    func_001F9DC0(&pos, &fwd, b[2]);
    build_look_at_matrix(&pos, &pos, &up, b[0]);
    FastVecCross(&side, &pos, &up);
    func_001F9DC0(&side, &side, 1.0f);
    build_look_at_matrix(&pos, &pos, &side, b[1]);
    FastVecAdd(b + 20, &target, &pos);
    if (D_0018C42C == 0) {
        qcopy(D_00187180, b + 20);
    }
    func_001FA648(b + 16, m);
    d = FastVecDot(&m[2], cam);
    FastVecScale(&proj, &m[2], d);
    FastVecSub(&diff, cam, &proj);
    yaw = 1.5707964f - FastArcSin(FastVecDot(&m[0], &diff) / FastVecLength(&diff));
    sign = -1.0f;
    if (FastVecDot(&diff, &m[1]) >= 0.0f) {
        sign = 1.0f;
    }
    yaw = yaw * sign;
    if (FastAbsF(dyaw) > 1.5707964f
        && ((dyaw >= 0.0f && sign < 0.0f) || (dyaw < 0.0f && sign >= 0.0f))) {
        if (yaw < 0.0f) {
            yaw += 6.2831855f;
        } else {
            yaw -= 6.2831855f;
        }
    }
    turn = yaw * step;
    if (FastAbsF(turn) < 1e-5f) {
        qcopy(&r0, &m[0]);
        qcopy(&r1, &m[1]);
    } else {
        build_quaternion_from_axis_angle(&r2, &m[2], turn);
        RotateVectorByQuaternion(&r0, &m[0], &r2);
        RotateVectorByQuaternion(&r1, &m[1], &r2);
    }
    if (FastAbsF(yaw) < 1e-5f) {
        qcopy(&r2, &m[0]);
    } else {
        build_quaternion_from_axis_angle(&q, &m[2], yaw);
        RotateVectorByQuaternion(&r2, &m[0], &q);
    }
    yaw = 1.5707964f - FastArcSin(FastVecDot(&r2, cam));
    d = FastVecDot(&r2, cam + 0x20);
    s2 = -1.0f;
    if (d >= 0.0f) {
        s2 = 1.0f;
    }
    yaw *= s2;
    build_look_at_matrix(&r0, &r0, &r1, yaw * step);
    func_001F9DC0(D_00187390, &r0, 1.0f);
    FastVecCross(D_00187390 + 0x10, D_00187390, D_0013F6E0);
    func_001F9DC0(D_00187390 + 0x10, D_00187390 + 0x10, -1.0f);
    FastVecCross(D_00187390 + 0x20, D_00187390 + 0x10, D_00187390);
    func_00215328(b + 24, D_00187390);
    func_00215328(b + 16, D_00187390);
    func_001F9908((int *)(b + 3));
    return 0;
}

extern int D_0018C42C;
extern int func_001ECEA0(void *, void *);
extern int func_001ED080(void *, void *);

/* Picks the update path by the flag at D_001872B0+2 (func_001ECEA0 when
   clear, func_001ED080 when set), each passed arg0 and a slot inside
   D_001872B0. On success, unless D_0018C42C is set, copies four 16-byte
   vectors from arg0 into D_00187390's block (retail's qcopy); the last
   destination is -0x210 from D_00187390, a different member reached by
   pointer arithmetic on the same char array. Either way it clears
   D_001872B0's leading halfword and its flag byte. */
void Camera_UpdateBlend(char *arg0) {
    char *base = D_001872B0;
    int result;

    if (*(unsigned char *)(base + 2) == 0) {
        result = Camera_StepBlend(arg0, base + 0x10);
    } else {
        result = Camera_UpdateTowardTarget(arg0, base + 0x70);
    }
    if (result != 0) {
        if (D_0018C42C == 0) {
            qcopy(D_00187390, arg0);
            qcopy(D_00187390 + 0x10, arg0 + 0x10);
            qcopy(D_00187390 + 0x20, arg0 + 0x20);
            qcopy(D_00187390 - 0x210, arg0 + 0x30);
        }
        *(short *)base = 0;
        base[2] = 0;
    }
}

extern float func_001FA888(int arg0);
extern float func_001FA7D8(float x);
extern float func_001F9F90(float x);
extern void func_001F9DC0(void *dst, void *src, float len);
extern void func_001F9BD8(void *dst, void *a, void *b);
extern char D_001873B0[];

/* Camera shake update. arg0: +0 amplitude, +4 sample (out), +8 countdown,
   +0xC longest countdown. With the active camera (D_001871C0) in state 6
   both counters are cleared; with the countdown at 0 only +0xC is.
   Otherwise the countdown is decremented (func_001F9908, saturating) and
   sample = amplitude * cos(wrap(2 * count)) * (count / longest)^2
   (func_001FA888 int->float, func_001FA7D8 wrap to [-pi, pi],
   func_001F9F90 cos) scales one of two directions (arg1) into the shake
   offset D_00187180 (func_001F9DC0 scale, func_001F9BD8 add). The
   countdown-zero exit is the else arm after the main path (retail's
   `b L800` over it), and the state-6 stores are written 8 then 0xC so
   the scheduler emits 0xC first and cross-jumping leaves them alone. */
void Camera_UpdateShake(char *arg0, int arg1) {
    char buf[16];

    if (D_001871C0 != 0 && *(short *)((char *)D_001871C0 + 0x86) == 6) {
        *(int *)(arg0 + 8) = 0;
        *(int *)(arg0 + 0xC) = 0;
    } else if (*(int *)(arg0 + 8) != 0) {
        float ratio;
        float shake;

        if (*(int *)(arg0 + 0xC) < *(int *)(arg0 + 8)) {
            *(int *)(arg0 + 0xC) = *(int *)(arg0 + 8);
        }
        func_001F9908((int *)(arg0 + 8));
        ratio = IntToFloat(*(int *)(arg0 + 8)) / IntToFloat(*(int *)(arg0 + 0xC));
        shake = *(float *)arg0
                * FastCos(FastNormalizeAngle(IntToFloat(*(int *)(arg0 + 8)) * 2.0f))
                * ratio * ratio;
        *(float *)(arg0 + 4) = shake;
        if (arg1 == 0) {
            func_001F9DC0(buf, D_001873B0, shake);
        } else {
            func_001F9DC0(buf, D_00187390, shake);
        }
        FastVecAdd(D_00187180, D_00187180, buf);
    } else {
        *(int *)(arg0 + 0xC) = 0;
    }
}
__asm__(".section .text\n\tnop\n");

struct MobyClass;
struct Manip;
struct Moby {
    Vec4f bsphere;
    Vec4f pos;
    u8 state; /* >= 0xFE: dead, waiting to respawn */
    u8 pad21[3];
    struct MobyClass *pclass;
    struct Moby *next;
    s32 unk2C;
    u8 pad30[4];
    u16 flags;
    u8 pad36[2];
    u64 spawn_frame; /* frame count at which it may respawn */
    u8 pad40[0x10];
    u8 frame; /* animation frame */
    u8 prev_frame; /* frame index in prev_seq */
    u8 seq; /* animation sequence id */
    u8 prev_seq;
    u8 pad54[0x10];
    struct Manip *manips;
    void *cur_frame_data;
    void *prev_frame_data;
    u8 pad70[4];
    void (*update)(struct Moby *moby);
    u8 *pvars;
    u8 unk7C;
    u8 pad7D;
    u8 unk7E;
    u8 pad7F[0x27];
    s16 oclass;
    u8 padA8[0x58];
};
struct Player {
    u8 pad0[0x80];
    Vec4 pos;
    u8 pad90[0x8];
    f32 unk98;
    u8 pad9C[0x1F4];
    Vec4 unk290;
    u8 pad2A0[0x5C];
    struct Moby *unk2FC;
    u8 pad300[0x1D84];
    s32 unk2084;
    u8 pad2088[0x1FC];
    s32 unk2284;
};
struct CamColl {
    Vec4 pos;
    f32 vel;
    u8 pad14[0xC];
    Vec4 dir;
    Vec4 unk30;
    Vec4 unk40;
    f32 dir_vel[4];
    Vec4 unk60;
    Vec4 unk70;
    Vec4 unk80;
    Vec4 unk90;
    f32 unkA0;
    f32 unkA4;
    f32 unkA8;
    f32 hist[5];
    u8 padC0[0x14];
    struct Moby *unkD4;
    f32 unkD8;
    f32 unkDC;
};
extern struct Player D_0013F450_ED818 __asm__("D_0013F450");
extern struct CamColl D_001871D0_ED818 __asm__("D_001871D0");
extern f32 func_001EC120_ED818(f32 *vel, f32 from, f32 to, f32 stiffness, f32 damping, f32 max) __asm__("func_001EC120");
extern float func_001F9B88(float input);
extern void func_001F9BF0(void *out, void *a, void *b);
extern void func_001F9C30(void *out, void *a, f32 s);
extern f32 func_001F9C78(void *a, void *b);
extern f32 func_001F9CB8(void *a);
extern void func_001F9DC0(void *out, void *a, f32 len);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/gameplay/camera/fun_001ed470.c, FUN_001ed470. */
void CameraPreUpdate(void) {
    Vec4 dir;
    Vec4 proj;
    struct CamColl *cam;
    f32 d;
    s32 i;
    struct Moby *m;

    cam = &D_001871D0_ED818;
    func_001F9DC0(&dir, &D_0013F450_ED818.unk290, -1.0f);
    qcopy(&cam->unk40, &cam->unk30);
    qcopy(&cam->unk30, &dir);
    if (FastVecDot(&cam->dir, &dir) < -0.98f) {
        dir.f[0] += 0.2f;
        dir.f[1] += 0.2f;
        dir.f[2] += 0.2f;
    }
    cam->dir.f[0] =
        func_001EC120_ED818(&cam->dir_vel[0], cam->dir.f[0], dir.f[0], 0.015f, 0.2f, 0.0f);
    cam->dir.f[1] =
        func_001EC120_ED818(&cam->dir_vel[1], cam->dir.f[1], dir.f[1], 0.015f, 0.2f, 0.0f);
    cam->dir.f[2] =
        func_001EC120_ED818(&cam->dir_vel[2], cam->dir.f[2], dir.f[2], 0.015f, 0.2f, 0.0f);
    func_001F9DC0(&cam->dir, &cam->dir, 1.0f);

    FastVecSub(&cam->unk70, &D_0013F450_ED818.pos, &cam->unk60);
    cam->unkA0 = FastVecLength(&cam->unk70);
    d = FastVecDot(&cam->unk70, &dir);
    cam->unkA8 = d;
    func_001F9DC0(&proj, &dir, d);
    qcopy(&cam->unk90, &proj);
    FastVecSub(&cam->unk80, &cam->unk70, &proj);
    cam->unkA4 = FastVecLength(&cam->unk80);
    FastVecScale(&cam->unk80, &cam->unk80, 1.0f / cam->unkA4);
    qcopy(&cam->unk60, &D_0013F450_ED818.pos);

    if (D_0013F450_ED818.unk2284 != 0x50 || D_0013F450_ED818.unk2084 == 0x11) {
        cam->pos.f[0] = D_0013F450_ED818.pos.f[0];
        cam->pos.f[1] = D_0013F450_ED818.pos.f[1];
        cam->pos.f[2] =
            func_001EC120_ED818(&cam->vel, cam->pos.f[2], D_0013F450_ED818.pos.f[2], 0.0075f, 0.175f, 0.0f);
        cam->pos.f[3] = D_0013F450_ED818.pos.f[2];
    } else {
        cam->pos.f[0] = D_0013F450_ED818.pos.f[0];
        cam->pos.f[1] = D_0013F450_ED818.pos.f[1];
    }

    for (i = 0; i < 4; i++) {
        cam->hist[i] = cam->hist[i + 1];
    }
    cam->hist[i] = D_0013F450_ED818.unk98;

    m = D_0013F450_ED818.unk2FC;
    if (m != 0 && m->oclass != 0x4BA && m->oclass != 0x336) {
        if (m == cam->unkD4) {
            cam->unkDC = m->pos.z - cam->unkD8;
            if (FastAbsF(cam->unkDC) < 0.001f) {
                cam->unkDC = 0.0f;
            }
            cam->unkD8 = cam->unkD4->pos.z;
        } else {
            cam->unkD4 = m;
            cam->unkDC = 0.0f;
            cam->unkD8 = m->pos.z;
        }
    } else {
        cam->unkDC = 0.0f;
        cam->unkD4 = 0;
        cam->unkD8 = D_0013F450_ED818.pos.f[2];
    }
}

extern char D_00187040[];
extern char D_00194220[];
extern int D_0015F6E8 MACRO_ADDR;
extern int func_001EFE10(void *, void *, int, int, int);
extern int func_001F0F00(void);
extern float func_00214440(void *, int);

/* Camera-inside-water test: cast a ray through the camera focus from
   0.75 above to 0.75 below (up to six hits); on the first hit that is
   not a water surface, flag D_00187040+0x394 when the focus is below the
   surface height + 0.04. Off for camera mode 6 or while D_0015F6E8 is
   set. */
void Camera_UpdateUnderwaterFlag(void) {
    char *cam = D_00187040;
    float a[4];
    float b[4];
    int i;

    if (*(short *)(*(char **)(cam + 0x180) + 0x86) == 6 || D_0015F6E8 != 0) {
        *(int *)(cam + 0x394) = 0;
        return;
    }
    qcopy(a, cam + 0x140);
    qcopy(b, cam + 0x140);
    a[2] += 0.75f;
    b[2] -= 0.75f;
    i = 0;
    while (i < 6 && func_001EFE10(a, b, 0x12, 0, 0) != 0) {
        if (func_001F0F00() == 0) {
            float h = GetGroundHeight(D_00194220, 0) + 0.04f;
            char *c2 = D_00187040;

            if (*(float *)(c2 + 0x148) < h) {
                *(int *)(c2 + 0x394) = 1;
            } else {
                *(int *)(c2 + 0x394) = 0;
            }
            return;
        }
        qcopy(a, D_00194220);
        i++;
        a[2] -= 0.01f;
    }
}

extern int D_0015F09C MACRO_ADDR;
extern int D_0015F0A0 MACRO_ADDR;
extern int D_0015F098 MACRO_ADDR;

/* Picks the camera's draw modes: D_0015F09C is 0x14, or 0x34 in states
   0x11/0x12 or mode 0x73 of D_0013F450, back to 0x14 when its +0x2F0
   height is below D_00187180+8 (except in state 0x11); D_0015F0A0 keeps
   the value before bit 0x80 is set. D_0015F098 is 0xB4 ORed with the +0xC0 mode of
   D_001871D0, which the first set flag of D_0013F450's 0x12E5, 0x12EB,
   0x12E6, 0x12EC, 0x12E4 overrides. Each arm ORs the new mode in itself
   (the constants fold per arm, as in retail), and each block reads
   D_0013F450 through its own local (%hi kept, %lo rebuilt). */
void Camera_PickDrawModes(void) {
    char *c = D_001871D0;
    char *g = D_0013F450;
    int st;

    D_0015F09C = 0x14;
    st = *(int *)(g + 0x208C);
    if (st == 0x11 || st == 0x12 || *(int *)(g + 0x2084) == 0x73) {
        D_0015F09C = 0x34;
    }
    {
        char *g2 = D_0013F450;
        if (*(int *)(g2 + 0x208C) != 0x11
            && *(float *)(g2 + 0x2F0) < *(float *)(D_00187180 + 8)) {
            D_0015F09C = 0x14;
        }
    }
    {
        unsigned char *g3 = (unsigned char *)D_0013F450;
        D_0015F0A0 = D_0015F09C;
        D_0015F09C |= 0x80;
        D_0015F098 = 0xB4;
        if (g3[0x12E5]) {
            *(int *)(c + 0xC0) = 0x100;
            D_0015F098 |= *(int *)(c + 0xC0);
        } else if (g3[0x12EB]) {
            *(int *)(c + 0xC0) = 0xB00;
            D_0015F098 |= *(int *)(c + 0xC0);
        } else if (g3[0x12E6]) {
            *(int *)(c + 0xC0) = 0x300;
            D_0015F098 |= *(int *)(c + 0xC0);
        } else if (g3[0x12EC]) {
            *(int *)(c + 0xC0) = 0xD00;
            D_0015F098 |= *(int *)(c + 0xC0);
        } else if (g3[0x12E4]) {
            *(int *)(c + 0xC0) = 0;
            D_0015F098 |= *(int *)(c + 0xC0);
        } else {
            D_0015F098 |= *(int *)(c + 0xC0);
        }
    }
}

extern char D_00187040[];
extern float D_0015F53C MACRO_ADDR;

/* D_0015F53C is a MACRO_ADDR float: $gp-relative in the delay slots,
   lui $1 in the body. */
void func_001EDE08(void) {
    char *c = D_00187040;
    float a = *(float *)(c + 0x258);

    if (a != 0.0f) {
        float t = D_0015F53C - a;
        D_0015F53C = t;
        if (t <= 0.0f) {
            *(float *)(c + 0x258) = 0.0f;
            D_0015F53C = 0.0f;
        }
    }
}

extern int D_001E6700;
extern char D_001871A0[];
extern unsigned char D_0015EEB4_m[4] __asm__("D_0015EEB4") MACRO_ADDR;
extern void func_001EDE08(void);
extern void func_001EDCE8(void);
extern void func_001ED818(void);
extern void func_001ECC48(void *);
extern void func_001FA460(void *, void *);
extern void func_002153E8(void *, void *);
extern void func_001EE858(void *);

/* Camera update, once per frame: count the frame, run the camera
   steps, then (unless the D_0018C418 freeze flag is set) take the view
   from the target object (mode 3 blends it through func_001ED658) and
   refresh its Euler angles; run func_001ED708 on the two vectors at
   D_001871A0, func_001EDB98 and func_001EE858, and, with
   D_0015EEB4 set, the matrix's third row as a cross product. */
void UpdateCamera(void) {
    char *c;
    char *target;
    char *v;
    float m[16];

    if (D_0015F6E8 == 5) {
        if (D_001E6700 != 0) {
            return;
        }
        *(short *)D_001872B0 = 0;
        D_001872B0[2] = 0;
    }
    c = D_00187040;
    (*(int *)(c + 0x398))++;
    func_001EDE08();
    Camera_PickDrawModes();
    CameraPreUpdate();
    UpdateAllCameras();
    target = *(char **)(c + 0x180);
    if ((unsigned short)(*(unsigned short *)(c + 0x270) - 1) < 2) {
        Camera_StartBlend(*(void **)(c + 0x184));
    }
    if (*(short *)(c + 0x270) == 3) {
        Camera_UpdateBlend(target);
    } else {
        char *st = D_0018C418;
        if (*(int *)(st + 0x14) != 0) {
            goto frozen;
        }
        qcopy(c + 0x140, target + 0x30);
        qcopy(c + 0x350, target);
        qcopy(c + 0x360, target + 0x10);
        qcopy(c + 0x370, target + 0x20);
    }
    {
        char *st = D_0018C418;
        if (*(int *)(st + 0x14) == 0) {
            func_001FA460(m, D_00187390);
            MatrixToEuler(m, D_00187390 - 0x200);
        }
    }
frozen:
    v = D_001871A0;
    Camera_UpdateShake(v, 0);
    Camera_UpdateShake(v + 0x10, 1);
    Camera_UpdateUnderwaterFlag();
    Camera_UpdateEnvironment(v - 0x20);
    if (D_0015EEB4_m[0] != 0) {
        FastVecCross(v + 0x200, v + 0x210, v + 0x1F0);
    }
}

LINKER_REMNANT("asm/remnants/text", func_001EDFD8);

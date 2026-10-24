#include "common.h"
#include "vector.h"
#include "callees.h"
#include "records.h"
#include "structs.h"
#include "camera.h"
#include "hero.h"

/*
 * camera.cpp in the original source; text 0x1EC038-0x1EDFF8.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
/* The camera block and the debug free camera (include/camera.h). */
extern CameraSystem D_00187040;
extern DebugFreeCam D_0018C418;
extern char D_0013E650[];
extern int D_0015F694;
extern int func_001E97C8(void *arg0);

/* Same signature as the declaration further down this file; duplicate
   identical declarations are legal and avoid a signature clash. */
extern void func_001F9A98(void *, void *, int);
extern Camera D_00189310;
extern char D_001899D0[];

/* BackupCurrentCam: copies the current camera and its 0x280-byte state buffer
   to backup_cam / gCamBackupState. */
void BackupCurrentCam(void) {
    FastMemCopy(&backup_cam, gCamera.current, 0xA0);
    FastMemCopy(gCamBackupState, gCamBackupState - 0x500, 0x280); /* from gCamCurState */
    backup_cam.state_buf = gCamBackupState;
}

extern int D_0015F08C MACRO_ADDR;
extern void (*D_001893B0[])(void);

/* ExecuteCamPostUpdFuncs: runs the cam_post_upd_routine_cnt callbacks queued in
   gCamPostUpdFuncs, then empties the queue. */
/* cam_post_upd_routine_cnt as MACRO_ADDR removes the hoisted address register (frame
   0x30); a plain indexed loop. */
void ExecuteCamPostUpdFuncs(void) {
    int i;

    for (i = 0; i < cam_post_upd_routine_cnt; i++) {
        gCamPostUpdFuncs[i]();
    }
    cam_post_upd_routine_cnt = 0;
}

/* Not a standalone function: no `jr $31` -- dead-value computation
   (`$v0 = 0` twice with intervening nops) then a store, falling through
   to whatever follows. Same fallthrough-fragment category as
   func_00113AD8 in core_text. */
LINKER_REMNANT("asm/remnants/text", func_001EC108);

extern float func_001F9B88(float);

/* Cam_InterpValues(from, to, vel, stiffness, damping, max): steps *vel
   toward to - from (a damped spring: stiffness * diff - damping * vel),
   clamps it to +-max and to +-func_001F9B88(to - from), and returns
   from + *vel. The nop between the first compare and its bc1f is ps2eeas's
   (tools/ps2eeas_nops.py). */
float Cam_InterpValues(float from, float to, float *vel, float stiffness, float damping, float max) {
    float diff = to - from;
    float v = *vel;

    v = v + (stiffness * diff - damping * v);
    *vel = v;
    if (max != 0.0f) {
        if (max < v) {
            *vel = max;
        } else if (v < -max) {
            *vel = -max;
        }
    }
    if (FastAbsF(diff) < *vel) {
        *vel = FastAbsF(diff);
    } else if (-FastAbsF(diff) > *vel) {
        *vel = -FastAbsF(diff);
    }
    return from + *vel;
}

/* Not a standalone function: single `addiu $sp,$sp,0x50`, no `jr $31` --
   fallthrough fragment, same category as func_00113AD8 in core_text. */
LINKER_REMNANT("asm/remnants/text", func_001EC208);

extern void func_0020D678(void *); /* DeleteMoby */

/* Camera_handleCollWithHero: spawns (via func_001E97C8) or deletes the
   moby kept at follow.hero_coll_moby, depending on the camera's id: camera
   id 0 keeps the hero collision moby, any other id deletes it. The `== 0`
   arm comes first, and DeleteMoby takes the moby as its argument. */
void Camera_handleCollWithHero(Camera *cam) {
    CameraFollow *f = &gCamera.follow;

    if (cam->id == 0) {
        if (f->hero_coll_moby == 0) {
            f->hero_coll_moby = (struct Moby *)noop_callback_l((char *)f - 0x50); /* &gCamera.pos */
        }
    } else {
        if (f->hero_coll_moby != 0) {
            DeleteMoby(f->hero_coll_moby);
            f->hero_coll_moby = 0;
        }
    }
}

/* 0x14-byte camera-type records (camera_types), indexed by Camera.type (+0x8C).
   Declared as a real struct array, not `char[]` + byte offset: the two
   forms are not codegen-equivalent here. Retail emits `addu $2,$2,$3`
   (base, index); a char-pointer form emits `addu $2,$3,$2` (index,
   base) and no amount of reordering the C addition changes it, because
   GCC canonicalises the PLUS before operand order is chosen. Indexing
   a typed array puts the base first. See func_001EC270/func_001EC780. */
extern CameraType D_001E8F80[];

/* Camera_runSetupToNewCam: calls the camera type's activate hook
   (camera_types[cam->type]); the typed-array index is what puts the table
   base first in the addu. */
void Camera_runSetupToNewCam(Camera *cam) {
    void (*fn)(Camera *) = camera_types[cam->type].activate;
    if (fn != 0) {
        fn(cam);
    }
}

extern CameraDef *D_0015F090 MACRO_ADDR;
extern s32 D_0015EE84 MACRO_ADDR;
extern u8 D_00189750[];
extern void func_001EC038(void);
extern void func_001F9A98(void *dst, void *src, s32 size);
extern s32 func_001FA898(f32 transition_duration);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/textbin/fun_001ebf10.c, switch_active_camera_record. */
/* Switches the active camera to next_camera. The blend comes from the old
   camera's transition_state and the new one's info->blend_kind (1 eased,
   5 orbit, 3 and 6 hand-off); it is installed in gCamera.blend, then the
   state buffers are swapped and the new camera is activated. */
void switch_active_camera_record(Camera *next_camera) {

    Camera *previous_camera;
    CameraInfo *info;
    f32 transition_duration;
    f32 mode_one_duration;
    f32 handoff_duration;
    f32 *previous_position;
    Vec4f *position;
    s32 blend_kind = 0;
    s32 previous_transition_state;

    info = gCameraDefs[next_camera->def_index].info;
    previous_camera = gCamera.current;
    previous_transition_state = previous_camera->transition_state;
    if (info != 0) {
        blend_kind = info->blend_kind;
    }
    if (previous_transition_state == 4) {
        next_camera->hard_cut = 1;
    } else if (previous_transition_state == 2 || blend_kind == 1 || blend_kind == 5) {
        if (blend_kind == 1) {
            mode_one_duration = next_camera->blend_time;
            gCamera.blend.next_kind = 0;
            if (mode_one_duration > 0.0f) {
                gCamera.blend.lerp.pos_rate_cfg = mode_one_duration;
                gCamera.blend.lerp.rot_rate_cfg = mode_one_duration;
            } else {
                gCamera.blend.lerp.pos_rate_cfg = 0.018f;
                gCamera.blend.lerp.rot_rate_cfg = 0.018f;
            }
        } else if (blend_kind == 5) {
            transition_duration = next_camera->blend_time;
            gCamera.blend.next_kind = 2;
            if (transition_duration > 0.0f) {
                gCamera.blend.orbit.frames =
                    truncate_float_to_s32(transition_duration);
            } else {
                gCamera.blend.orbit.frames = 40;
            }
        }
        if (gCamera.blend.phase == 0) {
            gCamera.blend.phase = 1;
        } else {
            gCamera.blend.phase = 2;
        }
    } else if (previous_transition_state == 3 || previous_transition_state == 5 ||
               blend_kind == 3 || blend_kind == 6) {
        qcopy(&next_camera->pos, &previous_camera->pos);
        qcopy(&next_camera->basis[0], &previous_camera->basis[0]);
        qcopy(&next_camera->basis[1], &previous_camera->basis[1]);
        qcopy(&next_camera->basis[2], &previous_camera->basis[2]);
        next_camera->handoff_state = 2;
        if (previous_camera->transition_state == 5 || blend_kind == 6) {
            handoff_duration = next_camera->blend_time;
            gCamera.blend.next_kind = 0;
            if (handoff_duration > 0.0f) {
                gCamera.blend.lerp.pos_rate_cfg = handoff_duration;
                gCamera.blend.lerp.rot_rate_cfg = handoff_duration;
            } else {
                gCamera.blend.lerp.pos_rate_cfg = 0.018f;
                gCamera.blend.lerp.rot_rate_cfg = 0.018f;
                if (D_0015EE84 == 1) {
                    gCamera.blend.lerp.pos_rate_cfg = 0.01f;
                    gCamera.blend.lerp.rot_rate_cfg = 0.01f;
                }
            }
            if (gCamera.blend.phase == 0) {
                gCamera.blend.phase = 1;
            } else {
                gCamera.blend.phase = 2;
            }
        }
    } else {
        next_camera->hard_cut = 1;
    }

    position = &next_camera->pos;
    previous_camera->transition_state = 0;
    previous_camera->handoff_state = 0;
    previous_camera->hard_cut = 0;
    gCamera.previous = previous_camera;
    FastMemCopy(gCamPrevState, gCamPrevState - 0x280,
                           0x280);
    gCamera.previous->state_buf = gCamPrevState;
    gCamera.current = next_camera;
    next_camera->state_buf = gCamPrevState - 0x280;
    gCamera.frames = 0;
    Camera_runSetupToNewCam(next_camera);
    BackupCurrentCam();
    /* The activate hook runs before the debug free camera flag is read; prev_pos is updated either way. */
    if (occl_cam_state.active == 0) {
        qcopy(&gCamera.pos, &next_camera->pos);
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
extern CameraDef *D_0015F090 MACRO_ADDR;
extern CamRec20 *D_0015F040 MACRO_ADDR;
extern char D_0013F450[];
extern char D_0013F4D0[];
extern int func_00215570(void *arg0, int arg1);

/* Camera_ActivationCheckPriority(cur, other): whether camera `cur` should
   take over from `other`. An inactive camera (priority 0) never does; the
   camera type's check hook (camera_types[cur->type].check) may decide first
   (-1 no, 1 yes); otherwise the activation rule in cur->activation decides:
   0 (and 1/2 once handoff_state is set) by priority, 4 while the hero is
   inside the camera's clip volume, 7 by the hero's mode and the camera id.
   Each case ends in `if (x) return 1;` falling out to the one shared
   `return 0;`, which is what lets cross-jumping and reorg give retail's
   branches; case 7's final test shares its `return 1` with the g < 0 exit
   so its `$v0 = 1` is not hoisted above the load. */
int Camera_ActivationCheckPriority(Camera *cur, Camera *other) {
    unsigned char *state = (unsigned char *)&cur->activation; /* state[8] priority, state[9] handoff_state */

    if (cur->priority == 0) {
        return 0;
    }
    {
        int (*fn)(Camera *, Camera *) = camera_types[cur->type].check;
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
        if (state[9] == 0) { /* handoff_state */
            return 0;
        }
        /* fallthrough */
    case 0:
        if (other == 0) {
            return 1;
        }
        if (other->transition_state != 0) {
            return 1;
        }
        if (state[8] > other->priority) {
            return 1;
        }
        break;
    case 4: {
        CameraInfo *info = gCameraDefs[cur->def_index].info;

        if (other != 0 && other->transition_state == 0 && !(state[8] > other->priority)) {
            return 0;
        }
        if (is_point_inside_clip_volume(gHeroPos, info->clip_volume)) {
            return 1;
        }
        break;
    }
    case 7: {
        char *base = D_0013F450;
        short e = cur->id;

        if (e != *(int *)(base + 0x2284)) {
            return 0;
        }
        if (other->transition_state == 0 && !(state[8] > other->priority)) {
            return 0;
        }
        if (e != 3) {
            return 1;
        }
        {
            CameraDef *rec = &gCameraDefs[cur->def_index];
            int g = rec->info->rule7_index;

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

/* Camera_Exit: calls the camera type's exit hook (camera_types[cam->type].exit);
   UpdateAllCameras calls it on the current camera first thing each frame. */
void Camera_Exit(Camera *cam) {
    void (*fn)(Camera *) = camera_types[cam->type].exit;
    if (fn != 0) {
        fn(cam);
    }
}

extern int D_00189C50[];
extern Camera D_00187510[];
extern int func_001EC5B8(Camera *cur, Camera *other);
extern void func_001EC2B8(Camera *next_camera);

/* UpdateAllCameras: the per-frame camera pass. Exits the current camera
   (Camera_Exit), scans the 48 camera slots (gCameraSlotUsed[i] != 0 = in
   use, gCameraSlots[i] = the Camera record) and keeps whichever
   Camera_ActivationCheckPriority prefers over the current one. If that
   changed the camera, switch_active_camera_record switches to it. Then the new
   camera's update hook (read before Camera_handleCollWithHero, as retail
   does), a copy of its position into prev_pos, and the post-update queue.
   Returns -1, which the one caller ignores.

   The indexed loop is what gives retail's preheader: strength reduction
   builds the two slot pointers in its order, and loop reversal makes the
   count run down 47..0. Indexing the camera-type table by the typed
   Camera.type member puts the base first in the addu. */
int UpdateAllCameras(void) {
    Camera *cur = gCamera.current;
    int changed = 0;
    Camera *rec;
    int i;

    Camera_Exit(cur);

    for (i = 0; i < 0x30; i++) {
        if (gCameraSlotUsed[i] != 0) {
            rec = &gCameraSlots[i];
            if (rec != cur) {
                if (Camera_ActivationCheckPriority(rec, cur)) {
                    cur = rec;
                    changed = 1;
                }
            }
        }
    }

    if (changed) {
        switch_active_camera_record(cur);
    }
    {
        void (*update)(Camera *) = camera_types[cur->type].update;
        float *src;
        float *dst;
        float v;

        Camera_handleCollWithHero(cur);
        if (update != 0) {
            update(cur);
        }
        src = &cur->pos.x;
        dst = &cur->prev_pos.x;
        v = src[0];
        cur->prev_pos.x = v;
        dst[1] = src[1];
        dst[2] = src[2];
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

/* capture_camera_orientation: the hero moby's axes go into blend.orbit.fwd
   and blend.orbit.up, its yaw/pitch/distance to the hero into blend.orbit
   (yaw, pitch, dist), and blend.orbit.rot is copied from saved_rot. The
   builds three unit vectors from D_0013F450's +0x2080 pointer table
   (+0xC0/+0xD0/+0xE0 offsets, re-read at each call as retail does). */
void capture_camera_orientation(void) {
    char *g = D_0013F450;
    CamBlend *r = &gCamera.blend;
    char row0[16];
    char row1[16];
    char row2[16];

    func_001F9DC0(row0, *(char **)(g + 0x2080) + 0xC0, 1.0f);
    func_001F9DC0(row1, *(char **)(g + 0x2080) + 0xD0, 1.0f);
    func_001F9DC0(row2, *(char **)(g + 0x2080) + 0xE0, 1.0f);

    qcopy(&r->orbit.fwd, row0);
    qcopy(&r->orbit.up, row2);

    Camera_Pos2Polar3d((float *)&r->orbit.yaw, &r->orbit.pos, g + 0x80, row0, row1, row2);

    qcopy(&r->orbit.rot, &r->orbit.saved_rot);
}

extern void func_001F9BD8(void *, void *, void *);
extern char D_0013F590[];

/* save_camera_vectors: with no blend running (kind 0), copies the
   lerp position into orbit.pos and the lerp rotation into orbit.saved_rot;
   for next_kind 2 the hero's platform movement (D_0013F590) is added to
   orbit.pos first. Mirror of restore_camera_vectors. */
void save_camera_vectors(void) {
    CamBlend *b = &gCamera.blend;
    if (b->kind == 0) {
        qcopy(&b->orbit.pos, &b->lerp.pos);
        if (b->next_kind == 2) {
            FastVecAdd(&b->orbit.pos, D_0013F590, &b->orbit.pos);
        }
        qcopy(&b->orbit.saved_rot, &b->lerp.rot);
    }
}

/* restore_camera_vectors: with a blend running (kind set), copies
   orbit.pos and orbit.saved_rot back into lerp.pos and lerp.rot. */
void restore_camera_vectors(void) {
    CamBlend *b = &gCamera.blend;
    if (b->kind != 0) {
        qcopy(&b->lerp.pos, &b->orbit.pos);
        qcopy(&b->lerp.rot, &b->orbit.saved_rot);
    }
}

/* The final mode store goes through this alias: written through
   D_001872B0 itself, gcse keeps the entry's %hi alive in a saved register
   across the calls, where retail rebuilds it. */
extern short D_001872B0_h __asm__("D_001872B0") NOT_SDA;
extern void func_00215328(void *, void *);
extern int func_001F98C0(int);
extern float func_001FA888(int);

/* start_camera_blend(from): captures the blend source from the previous
   camera and starts the blend (phase 3). When the phase is 1 or 2, the
   sub-mode (next_kind) picks what is captured from it: 0 the lerp pair
   (pos, rot), 2 the orbit pair and capture_camera_orientation's angles,
   otherwise the hero's axes and Camera_Pos2Polar3d's yaw/pitch/dist into
   the orbit. Other phases restore through save_camera_vectors and
   restore_camera_vectors. Then the phase becomes 3, kind takes
   next_kind, and either the lerp resets or the orbit frame count advances. */
void start_camera_blend(Camera *from) {
    CamBlend *r = &gCamera.blend;
    char local0[16];
    char local1[16];
    char local2[16];
    unsigned char sub;

    if (r->phase == 1) {
        sub = r->next_kind;
        if (sub == 0) {
            qcopy(&r->lerp.pos, &from->pos);
            extract_matrix_rotation_quaternion(&r->lerp.rot, from);
        } else if (sub == 2) {
            qcopy(&r->orbit.pos, &from->pos);
            extract_matrix_rotation_quaternion(&r->orbit.saved_rot, from);
            capture_camera_orientation();
        } else {
            char *g = D_0013F450;

            func_001F9DC0(local0, *(char **)(g + 0x2080) + 0xC0, 1.0f);
            func_001F9DC0(local1, *(char **)(g + 0x2080) + 0xD0, 1.0f);
            func_001F9DC0(local2, *(char **)(g + 0x2080) + 0xE0, 1.0f);
            /* &gCamera.current->pos: the previous camera is the one the switch saved */
            Camera_Pos2Polar3d((float *)&r->orbit.yaw, &from->pos,
                          &(*(Camera **)((char *)r - 0xF0))->pos,
                          local0, local1, local2);
            extract_matrix_rotation_quaternion(&r->orbit.rot, from);
            qcopy(&r->orbit.saved_rot, &r->orbit.rot);
        }
    } else {
        sub = r->next_kind;
        if (sub == 2) {
            save_camera_vectors();
            capture_camera_orientation();
        } else if (sub == 1) {
            save_camera_vectors();
            qcopy(&r->orbit.rot, &r->orbit.saved_rot);
        } else if (sub == 0) {
            restore_camera_vectors();
        }
    }
    D_001872B0_h = 3; /* blend.phase */
    r->kind = r->next_kind;
    if (r->kind == 0) {
        CamBlendLerp *a = &r->lerp;

        a->pos_rate = a->pos_rate_cfg;
        a->pos_t = 0;
        qcopy(&r->lerp.from_pos, &r->lerp.pos);
        a->rot_t = 0;
        a->rot_rate = a->rot_rate_cfg;
        qcopy(&r->lerp.from_rot, &r->lerp.rot);
    } else {
        CamBlendOrbit *o = &r->orbit;
        int n = ++o->frames;

        o->frames_left = func_001F98C0(n);
        o->inv_frames = 1.0f / IntToFloat(o->frames_left);
    }
}

extern char D_00187390[];
extern float D_0015EE60 MACRO_ADDR;
extern float func_00214220(float, float, float);
extern void func_001FA6C0(void *, void *);
extern void func_001FA480(void *, void *);

/* step_camera_blend(to, lerp): one step of the eased blend toward the target
   camera. While the position (pos_t) or rotation (rot_t) blend has not
   reached 1, it moves lerp.from_pos toward to's position by the eased factor,
   copies that to gCamera.pos unless the debug free camera is active, slerps
   the rotation from lerp.from_rot into lerp.rot and loads it as the view
   matrix, then advances both blends by their rates times D_0015EE60, capped
   at 1. Returns 1 once both are complete. */
int step_camera_blend(Camera *to, CamBlendLerp *lerp) {
    float m[4];
    float q[16];
    DebugFreeCam *st;
    float t;
    float *rot;

    if (lerp->pos_t == 1.0f && lerp->rot_t == 1.0f) {
        return 1;
    }
    t = CosineInterpolate(0.0f, 1.0f, lerp->pos_t);
    FastVecAdd(&lerp->from_pos, D_0013F590, &lerp->from_pos);
    st = &occl_cam_state;
    lerp->pos.f[0] = lerp->from_pos.f[0] + (to->pos.x - lerp->from_pos.f[0]) * t;
    lerp->pos.f[1] = lerp->from_pos.f[1] + (to->pos.y - lerp->from_pos.f[1]) * t;
    lerp->pos.f[2] = lerp->from_pos.f[2] + (to->pos.z - lerp->from_pos.f[2]) * t;
    if (st->active == 0) {
        qcopy(&gCamera.pos, &lerp->pos);
    }
    extract_matrix_rotation_quaternion(m, to);
    rot = lerp->rot.f;
    t = CosineInterpolate(0.0f, 1.0f, lerp->rot_t);
    func_001FA5C8(rot, &lerp->from_rot, m, t);
    quaternion_to_matrix(rot, q);
    if (st->active == 0) {
        func_001FA480(gCamera.matrix, q);
    }
    lerp->pos_t += lerp->pos_rate * D_0015EE60;
    if (1.0f < lerp->pos_t) {
        lerp->pos_t = 1.0f;
    }
    lerp->rot_t += lerp->rot_rate * D_0015EE60;
    if (1.0f < lerp->rot_t) {
        lerp->rot_t = 1.0f;
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
extern char D_0013F6E0[];

typedef struct {
    float v[4];
} Vec;

/* advance_alternate_camera_transition(cam, o): one orbit step of the switch blend.
   Steps yaw, pitch and distance (o->yaw, o->pitch, o->dist) toward the target
   orientation, rebuilds the orbit position and rotation and the camera basis
   (gCamera.matrix), and returns 1 once the orbit's frames are used up.
   Adapted from Lombyte (GPL-3.0) for PAL: src/gameplay/camera/fun_001eccd8.c, FUN_001eccd8. */
int advance_alternate_camera_transition(Camera *cam, CamBlendOrbit *o) {
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

    if (o->frames_left <= 0) {
        return 1;
    }
    step = 1.0f / CosineInterpolate(1.0f, (float)o->frames, (float)o->frames_left * o->inv_frames);
    if (gCamera.blend.kind == 2) {
        qcopy(&target, gHeroPos);
        qcopy(&fwd, &o->fwd);
        qcopy(&up, &o->up);
        FastVecCross(&side, &fwd, &up);
        Camera_Pos2Polar3d(ang.v, &cam->pos, &target, &fwd, &side, &up);
    } else {
        qcopy(&target, &cam->pos);
        g = D_0013F450;
        func_001F9DC0(&fwd, *(char **)(g + 0x2080) + 0xC0, 1.0f);
        func_001F9DC0(&up, *(char **)(g + 0x2080) + 0xE0, 1.0f);
        ang.v[2] = 0.0f;
        ang.v[1] = 0.0f;
        ang.v[0] = 3.1415927f;
    }
    dyaw = FastSubRots(ang.v[0], o->yaw);
    o->yaw = FastAddRots(o->yaw, dyaw * step);
    dpitch = FastSubRots(ang.v[1], o->pitch);
    o->pitch = FastAddRots(o->pitch, dpitch * step);
    o->dist = o->dist + (ang.v[2] - o->dist) * step;
    func_001F9DC0(&pos, &fwd, o->dist);
    build_look_at_matrix(&pos, &pos, &up, o->yaw);
    FastVecCross(&side, &pos, &up);
    func_001F9DC0(&side, &side, 1.0f);
    build_look_at_matrix(&pos, &pos, &side, o->pitch);
    FastVecAdd(&o->pos, &target, &pos);
    if (occl_cam_state.active == 0) {
        qcopy(&gCamera.pos, &o->pos);
    }
    func_001FA648(&o->rot, m);
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
        rotate_vector_by_quaternion(&r0, &m[0], &r2);
        rotate_vector_by_quaternion(&r1, &m[1], &r2);
    }
    if (FastAbsF(yaw) < 1e-5f) {
        qcopy(&r2, &m[0]);
    } else {
        build_quaternion_from_axis_angle(&q, &m[2], yaw);
        rotate_vector_by_quaternion(&r2, &m[0], &q);
    }
    yaw = 1.5707964f - FastArcSin(FastVecDot(&r2, cam));
    d = FastVecDot(&r2, &cam->basis[2]);
    s2 = -1.0f;
    if (d >= 0.0f) {
        s2 = 1.0f;
    }
    yaw *= s2;
    build_look_at_matrix(&r0, &r0, &r1, yaw * step);
    func_001F9DC0(&gCamera.matrix[0], &r0, 1.0f);
    FastVecCross(&gCamera.matrix[1], &gCamera.matrix[0], D_0013F6E0);
    func_001F9DC0(&gCamera.matrix[1], &gCamera.matrix[1], -1.0f);
    FastVecCross(&gCamera.matrix[2], &gCamera.matrix[1], &gCamera.matrix[0]);
    extract_matrix_rotation_quaternion(&o->saved_rot, gCamera.matrix);
    extract_matrix_rotation_quaternion(&o->rot, gCamera.matrix);
    func_001F9908(&o->frames_left);
    return 0;
}

extern int func_001ECEA0(Camera *, CamBlendLerp *);
extern int func_001ED080(Camera *, CamBlendOrbit *);

/* update_camera_blend(cam): steps the running blend. With kind 0 it is
   step_camera_blend into gCamera.blend.lerp, otherwise advance_alternate_camera_transition
   into gCamera.blend.orbit. On success, unless the debug free camera is active,
   cam's basis and position are copied into gCamera.matrix and gCamera.pos, then
   the blend's phase and kind are cleared. */
void update_camera_blend(Camera *cam) {
    CamBlend *b = &gCamera.blend;
    int result;

    if (b->kind == 0) {
        result = step_camera_blend(cam, &b->lerp);
    } else {
        result = advance_alternate_camera_transition(cam, &b->orbit);
    }
    if (result != 0) {
        if (occl_cam_state.active == 0) {
            qcopy(&gCamera.matrix[0], &cam->basis[0]);
            qcopy(&gCamera.matrix[1], &cam->basis[1]);
            qcopy(&gCamera.matrix[2], &cam->basis[2]);
            qcopy(&gCamera.pos, &cam->pos);
        }
        b->phase = 0;
        b->kind = 0;
    }
}

extern float func_001FA888(int arg0);
extern float func_001FA7D8(float x);
extern float func_001F9F90(float x);
extern void func_001F9DC0(void *dst, void *src, float len);
extern void func_001F9BD8(void *dst, void *a, void *b);

/* apply_camera_shake(s, along_row0): one shake channel (gCamera.shake[0] or
   [1]). With the active camera (gCamera.current) in state 6 both counters are
   cleared; with the countdown at 0 only frames_max is. Otherwise the countdown
   is decremented (func_001F9908, saturating) and offset = amplitude *
   cos(wrap(2 * count)) * (count / longest)^2 (func_001FA888 int->float,
   func_001FA7D8 wrap to [-pi, pi], func_001F9F90 cos) scales one of two
   directions (matrix[2] for along_row0 0, else matrix[0]) into gCamera.pos
   (func_001F9DC0 scale, func_001F9BD8 add). The countdown-zero exit is the else
   arm after the main path (retail's `b L800` over it), and the state-6 stores
   are written frames then frames_max so the scheduler emits frames_max first
   and cross-jumping leaves them alone. */
void apply_camera_shake(CameraShake *s, int along_row0) {
    char buf[16];

    if (gCamera.current != 0 && gCamera.current->id == 6) {
        s->frames = 0;
        s->frames_max = 0;
    } else if (s->frames != 0) {
        float ratio;
        float shake;

        if (s->frames_max < s->frames) {
            s->frames_max = s->frames;
        }
        func_001F9908(&s->frames);
        ratio = IntToFloat(s->frames) / IntToFloat(s->frames_max);
        shake = s->amplitude
                * FastCos(FastNormalizeAngle(IntToFloat(s->frames) * 2.0f))
                * ratio * ratio;
        s->offset = shake;
        if (along_row0 == 0) {
            func_001F9DC0(buf, &gCamera.matrix[2], shake);
        } else {
            func_001F9DC0(buf, &gCamera.matrix[0], shake);
        }
        FastVecAdd(&gCamera.pos, &gCamera.pos, buf);
    } else {
        s->frames_max = 0;
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
extern struct Player D_0013F450_ED818 __asm__("D_0013F450");
extern f32 func_001EC120_ED818(f32 *vel, f32 from, f32 to, f32 stiffness, f32 damping, f32 max) __asm__("func_001EC120");
extern float func_001F9B88(float input);
extern void func_001F9BF0(void *out, void *a, void *b);
extern void func_001F9C30(void *out, void *a, f32 s);
extern f32 func_001F9C78(void *a, void *b);
extern f32 func_001F9CB8(void *a);
extern void func_001F9DC0(void *out, void *a, f32 len);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/gameplay/camera/fun_001ed470.c, FUN_001ed470. */
/* update_camera_tracking_state: tracks the hero for gCamera.follow each frame: the
   smoothed up direction (up, springs up_vel toward the hero's gravity
   inverse), the hero movement split along and across up, the z spring (pos,
   z_vel), the hero's yaw history, and the moby the hero stands on with its
   height change (ground_moby, ground_z, ground_dz). */
void update_camera_tracking_state(void) {
    Vec4 dir;
    Vec4 proj;
    CameraFollow *f;
    f32 d;
    s32 i;
    struct Moby *m;

    f = &gCamera.follow;
    func_001F9DC0(&dir, &D_0013F450_ED818.unk290, -1.0f);
    qcopy(&f->up_target_prev, &f->up_target);
    qcopy(&f->up_target, &dir);
    if (FastVecDot(&f->up, &dir) < -0.98f) {
        dir.f[0] += 0.2f;
        dir.f[1] += 0.2f;
        dir.f[2] += 0.2f;
    }
    f->up.f[0] =
        func_001EC120_ED818(&f->up_vel[0], f->up.f[0], dir.f[0], 0.015f, 0.2f, 0.0f);
    f->up.f[1] =
        func_001EC120_ED818(&f->up_vel[1], f->up.f[1], dir.f[1], 0.015f, 0.2f, 0.0f);
    f->up.f[2] =
        func_001EC120_ED818(&f->up_vel[2], f->up.f[2], dir.f[2], 0.015f, 0.2f, 0.0f);
    func_001F9DC0(&f->up, &f->up, 1.0f);

    FastVecSub(&f->hero_delta, &D_0013F450_ED818.pos, &f->hero_prev);
    f->hero_dist = FastVecLength(&f->hero_delta);
    d = FastVecDot(&f->hero_delta, &dir);
    f->up_dist = d;
    func_001F9DC0(&proj, &dir, d);
    qcopy(&f->hero_delta_up, &proj);
    FastVecSub(&f->hero_delta_side, &f->hero_delta, &proj);
    f->side_dist = FastVecLength(&f->hero_delta_side);
    FastVecScale(&f->hero_delta_side, &f->hero_delta_side, 1.0f / f->side_dist);
    qcopy(&f->hero_prev, &D_0013F450_ED818.pos);

    if (D_0013F450_ED818.unk2284 != 0x50 || D_0013F450_ED818.unk2084 == 0x11) {
        f->pos.f[0] = D_0013F450_ED818.pos.f[0];
        f->pos.f[1] = D_0013F450_ED818.pos.f[1];
        f->pos.f[2] =
            func_001EC120_ED818(&f->z_vel, f->pos.f[2], D_0013F450_ED818.pos.f[2], 0.0075f, 0.175f, 0.0f);
        f->pos.f[3] = D_0013F450_ED818.pos.f[2];
    } else {
        f->pos.f[0] = D_0013F450_ED818.pos.f[0];
        f->pos.f[1] = D_0013F450_ED818.pos.f[1];
    }

    for (i = 0; i < 4; i++) {
        f->yaw_hist[i] = f->yaw_hist[i + 1];
    }
    f->yaw_hist[i] = D_0013F450_ED818.unk98;

    m = D_0013F450_ED818.unk2FC;
    if (m != 0 && m->oclass != 0x4BA && m->oclass != 0x336) {
        if (m == f->ground_moby) {
            f->ground_dz = m->pos.z - f->ground_z;
            if (FastAbsF(f->ground_dz) < 0.001f) {
                f->ground_dz = 0.0f;
            }
            f->ground_z = f->ground_moby->pos.z;
        } else {
            f->ground_moby = m;
            f->ground_dz = 0.0f;
            f->ground_z = m->pos.z;
        }
    } else {
        f->ground_dz = 0.0f;
        f->ground_moby = 0;
        f->ground_z = D_0013F450_ED818.pos.f[2];
    }
}

extern char D_00194220[];
extern int D_0015F6E8 MACRO_ADDR;
extern int func_001EFE10(void *, void *, int, int, int);
extern int func_001F0F00(void);
extern float func_00214440(void *, int);

/* Camera-inside-water test: cast a ray through the eye position
   (gCamera.pos) from 0.75 above to 0.75 below (up to six hits); on the
   first hit that is not a water surface, set gCamera.underwater when the
   eye is below the surface height + 0.04. Off for camera id 6 or while
   D_0015F6E8 is set. */
void update_camera_underwater_flag(void) {
    CameraSystem *cam = &gCamera;
    float a[4];
    float b[4];
    int i;

    if (cam->current->id == 6 || D_0015F6E8 != 0) {
        cam->underwater = 0;
        return;
    }
    qcopy(a, &cam->pos);
    qcopy(b, &cam->pos);
    a[2] += 0.75f;
    b[2] -= 0.75f;
    i = 0;
    while (i < 6 && func_001EFE10(a, b, 0x12, 0, 0) != 0) {
        if (func_001F0F00() == 0) {
            float h = resolve_camera_surface_height(D_00194220, 0) + 0.04f;
            CameraSystem *c2 = &gCamera;

            if (c2->pos.f[2] < h) {
                c2->underwater = 1;
            } else {
                c2->underwater = 0;
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

/* refresh_camera_control_flags: the probe masks from the hero's state and ground.
   cam_coll_flag is 0x14, or 0x34 in hero states 0x11/0x12 or mode 0x73 of
   D_0013F450, back to 0x14 when its +0x2F0 height is below gCamera.pos.z
   (except in state 0x11); cam_coll_flag_prev keeps the value before bit 0x80
   is set. cam_coll_mode is 0xB4 ORed with follow.coll_bits, which the first set
   flag of D_0013F450's 0x12E5, 0x12EB, 0x12E6, 0x12EC, 0x12E4 overrides. Each
   arm ORs the new mode in itself (the constants fold per arm, as in retail),
   and each block reads D_0013F450 through its own local (%hi kept, %lo
   rebuilt). */
extern Hero D_0013F450_HERO __asm__("D_0013F450");

void refresh_camera_control_flags(void) {
    CameraFollow *f = &gCamera.follow;
    Hero *g = &D_0013F450_HERO;
    int st;

    cam_coll_flag = 0x14;
    st = g->state.control_mode;
    if (st == 0x11 || st == 0x12 || g->state.current == 0x73) {
        cam_coll_flag = 0x34;
    }
    {
        Hero *g2 = &D_0013F450_HERO;
        if (g2->state.control_mode != 0x11
            && g2->water_surface_z < gCamera.pos.f[2]) {
            cam_coll_flag = 0x14;
        }
    }
    {
        Hero *g3 = &D_0013F450_HERO;
        cam_coll_flag_prev = cam_coll_flag;
        cam_coll_flag |= 0x80;
        cam_coll_mode = 0xB4;
        if (g3->selector_1) {
            f->coll_bits = 0x100;
            cam_coll_mode |= f->coll_bits;
        } else if (g3->selector_11) {
            f->coll_bits = 0xB00;
            cam_coll_mode |= f->coll_bits;
        } else if (g3->selector_3) {
            f->coll_bits = 0x300;
            cam_coll_mode |= f->coll_bits;
        } else if (g3->selector_13) {
            f->coll_bits = 0xD00;
            cam_coll_mode |= f->coll_bits;
        } else if (g3->base_condition) {
            f->coll_bits = 0;
            cam_coll_mode |= f->coll_bits;
        } else {
            cam_coll_mode |= f->coll_bits;
        }
    }
}

extern float D_0015F53C MACRO_ADDR;

/* Takes gCamera.follow.fade_rate off gFade each frame; both stop at 0.
   gFade (D_0015F53C) is a MACRO_ADDR float: $gp-relative in the delay
   slots, lui $1 in the body. */
void advance_timed_camera_control(void) {
    CameraSystem *c = &gCamera;
    float a = c->follow.fade_rate;

    if (a != 0.0f) {
        float t = gFade - a;
        gFade = t;
        if (t <= 0.0f) {
            c->follow.fade_rate = 0.0f;
            gFade = 0.0f;
        }
    }
}

extern int D_001E6700;
extern unsigned char D_0015EEB4_m[4] __asm__("D_0015EEB4") MACRO_ADDR;
extern void func_001EDE08(void);
extern void func_001EDCE8(void);
extern void func_001ED818(void);
extern void func_001ECC48(Camera *);
extern void func_001FA460(void *, void *);
extern void func_002153E8(void *, void *);
extern void func_001EE858(void *);

/* Camera update, once per frame: count the frame, run the camera
   steps, then (unless the debug free camera is active) take the view
   from the current camera (blend phase 3 blends it through
   update_camera_blend) and refresh its Euler angles; run the two shake
   channels, the underwater test and the environment update, and, with
   D_0015EEB4 (mirrored levels) set, rebuild matrix[1] as a cross
   product. The blend reset and the shake calls go through local
   pointers to their sub-blocks, as retail keeps them in registers. */
void update_camera(void) {
    CameraSystem *c;
    Camera *target;
    CameraShake *v;
    float m[16];

    if (D_0015F6E8 == 5) {
        if (text_screen_active != 0) {
            return;
        }
        {
            CamBlend *b = &gCamera.blend;
            b->phase = 0;
            b->kind = 0;
        }
    }
    c = &gCamera;
    c->frames++;
    advance_timed_camera_control();
    refresh_camera_control_flags();
    update_camera_tracking_state();
    UpdateAllCameras();
    target = c->current;
    if ((unsigned short)(c->blend.phase - 1) < 2) {
        start_camera_blend(c->previous);
    }
    if (c->blend.phase == 3) {
        update_camera_blend(target);
    } else {
        DebugFreeCam *st = &occl_cam_state;
        if (st->active != 0) {
            goto frozen;
        }
        qcopy(&c->pos, &target->pos);
        qcopy(&c->matrix[0], &target->basis[0]);
        qcopy(&c->matrix[1], &target->basis[1]);
        qcopy(&c->matrix[2], &target->basis[2]);
    }
    {
        DebugFreeCam *st = &occl_cam_state;
        if (st->active == 0) {
            func_001FA460(m, camera_rows);
            matrix_to_euler(m, camera_rows - 0x200);
        }
    }
frozen:
    v = gCamera.shake;
    apply_camera_shake(&v[0], 0);
    apply_camera_shake(&v[1], 1);
    update_camera_underwater_flag();
    update_camera_environment_from_regions((char *)v - 0x20);       /* &gCamera.pos */
    if (D_0015EEB4_m[0] != 0) {
        FastVecCross((char *)v + 0x200, (char *)v + 0x210, (char *)v + 0x1F0); /* matrix[1] = matrix[2] x matrix[0] */
    }
}

LINKER_REMNANT("asm/remnants/text", func_001EDFD8);

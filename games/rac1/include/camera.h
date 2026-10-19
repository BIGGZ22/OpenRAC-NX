#ifndef CAMERA_H
#define CAMERA_H

/*
 * The camera system: the 48 camera slots, the camera-type table and the
 * 0x3A0-byte camera block the renderer reads. Map, callers and evidence:
 * docs/kb/refactor-camera.md.
 *
 * Types only, like the other shared headers: a file declares the globals
 * it uses itself (`extern CameraSystem D_00187040;`, readable name gCamera).
 * The executable's camera code (src/game/camera.c) and the level copy of it
 * (src/overlays/shared/camera_*.c) share these layouts; only the addresses
 * differ (executable D_00187040, level 00 D_L00_00166D80, other levels
 * packed elsewhere).
 *
 * Offsets are checked at compile time (the CAMERA_AT lines); a wrong field
 * stops the build instead of changing code. Field names are what the
 * matched code shows; unkXXX = accessed, meaning unknown; padXXX = not
 * accessed by the camera code.
 */

#include "common.h"
#include "vector.h"

struct Moby;

#define CAMERA_AT(type, field, off) \
    typedef char camera_at_##type##_##field[((unsigned long)&((type *)0)->field == (off)) ? 1 : -1]
#define CAMERA_SIZE(type, size) \
    typedef char camera_size_##type[(sizeof(type) == (size)) ? 1 : -1]

/* Three floats with 4-byte alignment. */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} CameraVec3;

/* One camera slot, 0xA0 bytes: gCameraSlots[48] (D_00187510; level code
   D_L00_00167250). Camera_InitSlots fills one per row of the level's camera
   table (gCameraDefs). */
typedef struct Camera {
    VecQuad basis[3];          /* 0x00 rotation rows; UpdateCamera copies them to gCamera.matrix */
    Vec4f pos;                 /* 0x30 position */
    u8 type_data[0x24];        /* 0x40 per-type state (read by the type's hooks) */
    CameraVec3 prev_pos;       /* 0x64 pos as of the last frame or switch */
    void *state_buf;           /* 0x70 the type's 0x280-byte state: gCamCurState while current */
    s32 activation;            /* 0x74 activation rule (CameraInfo.activation): 0, 1, 2, 4, 7 */
    f32 blend_time;            /* 0x78 blend rate or frames for the switch to it; <= 0: default */
    u8 priority;               /* 0x7C 0 = never activates; higher wins (CameraInfo.priority) */
    u8 handoff_state;          /* 0x7D rules 1 and 2 only activate once it is set */
    s16 transition_state;      /* 0x7E nonzero: any eligible camera may take over; 2..5 pick the blend */
    s16 unk80;                 /* 0x80 -1 after Camera_InitSlots */
    s16 unk82;                 /* 0x82 -1 after Camera_InitSlots */
    s16 def_index;             /* 0x84 row of gCameraDefs */
    s16 id;                    /* 0x86 camera id (CameraDef.id): 0 behind the hero; 6 turns shake off */
    u8 pad88;
    u8 hit_wall;               /* 0x89 MoveCameraToward hit something */
    s16 snap;                  /* 0x8A nonzero: MoveCameraToward jumps to the target once */
    s16 type;                  /* 0x8C index into gCameraTypes (Camera_FindTypeIndex(id)) */
    s16 hard_cut;              /* 0x8E 1 when it became current without a blend */
    u8 pad90[0x10];
} Camera;
CAMERA_AT(Camera, pos, 0x30);
CAMERA_AT(Camera, prev_pos, 0x64);
CAMERA_AT(Camera, state_buf, 0x70);
CAMERA_AT(Camera, activation, 0x74);
CAMERA_AT(Camera, priority, 0x7C);
CAMERA_AT(Camera, def_index, 0x84);
CAMERA_AT(Camera, id, 0x86);
CAMERA_AT(Camera, type, 0x8C);
CAMERA_AT(Camera, hard_cut, 0x8E);
CAMERA_SIZE(Camera, 0xA0);

/* A camera type, one 0x14-byte row of the camera-type table (the camvtbl
   record: executable D_001E8F80, level D_L00_001EAC00), id -1 ends it. Each
   hook gets the camera; a null hook is skipped. */
typedef struct CameraType {
    s32 id;                                        /* 0x00 Camera.id this type serves; -1 ends the table */
    s32 (*check)(Camera *cam, Camera *current);    /* 0x04 -1 never, 1 always, else the activation rule decides */
    void (*activate)(Camera *cam);                 /* 0x08 when it becomes current (Camera_runSetupToNewCam) */
    void (*update)(Camera *cam);                   /* 0x0C each frame while current (UpdateAllCameras) */
    void (*exit)(Camera *cam);                     /* 0x10 Camera_Exit; UpdateAllCameras calls it first each frame */
} CameraType;
CAMERA_AT(CameraType, check, 0x04);
CAMERA_AT(CameraType, exit, 0x10);
CAMERA_SIZE(CameraType, 0x14);

/* The level's parameters for one camera, pointed at by CameraDef.info. */
typedef struct CameraInfo {
    u8 pad00[4];
    Camera *slot;              /* 0x04 the slot Camera_InitSlots gave it */
    u8 pad08[4];
    s32 clip_volume;           /* 0x0C rule 4: active while the hero is inside it */
    u8 pad10[0xC];
    u8 priority;               /* 0x1C copied to Camera.priority */
    u8 blend_kind;             /* 0x1D 1 eased blend, 5 orbit blend, 3 and 6 hand-off */
    u8 pad1E;
    u8 activation;             /* 0x1F copied to Camera.activation */
    u8 pad20[4];
    s32 rule7_index;           /* 0x24 rule 7: row of D_0015F040, < 0 none */
} CameraInfo;
CAMERA_AT(CameraInfo, clip_volume, 0x0C);
CAMERA_AT(CameraInfo, blend_kind, 0x1D);
CAMERA_AT(CameraInfo, rule7_index, 0x24);

/* One row of the level's camera table (gCameraDefs, D_0015F090; level
   D_L00_0015F050, gCameraDefCountLvl rows). */
typedef struct CameraDef {
    u8 pad00[0xC];
    u16 id;                    /* 0x0C copied to Camera.id */
    u8 pad0E[2];
    s32 unk10;                 /* 0x10 */
    u8 pad14[8];
    CameraInfo *info;          /* 0x1C */
} CameraDef;
CAMERA_AT(CameraDef, id, 0x0C);
CAMERA_AT(CameraDef, info, 0x1C);
CAMERA_SIZE(CameraDef, 0x20);

/* A shake channel (gCamera.shake[0], [1]; Camera_UpdateShake). */
typedef struct CameraShake {
    f32 amplitude;             /* 0x0 */
    f32 offset;                /* 0x4 this frame's sample, added along a view row */
    s32 frames;                /* 0x8 countdown */
    s32 frames_max;            /* 0xC longest countdown since it started */
} CameraShake;
CAMERA_SIZE(CameraShake, 0x10);

/* What CameraPreUpdate tracks of the hero each frame: gCamera.follow
   (D_001871D0, level D_L00_00166F10). */
typedef struct CameraFollow {
    Vec4 pos;                  /* 0x00 hero position, z sprung (w: raw z) */
    f32 z_vel;                 /* 0x10 spring velocity of pos.z */
    u8 pad14[0xC];
    Vec4 up;                   /* 0x20 sprung up direction (minus the hero's gravity_dir) */
    Vec4 up_target;            /* 0x30 this frame's up */
    Vec4 up_target_prev;       /* 0x40 last frame's up */
    f32 up_vel[4];             /* 0x50 spring velocities of up */
    Vec4 hero_prev;            /* 0x60 hero position last frame */
    Vec4 hero_delta;           /* 0x70 hero movement this frame */
    Vec4 hero_delta_side;      /* 0x80 its part across up, then normalised */
    Vec4 hero_delta_up;        /* 0x90 its part along up */
    f32 hero_dist;             /* 0xA0 |hero_delta| */
    f32 side_dist;             /* 0xA4 |part across up| */
    f32 up_dist;               /* 0xA8 part along up */
    f32 yaw_hist[5];           /* 0xAC hero yaw (motion.rot.z), oldest first */
    s32 coll_bits;             /* 0xC0 ground-type bits ORed into gCamLineMask */
    struct Moby *hero_coll_moby; /* 0xC4 collision moby kept for camera id 0 */
    f32 fade_rate;             /* 0xC8 Camera_UpdateFade takes it off gFade each frame */
    s32 unkCC;                 /* 0xCC */
    s32 unkD0;                 /* 0xD0 */
    struct Moby *ground_moby;  /* 0xD4 moby the hero stands on (not class 0x4BA/0x336) */
    f32 ground_z;              /* 0xD8 its z last frame */
    f32 ground_dz;             /* 0xDC its z change this frame (0 under 0.001) */
} CameraFollow;
CAMERA_AT(CameraFollow, up_vel, 0x50);
CAMERA_AT(CameraFollow, yaw_hist, 0xAC);
CAMERA_AT(CameraFollow, coll_bits, 0xC0);
CAMERA_AT(CameraFollow, ground_moby, 0xD4);
CAMERA_SIZE(CameraFollow, 0xE0);

/* The eased blend Camera_StepBlend runs (gCamera.blend.lerp). */
typedef struct CamBlendLerp {
    f32 rot_t;                 /* 0x00 rotation progress 0..1 */
    f32 rot_rate;              /* 0x04 per frame, times D_0015EE60 */
    f32 rot_rate_cfg;          /* 0x08 set by the switch (0.018, 0.01, or Camera.blend_time) */
    f32 pos_t;                 /* 0x0C position progress 0..1 */
    f32 pos_rate;              /* 0x10 */
    f32 pos_rate_cfg;          /* 0x14 */
    u8 pad18[8];
    Vec4 from_rot;             /* 0x20 quaternion it starts from */
    Vec4 from_pos;             /* 0x30 position it starts from */
    Vec4 pos;                  /* 0x40 blended position */
    Vec4 rot;                  /* 0x50 blended rotation quaternion */
} CamBlendLerp;
CAMERA_AT(CamBlendLerp, from_rot, 0x20);
CAMERA_SIZE(CamBlendLerp, 0x60);

/* The yaw/pitch/distance blend Camera_UpdateTowardTarget runs (gCamera.blend.orbit). */
typedef struct CamBlendOrbit {
    f32 yaw;                   /* 0x00 */
    f32 pitch;                 /* 0x04 */
    f32 dist;                  /* 0x08 */
    s32 frames_left;           /* 0x0C counted down (FastDecTimer) */
    f32 inv_frames;            /* 0x10 1 / frames_left at the start */
    s32 frames;                /* 0x14 length; the switch sets it (40, or Camera.blend_time) */
    u8 pad18[8];
    Vec4 fwd;                  /* 0x20 axes the angles are measured in */
    Vec4 up;                   /* 0x30 */
    Vec4 rot;                  /* 0x40 rotation quaternion */
    Vec4 pos;                  /* 0x50 position */
    Vec4 saved_rot;            /* 0x60 quaternion kept across a restart */
} CamBlendOrbit;
CAMERA_AT(CamBlendOrbit, fwd, 0x20);
CAMERA_SIZE(CamBlendOrbit, 0x70);

/* The switch blend: gCamera.blend (D_001872B0, level D_L00_00166FF0). */
typedef struct CamBlend {
    s16 phase;                 /* 0x00 0 none, 1 start, 2 restart, 3 running */
    u8 kind;                   /* 0x02 running blend: 0 lerp, else orbit (2: around the hero) */
    u8 next_kind;              /* 0x03 the kind the switch asked for */
    u8 pad04[0xC];
    CamBlendLerp lerp;         /* 0x10 */
    CamBlendOrbit orbit;       /* 0x70 */
} CamBlend;
CAMERA_AT(CamBlend, lerp, 0x10);
CAMERA_AT(CamBlend, orbit, 0x70);
CAMERA_SIZE(CamBlend, 0xE0);

/* The camera block, 0x3A0 bytes: gCamera (D_00187040; level 00
   D_L00_00166D80, gCameraLvl). transition.c clears it on a level load. */
typedef struct CameraSystem {
    u8 pad000[0x40];
    f32 view_matrix[4][4];     /* 0x040 the renderer's view matrix (draw.c) */
    u8 pad080[0xC0];
    Vec4 pos;                  /* 0x140 eye position (D_00187180) */
    Vec4 rot;                  /* 0x150 Euler angles of matrix; z is the yaw (D_00187198) */
    CameraShake shake[2];      /* 0x160 (D_001871A0) */
    Camera *current;           /* 0x180 (D_001871C0) */
    Camera *previous;          /* 0x184 */
    u8 pad188[8];
    CameraFollow follow;       /* 0x190 (D_001871D0) */
    CamBlend blend;            /* 0x270 (D_001872B0) */
    Vec4 matrix[3];            /* 0x350 view rotation rows (D_00187390, D_001873A0, D_001873B0) */
    s32 unk380[5];             /* 0x380 zeroed by Camera_InitSlots */
    s32 underwater;            /* 0x394 the eye is under water (D_001873D4) */
    s32 frames;                /* 0x398 frames since the last switch */
    u8 pad39C[4];
} CameraSystem;
CAMERA_AT(CameraSystem, view_matrix, 0x40);
CAMERA_AT(CameraSystem, pos, 0x140);
CAMERA_AT(CameraSystem, shake, 0x160);
CAMERA_AT(CameraSystem, current, 0x180);
CAMERA_AT(CameraSystem, follow, 0x190);
CAMERA_AT(CameraSystem, blend, 0x270);
CAMERA_AT(CameraSystem, matrix, 0x350);
CAMERA_AT(CameraSystem, underwater, 0x394);
CAMERA_AT(CameraSystem, frames, 0x398);
CAMERA_SIZE(CameraSystem, 0x3A0);

/* The debug free camera (D_0018C418, level D_L00_0016C158; DebugFreeCameraUpdate). */
typedef struct DebugFreeCam {
    u8 pad00[8];
    s32 flags;                 /* 0x08 bit 3 read by DebugFreeCameraUpdate */
    u8 pad0C[8];
    s32 active;                /* 0x14 nonzero: the camera code stops writing gCamera's view */
    u8 pad18[4];
    s32 unk1C;                 /* 0x1C cleared on a level load (transition.c) */
} DebugFreeCam;
CAMERA_AT(DebugFreeCam, active, 0x14);

#undef CAMERA_AT
#undef CAMERA_SIZE

#endif /* CAMERA_H */

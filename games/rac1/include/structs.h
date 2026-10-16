#ifndef STRUCTS_H
#define STRUCTS_H

/*
 * Recovered struct layouts.
 *
 * These exist to retire the wall of `*(int *)(s + 0x174)` in src/, which
 * is the long-term goal for this project. They are held to one hard
 * rule: **introducing a struct must not change a single byte**. Field
 * access through a correctly-laid-out struct compiles identically to the
 * offset arithmetic it replaces, so every conversion is verified with
 * tools/sweep_matches.py and reverted if the count moves.
 *
 * Fields are named only where their purpose is actually established.
 * `unkNN` is deliberate: a wrong name is worse than no name, and this
 * file is read as documentation.
 */

/*
 * Node at +0x1E4 of a larger object. Ghidra's caller cross-reference is
 * what identified the embedding: func_00113A70(parent + 0x1E4, 4, 0,
 * parent), i.e. it is constructed in place and handed a back-pointer to
 * its parent, which is what `owner` holds.
 *
 * The four function pointers at 0x20..0x2C are installed together by
 * that same constructor and always with the same four routines, so this
 * is a fixed dispatch block rather than a per-instance vtable.
 *
 * The dispatch routines themselves (func_001162B8, func_001163A0) fill
 * in the rest: they pass `owner` and `handle` down to the layer below,
 * accumulate into `pos`, and set/clear bit 0x1000 of `flags` to mark
 * whether that call succeeded. Note 0x1C and 0x54 are different things
 * -- 0x1C is a pointer to the node itself, 0x54 is the parent -- which
 * is only visible once the constructor and a dispatch routine are read
 * together.
 */
typedef struct Node1E4 {
    /* 0x00 */ int   unk00;
    /* 0x04 */ int   unk04;
    /* 0x08 */ int   unk08;
    /* 0x0C */ short flags;     /* bit 0x1000: last dispatch succeeded */
    /* 0x0E */ short handle;    /* passed down to the layer below */
    /* 0x10 */ int   unk10;
    /* 0x14 */ int   unk14;
    /* 0x18 */ int   unk18;
    /* 0x1C */ void *self;      /* points at this node */
    /* 0x20 */ void *fn20;
    /* 0x24 */ void *fn24;
    /* 0x28 */ void *fn28;
    /* 0x2C */ void *fn2C;
    /* 0x30 */ char  unk30[0x20];
    /* 0x50 */ int   pos;       /* accumulated by the dispatch routines */
    /* 0x54 */ void *owner;     /* the parent object this is embedded in */
} Node1E4;

/*
 * Object handled by the func_0023Cxxx family. Recovered by reading the
 * family together rather than one function at a time: the constructor
 * (func_0023C0E0) shows which fields are cleared, func_0023C088 shows
 * which are handed to the layer below, and func_0023C2B0/func_0023C2C0
 * show which are tested.
 *
 * Only `state` is named. It is set to 0 by the constructor, set to 2
 * after the submit in func_0023C088, and tested non-zero before
 * teardown in func_0023C2C0 -- that is enough to call it a state. The
 * rest keep unkNN: 0x4C is rounded down to a 0x400 multiple and 0x50 is
 * compared against 0x1000, which hints at a buffer size and a fill
 * level, but hinting is not knowing and a wrong name here would
 * propagate into every caller.
 */
typedef struct Obj23C {
    /* 0x00 */ int  state;
    /* 0x04 */ char unk04[0x10];
    /* 0x14 */ int  unk14;
    /* 0x18 */ int  unk18;
    /* 0x1C */ char unk1C[0x14];
    /* 0x30 */ int  unk30;
    /* 0x34 */ int  unk34;
    /* 0x38 */ int  unk38;
    /* 0x3C */ int  unk3C;
    /* 0x40 */ int  unk40;
    /* 0x44 */ int  unk44;
    /* 0x48 */ int  unk48;
    /* 0x4C */ int  unk4C;
    /* 0x50 */ int  unk50;
    /* 0x54 */ int  unk54;
    /* 0x58 */ int  unk58;
    /* 0x5C */ int  unk5C;
} Obj23C;

/*
 * A 3-element array at +0x1B8 of some parent object, stride 0x10.
 *
 * The stride is not a guess from one function: func_0012BBF8 walks
 * +0x1B8/+0x1C8/+0x1D8 and then +0x1BC/+0x1CC/+0x1DC (the +0x00 and
 * +0x04 fields, column-major), while the banked decode of func_00129600
 * independently uses +0x1B8/+0x1C4, +0x1C8/+0x1D4, +0x1D8/+0x1E4 -- the
 * +0x00 and +0x0C fields of the same three bases. Two unrelated
 * functions agreeing on the same 0x10 grid is what makes this a real
 * layout rather than a pattern in one function's offsets.
 *
 * Nothing is named. unk00 and unk04 are pointers to objects that have
 * something at their own +0x28 (func_0012BBF8 zeroes it). unk0C is
 * chosen *instead of* unk00 in func_00129600 depending on a mode field,
 * so it is probably a pointer of the same kind -- "probably" is why it
 * keeps its unk name. unk08 is never touched by anything decompiled so
 * far and is a placeholder holding the stride, not an observed field.
 *
 * Note the array runs 0x1B8..0x1E8, so its last element covers 0x1E4.
 * That is NOT the Node1E4 above: a different parent object happens to
 * have a node at the same offset. Do not conflate them.
 */
typedef struct Slot1B8 {
    /* 0x00 */ void *unk00;
    /* 0x04 */ void *unk04;
    /* 0x08 */ int   unk08;
    /* 0x0C */ void *unk0C;
} Slot1B8;  /* 0x10 */

/*
 * The object a wrapper holds at its +0x40, and the owner of the Slot1B8
 * array above.
 *
 * The identification is by call site, not by pattern-matching offsets:
 * func_0012C200 computes `inner = *(p + 0x40)` and then calls
 * func_0012C278(inner) directly, which is what proves func_0012C278's
 * argument is this object and not the wrapper. func_00129C78 shares the
 * 0x08/0xAC/0x118 field set with it, so it takes this object too.
 *
 * Nothing here is named -- the field roles are not established yet.
 * unk118 and unk0AC are subtracted from each other in func_0012C200 to
 * produce the wrapper's 0x08, and unk008 is compared against 2 as a
 * state, but that is suggestive rather than settled.
 *
 * SIZE IS NOT KNOWN. The trailing padding runs to 0x820 only because
 * that is the highest field anything decompiled so far touches
 * (func_00129C78), so sizeof(Obj40) is a floor, not the real size. Do
 * not embed this by value, allocate it, or index an array of it -- it is
 * only ever used through a pointer to memory the game already owns.
 */
/*
 * One entry of the handler table at Obj40 +0x0C.
 *
 * This is what explains a shape that looks wrong in the raw offsets.
 * func_0012BC50 indexes with a stride of 8 but writes at +0x0C and
 * +0x10, which overlaps the next entry and reads as nonsense; so does
 * func_0012BC78, which loads a function pointer from +0x0C and calls it
 * with the value at +0x10. Both make sense the moment the array is
 * based at +0x0C instead of at +0x00: entry i is at 0x0C + i*8, fn at
 * its +0x00 and data at its +0x04. The two functions then line up
 * exactly -- func_0012BC50 installs the pair that func_0012BC78 later
 * loads and invokes.
 *
 * The array LENGTH (0x14) is not established. It is the span between
 * +0x0C and the next known field at +0xAC, so it is an upper bound on
 * what fits, not a count anything observed. The two indexed accessors
 * take their index from the caller and never bound it. func_0012CC80
 * is the one place a constant index appears -- it passes +0x4C, which
 * is exactly &handlers[8] -- and that is the only direct evidence the
 * array reaches even that far.
 */
/*
 * WARNING -- converting callers to this struct is NOT byte-neutral.
 *
 * The three functions in this family (func_0012BBF8, func_0012BC50,
 * func_0012BC78) were converted to struct access and all three changed
 * size against retail: 84->88, 36->32, 80->84. They have been restored
 * to raw offset arithmetic and are byte-exact again. A size mismatch is
 * the most expensive mistake in this project -- it shifts every later
 * function -- so this is not a near-miss to tolerate.
 *
 * The layout below is still believed correct and is kept for reading:
 * basing the array at +0x0C rather than +0x00 is what makes
 * func_0012BC50's stride-8 indexing with writes at +0xC/+0x10 stop
 * overlapping the next entry, and func_0012BC78 loads and calls exactly
 * the pair func_0012BC50 installs. But "the layout explains the code"
 * and "the struct compiles to the same instructions" are different
 * claims, and only the first one is established here.
 *
 * Before reusing this for conversion, rebuild and check the sweep --
 * do not assume byte-neutrality from the layout being right.
 */
typedef struct Handler {
    /* 0x00 */ void *fn;
    /* 0x04 */ int   data;
} Handler;  /* 0x8 */

typedef struct Obj40 {
    /* 0x000 */ char    unk000[0x4];
    /* 0x004 */ int     unk004;
    /* 0x008 */ int     unk008;
    /* 0x00C */ Handler handlers[0x14];
    /* 0x0AC */ int     unk0AC;
    /* 0x0B0 */ char    unk0B0[0x68];
    /* 0x118 */ int     unk118;
    /* 0x11C */ char    unk11C[0x4];
    /* 0x120 */ int     unk120;
    /* 0x124 */ char    unk124[0x2C];
    /* 0x150 */ int     unk150;
    /* 0x154 */ char    unk154[0x20];
    /* 0x174 */ int     unk174;
    /* 0x178 */ char    unk178[0x40];
    /* 0x1B8 */ Slot1B8 slots[3];
    /* 0x1E8 */ char    unk1E8[0x638];
    /* 0x820 */ int     unk820;
} Obj40;

/*
 * The wrapper that holds an Obj40 at its +0x40. Small, but it is the
 * object most of the func_0012Bxxx/func_0012Cxxx entry points actually
 * receive -- they immediately load ->obj and work through that, which
 * is the indirection func_0012C200 makes explicit by loading it and
 * passing it straight to func_0012C278.
 */
typedef struct Wrapper {
    /* 0x00 */ char   unk00[0x8];
    /* 0x08 */ int    unk08;
    /* 0x0C */ char   unk0C[0x34];
    /* 0x40 */ Obj40 *obj;
} Wrapper;

/*
 * OPEN QUESTION -- deliberately NOT defined: the global at D_0013D390.
 *
 * Two functions touch it. func_00209290 gives 0xB0, 0xCC, 0xE4, 0xE8
 * and 0x20/0x3C/0x58/0x74/0x90; func_00209418 gives 0x1C, 0xE4, 0xE8.
 * The 0x20..0x90 run is a clean stride-0x1C array of five elements,
 * each with a sentinel of -1 in its first word.
 *
 * The blocker is that func_00209290 also indexes `(b + 0xB0) + idx *
 * 0xC0`, and a 0xC0 stride starting at 0xB0 swallows 0xCC, 0xE4 and
 * 0xE8, which the same function writes as plain fields. Those two
 * readings cannot both be siblings in one struct, and two functions is
 * not enough to say which is wrong -- 0xB0 may not be the true array
 * base, or the index may be bounded in a way neither function shows.
 * Forcing a layout here would bake in a guess, so this stays raw until
 * a third user of D_0013D390 is decompiled and settles it.
 */

/*
 * Memory card state machine enum (CardState).
 * Recovered from retail string table at 0x0015FE78 and pointer table at 0x001A04D0.
 */
typedef enum CardState {
    CS_INIT = 0,
    CS_GOOD_SAVE = 1,
    CS_WARNING = 2,
    CS_NOCARD = 3,
    CS_WAIT_FOR_CARD = 4,
    CS_UNFORMATTED = 5,
    CS_PROMPT_FORMAT = 6,
    CS_FORMAT_PENDING = 7,
    CS_FORMATTING = 8,
    CS_FORMATTED = 9,
    CS_CHECK_SAVE = 10,
    CS_CHECKING_SAVE = 11,
    CS_NOSAVE = 12,
    CS_PROMPT_CREATE_SAVE = 13,
    CS_CREATE_SAVE_PENDING = 14,
    CS_CREATING_SAVE = 15,
    CS_NEWCARD = 16,
    CS_FORMAT_FAILED = 17,
    CS_CREATE_FAILED = 18,
    CS_NO_ROOM = 19,
    CS_LOAD_FAILED = 20,
    CS_SAVE_FAILED = 21,
    CS_SAVING = 22,
    CS_PROMPT_BEGIN_UNFORMATTED = 23,
    CS_PROMPT_BEGIN_NOSAVE = 24
} CardState;

/*
 * Save Game IFF (Interchange File Format) Chunk IDs.
 * Recovered from save file streams and memcard_PrepData / memcard_RestoreData handlers.
 */
typedef enum SaveGameBlockId {
    SAVE_BLOCK_LEVEL                    = 0,     /* Current / last level ID (int) */
    SAVE_BLOCK_BOLT_COUNT               = 1,     /* Total bolt count (int) */
    SAVE_BLOCK_GAME_COMPLETES           = 2,     /* Challenge mode / playthrough counter (int) */
    SAVE_BLOCK_ELAPSED_TIME             = 3,     /* In-game elapsed time (int) */
    SAVE_BLOCK_LAST_SAVE_TIME           = 4,     /* Timestamp (sceCdCLOCK) */
    SAVE_BLOCK_GLOBAL_FLAGS             = 5,     /* Global progression bit flags */
    SAVE_BLOCK_CHEATS_ACTIVATED         = 7,     /* Active cheats bitmask */
    SAVE_BLOCK_SKILL_POINTS             = 8,     /* Skill points unlocked bitmask */
    SAVE_BLOCK_AMMO                     = 9,     /* Current ammo table */
    SAVE_BLOCK_UNLOCKS                  = 10,    /* Weapon / Gadget unlock bitmask */
    SAVE_BLOCK_PURCHASABLE_VENDOR_ITEMS = 12,    /* Vendor purchase availability */
    SAVE_BLOCK_GALACTIC_MAP             = 14,    /* Visited planet coordinates / map state */
    SAVE_BLOCK_HELP_MESSAGES            = 16,    /* Help prompt messages */
    SAVE_BLOCK_HELP_MISC                = 17,    /* Miscellaneous help data */
    SAVE_BLOCK_HELP_GADGETS             = 18,    /* Gadget help prompts */
    SAVE_BLOCK_CAMERA_UP_DOWN_MODE      = 25,    /* Invert pitch setting (int) */
    SAVE_BLOCK_CAMERA_LEFT_RIGHT_MODE   = 26,    /* Invert yaw setting (int) */
    SAVE_BLOCK_CAMERA_ROTATION_SPEED    = 27,    /* Camera sensitivity (int) */
    SAVE_BLOCK_CHEATS_EVER_ACTIVATED    = 37,    /* Permanent cheat taint flag */
    SAVE_BLOCK_TOTAL_PLAY_TIME          = 1003,  /* Lifetime playtime (int) */
    SAVE_BLOCK_TOTAL_DEATHS             = 1005,  /* Lifetime death counter (int) */
    SAVE_BLOCK_HELP_LOG                 = 1010,  /* Help log history */
    SAVE_BLOCK_HELP_LOG_POS             = 1011   /* Help log write cursor (int) */
} SaveGameBlockId;

typedef enum SaveLevelBlockId {
    SAVE_LEVEL_BLOCK_VISITED            = 3001,  /* 0=unvisited, 1=visited, 2=completed (char) */
    SAVE_LEVEL_BLOCK_GOLD_BOLTS         = 3003,  /* Gold bolts collected in level */
    SAVE_LEVEL_BLOCK_SEGMENTS_COMPLETED = 3004,  /* Level mission segments completed */
    SAVE_LEVEL_BLOCK_TOTAL_BOLTS        = 4000,  /* Total bolts collected on this level */
    SAVE_LEVEL_BLOCK_TOTAL_DEATHS       = 4002   /* Total deaths on this level (int) */
} SaveLevelBlockId;

/*
 * IOP Stash subsystem structures and enums.
 * Recovered from unstripped IOPSTASH.IRX STABS debug symbols in the June 25, 2002 prototype.
 * Original source path: C:\code\i5\stash\iopstash.c
 */
enum {
    IOP_STASH_SEND  = 0,
    IOP_STASH_FETCH = 1,
    IOP_STASH_INFO  = 2
};

typedef struct StashBlock {
    /* 0x00 */ int   ram;      /* IOP stash RAM address */
    /* 0x04 */ int   qwc;      /* quadword count (16 bytes per unit) */
    /* 0x08 */ int   comment;  /* char* debug comment / tag */
    /* 0x0C */ int   pad;
} StashBlock;

typedef struct StashFetch {
    /* 0x00 */ int   ram;
    /* 0x04 */ int   pad[3];
} StashFetch;

typedef struct StashGetInfo {
    /* 0x00 */ int   base;
    /* 0x04 */ int   size;
    /* 0x08 */ int   pad[2];
} StashGetInfo;

typedef struct StashInfo {
    /* 0x00 */ int   base;
    /* 0x04 */ int   size;
    /* 0x08 */ int   cd[10];   /* sceSifClientData (0x28 bytes) */
    /* 0x30 */ int   free;     /* next free address in stash RAM */
    /* 0x34 */ int   block;    /* allocated block count (max 0x40) */
} StashInfo;

/*
 * Geometry and Math structures from Insomniac STABS (.mdebug).
 */
typedef struct BSphere {
    float x;
    float y;
    float z;
    float rad;
} BSphere;

typedef struct vec4 {
    float x;
    float y;
    float z;
    float w;
} vec4;

/*
 * Authentic Insomniac MobyInstance structure (256 bytes / 0x100).
 * Demangled signature: InitMobyInstance(MobyInstance *, int)
 */
typedef struct MobyInstance {
    /* 0x00 */ BSphere       bSphere;              /* bounding sphere in world space */
    /* 0x10 */ vec4          pos;                  /* world position (x, y, z, w) */
    /* 0x20 */ unsigned char state;                /* current moby state (0xFE=free, 0xFF=tail) */
    /* 0x21 */ unsigned char group;                /* collision / grouping index */
    /* 0x22 */ unsigned char mClass;               /* class sub-type / moby class byte */
    /* 0x23 */ unsigned char alpha;                /* opacity / blend alpha (default 0x80) */
    /* 0x24 */ void         *pClass;               /* pointer to class definition */
    /* 0x28 */ struct MobyInstance *pChain;        /* next moby in chain / update list */
    /* 0x2C */ unsigned char collDamage;           /* collision damage value */
    /* 0x2D */ unsigned char deathCnt;             /* death counter */
    /* 0x2E */ unsigned short occlIndex;           /* occlusion index */
    /* 0x30 */ unsigned char updateDist;           /* update distance */
    /* 0x31 */ unsigned char drawn;                /* drawn flag */
    /* 0x32 */ unsigned short drawDist;            /* draw distance threshold */
    /* 0x34 */ unsigned short modeBits;            /* mode flags (0x40 = no pre-update) */
    /* 0x36 */ unsigned short modeBits2;           /* secondary mode flags */
    /* 0x38 */ unsigned long lights;               /* light bitmask / color */
    /* 0x40 */ void         *animSeq;              /* animation sequence pointer */
    /* 0x44 */ float         animSeqT;             /* animation time parameter */
    /* 0x48 */ float         animSpeed;            /* animation playback speed */
    /* 0x4C */ short         animIScale;           /* animation interpolation scale */
    /* 0x4E */ short         poseCacheEntryIndex;  /* pose cache index */
    /* 0x50 */ void         *animLayers;           /* animation layer list */
    /* 0x54 */ unsigned char animSeqId;            /* active sequence ID */
    /* 0x55 */ unsigned char animFlags;            /* animation flags */
    /* 0x56 */ unsigned char lSeq;                 /* loop / layer sequence */
    /* 0x57 */ unsigned char jointCnt;             /* joint count */
    /* 0x58 */ void         *jointCache;           /* joint matrix cache */
    /* 0x5C */ void         *pManipulator;         /* IK / joint manipulator */
    /* 0x60 */ unsigned int  glow_rgba;            /* glow color (RGBA) */
    /* 0x64 */ unsigned char lod_trans;            /* LOD transition */
    /* 0x65 */ unsigned char lod_trans2;           /* secondary LOD transition */
    /* 0x66 */ unsigned char metal;                /* metal surface flag */
    /* 0x67 */ unsigned char subState;             /* sub-state */
    /* 0x68 */ unsigned char prevState;            /* previous state */
    /* 0x69 */ unsigned char stateType;            /* state type enum */
    /* 0x6A */ unsigned short stateTimer;          /* frames in current state */
    /* 0x6C */ unsigned char soundTrigger;         /* sound trigger */
    /* 0x6D */ unsigned char soundDesired;         /* sound desired */
    /* 0x6E */ unsigned short soundChannel;        /* audio channel index */
    /* 0x70 */ float         scale;                /* actor scale */
    /* 0x74 */ unsigned short bangles;             /* bangles / attachment bits */
    /* 0x76 */ unsigned char shadow;               /* shadow flag */
    /* 0x77 */ unsigned char shadow_index;         /* shadow texture index */
    /* 0x78 */ float         shadow_plane;         /* shadow ground plane Y */
    /* 0x7C */ float         shadow_range;         /* shadow max distance */
    /* 0x80 */ BSphere       lSphere;              /* local bounding sphere */
    /* 0x90 */ void         *netObject;            /* network / multiplayer object */
    /* 0x94 */ unsigned short updateID;            /* update tick ID */
    /* 0x96 */ unsigned short spad0;               /* scratchpad scratch var */
    /* 0x98 */ void         *collData;             /* collision mesh / pill data */
    /* 0x9C */ unsigned int  collActive;           /* active collision bitmask */
    /* 0xA0 */ int           collCnt;              /* collision check count */
    /* 0xA4 */ unsigned char grid_min_x;           /* PVS grid bounds min X */
    /* 0xA5 */ unsigned char grid_min_y;           /* PVS grid bounds min Y */
    /* 0xA6 */ unsigned char grid_max_x;           /* PVS grid bounds max X */
    /* 0xA7 */ unsigned char grid_max_y;           /* PVS grid bounds max Y */
    /* 0xA8 */ void        (*pUpdate)(struct MobyInstance *); /* per-tick update function */
    /* 0xAC */ void         *pVar;                 /* private moby state (0x80 bytes per moby) */
    /* 0xB0 */ unsigned char mission;              /* mission ID */
    /* 0xB1 */ unsigned char pad;
    /* 0xB2 */ short         UID;                  /* unique instance ID in level */
    /* 0xB4 */ short         bolts;                /* bolt drop reward */
    /* 0xB6 */ unsigned short xp;                  /* experience / nanotech reward */
    /* 0xB8 */ struct MobyInstance *pParent;       /* parent moby pointer */
    /* 0xBC */ short         oClass;               /* Object Class ID */
    /* 0xBE */ unsigned char triggers;             /* trigger flags */
    /* 0xBF */ unsigned char standarddeathcalled;  /* death handler invoked flag */
    /* 0xC0 */ float         rMtx[3][4];           /* 3x4 orientation/rotation matrix */
    /* 0xF0 */ vec4          rot;                  /* Euler rotation (pitch, yaw, roll) */
} MobyInstance;

/*
 * RaC1 Gadget and Weapon Enum (reused across the franchise and save-import).
 */
typedef enum RaC1GadgetId {
    GADGET_BOMB_GLOVE       = 0,
    GADGET_PYROCITOR        = 1,
    GADGET_BLASTER          = 2,
    GADGET_GLOVE_OF_DOOM    = 3,
    GADGET_SUCK_CANNON      = 4,
    GADGET_SWINGSHOT        = 5,
    GADGET_HYDRODISPLACER   = 6,
    GADGET_SONIC_SUMMONER   = 7,
    GADGET_RYNO             = 8,
    GADGET_WALLOPER         = 9,
    GADGET_VISIBOMB         = 10,
    GADGET_DECOY_GLOVE      = 11,
    GADGET_TESLA_CLAW       = 12,
    GADGET_TAUNTER          = 13,
    GADGET_TRESPASSER       = 14,
    GADGET_METAL_DETECTOR   = 15,
    GADGET_MAGNEBOOTS       = 16,
    GADGET_GRIND_BOOTS      = 17,
    GADGET_HOVERBOARD       = 18,
    GADGET_HELI_PACK        = 19,
    GADGET_THRUSTER_PACK    = 20,
    GADGET_HYDRO_PACK       = 21,
    GADGET_O2_MASK          = 22,
    GADGET_PILOTS_HELMET    = 23,
    GADGET_MORPH_O_RAY      = 24,
    GADGET_CODEBOT          = 25,
    GADGET_HOLOGUISE        = 26,
    GADGET_PDA              = 27,
    GADGET_PERSUADER        = 28
} RaC1GadgetId;

#endif /* STRUCTS_H */


#ifndef RECORDS_H
#define RECORDS_H

/* Small records many level files read through the same layout. */

/* A function-pointer dispatch entry (0x14 bytes) in the camera and mode tables. */
typedef struct {
    char unk_00[8];
    void (*fn_08)(void *);
    char unk_0C[4];
    void (*fn_10)(void *);
} DispatchRec;

/* The slot tables of the pad-driven selector state at D_001A01F0. */
typedef struct {
    int _pad0[0x9E];
    int use[5];
    int flags[5];
    int sel;
    int size[5];
} PadSlots;

/* An animation sequence header: only the frame count is read. */
typedef struct {
    char _pad00[0x10];
    unsigned char nframes;
} AnimSeq;

#endif /* RECORDS_H */

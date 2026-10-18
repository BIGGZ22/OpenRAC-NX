/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    char _pad0[0x0C];
    unsigned char field_C;
} GDE_SoundTail;

typedef struct {
    char _pad0[0x10];
    unsigned int flags;
    char _pad1[0x18];
    GDE_SoundTail *tail;
} GDE_Sound;

int SND_IsLooping__FPC9GDE_Sound(const GDE_Sound *s) {
    if (s == 0) {
        return 0;
    }
    if (s->flags & 1) {
        return 1;
    }
    if (s->flags & 4) {
        if (s->tail->field_C) {
            return 1;
        }
    }
    return 0;
}

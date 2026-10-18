typedef struct {
    int left;
} HUD_BoundingBox16;

extern int GetWidth__17HUD_BoundingBox16(HUD_BoundingBox16 *self);

int GetCenterX__17HUD_BoundingBox16(HUD_BoundingBox16 *self) {
    return (GetWidth__17HUD_BoundingBox16(self) >> 1) + self->left;
}

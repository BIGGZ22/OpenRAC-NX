typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} tHUD_BoundingBox;

int GetWidth__17HUD_BoundingBox16(tHUD_BoundingBox *self) {
    return (self->right - self->left) + 1;
}

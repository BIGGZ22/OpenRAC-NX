struct ClipPlanes;

extern void *gVisibilityPlanes;
extern char gDefaultPlanes[];

void DISPLAY_SetVisibilityPlanes__FP10ClipPlanes(struct ClipPlanes *planes) {
    if (planes == 0) {
        gVisibilityPlanes = gDefaultPlanes;
    } else {
        gVisibilityPlanes = planes;
    }
}
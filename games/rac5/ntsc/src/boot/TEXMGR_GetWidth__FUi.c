typedef struct { char b[100]; } tGIM_SLOT;
typedef struct SceGimPicture { int a; int b; int c; int d; } SceGimPicture;
typedef struct SceGimImage { unsigned short f0, f2, f4, f6, width, height; } SceGimImage;

extern tGIM_SLOT gim_slots[];
extern SceGimImage *sceGimPictureGetImage__FP13SceGimPicture(SceGimPicture *p);

int TEXMGR_GetWidth__FUi(unsigned int id) {
    return sceGimPictureGetImage__FP13SceGimPicture((SceGimPicture *)&gim_slots[id])->width;
}

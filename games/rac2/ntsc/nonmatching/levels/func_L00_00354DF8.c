/* G8 trial: 284 bytes against 272; pointer/base allocation remains different. */
typedef struct {
    unsigned short code;
    unsigned short mode;
    void *data;
    unsigned short secondary;
    unsigned short unused;
} AranosMenuRecord;

typedef struct {
    unsigned short code;
    unsigned short unused02;
    unsigned short secondary;
    unsigned char unused06[6];
} AranosMenuLookup;

typedef struct {
    unsigned char unused00[0x30];
    unsigned int flags;
    unsigned char unused34[12];
    int selection;
} AranosMenuObject;

typedef struct {
    int ids[28];
} AranosMenuInputs;

extern AranosMenuInputs D_00139948 __attribute__((nosda));
extern AranosMenuRecord D_L00_0021CEF0[29] __attribute__((nosda));
extern AranosMenuLookup D_L00_0028B620[] __attribute__((nosda));
extern unsigned char D_L00_002870C0[] __attribute__((nosda));
extern int D_L00_001AA830[] __attribute__((sda));
extern int D_L00_001C95D0 __attribute__((nosda));

int func_L00_00354DF8(AranosMenuObject *object)
{
    int count = 0;
    if (D_00139948.ids[0] != 0) {
        int *input = D_00139948.ids;
        AranosMenuRecord *output = D_L00_0021CEF0;
        do {
            int id = input[0];
            count++;
            output->mode = 3;
            input++;
            output->data = D_L00_002870C0;
            output->code = D_L00_0028B620[id].code;
            output->secondary = D_L00_0028B620[id].secondary;
            output++;
        } while (count < 28 && input[0] != 0);
    }
    D_L00_0021CEF0[count].code = 0;
    object->selection = 0;
    object->flags |= 0x8000;
    if (D_L00_001AA830[0] != 0) {
        count = 1;
        if (D_L00_001C95D0 == D_L00_001AA830[0]) {
            object->selection = 0;
        } else {
            for (; D_L00_001AA830[count] != 0; count++) {
                if (D_L00_001C95D0 == D_L00_001AA830[count]) {
                    object->selection = count;
                    return 0;
                }
            }
        }
    }
    return 0;
}

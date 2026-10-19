typedef struct {
    int field00;
    int field04;
    int field08;
    int field0C;
    int field10;
    int field14;
    int field18;
    int field1C;
} tSemaParam;

typedef struct {
    int semaphore;
} HIGMutex;

extern int CreateSema(tSemaParam *param);

void *__8HIGMutex(HIGMutex *self) {
    tSemaParam param;
    param.field08 = 1;
    param.field04 = 1;
    param.field14 = 0;
    self->semaphore = CreateSema(&param);
    return self;
}

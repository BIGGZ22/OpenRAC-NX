extern void *V_01F209B8[];

void *__10bad_typeid(void *self) {
    *(void **)self = V_01F209B8;
    return self;
}

struct object {
    void *pc_begin;
    void *pc_end;
    void *fde_begin;
    void *fde_array;
    unsigned int count;
    struct object *next;
};

extern struct object *__frame_info_list;

void __register_frame_info_table(void *begin, struct object *ob) {
    ob->fde_array = begin;
    ob->next = __frame_info_list;
    __frame_info_list = ob;
    ob->fde_begin = begin;
    ob->pc_begin = 0;
    ob->count = 0;
    ob->pc_end = 0;
}

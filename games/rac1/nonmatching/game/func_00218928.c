/* CreatePart: a tail call into func_00218930 (the allocator) with a zero second argument; returns the
   particle it took. (The earlier worker's C, build-sn/fill/func_00218928, typed to match func_00218930's
   candidate.) */
void *func_00218930(int, int);

void *func_00218928(int arg0) {
    return func_00218930(arg0, 0);
}

short __get_eh_table_version(void *table) {
    return *(short *)((char *)table + 6);
}

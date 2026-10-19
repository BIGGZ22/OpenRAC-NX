/* libgcc2.c (GNU CC 2.9, L_eh): originating table language of an exception
   descriptor.  exception_descriptor { void *runtime_id_field;
   exception_lang_info lang; } with lang.language at +4, lang.version at +6
   (eh-common.h). */
short __get_eh_table_language(void *table) {
    return *(short *)((char *)table + 4);
}

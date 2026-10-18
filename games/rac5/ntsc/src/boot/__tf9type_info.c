/* __tf9type_info 0x01E99F40..0x01E99F80 : lazily initialised type_info for the
   type_info class itself (name string at 0x01F20A60).

   The name is referenced through a declared symbol, not through an integer
   literal cast to a pointer. The kit's original body used
   `(const char *)0x01F20A60`, which makes cc1 emit `lui $5,0x1F2 ; ori
   $5,$5,0xA60`; retail has `addiu $5,$5,0xA60` — one byte, 63/64. With the
   symbol declared here, the compiler emits the `%hi`/`%lo` pair exactly as it
   does for the other fourteen accessors, and the body links 64/64. The address
   lives in __tf9type_info.map (STR_name_01F20A60 = 0x01F20A60), the same mechanism the
   other RTTI bodies already use. No instruction is injected. */
typedef struct __type_info {
    const char *name;
    void *vtable;
} __type_info;

extern __type_info __ti9type_info; /* 0x01FEFA58 */
extern const char STR_name_01F20A60[];      /* 0x01F20A60, see sidecar .map */
extern void __rtti_user(__type_info *p, const char *name);

__type_info *__tf9type_info(void) {
    __type_info *p = &__ti9type_info;
    if (__ti9type_info.name == 0) {
        __rtti_user(p, STR_name_01F20A60);
    }
    return p;
}

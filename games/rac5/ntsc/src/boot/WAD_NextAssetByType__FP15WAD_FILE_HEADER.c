/* Walk the +0x70 sibling chain looking for the next entry whose +0x64 id
   matches the header's own id. */
struct WAD_FILE_HEADER {
    char pad00[0x64];
    int type_id;                          /* 0x64 */
    char pad68[8];
    struct WAD_FILE_HEADER *next_by_type; /* 0x70 */
};

struct WAD_FILE_HEADER *WAD_NextAssetByType__FP15WAD_FILE_HEADER(struct WAD_FILE_HEADER *header) {
    struct WAD_FILE_HEADER *node = header->next_by_type;
    if (node != 0) {
        int type_id = header->type_id;
        do {
            if (node->type_id == type_id) break;
            node = node->next_by_type;
        } while (node != 0);
    }
    return node;
}

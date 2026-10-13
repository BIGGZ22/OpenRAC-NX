/* Carried from port/game/rac1-pal/hand/game_resource_bank.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native-only PAL 00205270..00205518: select and decompress a resident
 * class bank, relocate it, restore material references and upgrade state.
 * Reconstructed from retail instructions; not a byte-matched PS2 function. */
typedef struct BankGroup {
    unsigned char* packet;
    int quadwords;
    unsigned int auxiliary[2];
} BankGroup;
typedef struct BankMaterial {
    unsigned char reserved[0x1A];
    short index;
    unsigned int texture_base;
} BankMaterial;
typedef struct BankModel {
    BankGroup* groups;
    unsigned char count0, count1, count2, last_group;
    unsigned char reserved[0x20];
    BankMaterial* materials;
    unsigned int raw_value;
} BankModel;
typedef struct BankItem {
    unsigned char reserved[0x10];
    int class_id;
    unsigned char rest[0x38];
} BankItem;
extern int bank_count __asm__("D_0015FF48");
extern int bank_current __asm__("D_0015FF4C");
extern int bank_buffer __asm__("D_0015FF50");
extern unsigned int bank_texture_base __asm__("D_0015FF44");
extern int bank_classes[] __asm__("D_001CBAC0");
extern unsigned char* bank_compressed[] __asm__("D_001CBB20");
extern unsigned char bank_setup[] __asm__("D_001940C0");
extern unsigned char bank_slots[] __asm__("D_001B3AC0");
extern BankModel* bank_models[] __asm__("D_001B3200");
extern unsigned int bank_raw_values[] __asm__("D_001B6180");
extern unsigned char bank_textures[] __asm__("D_001CAAC0");
extern unsigned char bank_material_maps[][16] __asm__("D_001CBBE0");
extern short bank_material_indices[][16] __asm__("D_001CBD60");
extern BankItem bank_items[] __asm__("D_001863D0");
extern unsigned char bank_upgraded[] __asm__("D_0013E520");
extern unsigned long long bank_upgrade_state[] __asm__("D_0019E6F0");
extern void bank_flush(int) __asm__("func_00118A80");
extern int bank_decompress(void*, void*) __asm__("func_0020B618");
extern void bank_relocate(BankModel*, void*, void*, int) __asm__("func_00203338");

void FUN_00204a40(int class_id, int buffer) {
    BankModel* model;
    int i, slot;
    unsigned int texture_base;
    if (bank_current >= 0 && bank_classes[bank_current] == class_id) return;
    bank_current = 0;
    while (bank_current < bank_count && bank_classes[bank_current] != class_id)
        ++bank_current;
    /* Retail expects the requested class in this table. Preserve its
     * terminal index if absent; do not substitute a different class. */
    if (buffer == -1) buffer = bank_buffer == 0;
    model = (BankModel*)(*(unsigned char**)(bank_setup + 0x10) + buffer * 0x18000);
    bank_buffer = buffer;
    bank_flush(0);
    bank_decompress(bank_compressed[bank_current], model);
    bank_flush(0);
    slot = bank_slots[class_id];
    bank_models[slot] = model;
    bank_raw_values[slot] = model->raw_value;
    bank_relocate(model, bank_textures, bank_material_maps[bank_current], class_id);
    texture_base = bank_texture_base;
    for (i = 0; i < 16; ++i) {
        int index = bank_material_indices[bank_current][i];
        if (index >= 0) {
            bank_models[slot]->materials[i].index = (short)index;
            bank_models[slot]->materials[i].texture_base = texture_base;
        }
    }
    for (i = 0; i < 37; ++i) {
        if (bank_items[i].class_id == class_id) {
            if (bank_upgraded[i] != 0 && bank_models[slot]->count2 != 0) {
                BankGroup* group = &bank_models[slot]->groups[bank_models[slot]->last_group];
                unsigned char* end = group->packet + (group->quadwords - 4) * 16;
                *(unsigned long long*)(end + 0x20) = bank_upgrade_state[0];
                *(unsigned long long*)(end + 0x30) = bank_upgrade_state[2];
            }
            return;
        }
    }
}

#include "include_asm.h"

extern int ref_L00_00312060_01;

extern unsigned char ref_L00_00312060_00[];

void func_002A0430(unsigned int index)
{
    if (index != 255u)
        ref_L00_00312060_00[index + (unsigned int)ref_L00_00312060_01 * 16u] |= 0x80u;
}

void func_002A0468(unsigned long long *command_words, unsigned long long upper_field, unsigned long long middle_field, unsigned long long low_field, unsigned long long tail_field);

/* rac1-decomp src/game/mobyfunc.c: func_0020E340 */
/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/rendering/commands/pack_render_command_fields.c, PackRenderCommandFields. */
void func_002A0468(unsigned long long *command_words, unsigned long long upper_field,
                             unsigned long long middle_field, unsigned long long low_field, unsigned long long tail_field) {
    int command_word_index;
    if (middle_field) {
        upper_field <<= 32;
        low_field <<= 8;
        tail_field <<= 16;
    } else {
        upper_field <<= 32;
        low_field <<= 8;
        tail_field <<= 16;
    }
    {
        register unsigned long long packed_command;
        packed_command = upper_field | middle_field;
        packed_command |= low_field;
        packed_command |= tail_field;
        command_word_index = 7;
        command_words[command_word_index] = packed_command;
    }
}

INCLUDE_ASM("asm/game", func_002A0488);

INCLUDE_ASM("asm/game", func_002A04B8);

ASM_FUNC("asm/game", func_002A04E8);

INCLUDE_ASM("asm/game", func_002A0558);

INCLUDE_ASM("asm/game", func_002A05A8);

INCLUDE_ASM("asm/game", func_002A08A0);

INCLUDE_ASM("asm/game", func_002A0A28);

ASM_FUNC("asm/game", func_002A0B88);

ASM_FUNC("asm/game", func_002A0BE8);

INCLUDE_ASM("asm/game", func_002A0C68);

INCLUDE_ASM("asm/game", func_002A0F48);

INCLUDE_ASM("asm/game", func_002A10E8);

INCLUDE_ASM("asm/game", func_002A1130);

INCLUDE_ASM("asm/game", func_002A1378);

INCLUDE_ASM("asm/game", func_002A1490);

INCLUDE_ASM("asm/game", func_002A1738);

INCLUDE_ASM("asm/game", func_002A2450);

INCLUDE_ASM("asm/game", func_002A3110);

ASM_FUNC("asm/game", func_002A3DB8);

ASM_FUNC("asm/game", func_002A3F10);

INCLUDE_ASM("asm/game", func_002A4078);

ASM_FUNC("asm/game", func_002A4AD8);

ASM_FUNC("asm/game", func_002A4D88);

INCLUDE_ASM("asm/game", func_002A4E68);

INCLUDE_ASM("asm/game", func_002A6640);

INCLUDE_ASM("asm/game", func_002A677C);

INCLUDE_ASM("asm/game", func_002A68A8);

INCLUDE_ASM("asm/game", func_002A6AE0);

INCLUDE_ASM("asm/game", func_002A6C58);


INCLUDE_ASM("asm/game", func_002A7470);

INCLUDE_ASM("asm/game", func_002A75E0);

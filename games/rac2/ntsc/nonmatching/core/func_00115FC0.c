#include "common.h"

/* libc.a newlib _mbtowc_r - multibyte to wide character, the SHIFT_JIS,
 * EUC-JP and ISO-2022-JP ("C-JIS") paths of it.
 *
 * This C is exact: tools/check.py func_00115FC0 --diff reports
 * `1 of 1 places match`, and it was written into src/core/00115FC0.c with the
 * quad-save profile before being moved here. tools/build.py nevertheless
 * refuses to place it in the boot link -
 *   `func_00115FC0: needs .rodata at its place, which is not C`
 * because the `switch (action)` below (eight dense cases over JIS_ACTION)
 * compiles to a jump table, and retail keeps that table at 0x13A3C0, in the
 * data run that follows the three locale strings. C can place a function but
 * not the table a switch needs, so this stays a near-miss, exactly like
 * func_00118CC8. The two enum tables the body reads are the same story:
 * JIS_state_table at 0x133E80 and JIS_action_table at 0x134030.
 *
 * Transcribed from the SDK's own newlib tree, which is in this repository at
 * build/sdk-compiler-build/source/ee/newlib/libc/stdlib/mbtowc_r.c (revision
 * shipped with the EE toolchain; libc.a). The range macros are that tree's
 * mbctype.h verbatim. Only the function name, the _reent stand-in and the
 * JIS_CHAR_TYPE member called NUL (a macro elsewhere) differ; the algorithm,
 * statement order and expressions are unchanged.
 *
 * Confirmed against RAC2's own bytes rather than assumed:
 *   - the locale field is at 0x34 (`lw $a0,0x34($s1)`, then strlen, then the
 *     three strcmp's), and the three compared strings live at 0x13A3A0,
 *     0x13A3A8 and 0x13A3B0: "C-SJIS\0", "C-EUCJP\0", "C-JIS\0";
 *   - `if (pwc == NULL) pwc = &dummy` is the `daddu $s3,$sp,$zero` followed by
 *     `movn $s3,$a1,$a1`;
 *   - the two enum tables are int[12][9] and sit adjacently: JIS_state_table
 *     at 0x133E80 (0x1B0 bytes) and JIS_action_table at 0x134030, which is
 *     exactly 12*9*4 apart, and the action switch dispatches through the
 *     eight-entry table at 0x13A3C0 (0x116260, 0x116274, 0x116280, ...);
 *   - the tail is `beql $s0,$zero` / `sltu $v0,$zero,*t`, so the C-locale
 *     fall-through returns 1 or 0 rather than an explicit second test.
 *
 * The unit saves its registers in TImode slots (`sq`), so it is built with the
 * -mastra-quad-saves profile, like its neighbours malloc (00115F28.c) and
 * malloc_extend_top (00116460.c). */

typedef struct {
    u8 pad000[0x34];
    const char *_current_locale; /* 0x34 */
} MbtowcReent;

typedef enum {
    ESCAPE,
    DOLLAR,
    BRACKET,
    AT,
    B,
    J,
    NULCH,
    JIS_CHAR,
    OTHER,
    JIS_C_NUM
} JIS_CHAR_TYPE;

typedef enum {
    ASCII,
    A_ESC,
    A_ESC_DL,
    JIS,
    JIS_1,
    JIS_2,
    J_ESC,
    J_ESC_BR,
    J2_ESC,
    J2_ESC_BR,
    DONE,
    INV,
    JIS_S_NUM
} JIS_STATE;

typedef enum { COPY_A, COPY_J, COPY_J2, MAKE_A, MAKE_J, NOOP, EMPTY, ERROR } JIS_ACTION;

#define ESC_CHAR 0x1b

#define _issjis1(c) ((c) >= 0x81 && (c) <= 0x9f || (c) >= 0xe0 && (c) <= 0xef)
#define _issjis2(c) ((c) >= 0x40 && (c) <= 0x7e || (c) >= 0x80 && (c) <= 0xfc)
#define _iseucjp(c) ((c) >= 0xa1 && (c) <= 0xfe)
#define _isjis(c) ((c) >= 0x21 && (c) <= 0x7e)

static JIS_STATE JIS_state_table[JIS_S_NUM][JIS_C_NUM] = {
    /*              ESCAPE   DOLLAR    BRACKET   AT       B       J        NUL      JIS_CHAR  OTHER */
    /* ASCII */ {A_ESC, DONE, DONE, DONE, DONE, DONE, DONE, DONE, DONE},
    /* A_ESC */ {DONE, A_ESC_DL, DONE, DONE, DONE, DONE, DONE, DONE, DONE},
    /* A_ESC_DL */ {DONE, DONE, DONE, JIS, JIS, DONE, DONE, DONE, DONE},
    /* JIS */ {J_ESC, JIS_1, JIS_1, JIS_1, JIS_1, JIS_1, INV, JIS_1, INV},
    /* JIS_1 */ {INV, JIS_2, JIS_2, JIS_2, JIS_2, JIS_2, INV, JIS_2, INV},
    /* JIS_2 */ {J2_ESC, DONE, DONE, DONE, DONE, DONE, INV, DONE, DONE},
    /* J_ESC */ {INV, INV, J_ESC_BR, INV, INV, INV, INV, INV, INV},
    /* J_ESC_BR */ {INV, INV, INV, INV, ASCII, ASCII, INV, INV, INV},
    /* J2_ESC */ {INV, INV, J2_ESC_BR, INV, INV, INV, INV, INV, INV},
    /* J2_ESC_BR */ {INV, INV, INV, INV, DONE, DONE, INV, INV, INV},
};

static JIS_ACTION JIS_action_table[JIS_S_NUM][JIS_C_NUM] = {
    /*              ESCAPE   DOLLAR    BRACKET   AT       B        J        NUL      JIS_CHAR  OTHER */
    /* ASCII */ {NOOP, COPY_A, COPY_A, COPY_A, COPY_A, COPY_A, EMPTY, COPY_A, COPY_A},
    /* A_ESC */ {COPY_A, NOOP, COPY_A, COPY_A, COPY_A, COPY_A, COPY_A, COPY_A, COPY_A},
    /* A_ESC_DL */ {COPY_A, COPY_A, COPY_A, MAKE_J, MAKE_J, COPY_A, COPY_A, COPY_A, COPY_A},
    /* JIS */ {NOOP, NOOP, NOOP, NOOP, NOOP, NOOP, ERROR, NOOP, ERROR},
    /* JIS_1 */ {ERROR, NOOP, NOOP, NOOP, NOOP, NOOP, ERROR, NOOP, ERROR},
    /* JIS_2 */ {NOOP, COPY_J2, COPY_J2, COPY_J2, COPY_J2, COPY_J2, ERROR, COPY_J2, COPY_J2},
    /* J_ESC */ {ERROR, ERROR, NOOP, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR},
    /* J_ESC_BR */ {ERROR, ERROR, ERROR, ERROR, NOOP, NOOP, ERROR, ERROR, ERROR},
    /* J2_ESC */ {ERROR, ERROR, NOOP, ERROR, ERROR, ERROR, ERROR, ERROR, ERROR},
    /* J2_ESC_BR */ {ERROR, ERROR, ERROR, ERROR, COPY_J, COPY_J, ERROR, ERROR, ERROR},
};

extern unsigned int strlen(const char *);

extern int strcmp(const char *, const char *);

int func_00115FC0(MbtowcReent *r, int *pwc, const char *s, unsigned int n, int *state) {
    int dummy;
    unsigned char *t = (unsigned char *)s;

    if (pwc == 0)
        pwc = &dummy;

    if (s != 0 && n == 0)
        return -1;

    if (r->_current_locale == 0 || (strlen(r->_current_locale) <= 1)) {
        /* fall-through */
    } else if (!strcmp(r->_current_locale, "C-SJIS")) {
        int char1 = *t;
        if (s == 0)
            return 0; /* not state-dependent */
        if (_issjis1(char1)) {
            int char2 = t[1];
            if (n <= 1)
                return -1;
            if (_issjis2(char2)) {
                *pwc = (((int)*t) << 8) + (int)(*(t + 1));
                return 2;
            } else
                return -1;
        }
    } else if (!strcmp(r->_current_locale, "C-EUCJP")) {
        int char1 = *t;
        if (s == 0)
            return 0; /* not state-dependent */
        if (_iseucjp(char1)) {
            int char2 = t[1];
            if (n <= 1)
                return -1;
            if (_iseucjp(char2)) {
                *pwc = (((int)*t) << 8) + (int)(*(t + 1));
                return 2;
            } else
                return -1;
        }
    } else if (!strcmp(r->_current_locale, "C-JIS")) {
        JIS_STATE curr_state;
        JIS_ACTION action;
        JIS_CHAR_TYPE ch;
        unsigned char *ptr;
        int i, curr_ch;

        if (s == 0) {
            *state = 0;
            return 1; /* state-dependent */
        }

        curr_state = (*state == 0 ? ASCII : JIS);
        ptr = t;

        for (i = 0; i < n; ++i) {
            curr_ch = t[i];
            switch (curr_ch) {
            case ESC_CHAR:
                ch = ESCAPE;
                break;
            case '$':
                ch = DOLLAR;
                break;
            case '@':
                ch = AT;
                break;
            case '(':
                ch = BRACKET;
                break;
            case 'B':
                ch = B;
                break;
            case 'J':
                ch = J;
                break;
            case '\0':
                ch = NULCH;
                break;
            default:
                if (_isjis(curr_ch))
                    ch = JIS_CHAR;
                else
                    ch = OTHER;
            }

            action = JIS_action_table[curr_state][ch];
            curr_state = JIS_state_table[curr_state][ch];

            switch (action) {
            case NOOP:
                break;
            case EMPTY:
                *state = 0;
                *pwc = 0;
                return i;
            case COPY_A:
                *state = 0;
                *pwc = (int)*ptr;
                return (i + 1);
            case COPY_J:
                *state = 0;
                *pwc = (((int)*ptr) << 8) + (int)(*(ptr + 1));
                return (i + 1);
            case COPY_J2:
                *state = 1;
                *pwc = (((int)*ptr) << 8) + (int)(*(ptr + 1));
                return (ptr - t) + 2;
            case MAKE_A:
            case MAKE_J:
                ptr = (unsigned char *)(t + i + 1);
                break;
            case ERROR:
            default:
                return -1;
            }
        }

        return -1; /* n < bytes needed */
    }

    /* otherwise this must be the "C" locale or unknown locale */
    if (s == 0)
        return 0; /* not state-dependent */

    *pwc = (int)*t;

    if (*t == '\0')
        return 0;

    return 1;
}

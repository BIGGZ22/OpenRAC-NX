# This file is used by the original compiler/assembler.
# Defines the expected assembly macros.

# A function symbol.
.macro glabel label, visibility=global
    .\visibility \label
    .type \label, @function
    \label:
        .ent \label
.endm

# The end of a function symbol.
.macro endlabel label
    .size \label, . - \label
    .end \label
.endm

# An alternative entry to a function.
# .aent isn't supported by the SN Systems ProDG GAS (2.9-ee-991111b) --
# it's only a debug-info marker, doesn't affect emitted bytes, so this
# just omits it rather than emitting the plain label twice.
.macro alabel label, visibility=global
    .\visibility \label
    .type \label, @function
    \label:
.endm

# A label referenced by an error handler table.
.macro ehlabel label, visibility=global
    .\visibility \label
    \label:
.endm


# A label referenced by a jumptable.
# visibility=global, not the splat default of local: jump tables can live
# in a different object (e.g. rodata) than the code referencing them, and
# a .local symbol can't resolve across objects at link time.
.macro jlabel label, visibility=global
    .\visibility \label
    \label:
.endm


# A data symbol.
.macro dlabel label, visibility=global
    .\visibility \label
    .type \label, @object
    \label:
.endm

# End of a data symbol.
.macro enddlabel label
    .size \label, . - \label
.endm


# Label to signal the symbol haven't been matched yet.
.macro nonmatching label, size=1
    .global \label\().NON_MATCHING
    .type \label\().NON_MATCHING, @object
    .size \label\().NON_MATCHING, \size
    \label\().NON_MATCHING:
.endm

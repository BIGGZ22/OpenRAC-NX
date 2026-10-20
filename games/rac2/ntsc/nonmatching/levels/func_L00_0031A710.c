/* Not a C row: catalogue defect. The 20-byte extent at 0x0031A710 is three bare float
   multiplies - `mul.s $f0,$f0,$f13` / `nop` / `mul.s $f0,$f0,$f13` / `nop` /
   `mul.s $f0,$f1,$f13` - with no prologue, no `jr $ra` and no store: a fragment of a
   neighbouring function (its consumer is in the function before or after the extent). */

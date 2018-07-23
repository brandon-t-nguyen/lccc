; This test checks for correctly assemebled
; ADD, AND, and NOT instructions with various
; forms of immediates
.orig x3000
        add     r0, r1, r3
        and     r1, r2, #0
        add     r3, r4, #15
        add     r5, r6, #-16
        and     r7, r0, xf
        and     r1, r2, x-f
        not     r3, r4
.end

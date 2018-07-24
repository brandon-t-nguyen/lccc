.orig x3000
main
        and     r0, r0, #0
        ld      r1, value
        ld      r2, loops
add_loop
        add     r0, r0, r1
        add     r2, r2, #-1
        brp     add_loop
        halt
        
loops   .fill   #10
value   .fill   #5
.end

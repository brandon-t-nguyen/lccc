.orig x4567
l1
    br      #255
l2
    brnzp   #-256
l3
    brn     l9
l4
    brz     l8
l5
    brp     l7
l6
    brnz    l5
l7
    brzp    l3
l8
    brnp    l2
l9
    brnzp   l1
    nop
.end

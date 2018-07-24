; This tests the durability of symbol resolution
.orig x3000
main
        .blkw #10
_label0
        .blkw #23
_label1 
        .fill main
        jsr _label1
        .blkw #100
_label2
        brnzp main
        jsr  _label0
        ld r0, _label0
.end

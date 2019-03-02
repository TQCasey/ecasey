    .global  _start
    .extern main
    .code 32
    .text
_start:
    bl      main
    b       .

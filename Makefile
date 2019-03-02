# All Phony Target.PHONY :all clean klib lib fsmain tty dbg 
.PHONY :all clean kernel boot klib lib fsmain term dbg g cpy lk fuck s

# Default starting position
all:  lib kernel boot fsmain shell netz t


lib:
	cd lib		; make all 
boot:
	cd boot 	; make all 
kernel:
	cd kernel 	; make all
fsmain:
	cd fs		; make all
shell:	
	cd sh		; make all
netz:
	cd net		; make all
clean :
	cd lib		; make clean
	cd boot		; make clean
	cd kernel 	; make clean
	cd fs		; make clean
	cd sh		; make clean
	cd net		; make clean
l:
	sudo dnw -a 0x00 boot/loader.bin

k:
	sudo dnw -a 0x20000 kernel/kernel.bin
f:
	sudo dnw -a 0x40000 fs/fs.bin
h:
	sudo dnw -a 0x60000 sh/sh.bin
m:
	sudo dnw -a 0x500000 tools/ram.img

t:
	find . -name "*.h" -o -name "*.S" -o -name "*.c" -o -name "*.cc" > cscope.files
	cscope -bkq -i cscope.files
	ctags -R --fields=+lS

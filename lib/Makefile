# Programs, flags, etc.
AR			= arm-linux-ar
ARF			= rc

CC	    	= arm-linux-gcc
CCF			= -I ../include  -nostdlib -Wall -Wstrict-prototypes -O0 \
			  -fomit-frame-pointer -ffreestanding -nostartfiles -c

# klib.a lib.a
KLIB		= klib.a 
LIB			= lib.a	

# kernel objects 
KOBJS		= string.o assert.o klib/uart.o klib/mmu.o klib/panic.o
OBJS		= string.o assert.o mail.o libc.o 

AOBJS		=
COBJS		= string.o assert.o mail.o libc.o klib/uart.o klib/mmu.o klib/panic.o

# All Phony Targets
.PHONY :all clean 

# Default starting position
all:	$(AOBJS) $(COBJS) $(KLIB) $(LIB)

clean:
	rm -f *.o $(KLIB) $(LIB) $(COBJS) $(AOBJS)

$(KLIB):	$(KOBJS)
	$(AR)	$(ARF)		$@ $^
$(LIB):		$(OBJS)
	$(AR)	$(ARF)		$@ $^

$(AOBJS): %.o: %.S
	$(CC)	$(CCF)	 	 	-o $@ $<
$(COBJS): %.o: %.c
	$(CC)	$(CCF)	 	 	-o $@ $<

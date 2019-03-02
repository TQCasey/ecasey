# Programs, flags, etc.
NAME		= kernel
LDS			= $(NAME).lds

CC	    	= arm-linux-gcc
CCF			= -I ../include  -nostdlib -Wall -Wstrict-prototypes -O0 \
			-fomit-frame-pointer -ffreestanding -nostartfiles -c \
			-Winline -finline-small-functions -finline

LD			= arm-linux-ld
LDF			= -T$(LDS) --gc-sections
LDPATH		= -lgcc -L /opt/EmbedSky/4.3.3/lib/gcc/arm-none-linux-gnueabi/4.3.3/armv4t

OCPY		= arm-linux-objcopy
OCPYF		= -O binary -S

DUMP		= arm-linux-objdump
BDUMPF		= -D -b binary -m arm 
EDUMPF		= -S -D -t 


BDP			= $(NAME).bin.dump
EDP			= $(NAME).elf.dump

BIN			= $(NAME).bin
ELF			= $(NAME).elf

AOBJS		= start.o page/dabt.o
COBJS		= kmain.o clock.o except.o exit.o kmalloc.o sched.o signal.o swi.o task.o timer.o vector.o ramdisk.o\
		  	ipc/kmail.o\
		  	drv/rtc.o drv/keys.o drv/led.o drv/touchpl.o drv/adc.o\
		  	page/abt.o page/fault.o\
		  	drv/lcd/fontlib.o drv/lcd/lcd.o\
		  	drv/blk/blk_drv.o\
		  	drv/usb/usbm.o\
		  	drv/dma/dma.o\
		  	drv/chr/tty.o
KLIB		= ../lib/klib.a
OBJS		= $(AOBJS) $(COBJS) $(KLIB)

# All Phony Targets
.PHONY :all clean 

# Default starting position
all:	$(OBJS) $(ELF) $(BIN) $(BDP) $(EDP)

$(BIN):		$(ELF)
	$(OCPY)	$(OCPYF)	   $< $@

$(ELF):		$(OBJS)
	$(LD)	$(LDF)		-o $@ $^ ${LDPATH}

$(BDP):	$(BIN)
	$(DUMP)	$(BDUMPF)	$< > $@
$(EDP):	$(ELF)
	$(DUMP)	$(EDUMPF)	$< > $@

clean:
	rm -f $(OBJS) $(BIN) $(BDP) $(EDP) $(ELF) 

$(AOBJS): %.o: %.S
	$(CC)	$(CCF)	 	 	-o $@ $<
$(COBJS): %.o: %.c
	$(CC)	$(CCF)	 	 	-o $@ $<

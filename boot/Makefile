# Programs, flags, etc.

LDS			= loader.lds

CC	    	= arm-linux-gcc
CCF			= -I ../include  -nostdlib -Wall -Wstrict-prototypes -O0 \
			  -fomit-frame-pointer -ffreestanding -nostartfiles -c

LD			= arm-linux-ld
LDF			= -T$(LDS)
LDPATH		= -lgcc -L /opt/EmbedSky/4.3.3/lib/gcc/arm-none-linux-gnueabi/4.3.3/armv4t

OCPY		= arm-linux-objcopy
OCPYF		= -O binary -S

DUMP		= arm-linux-objdump
BDUMPF		= -D -b binary -m arm 
EDUMPF		= -S -D -t 


BDP			= loader.dump
EDP			= loader.elf.dump
BIN			= loader.bin
ELF			= loader.elf
OBJS		= start.o loader.o nand.o led.o loaderk.o high_vect.o

# All Phony Targets
.PHONY :all clean 

# Default starting position
all:	$(OBJS) $(ELF) $(BIN) $(BDP) $(EDP)

$(BIN):		$(ELF)
	$(OCPY)	$(OCPYF)	   $< $@

$(ELF):		$(OBJS)
	$(LD)	$(LDF)		-o $@ $^ ${LDPATH}

clean:
	rm -f *.o *.elf *.bin *.dump *.map

# kernel objects making....
start.o:	start.S
	$(CC)	$(CCF) 		-o $@ $<
loader.o: 	loader.c
	$(CC)	$(CCF) 		-o $@ $<
nand.o:		nand.c
	$(CC)	$(CCF) 		-o $@ $<
led.o:		led.c
	$(CC)	$(CCF) 		-o $@ $<
loaderk.o:	loaderk.c
	$(CC)	$(CCF) 		-o $@ $<
high_vect.o:high_vect.S
	$(CC)	$(CCF) 		-o $@ $<


$(BDP):	$(BIN)
	$(DUMP)	$(BDUMPF)	$< > $@
$(EDP):	$(ELF)
	$(DUMP)	$(EDUMPF)	$< > $@


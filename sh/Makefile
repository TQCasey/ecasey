# Programs, flags, etc.
NAME		= sh
LDS			= $(NAME).lds

CC	    	= arm-linux-gcc
CCF			= -I ../include  -nostdlib -Wall -Wstrict-prototypes -O0 \
			  -fomit-frame-pointer -ffreestanding -nostartfiles -c -g 

LD			= arm-linux-ld
LDF			= -T$(LDS) --gc-sections
LDPATH		= -lgcc -L /opt/EmbedSky/4.3.3/lib/gcc/arm-none-linux-gnueabi/4.3.3/armv4t

OCPY		= arm-linux-objcopy
OCPYF		= -O binary -S

DUMP		= arm-linux-objdump
BDUMPF		= -D -b binary -m arm 
EDUMPF		= -S -D -t 

BDP			= $(NAME).dump
EDP			= $(NAME).elf.dump

BIN			= $(NAME).bin
ELF			= $(NAME).elf

AOBJS		= start.o
COBJS		= main.o 
LIB			= ../lib/lib.a
OBJS		= $(AOBJS) $(COBJS) $(LIB)

# All Phony Targets
.PHONY :all clean 

# Default starting position
all:	$(OBJS) $(ELF) $(BIN) $(BDP) $(EDP)

$(BIN):		$(ELF)
	$(OCPY)	$(OCPYF)	$< $@

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

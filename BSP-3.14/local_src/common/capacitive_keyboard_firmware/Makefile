APP=capacitive_keyboard_firmware
OBJS=$(APP).o i2c_slave.o qm_asm_tiny_mega.o

MCU=attiny88
TOOLS=/opt/OSELAS.Toolchain-1.99.3/avr/gcc-4.3.3-libc-1.6.6-binutils-2.19/bin
CC=$(TOOLS)/avr-gcc
GDB=$(TOOLS)/avr-gdb

INCLUDES = -I./ -I./qtouch_lib/include
CFLAGS= $(INCLUDES) -Os -gstabs -mmcu=$(MCU) \
       -Wall -std=gnu99 \
	   -D_QMATRIX_ -DQT_DELAY_CYCLES=4 \
	   -DPORT_X_1=B -DPORT_YA=D -DPORT_YB=C \
	   -DPORT_SMP=D -DSMP_PIN=7 -DNUM_X_PORTS=1 \
	   -DPORT_NUM_1=1 -DQT_NUM_CHANNELS=16 \
	   -DNUM_X_LINES=8 -DNUM_Y_LINES=2 \
	   -DQT_MAX_NUM_ROTORS_SLIDERS=2 \
	   -funsigned-char -funsigned-bitfields \
	   -fpack-struct -fshort-enums \
	   -D_DEBUG_INTERFACE_
LDFLAGS= -Wl,--defsym=__stack=0x300 -mmcu=$(MCU) -L./qtouch_lib/lib -lavr25g1s1_16qm_8x_2y_k_0rs
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gstabs

OBJCOPY=$(TOOLS)/avr-objcopy

AVRDUDE=/opt/avrdude/bin/avrdude
AVRDUDE_PROGRAMMER=jtag2isp
#AVRDUDE_PROGRAMMER=avrispmkii
AVRDUDE_PORT=usb

AVARICE=/opt/avarice/bin/avarice
AVARICE_DEVICE=

all: $(APP).hex

debug: $(APP).elf
	# Reset the DWEN fuse bit to enable debugWire
	sudo $(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -P $(AVRDUDE_PORT) -U hfuse:w:0x9F:m -B 100
	sleep 2
	# Start avarice
	sudo $(AVARICE) --program --file $< --part=$(MCU) --mkII --debugwire --jtag usb :4242 -B 100
	$(GDB) $<
	
erase:
	# Reset the main oscillator devide to default devided by 8 (1MHz)
	sudo $(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -P $(AVRDUDE_PORT) -U lfuse:w:0x6E:m -B 100
	sleep 2s
	sudo $(AVRDUDE) -p $(MCU) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) -e -B 100

program: $(APP).hex
	# Dont devide the internal main oscillator by 8 (8 MHz)
	sudo $(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -P $(AVRDUDE_PORT) -U lfuse:w:0xEE:m -B 100
	sleep 2s
	sudo $(AVRDUDE) -p $(MCU) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) -U flash:w:$< -B 100
 
qm_asm_tiny_mega.o: qm_asm_tiny_mega.S
	$(CC) $(ASMFLAGS) -c  $<

$(APP).hex: $(APP).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

$(APP).elf: $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(APP).hex $(APP).elf *.o *~ *.bak

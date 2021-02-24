PROG	= stk500v1
MCU 	= attiny13
PORT	= /dev/cu.usbmodem145401
FCPU    = 9600000

INCLUDE	= -I/Applications/Arduino.app/Contents/Java/hardware/arduino/cores/arduino -I/Users/scr34m/Documents/Arduino/hardware/attiny/variants/tiny8

CONF 	= /Applications/Arduino.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf
CC		= /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-g++
OBJCOPY = /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avr-objcopy
DUDE  	= /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avrdude

CFLAGS = -g -Wall -Os -mmcu=$(MCU) -Werror -DF_CPU=$(FCPU) $(INCLUDE)
OFLAGS = -j .text -j .data -O ihex

%.o: %.S
	@echo "ASM $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

main.elf: main.o serial.o
	@echo "ELF $<"
	@$(CC) $(CFLAGS) -o $@ $^

%.hex: %.elf
	@echo "OBJC $<"
	@$(OBJCOPY) $(OFLAGS) $< $@

%.upload: %.hex
	@echo "AVRDUDE $<"
	$(DUDE) -p $(MCU) -c $(PROG) -P $(PORT) -C $(CONF) -b 19200 -U lfuse:w:0x72:m -U hfuse:w:0xFF:m -U flash:w:$^

clean:
	rm -f main *.hex *.elf *.o


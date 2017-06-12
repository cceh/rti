#
# Initial configuration for an Arduino nano with ATmega328p processor.
#
# See: https://github.com/sudar/Arduino-Makefile/blob/master/arduino-mk-vars.md
#

ARDUINO_DIR       = /usr/share/arduino

BOARD_TAG         = nano328
BOARD_SUB         = atmega328p

MONITOR_PORT      = /dev/ttyUSB0
MONITOR_BAUDRATE  = 57600

AVRDUDE           = /usr/bin/avrdude
AVRDUDE_CONF      = /etc/avrdude.conf

CFLAGS_STD        = -std=gnu11
CXXFLAGS_STD      = -std=gnu++11
CXXFLAGS          = -pedantic -Wall -Wextra

OBJDIR            = bin/$(BOARD_TAG)

USER_LIB_PATH     = lib
ARDUINO_LIBS      =

# To program the chip using ISP and an AVR Dragon:
#
#    make ispload

ISP_PROG = dragon_isp
ISP_PORT = usb

# Get the fuse settings from boards.txt:
#   nano328.bootloader.extended_fuses=0xfd
#   nano328.bootloader.high_fuses=0xDA
#   nano328.bootloader.low_fuses=0xFF
# and get rid of the bootloader:

# changed: BOD = 4.3V = 110
ISP_EXT_FUSE  = 0xfa
# changed: BOOTSZ = min = 11, BOOTRST = off = 1
ISP_HIGH_FUSE = 0xdf
ISP_LOW_FUSE  = 0xff

LOCAL_CPP_SRCS  = rti.cpp

include $(ARDUINO_DIR)/Arduino.mk


myispload:
	/usr/bin/avrdude -q -v -p m328p -C /etc/avrdude.conf -c dragon_isp -b 57600 -P usb -e \
	-U lock:w:0xff:m -U efuse:w:0xfa:m -U hfuse:w:0xdf:m -U lfuse:w:0xff:m

	sleep 1

	# to get the -D flag, and, while we are at it, do verify
	/usr/bin/avrdude -q -v -p m328p -C /etc/avrdude.conf -c dragon_isp -b 57600 -P usb -D \
	-U flash:w:bin/nano328/rti.hex:i

	sleep 1

	/usr/bin/avrdude -q -v -p m328p -C /etc/avrdude.conf -c dragon_isp -b 57600 -P usb \
	-U lock:w:0xcf:m

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

include $(ARDUINO_DIR)/Arduino.mk

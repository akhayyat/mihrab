######################################################################
#                     Mihrab: Muslim Prayer Times
# Makefile
#
# Copyright (C) 2011  Ahmad Khayyat
#
# This file is part of Mihrab.
#
# Mihrab is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Foobar is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar. If not, see <http://www.gnu.org/licenses/>.
######################################################################

CC           = gcc
PROG         = mihrab
CFLAGS       = `pkg-config --cflags gtk+-3.0`
LFLAGS       = `pkg-config --libs gtk+-3.0` -litl -lgweather-3
DEBUG_FLAGS  = -DDEBUG -g
GWEATHER_DEF = -DGWEATHER_I_KNOW_THIS_IS_UNSTABLE
OBJ          = $(addprefix bin/$(CONFIG)/,mihrab.o gui.o strings.o)
BINDIR       = bin/$(CONFIG)
LANGS        = ar

ifeq ($(CONFIG), debug)
	CFLAGS += $(DEBUG_FLAGS)
else
	CONFIG = release
endif

all: bin/$(CONFIG)/$(PROG) $(addsuffix /LC_MESSAGES/$(PROG).mo,$(LANGS))

bin/$(CONFIG):
	mkdir -p bin/$(CONFIG)

bin/$(CONFIG)/$(PROG): $(OBJ)
	$(CC) $(OBJ) $(LFLAGS) -o $@

$(OBJ): src/common.h | bin/$(CONFIG)

bin/$(CONFIG)/mihrab.o: src/mihrab.c src/gui.h src/strings.h
	$(CC) -c src/mihrab.c $(CFLAGS) -o $@

bin/$(CONFIG)/gui.o: src/gui.c src/mihrab.h src/strings.h
	$(CC) -c src/gui.c $(CFLAGS) $(GWEATHER_DEF) -o $@

bin/$(CONFIG)/strings.o: src/strings.c
	$(CC) -c src/strings.c $(CFLAGS) -o $@

clean:
	rm -fr bin/

clean-$(CONFIG):
	rm -fr bin/$(CONFIG)/

######################################################################
# Translation Targets
######################################################################

pot: $(PROG).pot

$(PROG).pot: src/mihrab.c src/gui.c src/strings.c
	xgettext -d mihrab -o mihrab.pot --keyword=_ src/mihrab.c src/gui.c src/strings.c

%/LC_MESSAGES/$(PROG).mo: %/$(PROG).po
	mkdir -p $(@D)
	msgfmt -o $@ $^

.PHONY: pot

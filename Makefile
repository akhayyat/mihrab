CC           = gcc
PROG         = mihrab
CFLAGS       = `pkg-config --cflags gtk+-2.0`
LFLAGS       = `pkg-config --libs gtk+-2.0` -litl -lgweather
DEBUG_FLAGS  = -DDEBUG -g
GWEATHER_DEF = -DGWEATHER_I_KNOW_THIS_IS_UNSTABLE
OBJ          = $(addprefix bin/$(CONFIG)/,mihrab.o gui.o strings.o)
BINDIR       = bin/$(CONFIG)

ifeq ($(CONFIG), debug)
	CFLAGS += $(DEBUG_FLAGS)
else
	CONFIG = release
endif

all: bin/$(CONFIG)/$(PROG)

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

pot: src/mihrab.c src/gui.c src/strings.c
	xgettext -d mihrab -o mihrab.pot --keyword=_ src/mihrab.c src/gui.c src/strings.c

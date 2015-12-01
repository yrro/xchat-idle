PKGS := x11 xext xscrnsaver
PKGINCS := $(shell pkg-config --cflags $(PKGS))
PKGLIBS := $(shell pkg-config --libs $(PKGS))
CC := gcc
CPPFLAGS := $(PKGINCS)
CFLAGS := -g -O2 -std=c99 -Wall -Wextra -pedantic -fPIC
LDFLAGS := -Wl,--export-dynamic
LDLIBS := $(PKGLIBS)

idle.so: idle.o
	gcc $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f idle.so idle.o

install: idle.so
	install idle.so /usr/lib/xchat/plugins/

install-user: idle.so
	install idle.so ~/.xchat2/

.PHONY: clean install install-user

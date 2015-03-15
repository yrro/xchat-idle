CC := gcc
CFLAGS := -g -O2 -std=c99 -Wall -Wextra -pedantic -fPIC
LDFLAGS := -Wl,--export-dynamic
LDLIBS := -lX11 -lXext -lXss

idle.so: idle.o
	gcc $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f idle.so idle.o

.PHONY: clean

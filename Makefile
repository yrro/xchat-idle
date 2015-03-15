override CFLAGS := -std=c99 -Wall -Wextra -pedantic
override LDFLAGS := -lX11 -lXext -lXss

idle.so: idle.o
	gcc -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f idle.so idle.o

.PHONY: clean

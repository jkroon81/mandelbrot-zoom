cc      := gcc
ccflags := $(shell pkgconf sdl2 --cflags) -O2 -Wall -Werror
ldflags := $(shell pkgconf sdl2 --libs) -lm

src := $(sort $(wildcard *.c))

all : mandelbrot-zoom

%.o : %.c
	$(cc) $(ccflags) -c -o $@ $<

mandelbrot-zoom : $(src:%.c=%.o)
	$(cc) $^ $(ldflags) -o $@

clean :
	rm -f *~ $(src:%.c=%.o) mandelbrot-zoom

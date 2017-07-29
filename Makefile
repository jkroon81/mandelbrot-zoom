cc      := gcc
ccflags := $(shell sdl-config --cflags) -O2 -Wall -Werror
ldflags := $(shell sdl-config --libs) -lm

src := $(sort $(wildcard *.c))

all : mandelbrot-zoom

%.o : %.c
	$(cc) $(ccflags) -c -o $@ $<

mandelbrot-zoom : $(src:%.c=%.o)
	$(cc) $^ $(ldflags) -o $@

clean :
	rm -f *~ $(src:%.c=%.o) mandelbrot-zoom

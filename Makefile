cc      := gcc
ccflags := $(shell pkgconf sdl2 --cflags) -O2 -Wall -Werror -fopenmp
ldflags := $(shell pkgconf sdl2 --libs) -lm

all : mandelbrot-zoom

mandelbrot-zoom : mandelbrot-zoom.c
	$(cc) $(ccflags) $(ldflags) -o $@ $<

clean :
	rm -f mandelbrot-zoom

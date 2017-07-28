# Makefile for Mandelbrot-Zoom , Copyright Jacob Kroon 2003
NAME		= mandelbrot-zoom
VERSION		= 0.2.0
CC		= gcc
CFLAGS		+= -D_VERSION=\"$(VERSION)\" $(shell sdl-config --cflags) \
		  -march=athlon -O2 -g -Wall -Werror -D_DEBUG
LDFLAGS		= $(shell sdl-config --libs) -lm

#############################################################

CSOURCES	= $(wildcard *.c)
INCLUDES	= $(wildcard *.h)
OBJS		= $(CSOURCES:.c=.o)

all	: $(NAME)

%.d	: %.c
	set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@

include $(CSOURCES:.c=.d)

%.o	: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(NAME)

clean :
	rm -f *~ $(OBJS) $(NAME)

indent :
	indent -br -brs -sob -ce -c50 -npsl -npcs $(CSOURCES) $(INCLUDES)
	rm -f *~

dist :
	$(MAKE) clean
	mkdir $(NAME)-$(VERSION)
	cp $(CSOURCES) $(INCLUDES) COPYING Makefile $(NAME)-$(VERSION)
	tar -cf $(NAME)-$(VERSION).tar $(NAME)-$(VERSION)
	gzip -9 $(NAME)-$(VERSION).tar
	rm -Rf $(NAME)-$(VERSION)

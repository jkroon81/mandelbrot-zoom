/* Copyright (C) 2003 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <stdarg.h>
#include <SDL/SDL.h>

/* defines */
#define TITLE "Mandelbrot-Zoom v" _VERSION

/* typedefs */
typedef float FLOAT;

/* externals */
extern void mandelbrot_render(SDL_Surface *, FLOAT, FLOAT, FLOAT, FLOAT);
extern SDL_Surface *zoom(SDL_Surface *, float, int, int);

/* statics */
static SDL_Surface *_screen = NULL;

/* functions */
void error(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vprintf(format, ap);
  exit(-1);
}

int main(void)
{
  int loop = 1, i, px = 0, py = 0;
  double xmin = -1.0, xmax = 1.0, ymin = -1.0, ymax = 1.0, zoom_factor =
    1.0, p2x, p2y;
  char title[256];
  SDL_Event event;
  SDL_Surface *frame, *frame2;
  SDL_Color palette[256];

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    error("SDL : %s\n", SDL_GetError());
  if ((_screen = SDL_SetVideoMode(600, 600, 8, SDL_SWSURFACE)) == NULL)
    error("SDL : %s\n", SDL_GetError());
  SDL_WM_SetCaption(TITLE, TITLE);
  frame =
    SDL_CreateRGBSurface(SDL_SWSURFACE, _screen->w, _screen->h, 8, 0, 0, 0,
			 0);
  for (i = 0; i < 256; i++) {
    palette[i].r = i;
    palette[i].g = i;
    palette[i].b = i;
  }
  SDL_SetPalette(_screen, SDL_LOGPAL, palette, 0, 256);
  SDL_SetPalette(frame, SDL_LOGPAL, palette, 0, 256);
  mandelbrot_render(frame, xmin, xmax, ymin, ymax);
  SDL_BlitSurface(frame, NULL, _screen, NULL);
  SDL_Flip(_screen);
  while (loop) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
	px = event.motion.x;
	py = event.motion.y;
	switch (event.button.button) {
	case SDL_BUTTON_LEFT:
	  zoom_factor = 0.999;
	  break;
	case SDL_BUTTON_RIGHT:
	  zoom_factor = 1.001;
	  break;
	}
	break;
      case SDL_MOUSEBUTTONUP:
	p2x = (xmax - xmin) / _screen->w * px + xmin;
	p2y = (ymax - ymin) / _screen->h * py + ymin;
	xmin = p2x - zoom_factor * (p2x - xmin);
	xmax = p2x - zoom_factor * (p2x - xmax);
	ymin = p2y - zoom_factor * (p2y - ymin);
	ymax = p2y - zoom_factor * (p2y - ymax);
	mandelbrot_render(frame, xmin, xmax, ymin, ymax);
	SDL_BlitSurface(frame, NULL, _screen, NULL);
	SDL_Flip(_screen);
	zoom_factor = 1.0;
	break;
      case SDL_QUIT:
	loop = 0;
	break;
      }
    }
    if (zoom_factor != 1.0) {
      sprintf(title, TITLE " - Zoom : %.2f",
	      (2.0 / zoom_factor / (xmax - xmin)));
      SDL_WM_SetCaption(title, title);
      if (zoom_factor < 1.0)
	zoom_factor *= 0.97;
      else
	zoom_factor *= 1.03;
      frame2 = zoom(frame, zoom_factor, px, py);
      SDL_BlitSurface(frame2, NULL, _screen, NULL);
      SDL_FreeSurface(frame2);
      SDL_Flip(_screen);
    }
    SDL_Delay(10);
  }
  SDL_Quit();
  return 0;
}

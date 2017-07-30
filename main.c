/* Copyright (C) 2003 Jacob Kroon, see COPYING for details */

#include <stdlib.h>
#include <stdarg.h>
#include <SDL.h>

/* defines */
#define TITLE "Mandelbrot-Zoom"

/* typedefs */
typedef float FLOAT;

/* externals */
extern void mandelbrot_render(SDL_Surface *, FLOAT, FLOAT, FLOAT, FLOAT);
extern SDL_Surface *zoom(SDL_Surface *, float, int, int);

/* statics */
static SDL_Window *_screen = NULL;

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
  int loop = 1, px = 0, py = 0;
  double xmin = -1.0, xmax = 1.0, ymin = -1.0, ymax = 1.0, zoom_factor =
    1.0, p2x, p2y;
  char title[256];
  SDL_Event event;
  SDL_Renderer *rdr;
  SDL_Texture *txt;
  SDL_Surface *frame;
  int width, height;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    error("SDL : %s\n", SDL_GetError());
  if ((_screen = SDL_CreateWindow(TITLE,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  600,
                                  600,
                                  0)) == NULL)
    error("SDL : %s\n", SDL_GetError());
  SDL_GetWindowSize(_screen, &width, &height);
  SDL_SetWindowTitle(_screen, TITLE);
  rdr = SDL_CreateRenderer(_screen, -1, 0);
  if (rdr == NULL )
    error("SDL: %s\n", SDL_GetError());
  txt = SDL_CreateTexture(rdr,
                          SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_STREAMING,
                          width,
                          height);
  if (txt == NULL)
    error("SDL: %s\n", SDL_GetError());
  frame = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
                               0xff000000,
                               0x00ff0000,
                               0x0000ff00,
                               0x000000ff);
  mandelbrot_render(frame, xmin, xmax, ymin, ymax);
  SDL_UpdateTexture(txt, NULL, frame->pixels, frame->pitch);
  SDL_RenderClear(rdr);
  SDL_RenderCopy(rdr, txt, NULL, NULL);
  SDL_RenderPresent(rdr);
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
	p2x = (xmax - xmin) / width * px + xmin;
	p2y = (ymax - ymin) / height * py + ymin;
	xmin = p2x - zoom_factor * (p2x - xmin);
	xmax = p2x - zoom_factor * (p2x - xmax);
	ymin = p2y - zoom_factor * (p2y - ymin);
	ymax = p2y - zoom_factor * (p2y - ymax);
	mandelbrot_render(frame, xmin, xmax, ymin, ymax);
	SDL_UpdateTexture(txt, NULL, frame->pixels, frame->pitch);
	SDL_RenderClear(rdr);
	SDL_RenderCopy(rdr, txt, NULL, NULL);
	SDL_RenderPresent(rdr);
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
      SDL_SetWindowTitle(_screen, title);
      SDL_Rect src, dst;
      if (zoom_factor < 1.0) {
	zoom_factor *= 0.97;
        src.x = px - px * zoom_factor;
        src.y = py - py * zoom_factor;
        src.w = width * zoom_factor;
        src.h = height * zoom_factor;
        dst.x = 0;
        dst.y = 0;
        dst.w = width;
        dst.h = height;
      } else {
	zoom_factor *= 1.03;
        src.x = 0;
        src.y = 0;
        src.w = width;
        src.h = height;
        dst.x = px - px / zoom_factor;
        dst.y = py - py / zoom_factor;
        dst.w = width / zoom_factor;
        dst.h = height / zoom_factor;
      }
      SDL_RenderClear(rdr);
      SDL_RenderCopy(rdr, txt, &src, &dst);
      SDL_RenderPresent(rdr);
    }
    SDL_Delay(10);
  }
  SDL_Quit();
  return 0;
}

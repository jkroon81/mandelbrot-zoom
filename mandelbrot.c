/* Copyright (C) 2003 Jacob Kroon, see COPYING for details */

#include <SDL/SDL_video.h>

/* defines */
#define SQR(A) ((A)*(A))

/* structs */
typedef float FLOAT;

typedef struct {
  FLOAT a, b;
} Complex;

/* statics */
static SDL_Surface *_frame = NULL;
static FLOAT _xmin, _xmax, _ymin, _ymax;

/* functions */
static Uint8 eval_point(int x, int y)
{
  FLOAT pas = 0, pbs = 0;
  Complex p = { 0, 0 }, t = {
  0, 0}, c;
  Uint8 i = 0;

  c.a = (FLOAT) x / _frame->w * (_xmax - _xmin) + _xmin;
  c.b = (FLOAT) y / _frame->h * (_ymax - _ymin) + _ymin;
  do {
    t = p;
    p.a = pas - pbs + c.a;
    p.b = 2 * t.a * t.b + c.b;
    pas = SQR(p.a);
    pbs = SQR(p.b);
    if ((pas + pbs) > 4.0)
      break;
  } while (++i < 255);
  return i;
}

void mandelbrot_render(SDL_Surface * frame, FLOAT xmin, FLOAT xmax,
		       FLOAT ymin, FLOAT ymax)
{
  int x, y;

  _frame = frame;
  _xmin = xmin;
  _xmax = xmax;
  _ymin = ymin;
  _ymax = ymax;
  if (SDL_MUSTLOCK(frame))
    SDL_LockSurface(frame);
  for (x = 0; x < frame->w; x++)
    for (y = 0; y < frame->h; y++) {
      Uint8 val = eval_point(x, y);
      Uint32 *p = (Uint32*)((Uint8*)frame->pixels + frame->pitch * y + x * 4);
      *p = val | val << 8 | val << 16;
    }
  if (SDL_MUSTLOCK(frame))
    SDL_UnlockSurface(frame);
}

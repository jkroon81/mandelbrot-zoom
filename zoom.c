/* Copyright (C) 2003 Jacob Kroon, see COPYING for details */

#include <SDL_video.h>

/* functions */
SDL_Surface *zoom(SDL_Surface * orig, float factor, int cx, int cy)
{
  int x, y, sx, ex, sy, ey, xp, yp;
  SDL_Surface *surface;

  surface =
    SDL_CreateRGBSurface(SDL_SWSURFACE, orig->w, orig->h, 8, 0, 0, 0, 0);
  SDL_SetPalette(surface, SDL_LOGPAL, orig->format->palette->colors, 0, 256);
  sx = cx - cx * factor;
  ex = cx + (orig->w - cx) * factor;
  sy = cy - cy * factor;
  ey = cy + (orig->h - cy) * factor;
  if (SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);
  if (SDL_MUSTLOCK(orig))
    SDL_LockSurface(orig);
  for (y = 0; y < surface->h; y++) {
    yp = (int) ((double) y / surface->h * orig->h * factor + sy);
    for (x = 0; x < surface->w; x++) {
      Uint8 color;
      xp = (int) ((double) x / surface->w * orig->w * factor + sx);
      if (xp > 0 && xp < surface->w && yp > 0 && yp < surface->h)
	color = ((Uint8 *) orig->pixels)[xp + orig->pitch * yp];
      else
	color = 0;
      ((Uint8 *) surface->pixels)[x + y * surface->pitch] = color;
    }
  }
  if (SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
  if (SDL_MUSTLOCK(orig))
    SDL_UnlockSurface(orig);
  return surface;
}

#include <stdlib.h>
#include <stdarg.h>
#include <SDL.h>

#define TITLE "Mandelbrot-Zoom"
#define RENDER_TIME 30

typedef struct {
	double a, b;
} Complex;

static double xmin, xmax, ymin, ymax;
static int width, height;

static Uint8 eval_point(int x, int y)
{
	double pas = 0, pbs = 0;
	Complex p = { 0, 0 }, t = { 0, 0}, c;
	Uint8 i = 0;

	c.a = (double) x / width  * (xmax - xmin) + xmin;
	c.b = (double) y / height * (ymax - ymin) + ymin;
	do {
		t = p;
		p.a = pas - pbs + c.a;
		p.b = 2 * t.a * t.b + c.b;
		pas = p.a * p.a;
		pbs = p.b * p.b;
		if ((pas + pbs) > 4.0)
			break;
	} while (++i < 255);
	return i;
}

static void mandelbrot_render(Uint8 *data, int pitch)
{
	int x, y;

	for (x = 0; x < width; x++)
		for (y = 0; y < height; y++) {
			Uint8 val = eval_point(x, y);
			Uint32 *p = (Uint32*)(data + pitch * y + x * 4);
			*p = val | val << 8 | val << 16;
		}
}

static SDL_Window *_screen = NULL;

static void error(char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vprintf(format, ap);
	exit(-1);
}

static Uint32 render_cb(Uint32 interval, void *param)
{
	SDL_Event event;

	event.type = SDL_USEREVENT;
	SDL_PushEvent(&event);
	return interval;
}

int main(void)
{
	int px = 0, py = 0, pitch;
	double zoom = 1.0, zoom_mul = 1.0, p2x, p2y;
	char title[256];
	SDL_Event event;
	SDL_Renderer *rdr;
	SDL_Texture *txt;
	Uint8 *frame;
	Uint32 mstate;
	SDL_TimerID render_tid = 0;

	xmin = -1.0;
	xmax = 1.0;
	ymin = -1.0;
	ymax = 1.0;

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
	pitch = width * 4;
	frame = malloc(height * pitch);
	if (frame == NULL)
		error("Error allocating frame\n");
	mandelbrot_render(frame, pitch);
	SDL_UpdateTexture(txt, NULL, frame, pitch);
	SDL_RenderClear(rdr);
	SDL_RenderCopy(rdr, txt, NULL, NULL);
	SDL_RenderPresent(rdr);
	while (SDL_WaitEvent(&event)) {
		switch(event.type) {
		case SDL_MOUSEMOTION:
			px = event.motion.x;
			py = event.motion.y;
			break;
		case SDL_QUIT:
			goto out;
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
			case SDL_BUTTON_LEFT:
				zoom_mul = 0.97;
				break;
			case SDL_BUTTON_RIGHT:
				zoom_mul = 1.03;
				break;
			}
			SDL_RemoveTimer(render_tid);
			SDL_FlushEvent(SDL_USEREVENT);
			render_tid = SDL_AddTimer(RENDER_TIME, render_cb, NULL);
			break;
		case SDL_MOUSEBUTTONUP:
			mstate = SDL_GetMouseState(NULL, NULL);
			if (mstate & SDL_BUTTON(SDL_BUTTON_LEFT))
				zoom_mul = 0.97;
			else if (mstate & SDL_BUTTON(SDL_BUTTON_RIGHT))
				zoom_mul = 1.03;
			if (mstate & (SDL_BUTTON(SDL_BUTTON_LEFT) |
			              SDL_BUTTON(SDL_BUTTON_RIGHT)))
				continue;
			SDL_RemoveTimer(render_tid);
			SDL_FlushEvent(SDL_USEREVENT);
			p2x = (xmax - xmin) / width * px + xmin;
			p2y = (ymax - ymin) / height * py + ymin;
			xmin = p2x - zoom * (p2x - xmin);
			xmax = p2x - zoom * (p2x - xmax);
			ymin = p2y - zoom * (p2y - ymin);
			ymax = p2y - zoom * (p2y - ymax);
			mandelbrot_render(frame, pitch);
			SDL_UpdateTexture(txt, NULL, frame, pitch);
			SDL_RenderClear(rdr);
			SDL_RenderCopy(rdr, txt, NULL, NULL);
			SDL_RenderPresent(rdr);
			zoom = 1.0;
			break;
		case SDL_USEREVENT:
			sprintf(title, TITLE " - Zoom : %.2f",
			        (2.0 / zoom / (xmax - xmin)));
			SDL_SetWindowTitle(_screen, title);
			SDL_Rect r;
			zoom *= zoom_mul;
			r.x = px - px / zoom;
			r.y = py - py / zoom;
			r.w = width / zoom;
			r.h = height / zoom;
			SDL_RenderClear(rdr);
			SDL_RenderCopy(rdr, txt, NULL, &r);
			SDL_RenderPresent(rdr);
			break;
		}
	}
out:
	SDL_Quit();
	return 0;
}

#include <stdlib.h>
#include <stdarg.h>
#include <SDL.h>

#define TITLE "Mandelbrot-Zoom"
#define RENDER_TIME 30

typedef struct {
	double a, b;
} Complex;

typedef struct {
	double xmin;
	double xmax;
	double ymin;
	double ymax;
} RenderArea;

static int width, height;

static Uint8 eval_point(RenderArea *a, int x, int y)
{
	double pas = 0, pbs = 0;
	Complex p = { 0, 0 }, t = { 0, 0}, c;
	Uint8 i = 0;

	c.a = (double) x / width  * (a->xmax - a->xmin) + a->xmin;
	c.b = (double) y / height * (a->ymax - a->ymin) + a->ymin;
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

static void mandelbrot_render(RenderArea *a, Uint8 *data, int pitch)
{
	int x, y;
	Uint32 *p;

#pragma omp parallel for private(x,y) shared(data, pitch, p)
	for (x = 0; x < width; x++)
		for (y = 0; y < height; y++) {
			Uint8 val = eval_point(a, x, y);
			p = (Uint32*)(data + pitch * y + x * 4);
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

static SDL_sem *render_sem;
static Uint8 *frame, *frame_a, *frame_b;
static RenderArea a_thread;
static int pitch;

static int render_thread(void *param)
{
	while(1) {
		SDL_SemWait(render_sem);
		Uint8 *dst = frame == frame_a ? frame_b : frame_a;
		mandelbrot_render(&a_thread, dst, pitch);
		printf("Render done\n");
	}
	return 0;
}

int main(void)
{
	int px = 0, py = 0;
	double zoom = 1.0, zoom_mul = 1.0, p2x, p2y;
	char title[256];
	SDL_Event event;
	SDL_Renderer *rdr;
	SDL_Texture *txt;
	Uint32 mstate;
	SDL_TimerID render_tid = 0;
	RenderArea a = {
		.xmin = -1.0,
		.xmax = 1.0,
		.ymin = -1.0,
		.ymax = 1.0
	};

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		error("SDL : %s\n", SDL_GetError());
	render_sem = SDL_CreateSemaphore(0);
	SDL_CreateThread(render_thread, "render-thread", NULL);
	if (render_sem == NULL)
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
	frame_a = malloc(height * pitch);
	if (frame_a == NULL)
		error("Error allocating frame a\n");
	frame_b = malloc(height * pitch);
	if (frame_b == NULL)
		error("Error allocating frame b\n");
	frame = frame_a;
	mandelbrot_render(&a, frame, pitch);
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
			break;
		case SDL_USEREVENT:
			sprintf(title, TITLE " - Zoom : %.2f",
			        (2.0 / zoom / (a.xmax - a.xmin)));
			SDL_SetWindowTitle(_screen, title);
			zoom *= zoom_mul;
			p2x = (a.xmax - a.xmin) / width * px + a.xmin;
			p2y = (a.ymax - a.ymin) / height * py + a.ymin;
			a.xmin = p2x - zoom * (p2x - a.xmin);
			a.xmax = p2x - zoom * (p2x - a.xmax);
			a.ymin = p2y - zoom * (p2y - a.ymin);
			a.ymax = p2y - zoom * (p2y - a.ymax);
			mandelbrot_render(&a, frame, pitch);
			SDL_UpdateTexture(txt, NULL, frame, pitch);
			SDL_RenderClear(rdr);
			SDL_RenderCopy(rdr, txt, NULL, NULL);
			SDL_RenderPresent(rdr);
			zoom = 1.0;
			SDL_FlushEvent(SDL_USEREVENT);
			SDL_SemPost(render_sem);
			break;
		}
	}
out:
	SDL_Quit();
	return 0;
}

#include <SDL2/SDL.h>
#include <ncurses.h>
#include <term.h>
#include "flipnote.h"

void	update_term(int frame, int framec)
{
	int	row;
	int	col;
	static WINDOW	*win = 0;
	char	msg[128];

	if (!win)
		win = newwin(9, COLS / 2, (LINES - 9) / 2, COLS / 2 - COLS / 4);
	getmaxyx(win, row, col);
	sprintf(msg, "Frame %d/%d", frame, framec);
	werase(win);
	box(win, 0, 0);
	mvwprintw(win, row/2, (col-strlen(msg)) / 2, "%s", msg);
	wrefresh(win);
}

int main(int argc, char **argv)
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
	SDL_Texture *texture;
	SDL_Event event;
	char author[12];
	int framec;
	int useterm;
	clock_t time;
	float ltime;

	setupterm(NULL, 1, &useterm);
	if (useterm < 0)
		useterm = 0;
	if (useterm)
		initscr();
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
	window = SDL_CreateWindow("flipview", SDL_WINDOWPOS_CENTERED,
							  SDL_WINDOWPOS_CENTERED, 256, 192,
							  SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, 0);
	t_flipnote *flipnote;
	if (argc != 2)
		return 0;
	flipnote = read_ppm_from_file(argv[1]);
	if (!flipnote)
	{
		printf("fail.\n");
		return 0;
	}
	conv_utf16_ansi(flipnote->header.root_author_name, author);
	if (useterm)
	{
		printw("flipview by tjans\nfile: %s\nauthor: %s",
			   argv[1], author);
		refresh();
	}
	else
		printf("flipview by tjans\nfile: %s\nauthor: %s\n",
				argv[1], author);
	t_frame *frame;
	frame = flipnote->frames;
	surface = SDL_CreateRGBSurfaceWithFormatFrom(frame->layer_1.framebuffer,
												 256, 192, 32, 256 * 4,
												 SDL_PIXELFORMAT_RGBX8888);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	framec = 0;
	ltime = (float)clock();
	while (1)
	{
		time = clock();
		if (time - ltime > CLOCKS_PER_SEC / 14)
		{
			if (useterm)
				update_term(framec + 1, flipnote->header.frame_count);
			ltime = (float)time;
			framec++;
			if (framec >= flipnote->header.frame_count)
				framec = 0;
			SDL_DestroyTexture(texture);
			frame = flipnote->frames + framec;
			surface = SDL_CreateRGBSurfaceWithFormatFrom(
					frame->layer_1.framebuffer,
					256, 192, 32, 256 * 4, SDL_PIXELFORMAT_RGBX8888);
			texture = SDL_CreateTextureFromSurface(renderer, surface);
		}
		SDL_PollEvent(&event);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		if (event.type == SDL_QUIT)
		{
			endwin();
			return (0);
		}
	}
}

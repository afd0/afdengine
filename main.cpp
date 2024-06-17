#include <iostream>
#include <cstring>
#include <cstdlib>
#include <SDL2/SDL.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

class Window {
	public:
		//u32 pixels[SCREEN_WIDTH][SCREEN_HEIGHT] = {0};
		u32 *pixels;

		SDL_Window *window = nullptr;
		SDL_Renderer *renderer = nullptr;
		SDL_Texture *texture = nullptr;

		Window(const char* wTitle, int wWidth, int wHeight)
		{
			window = SDL_CreateWindow(wTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, wWidth, wHeight, SDL_WINDOW_SHOWN);
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

			pixels = (u32*) malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);

			pixels[0] = 0xFFFFFFFF;
			pixels[SCREEN_WIDTH / 2 + (SCREEN_HEIGHT / 2) * SCREEN_WIDTH] = 0xFFFFFFFF;
		}

		void setColor(int r, int g, int b, int a)
		{
			SDL_SetRenderDrawColor(renderer, r, g, b, a);
		}

		void drawPixel(int x, int y, u32 color)
		{
			pixels[x + y * SCREEN_WIDTH] = color;
		}

		void display()
		{
			void *px;
			int pitch;
			SDL_LockTexture(texture, NULL, &px, &pitch);
				
			for (int y = 0; y < SCREEN_HEIGHT; y++)
			{
				std::memcpy(px, pixels, pitch * SCREEN_HEIGHT);
			}

			SDL_UnlockTexture(texture);

			SDL_RenderCopy(renderer, texture, NULL, NULL);

			SDL_RenderPresent(renderer);
		}

		void clear()
		{
			std::memset(pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
		}
};

int main(int argc, char* args[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	Window window("SDL2 Testing", SCREEN_WIDTH, SCREEN_HEIGHT);

	window.setColor(0, 0, 0, 255);
	window.clear();

	int anim = 0;

	bool running = true;

	while (running)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) > 0)
		{
			switch (e.type)
			{
				case SDL_QUIT:
					running = false;
					break;
			}
		}

		window.clear();

		if (anim > 520) { anim = 0;}
		else { anim++; }

		window.drawPixel(20 + anim, 20 + anim, 0xFFFFFFFF);

		window.display();
	}
}

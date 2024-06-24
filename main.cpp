#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <algorithm>

typedef uint8_t u8;
typedef uint32_t u32;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Color defs
#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define BLUE 0xFF0000FF

struct Vec2 {
	float x, y;
};

class Window {
	public:
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
		}

		void setColor(int r, int g, int b, int a)
		{
			SDL_SetRenderDrawColor(renderer, r, g, b, a);
		}

		void drawPixel(int x, int y, u32 color)
		{
			pixels[x + y * SCREEN_WIDTH] = color;
		}

		void drawLine(float x1, float y1, float x2, float y2, u32 color)
		{
			if (x1 > x2) { std::swap(x1, x2); std::swap (y1, y2); }
			Vec2 v = {x1 - x2, y1 - y2};
			float k = (float)(y2 - y1) / (float)(x2 - x1);

			bool n = false;
			if (k < 0) { k = -k; n = true; }

			if (std::isinf(k)) 
			{
				if (!n)
				{
					for (int y = 0; y < y2 - y1; y++)
					{
						drawPixel(x1, y1 + y, color);
					}
				} else {
					for (int y = 0; y < y1 - y2; y++)
					{
						drawPixel(x1, y1 - y, color);
					}
				}
				
			} else {

				for (int x = 0; x < x2 - x1; x++)
				{
					if (k > 1)
					{
						for (int y = 0; y < k; y++)
						{
							if (!n) { drawPixel(x1 + x, x * k + y1 + y, color); }
							else { drawPixel(x1 + x, x * -k + y1 - y, color); }
						}
					} else { 
						if (!n) { drawPixel(x1 + x, y1 + k * x, color); }
						else { drawPixel(x1 + x, y1 - k * x, color); }
					}
				}
			}
		}

		Vec2 lerp(Vec2 p1, Vec2 p2, float t)
		{
			return {p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y)};
		}

		void drawTriangleW(Vec2 p1, Vec2 p2, Vec2 p3, u32 color)
		{
			drawLine(p1.x, p1.y, p2.x, p2.y, color);
			drawLine(p2.x, p2.y, p3.x, p3.y, color);
			drawLine(p3.x, p3.y, p1.x, p1.y, color);
		}
		
		void drawTriangleUEF(Vec2 p1, Vec2 p2, Vec2 p3, u32 color)
		{
			if (p2.x < p3.x) { std::swap(p3, p2); }
			
			
			float h = p3.y - p1.y;
			float t = 1 / h;
			for (int y = 0; y < h; y++)
			{
				drawLine(lerp(p1, p3, y * t).x, p1.y + y, lerp(p1, p2, y * t).x, p1.y + y, color);
			}
			std::cout << p1.x << ", " << p1.y << " p1\n";

		}

		void drawTriangleF(Vec2 p1, Vec2 p2, Vec2 p3, u32 color)
		{
			if(p2.y < p1.y) { std::swap(p2, p1); }
			if(p3.y < p1.y) { std::swap(p3, p1); }
			if(p3.y < p2.y) { std::swap(p3, p2); }

			if(p1.y == p2.y) { drawTriangleUEF(p1, p2, p3, color); return; }

			float h = p3.y - p1.y;
			float sy = p2.y - p1.y;
			float t1 = 1 / h;
			float t2 = 1 / (sy);
			float t3 = 1 / (p3.y - p2.y);
			

			for (int y = 0; y < h; y++)
			{
				if (y < p2.y && y * t2 <= 1)
				{
					drawLine(lerp(p1, p3, y * t1).x, p1.y + y, lerp(p1, p2, y * t2).x, p1.y + y, color);
				} else {
					drawLine(lerp(p1, p3, y * t1).x, p1.y + y, lerp(p2, p3, (y - sy) * t3).x, p1.y + y, color);
				}
			}

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

		window.drawLine(0, 0, 799, 599, 0xFFFFFFFF);
		window.drawLine(799, 0, 0, 599, 0xFFFFFFFF);
		window.drawLine(10, 10, 100, 100, 0xFFFFFFFF);
		window.drawLine(10, 10, 20, 100, 0xFFFFFFFF);
		window.drawLine(10, 10, 100, 10, 0xFFFFFFFF);
		window.drawLine(10, 100, 10, 10, WHITE);

		window.drawTriangleW({0, 0}, {0, 40}, {30, 50}, RED);
		window.drawTriangleF({10, 10}, {155, 510}, {310, 410}, WHITE);
		window.drawTriangleF({100, 500}, {500, 200}, {300, 200}, RED);
		window.drawPixel(310, 410, RED);

		window.display();
	}
}

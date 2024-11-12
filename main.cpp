#include "SDL2/SDL_pixels.h"
#include <__nullptr>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <algorithm>
#include <vector>

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

#define PI 3.14159265

#define dCamPlane 5
#define FOV 90

float displayWidth = (2 * dCamPlane)/(tan(((FOV * PI) / 180) / 2));
// displayWidth / displayHeight = SWidth / SCREEN_HEIGHT
float displayHeight = (displayWidth * SCREEN_HEIGHT) / SCREEN_WIDTH;

struct Vec2 {
	float x, y;
};

struct Vec3 {
	float x, y, z;
};

struct tri {
	int f, s, t;
};

struct Object {
	std::vector<Vec3> points;
	std::vector<tri> tris;
	Vec3 location;
	float angleX, angleY, angleZ;
};

float degToRad(float angle) { return (180 / PI * angle); }

class Window {
	private:
		const float dl = 0.00125;

	public:
		u32 *pixels;

		SDL_Window *window = nullptr;
		SDL_Renderer *renderer = nullptr;
		SDL_Texture *texture = nullptr;

		SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

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
			float lx = x2 - x1;
			float ly = y2 - y1;

			float i;

			for (i = 0.0; i < 1.0; i += dl) {
				drawPixel(x1 + int(floor(lx * i + 0.5)), y1 + int(floor(ly * i + 0.5)), color);
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
		
		float edgeFunction(Vec2 p1, Vec2 p2, Vec2 p3)
		{
			return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
		}

		void drawTriangleF(Vec2 p1, Vec2 p2, Vec2 p3, u32 color)
		{
			/*if(p2.y < p1.y) { std::swap(p2, p1); }
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
			}*/

			float minX = std::min(std::min(p1.x, p2.x), p3.x);
			float minY = std::min(std::min(p1.y, p2.y), p3.y);

			float maxX = std::max(std::max(p1.x, p2.x), p3.x);
			float maxY = std::max(std::max(p1.y, p2.y), p3.y);

			float ABC = edgeFunction(p1, p3, p2);
			if (ABC < 0) { return; }

			Vec2 p;

			for (p.x = minX; p.x < maxX; p.x++) {
				for (p.y = minY; p.y < maxY; p.y++) {
					float ABP = edgeFunction(p1, p, p2);
					float BCP = edgeFunction(p2, p, p3);
					float CAP = edgeFunction(p3, p, p1);

					if (ABP >= 0 && BCP >= 0 && CAP >= 0) {
						drawPixel(p.x, p.y, color);
					}
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

Vec2 viewportToCanvas(Vec2 point) {
	return {(point.x * SCREEN_WIDTH)/displayWidth + 400, (point.y * SCREEN_HEIGHT)/displayHeight + 300};
}

Vec2 projectPoint(Vec3 point) {
	Vec2 v;
	v.x = (dCamPlane * point.x) / (point.z + dCamPlane);
	v.y = (dCamPlane * point.y) / (point.z + dCamPlane);
	v = viewportToCanvas(v);
	return v;
}

Vec3 Vector_CrossProduct(Vec3 v1, Vec3 v2) {
	Vec3 v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

Vec3 rotateX(Vec3 point, float angle) {
	angle = degToRad(angle);
	Vec3 p;
	p.x = point.x;
	p.y = point.y * cosf(angle) - point.z * sinf(angle);
	p.z = point.y * sinf(angle) + point.z * cosf(angle);
	return p;
}

Vec3 rotateY(Vec3 point, float angle) {
	angle = degToRad(angle);
	Vec3 p;
	p.x = point.x * cosf(angle) + point.z * sinf(angle);
	p.y = point.y;
	p.z = - point.x * sinf(angle) + point.z * cosf(angle);
	return p;
}


int main(int argc, char* args[]) {

	SDL_Init(SDL_INIT_EVERYTHING);

	Window window("SDL2 Testing", SCREEN_WIDTH, SCREEN_HEIGHT);

	window.setColor(0, 0, 0, 255);
	window.clear();

	std::cout << "Display Width: " << displayWidth << "\n";
	std::cout << "Display Height: " << displayHeight << "\n";

	bool running = true;

	float angle = 0;

	std::vector<Object *> objects;
	Object cube;

	objects.push_back(&cube);

	cube.points.push_back({-1, -1, -1});
	cube.points.push_back({-1, -1, 1});
	cube.points.push_back({1, -1, 1});
	cube.points.push_back({1, -1, -1});

	cube.points.push_back({-1, 1, -1});
	cube.points.push_back({-1, 1, 1});
	cube.points.push_back({1, 1, 1});
	cube.points.push_back({1, 1, -1});

	cube.tris.push_back({1, 3, 2});
	cube.tris.push_back({1, 4, 3});

	cube.tris.push_back({5, 6, 7});
	cube.tris.push_back({5, 7, 8});

	cube.tris.push_back({4, 7, 3});
	cube.tris.push_back({4, 8, 7});

	cube.tris.push_back({6, 5, 1});
	cube.tris.push_back({2, 6, 1});

	cube.tris.push_back({1, 8, 4});
	cube.tris.push_back({1, 5, 8});

	cube.tris.push_back({2, 3, 7});
	cube.tris.push_back({2, 7, 6});

	cube.location = {0, 0, 3};

	cube.angleX = 0; cube.angleY = 0; cube.angleZ = 0;

	Object cube2 = cube;

	cube2.location = {2, 2, 6};

	//objects.push_back(&cube2);

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

		cube.angleX += 0.001;
		cube.angleY += 0.0005;

		for (auto obj : objects) {

			std::vector<Vec2> projectedPoints;
			std::vector<Vec3> points;
			std::vector<Vec3> normals;

			for (int i = 0; i < obj->points.size(); i++) {
				Vec3 p = rotateX(obj->points[i], obj->angleX);
				p = rotateY(p, obj->angleY);
				p = {p.x + obj->location.x, p.y + obj->location.y, p.z + obj->location.z};

				points.push_back(p);
				projectedPoints.push_back(projectPoint(p));
			}
	
			for (int i = 0; i < obj->tris.size(); i++) {
				tri triangle = obj->tris[i];

				Vec3 normal = Vector_CrossProduct({points[triangle.s - 1].x - points[triangle.f - 1].x,
								   points[triangle.s - 1].y - points[triangle.f - 1].y,
								   points[triangle.s - 1].z - points[triangle.f - 1].z},
								  {points[triangle.t - 1].x - points[triangle.f - 1].x,
								   points[triangle.t - 1].y - points[triangle.f - 1].y,
								   points[triangle.t - 1].z - points[triangle.f - 1].z});

				u32 color = SDL_MapRGBA(window.format, int(normal.x * 50), int(normal.y * 50), int(normal.z * 50), 255);

				window.drawTriangleF(projectedPoints[triangle.f - 1], projectedPoints[triangle.s - 1], projectedPoints[triangle.t - 1], color);
				

				//window.drawLine(points[triangle.f - 1].x, points[triangle.f - 1].y, points[triangle.s - 1].x, points[triangle.s - 1].y, RED);
				//window.drawLine(points[triangle.s - 1].x, points[triangle.s - 1].y, points[triangle.t - 1].x, points[triangle.t - 1].y, GREEN);
				//window.drawLine(points[triangle.t - 1].x, points[triangle.t - 1].y, points[triangle.f - 1].x, points[triangle.f - 1].y, BLUE);
			}

			points.clear();

		}

		window.display();
	}
}

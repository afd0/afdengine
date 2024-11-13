#include "SDL2/SDL_keycode.h"
#include "SDL2/SDL_pixels.h"
#include <__nullptr>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <SDL2/SDL.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include <strstream>

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

#define dCamPlane 1
#define FOV 90

float displayWidth = (2 * dCamPlane)/(tan(((FOV * PI) / 180) / 2));
// displayWidth / displayHeight = SWidth / SCREEN_HEIGHT
float displayHeight = (displayWidth * SCREEN_HEIGHT) / SCREEN_WIDTH;

struct Vec2 {
	float x, y;
};

struct Vec3 {
	float x, y, z;
	float w = 0;
};

struct Mat4x4 {
	float m[4][4] = { 0 };
};

struct tri {
	int f, s, t;
};

struct Object {
	std::vector<Vec3> points;
	std::vector<tri> tris;
	Vec3 location;
	float angleX, angleY, angleZ;

	void loadFromObj(std::string file) {
		int trisLoaded = 0;

		std::ifstream f(file);
		if (!f.is_open()) {
			return;
		}

		while (!f.eof()) {
			char line[128];
			f.getline(line, 128);

			std::strstream s;
			s << line;

			char junk;

			if (line[0] == 'v') {
				Vec3 v;
				s >> junk >> v.x >> v.y >> v.z;
				points.push_back(v);
			}

			if (line[0] == 'f') {
				tri t;
				s >> junk >> t.f >> t.s >> t.t;
				tris.push_back(t);

				trisLoaded++;
			}
		}

		std::cout << trisLoaded;
	}
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
		
		float edgeFunction(Vec2 p1, Vec2 p2, Vec2 p3)
		{
			return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
		}

		void drawTriangleF(Vec2 p1, Vec2 p2, Vec2 p3, u32 color)
		{
			float minX = std::min(std::min(p1.x, p2.x), p3.x);
			float minY = std::min(std::min(p1.y, p2.y), p3.y);

			float maxX = std::max(std::max(p1.x, p2.x), p3.x);
			float maxY = std::max(std::max(p1.y, p2.y), p3.y);

			float ABC = edgeFunction(p1, p3, p2);
			if (ABC < 0) { return; }

			Vec2 p;

			for (p.x = minX; p.x < maxX; p.x++) {
				for (p.y = minY; p.y < maxY; p.y++) {
					if (p.x >= 0 && p.y >= 0 && p.x < SCREEN_WIDTH && p.y < SCREEN_HEIGHT) {
						float ABP = edgeFunction(p1, p, p2);
						float BCP = edgeFunction(p2, p, p3);
						float CAP = edgeFunction(p3, p, p1);

						if (ABP >= 0 && BCP >= 0 && CAP >= 0) {
							drawPixel(p.x, p.y, color);
						}
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

float Vector_DotProduct(Vec3 v1, Vec3 v2) {
	return(v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
}

Vec3 Vector_NormalizeVector(Vec3 v) {
	float l = sqrtf(Vector_DotProduct(v, v));
	return {v.x / l, v.y / l, v.z / l};
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

Vec3 rotateZ(Vec3 point, float angle) {
	angle = degToRad(angle);
	Vec3 p;
	p.x = point.x * cosf(angle) - point.y * sinf(angle);
	p.y = point.x * sinf(angle) + point.y * cosf(angle);
	p.z = point.z;
	return p;
}

Vec3 matrixMultiplyVec3(Mat4x4 m, Vec3 i) {
	Vec3 v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}

Mat4x4 matrixMultiply4x4(Mat4x4 a, Mat4x4 b) {
	Mat4x4 m;
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			m.m[r][c] = a.m[r][0] * b.m[0][c] + a.m[r][1] * b.m[1][c] + a.m[r][2] * b.m[2][c] + a.m[r][3] * b.m[3][c];
		}
	}
	return m;
}

Mat4x4 quickInverse(Mat4x4 m) {
	Mat4x4 matrix;
	matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
    	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
    	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
    	matrix.m[3][3] = 1.0f;
    	return matrix;
}

Mat4x4 xRotationMatrix(float angle) {
	angle = degToRad(angle);
	Mat4x4 m;
	m.m[0][0] = 1;
	m.m[1][1] = cosf(angle);
	m.m[1][2] = -sinf(angle);
	m.m[2][1] = sinf(angle);
	m.m[2][2] = cosf(angle);
	m.m[3][3] = 1;
	return m;
}

Mat4x4 yRotationMatrix(float angle) {
	angle = degToRad(angle);
	Mat4x4 m;
	m.m[0][0] = cosf(angle);
	m.m[0][2] = -sinf(angle);
	m.m[1][1] = 1;
	m.m[2][0] = sinf(angle);
	m.m[2][2] = cosf(angle);
	m.m[3][3] = 1;
	return m;
}

Mat4x4 zRotationMatrix(float angle) {
	angle = degToRad(angle);
	Mat4x4 m;
	m.m[0][0] = cosf(angle);
	m.m[0][1] = -sinf(angle);
	m.m[1][0] = sinf(angle);
	m.m[1][1] = cosf(angle);
	m.m[2][2] = 1;
	m.m[3][3] = 1;
	return m;
}

Mat4x4 translationMatrix(float x, float y, float z) {
	Mat4x4 m;
	m.m[0][0] = 1;
	m.m[1][1] = 1;
	m.m[2][2] = 1;
	m.m[3][3] = 1;
	m.m[3][0] = x;
	m.m[3][1] = y;
	m.m[3][2] = z;
	return m;
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

	Object teapot;
	teapot.loadFromObj("teapot.obj");
	objects.push_back(&teapot);
	teapot.location.z = 5;

	//objects.push_back(&cube);

	// --- Construct cube vertices ---
	cube.points.push_back({-1, -1, -1});
	cube.points.push_back({-1, -1, 1});
	cube.points.push_back({1, -1, 1});
	cube.points.push_back({1, -1, -1});

	cube.points.push_back({-1, 1, -1});
	cube.points.push_back({-1, 1, 1});
	cube.points.push_back({1, 1, 1});
	cube.points.push_back({1, 1, -1});

	// --- Construct cube tris ---
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

	cube.location = {0, 0, 5};

	cube.angleX = 0; cube.angleY = 0; cube.angleZ = 0;

	Object cube2 = cube;

	cube2.location = {2, 2, 6};

	Vec3 camPos = {0, 0, 0};
	Vec3 camRotation = {0, 0, 0};

	//camRotation.x = degToRad(45);

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

				case SDL_KEYDOWN:
					switch (e.key.keysym.sym)
					{
						case SDLK_DOWN:
							camRotation.x += 0.001;
							break;
						case SDLK_UP:
							camRotation.x -= 0.001;
							break;
						case SDLK_LEFT:
							camRotation.y -= 0.001;
							break;
						case SDLK_RIGHT:
							camRotation.y += 0.001;
							break;
						case SDLK_a:
							camPos.x -= 1;
							break;
						case SDLK_d:
							camPos.x += 1;
							break;
						case SDLK_e:
							camPos.y += 1;
							break;
						case SDLK_q:
							camPos.y -= 1;
							break;
						case SDLK_w:
							camPos.z += 1;
							break;
						case SDLK_s:
							camPos.z -= 1;
							break;
					}
					
				case SDL_KEYUP:
					break;
			}
		}

		window.clear();

		//cube.angleX += 0.001;
		//cube.angleY += 0.0005;
		//cube.angleZ += 0.0003;

		//camRotation.x += 0.0005;
		//camRotation.z += 0.0005;

		//camPos.y += 1;

		Mat4x4 matTranslation = translationMatrix(camPos.x, camPos.y, camPos.z);

		Mat4x4 matRotX = xRotationMatrix(camRotation.x);
		Mat4x4 matRotY = yRotationMatrix(camRotation.y);
		Mat4x4 matRotZ = zRotationMatrix(camRotation.z);

		Mat4x4 matWorldToCam;
		matWorldToCam = matrixMultiply4x4(matRotX, matRotZ);
		matWorldToCam = matrixMultiply4x4(matWorldToCam, matRotY);
		matWorldToCam = matrixMultiply4x4(matWorldToCam, matTranslation);

		matWorldToCam = quickInverse(matWorldToCam);

		//camPos.z -= 0.01;

		for (auto obj : objects) {

			std::vector<Vec2> projectedPoints;
			std::vector<Vec3> points;
			std::vector<Vec3> normals;

			for (int i = 0; i < obj->points.size(); i++) {
				Vec3 p = obj->points[i];

				// --- Rotate object by all axis ---
				p = rotateX(p, obj->angleX);
				p = rotateY(p, obj->angleY);
				p = rotateZ(p, obj->angleZ);

				// --- Move object by its location vector / Set object to it's scene location ---
				p = {p.x + obj->location.x, p.y + obj->location.y, p.z + obj->location.z};

				
				// --- Move object according to camera position ---
				p.x -= camPos.x; p.y -= camPos.y; p.z -= camPos.z;
				
				p = matrixMultiplyVec3(matWorldToCam, p);

				if (p.z < dCamPlane) { p.z = 0; }

				// --- Save projected point in a vector and unprojected one in a second one for normals ---
				points.push_back(p);
				projectedPoints.push_back(projectPoint(p));
				
			}
	
			for (int i = 0; i < obj->tris.size(); i++) {
				tri triangle = obj->tris[i];

				// --- Calculate vectors required for normal calculation ---
				Vec3 f = points[triangle.f - 1];

				Vec3 v1 = points[triangle.s - 1];
				v1.x -= f.x; v1.y -= f.y; v1.z -= f.z;

				Vec3 v2 = points[triangle.t - 1];
				v2.x -= f.x; v2.y -= f.y; v2.z -= f.z;

				// --- Calculate surface normal vector for tri ---
				Vec3 normal = Vector_NormalizeVector(Vector_CrossProduct(v1, v2));

				// --- Calculate dot product of tri normal vector and light vector for crude shading effect ---
				float dp = Vector_DotProduct(normal, Vector_NormalizeVector({2, 2, 1}));
				u32 color = SDL_MapRGBA(window.format, int((dp + 1) * 40), int((dp + 1) * 40), int((dp + 1) * 40), 255);

				// --- Draw current triangle to screen ---
				if (points[triangle.f - 1].z > dCamPlane && points[triangle.s - 1].z > dCamPlane && points[triangle.t - 1].z > dCamPlane) {
					window.drawTriangleF(projectedPoints[triangle.f - 1], projectedPoints[triangle.s - 1], projectedPoints[triangle.t - 1], color);
				}
				

				//window.drawLine(points[triangle.f - 1].x, points[triangle.f - 1].y, points[triangle.s - 1].x, points[triangle.s - 1].y, RED);
				//window.drawLine(points[triangle.s - 1].x, points[triangle.s - 1].y, points[triangle.t - 1].x, points[triangle.t - 1].y, GREEN);
				//window.drawLine(points[triangle.t - 1].x, points[triangle.t - 1].y, points[triangle.f - 1].x, points[triangle.f - 1].y, BLUE);
			}

			points.clear();

		}

		window.display();
	}
}

#include <SDL2/SDL.h>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
 #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define screenWidth 1024
#define screenHeight 720
#define MAX_TEXT_BUFFER_LENGTH              1024  

SDL_Window* window;
SDL_Surface* srf;
SDL_Renderer* render;
SDL_Texture* tex;
SDL_PixelFormat *fmt;

int w;
int h;





 const char *TextFormat(const char *text, ...)
{

    #define MAX_TEXTFORMAT_BUFFERS 4        // Maximum number of static buffers for text formatting
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int  index = 0;
    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using
    va_list args;
    va_start(args, text);
    vsprintf(currentBuffer, text, args);
    va_end(args);
    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

void lock()
{
    if (SDL_MUSTLOCK(srf))
        if (SDL_LockSurface(srf) < 0)
            return;
}

//Unlocks the windoween
void unlock()
{
    if (SDL_MUSTLOCK(srf))
        SDL_UnlockSurface(srf);
}

void DrawBuffer(Uint32* buffer)
{
    Uint32* bufp;
    bufp = (Uint32*)srf->pixels;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            *bufp = buffer[y * w + x];
            bufp++;
        }
        bufp += srf->pitch / 4;
        bufp -= w;
    }
}

void Flip()
{
    SDL_RenderClear(render);
    SDL_UpdateTexture(tex, NULL, srf->pixels, srf->pitch);
    SDL_RenderCopy(render, tex, NULL, NULL);
    SDL_RenderPresent(render);
}

//Clears the windoween to black
void Clear(Uint8 r ,Uint8 g,Uint8 b)
{
    SDL_SetRenderDrawColor(render, r, g, b, 255);
    SDL_RenderClear(render);

}


void set_pixel(int x, int y, Uint8 r ,Uint8 g,Uint8 b,Uint8 a=255)
{
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    Uint32 colorSDL = SDL_MapRGBA(fmt, r, g, b, a);
    Uint32* bufp;
    bufp = (Uint32*)srf->pixels + y * srf->pitch / 4 + x;
    *bufp = colorSDL;
}

Uint32 ColorToInt(const SDL_Color& c)
{
		return (c.a | (c.b << 8) | (c.g << 16) | (c.r << 24));
}

Uint32 RGBAToInt(Uint8 r,Uint8 g,Uint8 b,Uint8 a)
{
		return (a | (b << 8) | (g << 16) | (r << 24));
}


SDL_Color IntToColor(Uint32 color)
{
    SDL_Color ColorRGBA;
    ColorRGBA.r = (color & 0xFF000000) >> 24;
    ColorRGBA.g = (color & 0x00FF0000) >> 16;
    ColorRGBA.b = (color & 0x0000FF00) >> 8;
    ColorRGBA.a = (color & 0x000000FF);
    return ColorRGBA;
}

bool  LoadTexture(std::vector<Uint32> &out, const char* filename, int& w, int& h)
{
    int n;
    unsigned char* data = stbi_load(filename, &w, &h, &n, 4);
    if (data == NULL)
    {
        std::cout << "Unable to load texture: " << filename << std::endl;
        return false;
    }
    out.resize(w * h);

    for (int i = 0; i < (int)out.size(); i++)
    {
          Uint8 r = data[i * 4];
          Uint8 g = data[i * 4 + 1];
          Uint8 b = data[i * 4 + 2];
          Uint8 a = data[i * 4 + 3];

          out[i] =RGBAToInt(r,g,b,a);

    }
    stbi_image_free(data);
    return true;
}



Uint32 get_pixel(int x, int y,Uint8 *r,Uint8 *g,Uint8 *b,Uint8 *a)
{
    if (x < 0 || y < 0 || x >= w || y >= h) return 0;
    Uint32* bufp;
    bufp = (Uint32*)srf->pixels + y * srf->pitch / 4 + x;
    Uint32 colorSDL = *bufp;
 
    SDL_GetRGBA(colorSDL, fmt, r, g, b, a);
    return RGBAToInt(*r,*g,*b,*a);
}

bool hLine(int y, int x1, int x2, Uint8 r ,Uint8 g,Uint8 b,Uint8 a=255)
	{
		if (x2 < x1) { x1 += x2; x2 = x1 - x2; x1 -= x2; } //swap x1 and x2, x1 must be the leftmost endpoint
		if (x2 < 0 || x1 >= w || y < 0 || y >= h) return 0; //no single point of the line is on screen
		if (x1 < 0) x1 = 0; //clip
		if (x2 >= w) x2 = w - 1; //clip

		Uint32 colorSDL = SDL_MapRGBA(fmt, r, g, b, a);
		Uint32* bufp;
		bufp = (Uint32*)srf->pixels + y * srf->pitch / 4 + x1;
		for (int x = x1; x <= x2; x++)
		{
			*bufp = colorSDL;
			bufp++;
		}
		return 1;
	}

	bool vLine(int x, int y1, int y2, Uint8 r ,Uint8 g,Uint8 b,Uint8 a=255)
	{
		if (y2 < y1) { y1 += y2; y2 = y1 - y2; y1 -= y2; } //swap y1 and y2
		if (y2 < 0 || y1 >= h || x < 0 || x >= w) return 0; //no single point of the line is on screen
		if (y1 < 0) y1 = 0; //clip
		if (y2 >= w) y2 = h - 1; //clip

		Uint32 colorSDL = SDL_MapRGBA(fmt, r, g, b, a);
		Uint32* bufp;

		bufp = (Uint32*)srf->pixels + y1 * srf->pitch / 4 + x;
		for (int y = y1; y <= y2; y++)
		{
			*bufp = colorSDL;
			bufp += srf->pitch / 4;
		}
		return 1;
	}

    //Bresenham line from (x1,y1) to (x2,y2) with rgb color
	bool Line(int x1, int y1, int x2, int y2, Uint8 r ,Uint8 g,Uint8 b,Uint8 a=255)
	{
		if (x1 < 0 || x1 > w - 1 || x2 < 0 || x2 > w - 1 || y1 < 0 || y1 > h - 1 || y2 < 0 || y2 > h - 1) return 0;

		int deltax = std::abs(x2 - x1); //The difference between the x's
		int deltay = std::abs(y2 - y1); //The difference between the y's
		int x = x1; //Start x off at the first pixel
		int y = y1; //Start y off at the first pixel
		int xinc1, xinc2, yinc1, yinc2, den, num, numadd, numpixels, curpixel;

		if (x2 >= x1) //The x-values are increasing
		{
			xinc1 = 1;
			xinc2 = 1;
		}
		else //The x-values are decreasing
		{
			xinc1 = -1;
			xinc2 = -1;
		}
		if (y2 >= y1) //The y-values are increasing
		{
			yinc1 = 1;
			yinc2 = 1;
		}
		else //The y-values are decreasing
		{
			yinc1 = -1;
			yinc2 = -1;
		}
		if (deltax >= deltay) //There is at least one x-value for every y-value
		{
			xinc1 = 0; //Don't change the x when numerator >= denominator
			yinc2 = 0; //Don't change the y for every iteration
			den = deltax;
			num = deltax / 2;
			numadd = deltay;
			numpixels = deltax; //There are more x-values than y-values
		}
		else //There is at least one y-value for every x-value
		{
			xinc2 = 0; //Don't change the x for every iteration
			yinc1 = 0; //Don't change the y when numerator >= denominator
			den = deltay;
			num = deltay / 2;
			numadd = deltax;
			numpixels = deltay; //There are more y-values than x-values
		}
		for (curpixel = 0; curpixel <= numpixels; curpixel++)
		{
			set_pixel(x % w, y % h, r,g,b,a);  //Draw the current pixel
			num += numadd;  //Increase the numerator by the top of the fraction
			if (num >= den) //Check if numerator >= denominator
			{
				num -= den; //Calculate the new numerator value
				x += xinc1; //Change the x as appropriate
				y += yinc1; //Change the y as appropriate
			}
			x += xinc2; //Change the x as appropriate
			y += yinc2; //Change the y as appropriate
		}

		return 1;
	}

    //Bresenham circle with center at (xc,yc) with radius and red green blue color
	bool Circle(int xc, int yc, int radius, Uint8 cr ,Uint8 cg,Uint8 cb,Uint8 ca=255)
	{
		if (xc - radius < 0 || xc + radius >= w || yc - radius < 0 || yc + radius >= h) return 0;
		int x = 0;
		int y = radius;
		int p = 3 - (radius << 1);
		int a, b, c, d, e, f, g, h;
		while (x <= y)
		{
			a = xc + x; //8 pixels can be calculated at once thanks to the symmetry
			b = yc + y;
			c = xc - x;
			d = yc - y;
			e = xc + y;
			f = yc + x;
			g = xc - y;
			h = yc - x;
			set_pixel(a, b, cr,cg,cb,ca);
			set_pixel(c, d, cr,cg,cb,ca);
			set_pixel(e, f, cr,cg,cb,ca);
			set_pixel(g, f, cr,cg,cb,ca);
			if (x > 0) //avoid drawing pixels at same position as the other ones
			{
				set_pixel(a, d, cr,cg,cb,ca);
				set_pixel(c, b, cr,cg,cb,ca);
				set_pixel(e, h, cr,cg,cb,ca);
				set_pixel(g, h, cr,cg,cb,ca);
			}
			if (p < 0) p += (x++ << 2) + 6;
			else p += ((x++ - y--) << 2) + 10;
		}

		return 1;
	}

    void getScreenBuffer(std::vector<Uint32>& buffer)
	{
		Uint32* bufp;
		bufp = (Uint32*)srf->pixels;

		buffer.resize(w * h);

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				buffer[y * w + x] = *bufp;
				bufp++;
			}
			bufp += srf->pitch / 4;
			bufp -= w;
		}
	}



#define FLOOR_HORIZONTAL 1

#define texWidth 64 // must be power of two
#define texHeight 64 // must be power of two
#define mapWidth 24
#define mapHeight 24

int worldMap[mapWidth][mapHeight] =
{
  {8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6},
  {8,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6},
  {7,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,6,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,4,6,0,6,6,6},
  {7,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,3,3,0,3,3,3},
  {2,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3},
  {2,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {2,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {1,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3},
  {2,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
};


struct Sprite
{
  double x;
  double y;
  int texture;
};

#define numSprites 19

Sprite sprite[numSprites] =
{
  {20.5, 11.5, 10}, //green light in front of playerstart
  //green lights in every room
  {18.5,4.5, 10},
  {10.0,4.5, 10},
  {10.0,12.5,10},
  {3.5, 6.5, 10},
  {3.5, 20.5,10},
  {3.5, 14.5,10},
  {14.5,20.5,10},

  //row of pillars in front of wall: fisheye test
  {18.5, 10.5, 9},
  {18.5, 11.5, 9},
  {18.5, 12.5, 9},

  //some barrels around the map
  {21.5, 1.5, 8},
  {15.5, 1.5, 8},
  {16.0, 1.8, 8},
  {16.2, 1.2, 8},
  {3.5,  2.5, 8},
  {9.5, 15.5, 8},
  {10.0, 15.1,8},
  {10.5, 15.8,8},
};

Uint32 buffer[screenHeight][screenWidth]; // y-coordinate first because it works per scanline

//1D Zbuffer
double ZBuffer[screenWidth];

//arrays used to sort the sprites
int spriteOrder[numSprites];
double spriteDistance[numSprites];


//sort the sprites based on distance
void sortSprites(int* order, double* dist, int amount)
{
  std::vector<std::pair<double, int>> sprites(amount);
  for(int i = 0; i < amount; i++) {
    sprites[i].first = dist[i];
    sprites[i].second = order[i];
  }
  std::sort(sprites.begin(), sprites.end());
  // restore in reverse order to go from farthest to nearest
  for(int i = 0; i < amount; i++) {
    dist[i] = sprites[amount - i - 1].first;
    order[i] = sprites[amount - i - 1].second;
  }
}


int main()
{

       if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			printf("Unable to init SDL: %s\n", SDL_GetError());
			SDL_Quit();
			std::exit(1);
		}
        int width = screenWidth;
        int height= screenHeight;
        w = width;
        h = height;

		std::atexit(SDL_Quit);
	
		window = SDL_CreateWindow("DOOM", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
		
		if (window == NULL)
		{
			printf("Unable to set video: %s\n", SDL_GetError());
			SDL_Quit();
			std::exit(1);
		}

		render = SDL_CreateRenderer(window, -1, 0);
		if (render == NULL)
		{
			printf("Unable to set renderer: %s\n", SDL_GetError());
			SDL_Quit();
			std::exit(1);
		}

		srf = SDL_CreateRGBSurface(0, width, height, 32,
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF);
		if (srf == NULL)
		{
			printf("Unable to set surface: %s\n", SDL_GetError());
			SDL_Quit();
			std::exit(1);
		}

		tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,width, height);
		if (tex == NULL)
		{
			printf("Unable to set texture: %s\n", SDL_GetError());
			SDL_Quit();
			std::exit(1);
		}
		SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

		fmt = srf->format;
		if (fmt == NULL)
		{
			printf("Unable to set pixel format: %s\n", SDL_GetError());
			SDL_Quit();
			std::exit(1);
		}


            double posX = 22.0, posY = 11.5;  //x and y start position
            double dirX = -1.0, dirY = 0.0; //initial direction vector
            double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane

            double time = 0; 
            double oldTime = 0; 

            std::vector<Uint32> texture[11];
         
            int image_width;
            int image_height;

  

            LoadTexture(texture[0], "images/eagle.png", image_width, image_height);
            LoadTexture(texture[1], "images/redbrick.png", image_width, image_height);
            LoadTexture(texture[2], "images/purplestone.png", image_width, image_height);
            LoadTexture(texture[3], "images/greystone.png", image_width, image_height);
            LoadTexture(texture[4], "images/bluestone.png", image_width, image_height);
            LoadTexture(texture[5], "images/mossy.png", image_width, image_height);
            LoadTexture(texture[6], "images/wood.png", image_width, image_height);
            LoadTexture(texture[7], "images/colorstone.png", image_width, image_height);
            LoadTexture(texture[8], "images/barrel.png", image_width, image_height);
            LoadTexture(texture[9], "images/pillar.png", image_width, image_height);
            LoadTexture(texture[10], "images/greenlight.png", image_width, image_height);


            

        bool done =false;
        while(!done)
        {
              SDL_Event event;
                while (SDL_PollEvent(&event)) 
                {
                    
                    switch (event.type)
                    {
                        case SDL_QUIT:
                        {
                            done = true;
                            break;
                        }
                    }
                }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_ESCAPE]) break;   



    //FLOOR CASTING
    for(int y = screenHeight / 2 + 1; y < screenHeight; ++y)
    {
      // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
      float rayDirX0 = dirX - planeX;
      float rayDirY0 = dirY - planeY;
      float rayDirX1 = dirX + planeX;
      float rayDirY1 = dirY + planeY;

      // Current y position compared to the center of the screen (the horizon)
      int p = y - screenHeight / 2;

      // Vertical position of the camera.
      float posZ = 0.5 * screenHeight;

      // Horizontal distance from the camera to the floor for the current row.
      // 0.5 is the z position exactly in the middle between floor and ceiling.
      float rowDistance = posZ / p;

      // calculate the real world step vector we have to add for each x (parallel to camera plane)
      // adding step by step avoids multiplications with a weight in the inner loop
      float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
      float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

      // real world coordinates of the leftmost column. This will be updated as we step to the right.
      float floorX = posX + rowDistance * rayDirX0;
      float floorY = posY + rowDistance * rayDirY0;

      for(int x = 0; x < screenWidth; ++x)
      {
        // the cell coord is simply got from the integer parts of floorX and floorY
        int cellX = (int)(floorX);
        int cellY = (int)(floorY);

        // get the texture coordinate from the fractional part
        int tx = (int)(texWidth * (floorX - cellX)) & (texWidth - 1);
        int ty = (int)(texHeight * (floorY - cellY)) & (texHeight - 1);

        floorX += floorStepX;
        floorY += floorStepY;

        // choose texture and draw the pixel
        int checkerBoardPattern = (int(cellX + cellY)) & 1;
        int floorTexture;
        if(checkerBoardPattern == 0) floorTexture = 3;
        else floorTexture = 4;
        int ceilingTexture = 6;
        Uint32 color;

        // floor
        color = texture[floorTexture][texWidth * ty + tx];
        color = (color >> 1) & 8355711; // make a bit darker
        buffer[y][x] = color;

        //ceiling (symmetrical, at screenHeight - y - 1 instead of y)
        color = texture[ceilingTexture][texWidth * ty + tx];
        color = (color >> 1) & 8355711; // make a bit darker
        buffer[screenHeight - y - 1][x] = color;
      }
    }

     // WALL CASTING
    for(int x = 0; x < w; x++)
    {
      //calculate ray position and direction
      double cameraX = 2 * x / double(w) - 1; //x-coordinate in camera space
      double rayDirX = dirX + planeX * cameraX;
      double rayDirY = dirY + planeY * cameraX;

      //which box of the map we're in
      int mapX = int(posX);
      int mapY = int(posY);

      //length of ray from current position to next x or y-side
      double sideDistX;
      double sideDistY;

      //length of ray from one x or y-side to next x or y-side
      double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
      double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
      double perpWallDist;

      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;

      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit?

      //calculate step and initial sideDist
      if(rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
      }
      if(rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, either in x-direction, or in y-direction
        if(sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if(worldMap[mapX][mapY] > 0) hit = 1;
      }

      //Calculate distance of perpendicular ray (Euclidean distance would give fisheye effect!)
      if(side == 0) perpWallDist = (sideDistX - deltaDistX);
      else          perpWallDist = (sideDistY - deltaDistY);

      //Calculate height of line to draw on screen
      int lineHeight = (int)(h / perpWallDist);

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + h / 2;
      if(drawStart < 0) drawStart = 0;
      int drawEnd = lineHeight / 2 + h / 2;
      if(drawEnd >= h) drawEnd = h - 1;
      //texturing calculations
      int texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

      //calculate value of wallX
      double wallX; //where exactly the wall was hit
      if (side == 0) wallX = posY + perpWallDist * rayDirY;
      else           wallX = posX + perpWallDist * rayDirX;
      wallX -= floor((wallX));

      //x coordinate on the texture
      int texX = int(wallX * double(texWidth));
      if(side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
      if(side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

      // TODO: an integer-only bresenham or DDA like algorithm could make the texture coordinate stepping faster
      // How much to increase the texture coordinate per screen pixel
      double step = 1.0 * texHeight / lineHeight;
      // Starting texture coordinate
      double texPos = (drawStart - h / 2 + lineHeight / 2) * step;
      for(int y = drawStart; y < drawEnd; y++)
      {
        // Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
        int texY = (int)texPos & (texHeight - 1);
        texPos += step;
        Uint32 color = texture[texNum][texHeight * texY + texX];
        //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        if(side == 1) color = (color >> 1) & 8355711;
        buffer[y][x] = color;
      }

      //SET THE ZBUFFER FOR THE SPRITE CASTING
      ZBuffer[x] = perpWallDist; //perpendicular distance is used
    }

 //SPRITE CASTING
    //sort sprites from far to close
    for(int i = 0; i < numSprites; i++)
    {
      spriteOrder[i] = i;
      spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y)); //sqrt not taken, unneeded
    }
    sortSprites(spriteOrder, spriteDistance, numSprites);

    //after sorting the sprites, do the projection and draw them
    for(int i = 0; i < numSprites; i++)
    {
      //translate sprite position to relative to camera
      double spriteX = sprite[spriteOrder[i]].x - posX;
      double spriteY = sprite[spriteOrder[i]].y - posY;

      //transform sprite with the inverse camera matrix
      // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
      // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
      // [ planeY   dirY ]                                          [ -planeY  planeX ]

      double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication

      double transformX = invDet * (dirY * spriteX - dirX * spriteY);
      double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D, the distance of sprite to player, matching sqrt(spriteDistance[i])

      int spriteScreenX = int((w / 2) * (1 + transformX / transformY));

      //parameters for scaling and moving the sprites
      #define uDiv 1
      #define vDiv 1
      #define vMove 0.0
      int vMoveScreen = int(vMove / transformY);

      //calculate height of the sprite on screen
      int spriteHeight = abs(int(h / (transformY))) / vDiv; //using "transformY" instead of the real distance prevents fisheye
      //calculate lowest and highest pixel to fill in current stripe
      int drawStartY = -spriteHeight / 2 + h / 2 + vMoveScreen;
      if(drawStartY < 0) drawStartY = 0;
      int drawEndY = spriteHeight / 2 + h / 2 + vMoveScreen;
      if(drawEndY >= h) drawEndY = h - 1;

      //calculate width of the sprite
      int spriteWidth = abs(int (h / (transformY))) / uDiv; // same as height of sprite, given that it's square
      int drawStartX = -spriteWidth / 2 + spriteScreenX;
      if(drawStartX < 0) drawStartX = 0;
      int drawEndX = spriteWidth / 2 + spriteScreenX;
      if(drawEndX > w) drawEndX = w;

      //loop through every vertical stripe of the sprite on screen
      for(int stripe = drawStartX; stripe < drawEndX; stripe++)
      {
        int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
        //the conditions in the if are:
        //1) it's in front of camera plane so you don't see things behind you
        //2) ZBuffer, with perpendicular distance
        if(transformY > 0 && transformY < ZBuffer[stripe])
        {
          for(int y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
          {
            int d = (y - vMoveScreen) * 256 - h * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
            int texY = ((d * texHeight) / spriteHeight) / 256;
            Uint32 color = texture[sprite[spriteOrder[i]].texture][texWidth * texY + texX]; //get current color from the texture
            SDL_Color c = IntToColor(color);
            if (c.r != 0 || c.g != 0 || c.b != 0) buffer[y][stripe] = color; //paint pixel if it isn't black, black is the invisible color
            }
        }
      }
    }


    DrawBuffer(buffer[0]);

    //timing for input and FPS counter
    oldTime = time;
    time = SDL_GetTicks();
    double frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
    double fps =(1.0 / frameTime); //FPS counter
    SDL_SetWindowTitle(window, TextFormat("FPS:%f  %f",fps,frameTime));
 

    //speed modifiers
    double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
    double rotSpeed = frameTime * 3.0; //the constant value is in radians/second

    
    //move forward if no wall in front of you
    if(keys[SDL_SCANCODE_UP])
    {
      if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
    }
    //move backwards if no wall behind you
    if(keys[SDL_SCANCODE_DOWN])
    {
      if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= dirX * moveSpeed;
      if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
    }
    //rotate to the right
   if(keys[SDL_SCANCODE_RIGHT])
    {
      //both camera direction and camera plane must be rotated
      double oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    //rotate to the left
    if(keys[SDL_SCANCODE_LEFT])
    {
      //both camera direction and camera plane must be rotated
      double oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }

            Flip();
        
        }


        SDL_DestroyTexture(tex);
        SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);


        SDL_Quit();

        return 0;
}
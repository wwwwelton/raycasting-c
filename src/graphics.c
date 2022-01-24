#include "graphics.h"

static SDL_Window		*window = NULL;
static SDL_Renderer		*renderer = NULL;
static uint32_t			*colorBuffer = NULL;
static SDL_Texture		*colorBufferTexture;

bool	initializeWindow(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL.\n");
		return (false);
	}
	window = SDL_CreateWindow
	(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_RESIZABLE
	);
	if (!window)
	{
		fprintf(stderr, "Error creating SDL window.\n");
		return (false);
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		fprintf(stderr, "Error creating SDL renderer.\n");
		return (false);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// allocate the total amount of bytes in memory to hold our colorbuffer
	colorBuffer = (uint32_t *)malloc(sizeof(uint32_t) * (uint32_t)WINDOW_WIDTH * (uint32_t)WINDOW_HEIGHT);

	// create an SDL_texture to display the colorbuffer
	colorBufferTexture = SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		WINDOW_WIDTH,
		WINDOW_HEIGHT
	);

	return (true);
}

void	destroyWindow(void)
{
	free(colorBuffer);
	SDL_DestroyTexture(colorBufferTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void	clearColorBuffer(color_t color)
{
	for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
		colorBuffer[i] = color;
}

void	renderColorBuffer(void)
{
	SDL_UpdateTexture
	(
		colorBufferTexture,
		NULL,
		colorBuffer,
		(int)((uint32_t)WINDOW_WIDTH * sizeof(color_t))
	);
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void	drawPixel(int x, int y, color_t color)
{
	colorBuffer[(WINDOW_WIDTH * y) + x] = color;
}

void	drawRect(int x, int y, int width, int height, color_t color)
{
	for (int i = x; i <= (x + width); i++)
	{
		for (int j = y; j < (y + height); j++)
		{
			drawPixel(i, j, color);
		}
	}
}

void	drawLine(int x0, int y0, int x1, int y1, color_t color)
{
	int		deltaX;
	int		deltaY;
	int		longestSideLength;
	float	xIncrement;
	float	yIncrement;
	float	currentX;
	float	currentY;

	// diferences between start and end of the line
	deltaX = (x1 - x0);
	deltaY = (y1 - y0);

	// longest side of the line
	longestSideLength = (abs(deltaX) >= abs(deltaY)) ? abs(deltaX) : abs(deltaY);

	// find the increment values
	xIncrement = deltaX / (float)longestSideLength;
	yIncrement = deltaY / (float)longestSideLength;

	// start point
	currentX = x0;
	currentY = y0;

	// loop all the longest side until the end
	for (int i = 0; i < longestSideLength; i++)
	{
		// draw pixel, rounding the values to integer to get nearest pixel
		drawPixel(round(currentX), round(currentY), color);

		// increment the slope to get the next pixel
		currentX += xIncrement;
		currentY += yIncrement;
	}
}

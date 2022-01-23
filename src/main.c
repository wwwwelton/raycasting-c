#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "defs.h"
#include "graphics.h"
#include "map.h"
#include "player.h"
#include "ray.h"
#include "textures.h"

bool			isGameRunning = false;
int				ticksLastFrame;

void	setup(void)
{
	// Asks uPNG library to decode all PNG files and loads the wallTextures array
	loadWallTextures();
}

void	processInput(void)
{
	SDL_Event	event;

	SDL_PollEvent(&event);
	switch (event.type)
	{
		case SDL_QUIT:
		{
			isGameRunning = false;
			break;
		}
		case SDL_KEYDOWN:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
				isGameRunning = false;
			if (event.key.keysym.sym == SDLK_UP)
				player.walkDirection = +1;
			if (event.key.keysym.sym == SDLK_DOWN)
				player.walkDirection = -1;
			if (event.key.keysym.sym == SDLK_RIGHT)
				player.turnDirection = +1;
			if (event.key.keysym.sym == SDLK_LEFT)
				player.turnDirection = -1;
			break;
		}
		case SDL_KEYUP:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
				isGameRunning = false;
			if (event.key.keysym.sym == SDLK_UP)
				player.walkDirection = 0;
			if (event.key.keysym.sym == SDLK_DOWN)
				player.walkDirection = 0;
			if (event.key.keysym.sym == SDLK_RIGHT)
				player.turnDirection = 0;
			if (event.key.keysym.sym == SDLK_LEFT)
				player.turnDirection = 0;
			break;
		}
	}
}

void	update(void)
{
	int		timeToWait;
	float	deltaTime;

	// Compute how long we have until the reach the target frame time in milliseconds
	timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);

	// Only delay execution if we are running too fast
	if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH) {
		SDL_Delay(timeToWait);
	}

	// Compute the delta time to be used as an update factor when changing game objects
	deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

	// Store the milliseconds of the current frame to be used in the future
	ticksLastFrame = SDL_GetTicks();

	movePlayer(deltaTime);
	castAllRays();
}

void	renderWallProjection(void)
{
	float		distanceProjPlane;
	float		projectedWallHeight;
	int			wallStripHeight;
	int			wallTopPixel;
	int			wallBottomPixel;
	float		perpDistance;
	uint32_t	texelColor;
	int			textureOffsetX;
	int			textureOffsetY;
	int			distanceFromTop;
	int			texNum;
	int			texture_width;
	int			texture_height;

	for (int x = 0; x < NUM_RAYS; x++)
	{
		perpDistance = rays[x].distance * cos(rays[x].rayAngle - player.rotationAngle);
		distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
		projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;

		wallStripHeight = (int)projectedWallHeight;

		wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
		wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

		wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
		wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

		// set the color of the ceiling
		for (int y = 0; y < wallTopPixel; y++)
			// colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF444444;
			drawPixel(x, y, 0xFF444444);

		// calculate textureOffsetX
		if (rays[x].wasHitVertical)
		{
			// perform offset for the vertical hit
			textureOffsetX = (int)rays[x].wallHitY % TILE_SIZE;
		}
		else
		{
			// perform offset for the horizontal hit
			textureOffsetX = (int)rays[x].wallHitX % TILE_SIZE;
		}

		// get the correct texture id number from the map content
		texNum = rays[x].wallHitContent - 1;

		texture_width = wallTextures[texNum].width;
		texture_height = wallTextures[texNum].height;

		// render the wall from wallTopPixel to wallBottomPixel
		for (int y = wallTopPixel; y < wallBottomPixel; y++)
		{
			// calculate textureOffsetY
			distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
			textureOffsetY = distanceFromTop * ((float)texture_height / wallStripHeight);

			//set the color of the wall based on the color from texture
			texelColor = wallTextures[texNum].texture_buffer[(texture_width * textureOffsetY) + textureOffsetX];
			// colorBuffer[(WINDOW_WIDTH * y) + x] = texelColor;
			drawPixel(x, y, texelColor);
		}
		// set the color of the flor
		for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
			// colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF777777;
			drawPixel(x, y, 0xFF777777);
	}
}

void	render(void)
{
	// clear the color buffer
	clearColorBuffer(0xFF000000);

	renderWallProjection();

	// display the minimap
	renderMap();
	// renderRays();
	renderPlayer();

	renderColorBuffer();

}

void	releaseResources(void)
{
	freeWallTextures();
	destroyWindow();
}

int	main(void)
{
	isGameRunning = initializeWindow();

	setup();

	while (isGameRunning) {
		processInput();
		update();
		render();
	}

	releaseResources();

	return (0);
}

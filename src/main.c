#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "constants.h"
#include "textures.h"

const int	map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 2, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5}
};

struct	Player {
	float	x;
	float	y;
	float	width;
	float	height;
	int		turnDirection; // -1 for left, +1 for right
	int		walkDirection; // -1 for back, +1 for front
	float	rotationAngle;
	float	walkSpeed;
	float	turnSpeed;
}			player;

struct	Ray {
	float	rayAngle;
	float	wallHitX;
	float	wallHitY;
	float	distance;
	int		wasHitVertical;
	int		isRayFacingUp;
	int		isRayFacingDown;
	int		isRayFacingLeft;
	int		isRayFacingRight;
	int		wallHitContent;
}			rays[NUM_RAYS];

SDL_Window		*window = NULL;
SDL_Renderer	*renderer = NULL;
int				isGameRunning = FALSE;
int				ticksLastFrame;

uint32_t		*colorBuffer = NULL;
SDL_Texture		*colorBufferTexture;
uint32_t		*textures[NUM_TEXTURES];

int	initializeWindow(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL.\n");
		return (FALSE);
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
		return (FALSE);
	}
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		fprintf(stderr, "Error creating SDL renderer.\n");
		return (FALSE);
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	return (TRUE);
}

void	destroyWindow(void)
{
	free(colorBuffer);
	SDL_DestroyTexture(colorBufferTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void	setup(void)
{
	player.x = WINDOW_WIDTH / 2;
	player.y = WINDOW_HEIGHT / 2;
	player.width = 1;
	player.height = 1;
	player.turnDirection = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI / 2;
	player.walkSpeed = 200;
	player.turnSpeed = 45 * (PI / 180);

	// allocate the total amount of bytes in memory to hold our colorbuffer
	colorBuffer = (uint32_t *)malloc(sizeof(uint32_t) * (uint32_t)WINDOW_WIDTH * (uint32_t)WINDOW_HEIGHT);

	// create an SDL_texture to display the colorbuffer
	colorBufferTexture = SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		WINDOW_WIDTH,
		WINDOW_HEIGHT
	);

	// load some textures from the textures.h
	textures[0] = (uint32_t *) REDBRICK_TEXTURE;
	textures[1] = (uint32_t *) PURPLESTONE_TEXTURE;
	textures[2] = (uint32_t *) MOSSYSTONE_TEXTURE;
	textures[3] = (uint32_t *) GRAYSTONE_TEXTURE;
	textures[4] = (uint32_t *) COLORSTONE_TEXTURE;
	textures[5] = (uint32_t *) BLUESTONE_TEXTURE;
	textures[6] = (uint32_t *) WOOD_TEXTURE;
	textures[7] = (uint32_t *) EAGLE_TEXTURE;
}

int	mapHasWallAt(float x, float y)
{
	int	mapGridIndexX;
	int	mapGridIndexY;

	if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT)
		return (TRUE);

	mapGridIndexX = (floor)(x / TILE_SIZE);
	mapGridIndexY = (floor)(y / TILE_SIZE);
	return (map[mapGridIndexY][mapGridIndexX] != 0);
}

void	movePlayer(float deltaTime)
{
	float	moveStep;
	float	newPlayerX;
	float	newPlayerY;

	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	moveStep = player.walkDirection * player.walkSpeed * deltaTime;

	newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
	newPlayerY = player.y + sin(player.rotationAngle) * moveStep;

	if (!mapHasWallAt(newPlayerX, newPlayerY))
	{
		player.x = newPlayerX;
		player.y = newPlayerY;
	}
}

void	renderPlayer(void)
{
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect	playerRect =
	{
		player.x * MINIMAP_SCALE_FACTOR,
		player.y * MINIMAP_SCALE_FACTOR,
		player.width * MINIMAP_SCALE_FACTOR,
		player.height * MINIMAP_SCALE_FACTOR
	};
	SDL_RenderFillRect(renderer, &playerRect);

	SDL_RenderDrawLine
	(
		renderer,
		MINIMAP_SCALE_FACTOR * player.x,
		MINIMAP_SCALE_FACTOR * player.y,
		MINIMAP_SCALE_FACTOR * player.x + cos(player.rotationAngle) * 40,
		MINIMAP_SCALE_FACTOR * player.y + sin(player.rotationAngle) * 40
	);
}

float	normalizeAngle(float angle)
{
	angle = remainder(angle, TWO_PI);
	if (angle < 0)
		angle = TWO_PI + angle;
	return (angle);
}

int	distanceBetweenPoints(float x1, float y1, float x2, float y2)
{
	return ( sqrt( (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) ) );
}

void	castRay(float rayAngle, int stripId)
{
	int		isRayFacingDown;
	int		isRayFacingUp;
	int		isRayFacingRight;
	int		isRayFacingLeft;

	float	xintercept;
	float	yintercept;
	float	xstep;
	float	ystep;

	// horz
	int		foundHorzWallHit;
	float	horzWallHitX;
	float	horzWallHitY;
	int		horzWallContent;

	float	nextHorzTouchX;
	float	nextHorzTouchY;

	float	horzHitDistance;
	// horz

	// vert
	int		foundVertWallHit;
	float	vertWallHitX;
	float	vertWallHitY;
	int		vertWallContent;

	float	nextVertTouchX;
	float	nextVertTouchY;

	float	vertHitDistance;
	// vert

	float	xToCheck;
	float	yToCheck;

	rayAngle = normalizeAngle(rayAngle);

	isRayFacingDown = rayAngle > 0 && rayAngle < PI;
	isRayFacingUp = !isRayFacingDown;

	isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
	isRayFacingLeft = !isRayFacingRight;

	///////////////////////////////////////////
	// HORIZONTAL RAY-GRID INTERSECTION CODE //
	//////////////////////////////////////////
	foundHorzWallHit = FALSE;
	horzWallHitX = 0;
	horzWallHitY = 0;
	horzWallContent = 0;

	// Find the y-coordinate of the closest horizontal grid intersection
	yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
	yintercept += isRayFacingDown ? TILE_SIZE : 0;

	// Find the x-coordinate of the closest horizontal grid intersection
	xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

	// Calculate the increment xtep e and ystep
	ystep = TILE_SIZE;
	ystep *= isRayFacingUp ? -1 : 1;

	xstep = TILE_SIZE / tan(rayAngle);
	xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
	xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

	nextHorzTouchX = xintercept;
	nextHorzTouchY = yintercept;

	// Increment xstep and ystep until we find a wall
	while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH
		&& nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT)
	{
		// Decreases one pixel to ensure the point is inside
		// below the line and not on top
		xToCheck = nextHorzTouchX;
		yToCheck = nextHorzTouchY + (isRayFacingUp ? - 1 : 0);

		if (mapHasWallAt(xToCheck, yToCheck))
		{
			// found a wall hit
			horzWallHitX = nextHorzTouchX;
			horzWallHitY = nextHorzTouchY;
			horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
			foundHorzWallHit = TRUE;
			break ;
		}
		else
		{
			nextHorzTouchX += xstep;
			nextHorzTouchY += ystep;
		}
	}

	///////////////////////////////////////////
	// VERTICAL RAY-GRID INTERSECTION CODE //
	//////////////////////////////////////////
	foundVertWallHit = FALSE;
	vertWallHitX = 0;
	vertWallHitY = 0;
	vertWallContent = 0;

	// Find the x-coordinate of the closest vertical grid intersection
	xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
	xintercept += isRayFacingRight ? TILE_SIZE : 0;

	// Find the y-coordinate of the closest vertical grid intersection
	yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

	// Calculate the increment xtep e and ystep
	xstep = TILE_SIZE;
	xstep *= isRayFacingLeft ? -1 : 1;

	ystep = TILE_SIZE * tan(rayAngle);
	ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
	ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

	nextVertTouchX = xintercept;
	nextVertTouchY = yintercept;

	// Increment xstep and ystep until we find a wall
	while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH
		&& nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT)
	{
		// Decreases one pixel to ensure the point is inside
		// below the line and not on top
		xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
		yToCheck = nextVertTouchY;

		if (mapHasWallAt(xToCheck, yToCheck))
		{
			// found a wall hit
			vertWallHitX = nextVertTouchX;
			vertWallHitY = nextVertTouchY;
			vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
			foundVertWallHit = TRUE;
			break ;
		}
		else
		{
			nextVertTouchX += xstep;
			nextVertTouchY += ystep;
		}
	}

	// Calculate both horizontal and vertical hit distances
	// and choose the smallest one
	horzHitDistance = foundHorzWallHit
	? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
	: FLT_MAX;
	vertHitDistance = foundVertWallHit
	? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
	: FLT_MAX;

	if (vertHitDistance < horzHitDistance)
	{
		rays[stripId].distance = vertHitDistance;
		rays[stripId].wallHitX = vertWallHitX;
		rays[stripId].wallHitY = vertWallHitY;
		rays[stripId].wallHitContent = vertWallContent;
		rays[stripId].wasHitVertical = TRUE;
	}
	else
	{
		rays[stripId].distance = horzHitDistance;
		rays[stripId].wallHitX = horzWallHitX;
		rays[stripId].wallHitY = horzWallHitY;
		rays[stripId].wallHitContent = horzWallContent;
		rays[stripId].wasHitVertical = FALSE;
	}
	rays[stripId].rayAngle = rayAngle;
	rays[stripId].isRayFacingDown = isRayFacingDown;
	rays[stripId].isRayFacingUp = isRayFacingUp;
	rays[stripId].isRayFacingLeft = isRayFacingLeft;
	rays[stripId].isRayFacingRight = isRayFacingRight;
}

void	castAllRays(void)
{
	float	rayAngle;

	// start first ray subtracting half of our FOV
	rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
	for (int stripId = 0; stripId < NUM_RAYS; stripId++)
	{
		castRay(rayAngle, stripId);
		rayAngle += FOV_ANGLE / NUM_RAYS;
	}
}

void	renderMap(void)
{
	int	tileX;
	int	tileY;
	int	tileColor;

	for (int i = 0; i < MAP_NUM_ROWS; i++)
	{
		for (int j = 0; j < MAP_NUM_COLS; j++)
		{
			tileX = j * TILE_SIZE;
			tileY = i * TILE_SIZE;
			tileColor = map[i][j] != 0 ? 255 : 0;

			SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
			SDL_Rect	mapTileRect =
			{
				tileX * MINIMAP_SCALE_FACTOR,
				tileY * MINIMAP_SCALE_FACTOR,
				TILE_SIZE * MINIMAP_SCALE_FACTOR,
				TILE_SIZE * MINIMAP_SCALE_FACTOR
			};
			SDL_RenderFillRect(renderer, &mapTileRect);
		}
	}

}

void	renderRays()
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for (int i = 0; i < NUM_RAYS; i++)
	{
		SDL_RenderDrawLine(
			renderer,
			MINIMAP_SCALE_FACTOR * player.x,
			MINIMAP_SCALE_FACTOR * player.y,
			MINIMAP_SCALE_FACTOR * rays[i].wallHitX,
			MINIMAP_SCALE_FACTOR * rays[i].wallHitY
		);
	}

}

void	processInput(void)
{
	SDL_Event	event;

	SDL_PollEvent(&event);
	switch (event.type)
	{
		case SDL_QUIT:
		{
			isGameRunning = FALSE;
			break;
		}
		case SDL_KEYDOWN:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
				isGameRunning = FALSE;
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
				isGameRunning = FALSE;
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
	// Compute how long we have until the reach the target frame time in milliseconds
	int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);

	// Only delay execution if we are running too fast
	if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH) {
		SDL_Delay(timeToWait);
	}

	// Compute the delta time to be used as an update factor when changing game objects
	float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

	// Store the milliseconds of the current frame to be used in the future
	ticksLastFrame = SDL_GetTicks();

	movePlayer(deltaTime);
	castAllRays();
}

void	generate3DProjection(void)
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

	for (int i = 0; i < NUM_RAYS; i++)
	{
		perpDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);
		distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
		projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;

		wallStripHeight = (int)projectedWallHeight;

		wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
		wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

		wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
		wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

		// set the color of the ceiling
		for (int y = 0; y < wallTopPixel; y++)
			colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF333333;

		// calculate textureOffsetX
		if (rays[i].wasHitVertical)
		{
			// perform offset for the vertical hit
			textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
		}
		else
		{
			// perform offset for the horizontal hit
			textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;
		}

		// get the correct texture id number from the map content
		texNum = rays[i].wallHitContent - 1;

		// render the wall from wallTopPixel to wallBottomPixel
		for (int y = wallTopPixel; y < wallBottomPixel; y++)
		{
			// calculate textureOffsetY
			distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
			textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallStripHeight);

			//set the color of the wall based on the color from texture
			texelColor = textures[texNum][(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
			colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
		}
		// set the color of the flor
		for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
			colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF777777;
	}
}

void	clearColorBuffer(uint32_t color)
{
	for (int x = 0; x < WINDOW_WIDTH; x++)
	{
		for (int y = 0; y < WINDOW_HEIGHT; y++)
		{
			colorBuffer[(WINDOW_WIDTH * y) + x] = color;
		}
	}
}

void	renderColorBuffer(void)
{
	SDL_UpdateTexture
	(
		colorBufferTexture,
		NULL,
		colorBuffer,
		(int)((uint32_t)WINDOW_WIDTH * sizeof(uint32_t))
	);
	SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void	render(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	generate3DProjection();

	renderColorBuffer();
	// clear the color buffer
	clearColorBuffer(0xFF000000);

	// display the minimap
	renderMap();
	renderRays();
	renderPlayer();

	SDL_RenderPresent(renderer);
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

	destroyWindow();

	return (0);
}

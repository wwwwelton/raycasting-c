#include "wall.h"

// Change the color intensity based on factor value between 0 and 1
void	changeColorIntensity(color_t *color, float factor)
{
	color_t	a;
	color_t	r;
	color_t	g;
	color_t	b;

	a = (*color & 0xFF000000);
	r = (*color & 0x00FF0000) * factor;
	g = (*color & 0x0000FF00) * factor;
	b = (*color & 0x000000FF) * factor;

	*color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}

void	renderWallProjection(void)
{
	float		distanceProjPlane;
	float		projectedWallHeight;
	int			wallStripHeight;
	int			wallTopPixel;
	int			wallBottomPixel;
	float		perpDistance;
	color_t		texelColor;
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

			// set the color of the wall based on the color from the texture
			texelColor = wallTextures[texNum].texture_buffer[(texture_width * textureOffsetY) + textureOffsetX];

			// Make the pixel color darker if the ray was vertical
			if (rays[x].wasHitVertical)
			{
				changeColorIntensity(&texelColor, 0.7);
			}

			// colorBuffer[(WINDOW_WIDTH * y) + x] = texelColor;
			drawPixel(x, y, texelColor);
		}
		// set the color of the flor
		for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
			// colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF777777;
			drawPixel(x, y, 0xFF777777);
	}
}

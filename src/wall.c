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
	float		wallHeight;
	int			wallTopY;
	int			wallBottomY;
	float		perpDistance;
	color_t		texelColor;
	color_t		*wallTextureBuffer;
	int			textureOffsetX;
	int			textureOffsetY;
	int			distanceFromTop;
	int			texNum;
	int			textureWidth;
	int			textureHeight;

	for (int x = 0; x < NUM_RAYS; x++)
	{
		// Calculate the projection plane distance
		distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);

		// Calculate the perpendicular distance to avoid the fish-eye distortion
		perpDistance = rays[x].distance * cos(rays[x].rayAngle - player.rotationAngle);

		// Calculate the projected wall height
		wallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;

		// Find the wall top Y value
		wallTopY = (WINDOW_HEIGHT / 2) - (wallHeight / 2);
		wallTopY = wallTopY < 0 ? 0 : wallTopY;

		// Find the wall bottom Y value
		wallBottomY = (WINDOW_HEIGHT / 2) + (wallHeight / 2);
		wallBottomY = wallBottomY > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomY;

		// Draw the ceiling
		for (int y = 0; y < wallTopY; y++)
			// colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF444444;
			drawPixel(x, y, 0xFF444444);

		// Draw the textured wall
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

		// Get the correct texture id number from the map content
		texNum = rays[x].texture - 1;

		// Query the texture width and height from the upng
		textureWidth = upng_get_width(textures[texNum]);
		textureHeight = upng_get_height(textures[texNum]);

		// Render the wall from wallTopY to wallBottomY
		for (int y = wallTopY; y < wallBottomY; y++)
		{
			// calculate textureOffsetY
			distanceFromTop = y + (wallHeight / 2) - (WINDOW_HEIGHT / 2);
			textureOffsetY = distanceFromTop * ((float)textureHeight / wallHeight);

			// Set the color of the wall based on the color from the texture
			wallTextureBuffer = (color_t *)upng_get_buffer(textures[texNum]);
			texelColor = wallTextureBuffer[(textureWidth * textureOffsetY) + textureOffsetX];

			// Make the pixel color darker if the ray hit was vertical
			if (rays[x].wasHitVertical)
			{
				changeColorIntensity(&texelColor, 0.7);
			}

			// colorBuffer[(WINDOW_WIDTH * y) + x] = texelColor;
			drawPixel(x, y, texelColor);
		}
		// Draw the floor
		for (int y = wallBottomY; y < WINDOW_HEIGHT; y++)
			// colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF777777;
			drawPixel(x, y, 0xFF777777);
	}
}

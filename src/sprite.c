#include "sprite.h"
#include "utils.h"
#include "ray.h"

static sprite_t	sprites[NUM_SPRITES] =
{
	{ .x = 640, .y = 630, .texture = 9 },  // barrel
	{ .x = 660, .y = 690, .texture = 9 },  // barrel
	{ .x = 250, .y = 600, .texture = 11 }, // table
	{ .x = 250, .y = 600, .texture = 10},  // light
	{ .x = 300, .y = 400, .texture = 12 }, // guard
	{ .x = 90, .y = 100, .texture = 13 }  // armor
};

void	renderMapSprites(void)
{
	for (int i = 0; i < NUM_SPRITES; i++)
	{
		drawRect
		(
			sprites[i].x * MINIMAP_SCALE_FACTOR,
			sprites[i].y * MINIMAP_SCALE_FACTOR,
			2,
			2,
			(sprites[i].visible) ? 0xFF00FFFF : 0xFF444444
		);
	}
}

void	renderSpriteProjection(void)
{
	float		distanceProjPlane;
	sprite_t	visibleSprites[NUM_SPRITES];
	int			numVisibleSprites;
	float		angleSpritePlayer;
	sprite_t	sprite;
	float		spriteHeight;
	float		spriteWidth;
	float 		spriteTopY;
	float		spriteBottomY;
	float		spriteAngle;
	float		spriteScreenPoX;
	float		spriteLeftX;
	float		spriteRightX;
	int			textureWidth;
	int			textureHeight;
	color_t		*spriteTextureBuffer;
	color_t		texelColor;
	int			textureOffsetY;
	int			textureOffsetX;
	float		texelWidth;
	int			distanceFromTop;
	sprite_t	temp;
	float		perpDistance;

	numVisibleSprites = 0;

	// Find sprites that are visible (inside the FOV)
	for (int i = 0; i < NUM_SPRITES; i++)
	{
		// Angle between player and sprite
		angleSpritePlayer = player.rotationAngle - atan2(sprites[i].y - player.y, sprites[i].x - player.x);

		// Make sure the angle is always between 0 and 180 degrees
		if (angleSpritePlayer > PI)
			angleSpritePlayer -= TWO_PI;
		if (angleSpritePlayer < -PI)
			angleSpritePlayer += TWO_PI;
		angleSpritePlayer = fabs(angleSpritePlayer);

		// If sprite angle is less than half the FOV plus a small erro margin
		const float	EPSILON = 0.2;
		if (angleSpritePlayer < (FOV_ANGLE / 2) + EPSILON)
		{
			sprites[i].visible = true;
			sprites[i].angle = angleSpritePlayer;
			sprites[i].distance = distanceBetweenPoints(sprites[i].x, sprites[i].y, player.x, player.y);
			visibleSprites[numVisibleSprites] = sprites[i];
			numVisibleSprites++;
		}
		else
		{
			sprites[i].visible = false;
		}
	}

	// Sort sprites by distance using a naive bubble-sort algorithm
	for (int i = 0; i < numVisibleSprites - 1; i++)
	{
		for (int j = 0; j < numVisibleSprites; j++)
		{
			if (visibleSprites[i].distance < visibleSprites[j].distance)
			{
				temp = visibleSprites[i];
				visibleSprites[i] = visibleSprites[j];
				visibleSprites[j] = temp;
			}
		}
	}

	// Rendering all the visible sprites
	for (int i = 0; i < numVisibleSprites; i++)
	{
		sprite = visibleSprites[i];

		// Calculate the projection plane distance
		distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);

		// Calculate the perpendicular distance of the sprite to prevent fish-eye effect
		perpDistance = sprite.distance * cos(sprite.angle);

		// Calculate the sprite projected height and width (the same, as sprites are squared)
		spriteHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;
		spriteWidth = spriteHeight;

		// Sprite top Y
		spriteTopY = (WINDOW_HEIGHT / 2) - (spriteHeight / 2);
		spriteTopY = (spriteTopY < 0) ? 0 : spriteTopY;

		// Sprite bottom Y
		spriteBottomY = (WINDOW_HEIGHT / 2) + (spriteHeight / 2);
		spriteBottomY = (spriteBottomY > WINDOW_HEIGHT) ? WINDOW_HEIGHT : spriteBottomY;

		// Calculate the sprite X position in the projection plane
		spriteAngle = atan2(sprite.y - player.y, sprite.x - player.x) - player.rotationAngle;
		spriteScreenPoX = tan(spriteAngle) * distanceProjPlane;

		// SpriteLeftX
		spriteLeftX = (WINDOW_WIDTH / 2) + spriteScreenPoX - (spriteWidth / 2);

		// SpriteRightX
		spriteRightX = spriteLeftX + spriteWidth;

		// Query the width and height of the texture
		textureWidth = upng_get_width(textures[sprite.texture]);
		textureHeight = upng_get_height(textures[sprite.texture]);

		// Loop all the x values
		for (int x = spriteLeftX; x < spriteRightX; x++)
		{
			// Size of pixel based on sprite width
			texelWidth = (textureWidth / spriteWidth);
			// Get the x position inside the texture
			textureOffsetX = (x - spriteLeftX) * texelWidth;

			// Loop all the y values
			for (int y = spriteTopY; y < spriteBottomY; y++)
			{
				if (x > 0 && x < WINDOW_WIDTH && y > 0 && y < WINDOW_HEIGHT)
				{
					// Amount of pixels based on sprite height
					distanceFromTop = y + (spriteHeight / 2) - (WINDOW_HEIGHT / 2);
					// Get the y position inside the texture
					textureOffsetY = distanceFromTop * (textureHeight / spriteHeight);

					spriteTextureBuffer = (color_t *)upng_get_buffer(textures[sprite.texture]);
					texelColor = spriteTextureBuffer[(textureWidth * textureOffsetY) + textureOffsetX];

					// Only draws the sprite pixel if it is closer than the wall
					if (sprite.distance < rays[x].distance && texelColor != 0xFFFF00FF)
						drawPixel(x, y, texelColor);
				}
			}

		}

	}
}

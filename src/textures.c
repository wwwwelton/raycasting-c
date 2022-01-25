#include "textures.h"
#include <stdio.h>

static const char*	textureFileNames[NUM_TEXTURES] =
{
	"./images/redbrick.png",		// [0]
	"./images/purplestone.png",		// [1]
	"./images/mossystone.png",		// [2]
	"./images/graystone.png",		// [3]
	"./images/colorstone.png",		// [4]
	"./images/bluestone.png",		// [5]
	"./images/wood.png",			// [6]
	"./images/eagle.png",			// [7]
	"./images/pikuma.png",			// [8]
	"./images/barrel.png",			// [9]
	"./images/light.png",			// [10]
	"./images/table.png",			// [11]
	"./images/guard.png",			// [12]
	"./images/armor.png"			// [13]
};

void	loadTextures(void)
{
	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		upng_t*	upng;

		upng = upng_new_from_file(textureFileNames[i]);
		if (upng != NULL)
		{
			upng_decode(upng);
			if (upng_get_error(upng) == UPNG_EOK)
			{
				textures[i] = upng;
			}
			else
			{
				printf("Error decoding texture file %s \n", textureFileNames[i]);
			}
		}
		else
		{
			printf("Error loading texture %s \n", textureFileNames[i]);
		}
	}

}

void	freeTextures(void)
{
	for (int i = 0; i < NUM_TEXTURES; i++)
	{
		upng_free(textures[i]);
	}
}

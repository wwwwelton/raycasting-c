#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include "defs.h"
#include "graphics.h"

#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20

bool	mapHasWallAt(float x, float y);
bool	isInsideMap(float x, float y);
void	renderMapGrid(void);
int		getMapAt(int i, int j);

#endif

#ifndef RAY_H
#define RAY_H

#include <stdbool.h>
#include <limits.h>
#include "defs.h"
#include "player.h"

typedef struct	{
	float	rayAngle;
	float	wallHitX;
	float	wallHitY;
	float	distance;
	bool	wasHitVertical;
	int		wallHitContent;
}			ray_t;

extern ray_t	rays[NUM_RAYS];

void	normalizeAngle(float *angle);
int		distanceBetweenPoints(float x1, float y1, float x2, float y2);
bool	isRayFacingUp(float angle);
bool	isRayFacingDown(float angle);
bool	isRayFacingLeft(float angle);
bool	isRayFacingRight(float angle);
void	castAllRays(void);
void	castRay(float rayAngle, int stripId);
void	renderRays(void);

#endif

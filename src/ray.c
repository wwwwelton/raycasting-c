#include "ray.h"

ray_t	rays[NUM_RAYS];

void	normalizeAngle(float *angle)
{
	*angle = remainder(*angle, TWO_PI);
	if (*angle < 0)
		*angle = TWO_PI + *angle;
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
	bool	foundHorzWallHit;
	float	horzWallHitX;
	float	horzWallHitY;
	int		horzWallContent;

	float	nextHorzTouchX;
	float	nextHorzTouchY;

	float	horzHitDistance;
	// horz

	// vert
	bool	foundVertWallHit;
	float	vertWallHitX;
	float	vertWallHitY;
	int		vertWallContent;

	float	nextVertTouchX;
	float	nextVertTouchY;

	float	vertHitDistance;
	// vert

	float	xToCheck;
	float	yToCheck;

	normalizeAngle(&rayAngle);

	isRayFacingDown = rayAngle > 0 && rayAngle < PI;
	isRayFacingUp = !isRayFacingDown;

	isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
	isRayFacingLeft = !isRayFacingRight;

	///////////////////////////////////////////
	// HORIZONTAL RAY-GRID INTERSECTION CODE //
	//////////////////////////////////////////
	foundHorzWallHit = false;
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
	// while (nextHorzTouchX >= 0 && nextHorzTouchX <= MAP_NUM_COLS * TILE_SIZE
	// 	&& nextHorzTouchY >= 0 && nextHorzTouchY <= MAP_NUM_ROWS * TILE_SIZE)
	while(isInsideMap(nextHorzTouchX, nextHorzTouchY))
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
			horzWallContent = getMapAt((int)floor(yToCheck / TILE_SIZE), (int)floor(xToCheck / TILE_SIZE));
			foundHorzWallHit = true;
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
	foundVertWallHit = false;
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
	// while (nextVertTouchX >= 0 && nextVertTouchX <= MAP_NUM_COLS * TILE_SIZE
	// 	&& nextVertTouchY >= 0 && nextVertTouchY <= MAP_NUM_ROWS * TILE_SIZE)
	while(isInsideMap(nextVertTouchX, nextVertTouchY))
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
			vertWallContent = getMapAt((int)floor(yToCheck / TILE_SIZE), (int)floor(xToCheck / TILE_SIZE));
			foundVertWallHit = true;
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
		rays[stripId].wasHitVertical = true;
		rays[stripId].rayAngle = rayAngle;
	}
	else
	{
		rays[stripId].distance = horzHitDistance;
		rays[stripId].wallHitX = horzWallHitX;
		rays[stripId].wallHitY = horzWallHitY;
		rays[stripId].wallHitContent = horzWallContent;
		rays[stripId].wasHitVertical = false;
		rays[stripId].rayAngle = rayAngle;
	}
}

void	castAllRays(void)
{
	float	rayAngle;
	float	distanceProjPlane;

	// calculate the distance of the player to the projection plane
	distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
	// loop all the pixels columns
	for (int col = 0; col < NUM_RAYS; col++)
	{
		// calculate the ray angle
		rayAngle = player.rotationAngle + atan((col - NUM_RAYS / 2) / distanceProjPlane);
		castRay(rayAngle, col);
	}
}

void	renderRays(void)
{
	// increment by 50, to draw only a few rays on the minimap
	for (int i = 0; i < NUM_RAYS; i += 50)
	{
		drawLine(
			player.x * MINIMAP_SCALE_FACTOR,
			player.y * MINIMAP_SCALE_FACTOR,
			rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
			rays[i].wallHitY * MINIMAP_SCALE_FACTOR,
			0xFF0000FF
		);
	}
}

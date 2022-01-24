#include "utils.h"
#include "defs.h"
#include <math.h>

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

#include "building.h"

#include "../util.h"

void building_init(building_t* b, kindOf_building_t* t, float x, float y)
{
	b->o.t = O_BUILDING;
	b->o.x = x;
	b->o.y = y;
	b->o.w = t->width;
	b->o.h = t->height;
	b->t = t;
}
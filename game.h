#ifndef GAME_H
#define GAME_H

typedef struct game game_t;

#include "universe/universe.h"
#include "world/world.h"
#include "graphics.h"
#include "overlay/overlay.h"

struct game
{
	universe_t* u;
	world_t*    w;
	graphics_t* g;
	overlay_t*  o;
};

void game_init(game_t* g);
void game_exit(game_t* g);
void game_loop(game_t* g);

#endif
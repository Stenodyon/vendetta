/*\
 *  Role playing, management and strategy game
 *  Copyright (C) 2013-2014 Quentin SANTOS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
\*/

#include "world.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "../util.h"
#include "../voronoi/lloyd.h"

world_t* world_init(universe_t* u, int _w, int _h)
{
	world_t* w = CALLOC(world_t, 1);
	w->universe = u;

	// BEGIN map generation
	w->tilesx = _w;
	w->tilesy = _h;
	w->terrain = CALLOC(short, w->tilesx*w->tilesy);

	// generate Voronoi diagram
	vr_diagram_t v;
	vr_diagram_init(&v, w->tilesx, w->tilesy);
	size_t n_vrPoints = w->tilesx * w->tilesy / 50;
	for (size_t i = 0; i < n_vrPoints; i++)
	{
		double x = frnd(0, w->tilesx);
		double y = frnd(0, w->tilesy);
		vr_diagram_point(&v, (point_t){x,y});
	}
	vr_lloyd_relaxation(&v);
	vr_lloyd_relaxation(&v);
	vr_diagram_end(&v);

	// assign land types to Voronoi regions
	static const float land_probas[] = {0.8, 0.05, 0.05, 0.04, 0.01, 0,0,0,0,0,0.05};
	short region_types[v.n_regions];
	for (size_t i = 0; i < v.n_regions; i++)
		region_types[i] = 16 * rnd_pick(land_probas);

	// rasterise map
	// TODO: clean that thing
	// yes, I know, this is the single most ugly and horrible
	// batch of code of this project; however, getting pixels
	// from a polygon is not totally trivial; to be cleaned later
	for (size_t i = 0; i < v.n_regions; i++)
	{
		short t = region_types[i];

		// set tiles in the i-th Voronoi region to proper type

		// first, get vertical bounds on the region
		vr_region_t* r = v.regions[i];
		int minj = w->tilesy;
		int maxj = 0;
		for (size_t k = 0; k < r->n_edges; k++)
		{
			segment_t* s = &r->edges[k]->s;
			if (s->a->y < minj) minj = floor(s->a->y);
			if (s->a->y > maxj) maxj = floor(s->a->y);
			if (s->b->y < minj) minj = floor(s->b->y);
			if (s->b->y > maxj) maxj = floor(s->b->y);
		}
		if (maxj >= w->tilesy)
			maxj = w->tilesy - 1;

		// then, consider each so-selected slide row
		for (int j = minj; j <= maxj; j++)
		{
			// find the portion of the row in the region
			int mini = w->tilesx;
			int maxi = 0;
			point_t a = {mini, j};
			point_t b = {maxi, j};
			segment_t s = {&a, &b};
			for (size_t k = 0; k < r->n_edges; k++)
			{
				point_t p;
				if (!segment_intersect(&p, &s, &r->edges[k]->s))
					continue;
				if (p.x < mini)
					mini = floor(p.x);
				if (p.x > maxi)
					maxi = floor(p.x);
			}
			if (maxi >= w->tilesx)
				maxi = w->tilesx - 1;

			// set this portion to proper type
			for (int i = mini; i <= maxi; i++)
				TERRAIN(w,i,j) = t;
		}
	}

	vr_diagram_exit(&v);
	// END map generation

	// BEGIN land type borders
#define LAND_TYPE(I,J) (TERRAIN(w,I,J) / 16)
#define LAND_SAME(I,J) ( \
	((I) < 0 || (I) >= w->tilesx || (J) < 0 || (J) >= w->tilesy) ? \
	1 : \
	t == LAND_TYPE(I,J) \
)
	static const char type2tile[16] = {0,5,2,13,4,7,12,8,3,15,6,11,14,9,10,1};
	for (int i = 0; i < w->tilesx; i++)
	for (int j = 0; j < w->tilesy; j++)
	{
		int t = LAND_TYPE(i,j);
		if (t == 0)
			continue;
		char top    = LAND_SAME(i,j-1);
		char right  = LAND_SAME(i+1,j);
		char bottom = LAND_SAME(i,j+1);
		char left   = LAND_SAME(i-1,j);
		int neighbor = (top<<3) | (right<<2) | (bottom<<1) | (left<<0);
		TERRAIN(w,i,j) += type2tile[neighbor];
	}
	// END land type borders

	float width  = w->tilesx * TILE_SIZE;
	float height = w->tilesy * TILE_SIZE;

	// BEGIN character generation
	w->n_characters = 5;
	w->characters = CALLOC(character_t, w->n_characters);
	for (size_t i = 0; i < w->n_characters; i++)
	{
		character_t* c = &w->characters[i];
		character_init(c, u, w);
		character_setPosition(c, cfrnd(width), cfrnd(height));
	}
	// END character generation

	// BEGIN mine generation
	w->n_mines = w->tilesx*w->tilesy / 400;
	if (w->n_mines < u->n_mines)
		w->n_mines = u->n_mines;
	w->mines = CALLOC(mine_t, w->n_mines);
	static const float mine_probas[] = {0.22,0.22,0.20,0.10,0.08,0.06,0.06,0.06};
	for (size_t i = 0; i < u->n_mines; i++) // ensure there is at least one of each
	{
		mine_t* m = &w->mines[i];
		int type = i;
		mine_init(m, &u->mines[type]);
		world_randMine(w, m);
	}
	for (size_t i = u->n_mines; i < w->n_mines; i++)
	{
		mine_t* m = &w->mines[i];
		int type = rnd_pick(mine_probas);
		mine_init(m, &u->mines[type]);
		world_randMine(w, m);
	}
	// END mine generation

	w->n_buildings = 0;
	w->a_buildings = 0;
	w->buildings = NULL;

	return w;
}

void world_exit(world_t* w)
{
	for (size_t i = 0; i < w->n_buildings; i++)
	{
		building_exit(w->buildings[i]);
		free(w->buildings[i]);
	}
	free(w->buildings);

	for (size_t i = 0; i < w->n_mines; i++)
		mine_exit(&w->mines[i]);
	free(w->mines);

	for (size_t i = 0; i < w->n_characters; i++)
		character_exit(&w->characters[i]);
	free(w->characters);

	free(w->terrain);
	free(w);
}

void world_randMine(world_t* w, mine_t* m)
{
	int t;
	do
	{
		m->o.x = cfrnd(w->tilesx * TILE_SIZE - 32);
		m->o.y = 16 + cfrnd(w->tilesy * TILE_SIZE - 32);
		t = world_landAt(w, m->o.x, m->o.y);

		for (size_t i = 0; i < w->n_mines && &w->mines[i] < m; i++)
			if (object_overlap(&w->mines[i].o, &m->o))
			{
				t = 4;
				break;
			}
	} while (t == 4 || t == 10);
}

void world_doRound(world_t* w, float duration)
{
	for (size_t i = 0; i < w->n_characters; i++)
		character_doRound(&w->characters[i], duration);
}

int world_landAt(world_t* w, float x, float y)
{
	int i = TERRAINI(w,x);
	int j = TERRAINJ(w,y);
	return TERRAIN(w,i,j) / 16;
}

object_t* world_objectAt(world_t* w, float x, float y)
{
	for (size_t i = 0; i < w->n_buildings; i++)
		if (object_isAt((object_t*) w->buildings[i], x, y))
			return (object_t*) w->buildings[i];

	for (size_t i = 0; i < w->n_mines; i++)
		if (object_isAt((object_t*) &w->mines[i], x, y))
			return (object_t*) &w->mines[i];

	for (size_t i = 0; i < w->n_characters; i++)
		if (object_isAt((object_t*) &w->characters[i], x, y))
			return (object_t*) &w->characters[i];

	return NULL;
}

mine_t* world_findMine(world_t* w, float x, float y, kindOf_mine_t* t)
{
	mine_t* ret = NULL;
	float min_d = -1;
	for (size_t i = 0; i < w->n_mines; i++)
	{
		mine_t* m = &w->mines[i];
		if (t == NULL || m->t == t)
		{
			float d = object_distance(&m->o, x, y);
			if (min_d < 0 || d < min_d)
			{
				ret = m;
				min_d = d;
			}
		}
	}
	return ret;
}

building_t* world_findBuilding(world_t* w, float x, float y, kindOf_building_t* t)
{
	building_t* ret = NULL;
	float min_d = -1;
	for (size_t i = 0; i < w->n_buildings; i++)
	{
		building_t* m = w->buildings[i];
		if (t == NULL || m->t == t)
		{
			float d = object_distance(&m->o, x, y);
			if (min_d < 0 || d < min_d)
			{
				ret = m;
				min_d = d;
			}
		}
	}
	return ret;
}

char world_canBuild(world_t* w, float x, float y, kindOf_building_t* b)
{
	int mini = TERRAINI(w, x-b->width/2);
	int maxi = TERRAINI(w, x+b->width/2);
	int minj = TERRAINJ(w, y-b->height);
	int maxj = TERRAINJ(w, y);
	for (int i = mini; i <= maxi; i++)
		for (int j = minj; j <= maxj; j++)
			if (TERRAIN(w,i,j) / 16 != 0)
				return 0;

	object_t o = {O_BUILDING, x, y, b->width, b->height};
	for (size_t i = 0; i < w->n_mines; i++)
		if (object_overlap(&w->mines[i].o, &o))
			return 0;

	for (size_t i = 0; i < w->n_buildings; i++)
		if (object_overlap(&w->buildings[i]->o, &o))
			return 0;

	return 1;
}

building_t* world_addBuilding(world_t* w, kindOf_building_t* t, float x, float y)
{
	if (w->n_buildings == w->a_buildings)
	{
		w->a_buildings = w->a_buildings ? 2*w->a_buildings : 1;
		w->buildings = CREALLOC(w->buildings, building_t*, w->a_buildings);
	}

	building_t* b = CALLOC(building_t, 1);
	w->buildings[w->n_buildings++] = b;
	building_init(b, t, x, y);
	return b;
}
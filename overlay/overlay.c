#include "overlay.h"

#include <stdlib.h>
#include <stdio.h>

#include "../util.h"

#define PANEL_N_COLS 3

overlay_t* overlay_init(void)
{
	overlay_t* o = CALLOC(overlay_t, 1);
	o->selectedBuilding = NULL;
	return o;
}

void overlay_exit(overlay_t* o)
{
	free(o);
}

void draw_buildPanel(game_t* g)
{
	static sfSprite* sprite = NULL;
	if (sprite == NULL)
	{
		int id = graphics_spriteForImg(g->g, "buildings.png");
		sprite = g->g->sprites[id];
	}

	sfVector2f pos = {0, 0};
	for (int i = 0; i < g->u->n_buildings; i++)
	{
		sfIntRect rect = {28*i, 28*1, 28, 28};
		sfSprite_setTextureRect(sprite, rect);

		sfSprite_setPosition(sprite, pos);

		pos.x += 28;
		if (pos.x >= 28*PANEL_N_COLS)
		{
			pos.x = 0;
			pos.y += 28;
		}

		sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
	}
}

void draw_cursor(game_t* g)
{
	sfVector2i posi = sfMouse_getPositionRenderWindow(g->g->render);
	sfIntRect rect = {0, 0, 24, 24};

	if (g->o->selectedBuilding != NULL)
	{
		rect.left = 4 * 24;

		int id = g->o->selectedBuilding->sprite;
		sfSprite* sprite = g->g->sprites[id];

		sfIntRect rect = sfSprite_getTextureRect(sprite);
		sfVector2f posf = {posi.x - rect.width/2, posi.y - rect.height/2};
		sfSprite_setPosition(sprite, posf);

		sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
	}

	static sfSprite* sprite = NULL;
	if (sprite == NULL)
	{
		int id = graphics_spriteForImg(g->g, "cursors.png");
		sprite = g->g->sprites[id];
	}

	sfSprite_setTextureRect(sprite, rect);

	sfVector2f posf = {posi.x, posi.y};
	sfSprite_setPosition(sprite, posf);
	sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
}

void draw_overlay(game_t* g)
{
	draw_buildPanel(g);
	draw_cursor(g);
}

int overlay_catch(game_t* g, float x, float y)
{
	int i = y / 28;
	int j = x / 28;
	int id = PANEL_N_COLS*i + j;
	if (j < PANEL_N_COLS && id < g->u->n_buildings)
	{
		g->o->selectedBuilding = &g->u->buildings[id];
		return 1;
	}

	return 0;
}

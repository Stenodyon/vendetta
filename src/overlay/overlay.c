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

#include "overlay.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "../mem.h"

overlay_t* overlay_init(graphics_t* g)
{
	overlay_t* o = CALLOC(overlay_t, 1);

	   ov_build_init(&o->build);
	 swbuilding_init(&o->swbuilding,  g);
	    switems_init(&o->switems,     g);
	swmaterials_init(&o->swmaterials, g);
	   swskills_init(&o->swskills,    g);
	swequipment_init(&o->swequipment, g);

	o->n_subwindows = 5;
	o->sw = CALLOC(subwindow_t*, o->n_subwindows);
	o->sw[0] = &o->swbuilding .w;
	o->sw[1] = &o->swmaterials.w;
	o->sw[2] = &o->swequipment.w;
	o->sw[3] = &o->swskills   .w;
	o->sw[4] = &o->switems    .w;

	o->selected = -1;

	return o;
}

void overlay_exit(overlay_t* o)
{
	free(o->sw);

	swequipment_exit(&o->swequipment);
	   swskills_exit(&o->swskills);
	    switems_exit(&o->switems);
	swmaterials_exit(&o->swmaterials);
	 swbuilding_exit(&o->swbuilding);
	   ov_build_exit(&o->build);

	free(o);
}

int overlay_cursor(overlay_t* o, game_t* g)
{
	int cursor = -1;
	int i = overlay_draw(o, g, 0);
	if (i >= 0)
	{
		char w = i % 3;
		i /= 3;
		if (w == 0) // buttons
		{
			char buffer[1024];
			snprintf(buffer, 1024, "Fenêtre: %s", o->sw[i]->name);
			graphics_drawTooltip(g->g, buffer);
		}
		else if (w == 1) // orders
		{
			char buffer[1024];
			size_t n = 1024;
			size_t cur = 0;
			if (i == 0)
			{
			}
			else if (i == 1)
			{
				cur += snprintf(buffer+cur, n-cur, "Rentrer à la maison");
			}
			else if (i <= 9)
			{
				i -= 2;
				kindOf_mine_t* t = &g->u->mines[i];
				cur += snprintf(buffer+cur, n-cur, "Aller à %s", t->name);
			}
			else if (i == 10)
			{
				cur += snprintf(buffer+cur, n-cur, "Attaquer ennemi proche");
			}
			else if (i == 11)
			{
				cur += snprintf(buffer+cur, n-cur, "Attaquer bâtiment proche");
			}
			if (cur != 0)
				graphics_drawTooltip(g->g, buffer);
		}
		else if (w == 2) // statuses
		{
		}
	}
	else if ((cursor =    ov_build_cursor(&o->build,       g)) >= 0);
	else if ((cursor =  swbuilding_cursor(&o->swbuilding,  g)) >= 0);
	else if ((cursor =     switems_cursor(&o->switems,     g)) >= 0);
	else if ((cursor = swmaterials_cursor(&o->swmaterials, g)) >= 0);
	else if ((cursor =    swskills_cursor(&o->swskills,    g)) >= 0);
	else if ((cursor = swequipment_cursor(&o->swequipment, g)) >= 0);
	else
	{
		sfVector2i imouse = sfMouse_getPosition((sfWindow*) g->g->render);
		sfVector2f pos = sfRenderWindow_mapPixelToCoords(g->g->render, imouse, g->g->world_view);
		object_t* o = world_objectAt(g->w, pos.x, pos.y, &g->player->o);
		if (o != NULL)
		{
			if (o->t == O_CHARACTER)
			{
				character_t* c = (character_t*) o;
				if (c != g->player)
					cursor = 9;

				if (c->ai != NULL)
				{
					graphics_drawTooltip(g->g, c->ai->name);
				}
			}
			else if (o->t == O_MINE)
			{
				cursor = 1;

				mine_t* m = (mine_t*) o;
				transform_t* tr = &m->t->harvest;

				if (!(tr->n_res == 0 || tr->res[0].is_item))
				{
					int id = tr->res[0].id;
					kindOf_material_t* t = &g->u->materials[id];
					int skill = t->skill;

					char buffer[1024];
					float amount = floor(g->player->inventory.materials[id]);
					float max = character_maxOf(g->player, t);
					snprintf(buffer, 1024, "%s (%.0f/%0.f)", g->u->skills[skill].name, amount, max);
					graphics_drawTooltip(g->g, buffer);
				}
			}
			else if (o->t == O_BUILDING)
			{
				building_t* b = (building_t*) o;
				if (b->owner != g->player)
					cursor = 9;
				else if (b->build_progress == 1)
					cursor = 7;
				else
					cursor = 4;

				graphics_drawTooltip(g->g, b->t->name);
			}
		}
	}

	return cursor < 0 ? 0 : cursor;
}

static int overlay_buttons(overlay_t* o, game_t* g, char do_draw)
{
	sfVector2f mouse = {0,0};
	if (!do_draw)
		mouse = overlay_mouse(g->g);

	static sfSprite* sprite = NULL;
	if (sprite == NULL)
	{
		int id = graphics_spriteForImg(g->g, "buttons.png");
		sprite = g->g->sprites[id];
	}
	for (size_t i = 0; i < o->n_subwindows; i++)
	{
		subwindow_t* w = o->sw[i];

		sfVector2u size = sfRenderWindow_getSize(g->g->render);
		sfSprite_setPosition(sprite, (sfVector2f){size.x - 200 + 24*i, 5});

		sfColor color = w->visible ? (sfColor){255,255,255,127} : sfWhite;
		sfSprite_setColor(sprite, color);

		sfIntRect rect = {24*i, 0, 24, 24};
		sfSprite_setTextureRect(sprite, rect);
		if (do_draw)
			sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
		else if (sfSprite_contains(sprite, mouse))
			return i;
	}
	return -1;
}
static int overlay_orders(overlay_t* o, game_t* g, char do_draw)
{
	(void) o;

	sfVector2f mouse = {0,0};
	if (!do_draw)
		mouse = overlay_mouse(g->g);

	static sfSprite* sprite = NULL;
	if (sprite == NULL)
	{
		int id = graphics_spriteForImg(g->g, "orders.png");
		sprite = g->g->sprites[id];
	}

	float x = 100;
	float y = 0;
	for (size_t i = 1; i < 12; i++)
	{
		sfVector2f pos = {x, y};
		sfSprite_setPosition(sprite, pos);

		sfIntRect rect = {28*i, 0, 28, 28};
		sfSprite_setTextureRect(sprite, rect);

		if (do_draw)
			sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
		else if (sfSprite_contains(sprite, mouse))
			return i;

		x += 28;
	}
	return -1;
}
static int overlay_statuses(overlay_t* o, game_t* g, char do_draw)
{
	(void) o;

	sfVector2f mouse = {0,0};
	if (!do_draw)
		mouse = overlay_mouse(g->g);

	static sfText* text = NULL;
	if (text == NULL)
	{
		text = sfText_create();
		sfColor col = {255, 255, 255, 255};
		sfText_setColor(text, col);
		sfText_setFont(text, g->g->font);
		sfText_setCharacterSize(text, 15);
	}

	sfVector2u size = sfRenderWindow_getSize(g->g->render);
	float x = 10;
	float y = size.y - 10 - 30 * N_STATUSES;
	for (int i = 0; i < N_STATUSES; i++)
	{
		if (!do_draw &&
		    x <= mouse.x && mouse.x <= x + 150 &&
		    y <= mouse.y && mouse.y <= y + 20)
			return i;

		float p = g->player->statuses[i] / 20;
		if (do_draw)
			graphics_drawProgressBar(g->g, x, y, 150, 20, p, g->autoEat[i]);

		sfText_setUTF8(text, g->u->statuses[i].name); // TODO
		sfVector2f pos = {x+5, y};
		sfText_setPosition(text, pos);
		if (do_draw)
			sfRenderWindow_drawText(g->g->render, text, NULL);

		char buffer[1024];
		float amount = floor(g->player->statuses[i]);
		snprintf(buffer, 1024, "%.0f/%.0f", amount, floor(20.));
		sfText_setUTF8(text, buffer);
		sfFloatRect rect = sfText_getLocalBounds(text);
		pos.x = x + 140 - rect.width;
		sfText_setPosition(text, pos);
		if (do_draw)
			sfRenderWindow_drawText(g->g->render, text, NULL);

		y += 30;
	}
	return -1;
}
int overlay_draw(overlay_t* o, game_t* g, char do_draw)
{
	int i = overlay_buttons(o, g, do_draw);
	if (i >= 0)
		return 3*i+0;

	i = overlay_orders(o, g, do_draw);
	if (i >= 0)
		return 3*i+1;

	i = overlay_statuses(o, g, do_draw);
	if (i >= 0)
		return 3*i+2;

	if (!do_draw)
		return -1;

	   ov_build_draw(&o->build,       g, 1);
	swequipment_draw(&o->swequipment, g, 1);
	   swskills_draw(&o->swskills,    g, 1);
	swmaterials_draw(&o->swmaterials, g, 1);
	    switems_draw(&o->switems,     g, 1);
	 swbuilding_draw(&o->swbuilding,  g, 1);

	static sfText* text = NULL;
	if (text == NULL)
	{
		text = sfText_create();
		sfColor col = {255, 255, 255, 255};
		sfText_setColor(text, col);
		sfText_setFont(text, g->g->font);
		sfText_setCharacterSize(text, 15);
	}

	sfVector2u size = sfRenderWindow_getSize(g->g->render);

	char buffer[1024];
	character_currentAction(g->player, buffer, 1024);
	sfText_setUTF8(text, buffer);
	sfText_setPosition(text, (sfVector2f){180, size.y - 30});
	sfRenderWindow_drawText(g->g->render, text, NULL);

	snprintf(buffer, 1024, "FPS: %.0f", floor(g->g->fps));
	sfText_setUTF8(text, buffer);
	sfText_setPosition(text, (sfVector2f){size.x - 80, size.y - 30});
	sfRenderWindow_drawText(g->g->render, text, NULL);

	return -1;
}

void overlay_move(overlay_t* o, game_t* g, int x, int y)
{
	(void) x;
	(void) y;

	sfVector2i imouse = sfMouse_getPosition((sfWindow*) g->g->render);
	sfVector2f mouse = sfRenderWindow_mapPixelToCoords(g->g->render, imouse, g->g->overlay_view);

	float dx = mouse.x - o->lastx;
	float dy = mouse.y - o->lasty;
	o->lastx = mouse.x;
	o->lasty = mouse.y;

	if (o->selected < 0)
		return;

	subwindow_t* w = o->sw[o->selected];
	w->x += dx;
	w->y += dy;
}

int overlay_catch(overlay_t* o, game_t* g, int x, int y, int t)
{
	int i = overlay_draw(o, g, 0);
	if (i >= 0)
	{
		char w = i % 3;
		i /= 3;
		if (w == 0) // buttons
		{
			if (t == sfMouseLeft)
				o->sw[i]->visible ^= 1;
		}
		else if (w == 1) // orders
		{
			if (i == 0)
			{
			}
			else if (i == 1)
			{
				if (g->player->hasBuilding != NULL)
					g->player->go_o = &g->player->hasBuilding->o;
			}
			else if (i <= 9)
			{
				i -= 2;
				kindOf_mine_t* t = &g->u->mines[i];
				mine_t* m = world_findMine(g->w, g->player->o.x, g->player->o.y, t);
				if (m != NULL)
					g->player->go_o = (object_t*) m;
			}
			else if (i == 10)
			{
				character_t* t = world_findEnnemyCharacter(g->w, g->player);
				if (t != NULL)
					g->player->go_o = &t->o;
			}
			else if (i == 11)
			{
				building_t* t = world_findEnnemyBuilding(g->w, g->player);
				if (t != NULL)
					g->player->go_o = &t->o;
			}
		}
		else if (w == 2) // statuses
		{
			if (t == sfMouseLeft)
			{
				character_eatFor(g->player, i);
			}
			else if (t == sfMouseRight)
			{
				g->autoEat[i] ^= 1;
			}
		}
		return 1;
	}

	if (t == -sfMouseRight-1)
	{
		for (size_t i = 0; i < o->n_subwindows; i++)
			if (subwindow_catch(o->sw[i], x, y, t))
			{
				o->selected = i;
				break;
			}
		return 1;
	}
	else if (t == sfMouseRight)
	{
		if (o->selected >= 0)
		{
			o->selected = -1;
			return 1;
		}
	}

	return
	 swbuilding_catch(&o->swbuilding,  g, x, y, t) ||
	    switems_catch(&o->switems,     g, x, y, t) ||
	swmaterials_catch(&o->swmaterials, g, x, y, t) ||
	   swskills_catch(&o->swskills,    g, x, y, t) ||
	swequipment_catch(&o->swequipment, g, x, y, t) ||
	   ov_build_catch(&o->build,       g, x, y, t) ||
	0;
}

int overlay_wheel(overlay_t* o, int x, int y, int d)
{
	for (size_t i = 0; i < o->n_subwindows; i++)
		if (subwindow_wheel(o->sw[i], x, y, d))
			return 1;
	return 0;
}

sfVector2f overlay_mouse(graphics_t* g)
{
	sfVector2i mouse = sfMouse_getPosition((sfWindow*) g->render);
	return sfRenderWindow_mapPixelToCoords(g->render, mouse, g->overlay_view);
}

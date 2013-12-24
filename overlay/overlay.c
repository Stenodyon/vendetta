#include "overlay.h"

#include <math.h>

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
	sfVector2f pos = {0, 0};
	for (int i = 0; i < g->u->n_buildings; i++)
	{
		kindOf_building_t* b = &g->u->buildings[i];

		if (b->button_sprite < 0)
			continue;

		sfSprite* sprite = g->g->sprites[b->button_sprite];

		int ok = components_check(&b->build_req, &g->player->inventory);

		sfIntRect rect = {28*b->button_index, 28*ok, 28, 28};
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

void draw_swSkills(game_t* g)
{
	sfText* text = NULL;
	if (text == NULL)
	{
		sfColor color = {255, 255, 255, 255};

		text = sfText_create();
		sfText_setFont         (text, g->g->font);
		sfText_setCharacterSize(text, 18);
		sfText_setColor        (text, color);
	}

	sfVector2f pos = {PANEL_N_COLS * 28 + 10, 310};
	wchar_t buffer[1024] = L"Skills";

	sfText_setPosition(text, pos);
	sfText_setUnicodeString(text, (sfUint32*) buffer);
	sfRenderWindow_drawText(g->g->render, text, NULL);

	for (int i = 0; i < N_SPECIAL_SKILLS; i++)
	{
		skill_t s = g->player->sskills[i];
		if (s != 1)
		{
			pos.y += 20;
			swprintf(buffer, 1024, L"%ls %i", g->u->sskills[i].name, (int)floor(s*100));

			sfText_setPosition(text, pos);
			sfText_setUnicodeString(text, (sfUint32*) buffer);
			sfRenderWindow_drawText(g->g->render, text, NULL);
		}
	}

	for (int i = 0; i < g->u->n_materials; i++)
	{
		skill_t s = g->player->mskills[i];
		if (s != 1)
		{
			pos.y += 20;
			swprintf(buffer, 1024, L"%ls %i", g->u->materials[i].skill.name, (int)floor(s*100));

			sfText_setPosition(text, pos);
			sfText_setUnicodeString(text, (sfUint32*) buffer);
			sfRenderWindow_drawText(g->g->render, text, NULL);
		}
	}
}

void draw_swInventory(game_t* g)
{
	sfText* text = NULL;
	if (text == NULL)
	{
		sfColor color = {255, 255, 255, 255};

		text = sfText_create();
		sfText_setFont         (text, g->g->font);
		sfText_setCharacterSize(text, 18);
		sfText_setColor        (text, color);
	}

	sfVector2f pos = {PANEL_N_COLS * 28 + 310, 10};

	for (int i = 0; i < g->u->n_materials; i++)
	{
		const wchar_t* name = g->u->materials[i].name;
		int amount = floor(g->player->inventory.materials[i]);

		if (amount == 0)
			continue;

		wchar_t buffer[1024];
		swprintf(buffer, 1024, L"%ls: %i", name, amount);

		sfText_setPosition(text, pos);
		sfText_setUnicodeString(text, (sfUint32*) buffer);
		sfRenderWindow_drawText(g->g->render, text, NULL);

		pos.y += 20;
	}

	pos.y += 20;

	for (int i = 0; i < g->u->n_items; i++)
	{
		wchar_t* name = g->u->items[i].name;
		int amount = g->player->inventory.items[i];

		if (amount == 0)
			continue;

		wchar_t buffer[1024];
		swprintf(buffer, 1024, L"%ls: %i", name, amount);

		sfText_setPosition(text, pos);
		sfText_setUnicodeString(text, (sfUint32*) buffer);
		sfRenderWindow_drawText(g->g->render, text, NULL);

		pos.y += 20;
	}
}

void draw_swBuilding(game_t* g)
{
	building_t* b = g->player->inBuilding;
	if (b == NULL)
		return;

	sfText* text = NULL;
	if (text == NULL)
	{
		sfColor color = {255, 255, 255, 255};

		text = sfText_create();
		sfText_setFont         (text, g->g->font);
		sfText_setCharacterSize(text, 18);
		sfText_setColor        (text, color);
	}

	sfVector2f pos = {PANEL_N_COLS * 28 + 10, 10};

	sfText_setPosition(text, pos);
	sfText_setUnicodeString(text, (sfUint32*) b->t->name);
	sfRenderWindow_drawText(g->g->render, text, NULL);

	kindOf_building_t* t = b->t;

	pos.y += 20;
	if (t->make_res.n != 0)
	{
		component_t* c = &t->make_res.c[0];
		if (c->is_item)
			exit(1);

		const wchar_t* action = t->make_req.n == 0 ? L"Harvest" : L"Transform to";
		const wchar_t* name   = g->u->materials[c->id].name;
		wchar_t buffer[1024];
		swprintf(buffer, 1024, L"%ls %ls", action, name);

		sfText_setPosition(text, pos);
		sfText_setUnicodeString(text, (sfUint32*) buffer);
		sfRenderWindow_drawText(g->g->render, text, NULL);
	}

	pos.y += 20;
	for (int i = 0; i < t->item_n; i++)
	{
		pos.y += 20;

		components_t* l = &t->item_res[i];
		if (l->n == 0)
			continue;

		component_t* c = &l->c[0];
		if (!c->is_item)
			exit(1);

		wchar_t* name = g->u->items[c->id].name;
		wchar_t buffer[1024];
		if (i == b->item_current)
			swprintf(buffer, 1024, L"%ls (%i %%)", name, (int) floor(100*b->item_progress));
		else
			swprintf(buffer, 1024, L"%ls", name);

		sfText_setPosition(text, pos);
		sfText_setUnicodeString(text, (sfUint32*) buffer);
		sfRenderWindow_drawText(g->g->render, text, NULL);
	}

//	sfText_destroy(text); // TODO
}

void draw_cursor(game_t* g)
{
	sfVector2i posi = sfMouse_getPositionRenderWindow(g->g->render);
	sfIntRect rect = {0, 0, 24, 24};

	kindOf_building_t* b = g->o->selectedBuilding;
	if (b != NULL)
	{
		rect.left = 4 * 24;

		sfSprite* sprite = g->g->sprites[b->sprite];
		sfIntRect rect = {0, b->height*(b->n_sprites-1), b->width, b->height};
		sfSprite_setTextureRect(sprite, rect);
		sfVector2f posf = {posi.x - b->width/2, posi.y - b->height/2};
		sfSprite_setPosition(sprite, posf);
		sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
	}
	else
	{
		sfVector2f pos = sfRenderWindow_mapPixelToCoords(g->g->render, posi, g->g->world_view);
		object_t* o = world_objectAt(g->w, pos.x, pos.y);
		if (o != NULL)
		{
			if (o->t == O_MINE)
				rect.left = 1 * 24;
			else if (o->t == O_BUILDING)
			{
				building_t* b = (building_t*) o;
				if (b->build_progress == 1)
					rect.left = 7 * 24;
				else
					rect.left = 4 * 24;
			}
		}
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
	draw_swInventory(g);
	draw_swBuilding(g);
	draw_swSkills(g);
	draw_cursor(g);
}

int overlay_catch(game_t* g, float x, float y)
{
	if (g->player->inBuilding != NULL)
	{
		building_t* b = g->player->inBuilding;
		kindOf_building_t* t = b->t;

		sfText* text = NULL;
		if (text == NULL)
		{
			text = sfText_create();
			sfText_setFont         (text, g->g->font);
			sfText_setCharacterSize(text, 18);
		}

		sfVector2f pos = {PANEL_N_COLS * 28 + 10, 50};
		for (int i = 0; i < t->item_n; i++)
		{
			pos.y += 20;

			components_t* l = &t->item_res[i];
			if (l->n == 0)
				continue;

			component_t* c = &l->c[0];
			if (!c->is_item)
				exit(1);

			wchar_t* name = g->u->items[c->id].name;
			sfText_setPosition(text, pos);
			sfText_setUnicodeString(text, (sfUint32*) name);

			sfFloatRect rect = sfText_getGlobalBounds(text);
			if (!sfFloatRect_contains(&rect, x, y))
				continue;

			if (components_check(&t->item_req[i], &g->player->inventory))
			{
				b->item_current = i;
				b->item_progress = 0;
			}

			return 1;
		}
	}

	sfFloatRect rect = {0, 0, 28, 28};
	for (int i = 0; i < g->u->n_buildings; i++)
	{
		kindOf_building_t* b = &g->u->buildings[i];

		if (b->button_sprite < 0)
			continue;

		if (sfFloatRect_contains(&rect, x, y))
		{
			if (components_check(&b->build_req, &g->player->inventory))
				g->o->selectedBuilding = b;
			return 1;
		}

		rect.left += 28;
		if (rect.left >= 28*PANEL_N_COLS)
		{
			rect.left = 0;
			rect.top += 28;
		}
	}

	kindOf_building_t* b = g->o->selectedBuilding;
	if (b != NULL)
	{
		if (!components_check(&b->build_req, &g->player->inventory))
			return 1;

		components_apply(&b->build_req, &g->player->inventory, -1);

		int id = b->sprite;
		sfSprite* sprite = g->g->sprites[id];

		sfVector2i pix = {x, y};
		sfVector2f pos = sfRenderWindow_mapPixelToCoords(g->g->render, pix, g->g->world_view);

		sfIntRect rect = sfSprite_getTextureRect(sprite);
		world_addBuilding(g->w, b, pos.x, pos.y + rect.height/2);

		g->o->selectedBuilding = NULL;
		return 1;
	}

	return 0;
}

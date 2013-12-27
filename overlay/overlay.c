#include "overlay.h"

#include <math.h>

#include "../util.h"

overlay_t* overlay_init(void)
{
	overlay_t* o = CALLOC(overlay_t, 1);

	swbuilding_init (&o->swbuilding);
	swinventory_init(&o->swinventory);
	swskills_init   (&o->swskills);
	swequipment_init(&o->swequipment);

	ov_build_init(&o->build);

	return o;
}

void overlay_exit(overlay_t* o)
{
	ov_build_exit(&o->build);

	swequipment_exit(&o->swequipment);
	swskills_exit   (&o->swskills);
	swinventory_exit(&o->swinventory);
	swbuilding_exit (&o->swbuilding);

	free(o);
}

void overlay_cursor(overlay_t* o, game_t* g)
{
	sfVector2i cursor = sfMouse_getPositionRenderWindow(g->g->render);
	sfIntRect rect = {0, 0, 24, 24};

	if (ov_build_cursor(&o->build, g, cursor.x, cursor.y))
	{
		rect.left = 4 * 24;
	}
	else if (swbuilding_cursor (&o->swbuilding,  g, cursor.x, cursor.y));
	else if (swinventory_cursor(&o->swinventory, g, cursor.x, cursor.y));
	else if (swskills_cursor   (&o->swskills,    g, cursor.x, cursor.y));
	else if (swequipment_cursor(&o->swequipment, g, cursor.x, cursor.y));
	else
	{
		sfVector2f pos = sfRenderWindow_mapPixelToCoords(g->g->render, cursor, g->g->world_view);
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

	sfSprite_setPosition(sprite, (sfVector2f){cursor.x, cursor.y});
	sfRenderWindow_drawSprite(g->g->render, sprite, NULL);
}

void overlay_draw(overlay_t* o, game_t* g)
{
	// statuses
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
	float y = size.y - 10 - 30 * N_STATUSES;;
	for (int i = 0; i < N_STATUSES; i++)
	{
		float p = g->player->statuses[i] / 20;
		graphics_drawProgressBar(g->g, x, y, 150, 20, p);

		sfText_setUnicodeString(text, (sfUint32*) g->u->statuses[i].name);
		sfVector2f pos = {x+5, y};
		sfText_setPosition(text, pos);
		sfRenderWindow_drawText(g->g->render, text, NULL);

		wchar_t buffer[1024];
		swprintf(buffer, 1024, L"%.0f/%.0f", floor(g->player->statuses[i]), floor(20.));
		sfText_setUnicodeString(text, (sfUint32*) buffer);
		sfFloatRect rect = sfText_getLocalBounds(text);
		pos.x = x + 140 - rect.width;
		sfText_setPosition(text, pos);
		sfRenderWindow_drawText(g->g->render, text, NULL);

		y += 30;
	}

	swbuilding_draw (&o->swbuilding,  g);
	swinventory_draw(&o->swinventory, g);
	swskills_draw   (&o->swskills,    g);
	swequipment_draw(&o->swequipment, g);
	ov_build_draw   (&o->build,       g);

	overlay_cursor(g->o, g);
}

int overlay_catch(overlay_t* o, game_t* g, float x, float y, int t)
{
	return
	ov_build_catch   (&o->build,       g, x, y, t) ||
	swbuilding_catch (&o->swbuilding,  g, x, y, t) ||
	swinventory_catch(&o->swinventory, g, x, y, t) ||
	swskills_catch   (&o->swskills,    g, x, y, t) ||
	swequipment_catch(&o->swequipment, g, x, y, t) ||
	0;
}

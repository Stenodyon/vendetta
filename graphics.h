#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SFML/Graphics.h>

typedef struct graphics graphics_t;

struct graphics
{
	sfRenderWindow* render;
	sfImage**       images;
	sfSprite**      sprites;
};

graphics_t* graphics_init(void);
void        graphics_exit(graphics_t* g);

#endif
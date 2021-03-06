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

#ifndef W_INVENTORY_H
#define W_INVENTORY_H

typedef struct inventory inventory_t;

struct inventory
{
	float money;
	float* materials;
	float* items;
};

#include <sys/types.h>

void inventory_init(inventory_t* inv, size_t n_materials, size_t n_items);
void inventory_exit(inventory_t* inv);

float inventory_get(inventory_t* inv, char is_item, int id);
void  inventory_add(inventory_t* inv, char is_item, int id, float amount);
void  inventory_mov(inventory_t* inv, char is_item, int id, float amount, inventory_t* from);
void  inventory_pay(inventory_t* inv,                       float amount, inventory_t* from);

#endif

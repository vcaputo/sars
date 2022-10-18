/*
 *  Copyright (C) 2020 - Vito Caputo - <vcaputo@pengaru.com>
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 3 as published
 *  by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SFX_H
#define _SFX_H

#include <SDL_mixer.h>

typedef struct sfx_t {
	Mix_Chunk	*baby_infected;
	Mix_Chunk	*baby_held;
	Mix_Chunk	*baby_rescued;
	Mix_Chunk	*adult_infected;
	Mix_Chunk	*adult_captivated;
} sfx_t;

extern sfx_t	sfx;

void sfx_init(void);
void sfx_play(Mix_Chunk *chunk);

#endif

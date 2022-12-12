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

typedef enum sfx_voice_t {
	SFX_VOICE_ADULT,
	SFX_VOICE_TV,
	SFX_VOICE_BABY,
} sfx_voice_t;

typedef struct sfx_sound_t {
	sfx_voice_t	voice;
	Mix_Chunk	*chunk;
} sfx_sound_t;

typedef struct sfx_t {
	sfx_sound_t	baby_infected;
	sfx_sound_t	baby_hatted;
	sfx_sound_t	baby_held;
	sfx_sound_t	baby_rescued;
	sfx_sound_t	adult_armsfull;
	sfx_sound_t	adult_infected;
	sfx_sound_t	adult_captivated;
	sfx_sound_t	adult_maga;
	sfx_sound_t	adult_maskhit;
	sfx_sound_t	adult_mine;
	sfx_sound_t	adult_unmasked;
	sfx_sound_t	tv_talk[10];
} sfx_t;

extern sfx_t	sfx;

void sfx_init(void);
void sfx_play(sfx_sound_t *sound);

#endif

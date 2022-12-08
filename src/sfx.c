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

#include <SDL_mixer.h>

#include "sfx.h"

sfx_t	sfx;


void sfx_init(void)
{
	sfx.baby_infected = Mix_LoadWAV("assets/baby-infected.wav");
	sfx.baby_hatted = Mix_LoadWAV("assets/baby-hatted.wav");
	sfx.baby_held = Mix_LoadWAV("assets/baby-held.wav");
	sfx.baby_rescued = Mix_LoadWAV("assets/baby-rescued.wav");
	sfx.adult_armsfull = Mix_LoadWAV("assets/adult-armsfull.wav");
	sfx.adult_infected = Mix_LoadWAV("assets/adult-infected.wav");
	sfx.adult_captivated = Mix_LoadWAV("assets/adult-captivated.wav");
	sfx.adult_maskhit = Mix_LoadWAV("assets/adult-maskhit.wav");
	sfx.adult_mine = Mix_LoadWAV("assets/adult-mine.wav");
	sfx.adult_unmasked = Mix_LoadWAV("assets/adult-unmasked.wav");
	sfx.tv_talk[0] = Mix_LoadWAV("assets/talk/0.wav");
	sfx.tv_talk[1] = Mix_LoadWAV("assets/talk/1.wav");
	sfx.tv_talk[2] = Mix_LoadWAV("assets/talk/2.wav");
	sfx.tv_talk[3] = Mix_LoadWAV("assets/talk/3.wav");
	sfx.tv_talk[4] = Mix_LoadWAV("assets/talk/4.wav");
	sfx.tv_talk[5] = Mix_LoadWAV("assets/talk/5.wav");
	sfx.tv_talk[6] = Mix_LoadWAV("assets/talk/6.wav");
	sfx.tv_talk[7] = Mix_LoadWAV("assets/talk/7.wav");
	sfx.tv_talk[8] = Mix_LoadWAV("assets/talk/8.wav");
	sfx.tv_talk[9] = Mix_LoadWAV("assets/talk/9.wav");
}


void sfx_play(Mix_Chunk *chunk)
{
	if (chunk)
		Mix_PlayChannel(-1, chunk, 0);
}

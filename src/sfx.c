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

#include <assert.h>
#include <SDL_mixer.h>

#include "sfx.h"

sfx_t	sfx;


static void sfx_load(sfx_voice_t voice, const char *path, sfx_sound_t *res_sound)
{
	assert(path);
	assert(res_sound);

	(*res_sound).voice = voice;
	(*res_sound).chunk = Mix_LoadWAV(path);
}


void sfx_init(void)
{
	sfx_load(SFX_VOICE_BABY, "assets/baby-infected.wav", &sfx.baby_infected);
	sfx_load(SFX_VOICE_BABY, "assets/baby-hatted.wav", &sfx.baby_hatted);
	sfx_load(SFX_VOICE_BABY, "assets/baby-held.wav", &sfx.baby_held);
	sfx_load(SFX_VOICE_BABY, "assets/baby-rescued.wav", &sfx.baby_rescued);
	Mix_GroupChannels(0, 4, SFX_VOICE_BABY);

	sfx_load(SFX_VOICE_ADULT, "assets/adult-armsfull.wav", &sfx.adult_armsfull);
	sfx_load(SFX_VOICE_ADULT, "assets/adult-infected.wav", &sfx.adult_infected);
	sfx_load(SFX_VOICE_ADULT, "assets/adult-captivated.wav", &sfx.adult_captivated);
	sfx_load(SFX_VOICE_ADULT, "assets/adult-maga.wav", &sfx.adult_maga);
	sfx_load(SFX_VOICE_ADULT, "assets/adult-maskhit.wav", &sfx.adult_maskhit);
	sfx_load(SFX_VOICE_ADULT, "assets/adult-mine.wav", &sfx.adult_mine);
	sfx_load(SFX_VOICE_ADULT, "assets/adult-unmasked.wav", &sfx.adult_unmasked);
	Mix_GroupChannel(4, SFX_VOICE_ADULT);

	sfx_load(SFX_VOICE_TV, "assets/talk/0.wav", &sfx.tv_talk[0]);
	sfx_load(SFX_VOICE_TV, "assets/talk/1.wav", &sfx.tv_talk[1]);
	sfx_load(SFX_VOICE_TV, "assets/talk/2.wav", &sfx.tv_talk[2]);
	sfx_load(SFX_VOICE_TV, "assets/talk/3.wav", &sfx.tv_talk[3]);
	sfx_load(SFX_VOICE_TV, "assets/talk/4.wav", &sfx.tv_talk[4]);
	sfx_load(SFX_VOICE_TV, "assets/talk/5.wav", &sfx.tv_talk[5]);
	sfx_load(SFX_VOICE_TV, "assets/talk/6.wav", &sfx.tv_talk[6]);
	sfx_load(SFX_VOICE_TV, "assets/talk/7.wav", &sfx.tv_talk[7]);
	sfx_load(SFX_VOICE_TV, "assets/talk/8.wav", &sfx.tv_talk[8]);
	sfx_load(SFX_VOICE_TV, "assets/talk/9.wav", &sfx.tv_talk[9]);
	Mix_GroupChannel(5, SFX_VOICE_TV);
}


void sfx_play(sfx_sound_t *sound)
{
	assert(sound);

	if (sound->chunk) {
		int	channel;

		channel = Mix_GroupAvailable(sound->voice);
		if (channel < 0)
			channel = Mix_GroupOldest(sound->voice);

		Mix_PlayChannel(channel, sound->chunk, 0);
	}
}

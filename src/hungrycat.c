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

#include <SDL.h>
#include <assert.h>

#include <play.h>
#include <stage.h>

#include "glad.h"
#include "hungrycat-node.h"
#include "m4f.h"
#include "m4f-3dx.h"
#include "macros.h"
#include "sars.h"

#define HUNGRYCAT_FADE_MS	3333

#define HUNGRYCAT_FADEIN_MS	HUNGRYCAT_FADE_MS
#define HUNGRYCAT_SHOW_MS	HUNGRYCAT_FADE_MS
#define HUNGRYCAT_FADEOUT_MS	HUNGRYCAT_FADE_MS

#define HUNGRYCAT_FADE_TIMER	PLAY_TICKS_TIMER0

typedef enum hungrycat_state_t {
	HUNGRYCAT_STATE_FADEIN,
	HUNGRYCAT_STATE_SHOW,
	HUNGRYCAT_STATE_FADEOUT,
	HUNGRYCAT_STATE_LEAVE,
} hungrycat_state_t;

typedef struct hungrycat_t {
	hungrycat_state_t	state;
	stage_t			*node;
	m4f_t			model_x;
} hungrycat_t;


static void * hungrycat_init(play_t *play, int argc, char *argv[])
{
	sars_t		*sars = play_context(play, SARS_CONTEXT_SARS);
	hungrycat_t	*hungrycat;

	hungrycat = calloc(1, sizeof(hungrycat_t));
	fatal_if(!hungrycat, "Unable to allocate hungrycat_t");

	hungrycat->node = hungrycat_node_new(&(stage_conf_t){ .parent = sars->stage, .name = "hungrycat", .active = 1 }, &hungrycat->model_x);
	fatal_if(!hungrycat->node, "Unable to create hungrycat->node");

	hungrycat->model_x = m4f_identity();
	hungrycat->model_x = m4f_scale(&hungrycat->model_x, &(v3f_t){1.f, .25f, .5f});

	return hungrycat;
}


static void hungrycat_enter(play_t *play, void *context)
{
	hungrycat_t	*hungrycat = context;

	assert(hungrycat);

	play_music_set(play, 0, "assets/hungrycat.ogg");
	play_ticks_reset(play, HUNGRYCAT_FADE_TIMER);
}


static float fade_t(play_t *play)
{
	return (1.f / (float)HUNGRYCAT_FADE_MS) * (float)play_ticks(play, HUNGRYCAT_FADE_TIMER);
}


static void hungrycat_update(play_t *play, void *context)
{
	hungrycat_t	*hungrycat = context;

	assert(hungrycat);

	switch (hungrycat->state) {
	case HUNGRYCAT_STATE_FADEIN:
		stage_dirty(hungrycat->node);
		if (!play_ticks_elapsed(play, HUNGRYCAT_FADE_TIMER, HUNGRYCAT_FADE_MS)) {
			stage_set_alpha(hungrycat->node, fade_t(play));
			break;
		}

		stage_set_alpha(hungrycat->node, 1.f);
		hungrycat->state = HUNGRYCAT_STATE_SHOW;
		break;

	case HUNGRYCAT_STATE_SHOW:
		if (!play_ticks_elapsed(play, HUNGRYCAT_FADE_TIMER, HUNGRYCAT_FADE_MS))
			break;

		hungrycat->state = HUNGRYCAT_STATE_FADEOUT;
		break;

	case HUNGRYCAT_STATE_FADEOUT:
		stage_dirty(hungrycat->node);
		if (!play_ticks_elapsed(play, HUNGRYCAT_FADE_TIMER, HUNGRYCAT_FADE_MS)) {
			stage_set_alpha(hungrycat->node, 1.f - fade_t(play));
			break;
		}

		stage_set_alpha(hungrycat->node, 0.f);
		hungrycat->state = HUNGRYCAT_STATE_LEAVE;
		break;

	case HUNGRYCAT_STATE_LEAVE:
		play_context_enter(play, SARS_CONTEXT_GAME);
		break;

	default:
		assert(0);
	}
}


static void hungrycat_leave(play_t *play, void *context)
{
	hungrycat_t	*hungrycat = context;

	assert(hungrycat);

	/* we never reenter this context since it's just a splash, so
	 * the context leave is effectively the shutdown, cleanup.
	 */
	stage_free(hungrycat->node);
	free(hungrycat);
	/* XXX: this is icky though, play_context(SARS_CONTEXT_HUNGRYCAT) will
	 * return a dangling pointer!  Not that it occurs anywhere though.
	 */
}


const play_ops_t	hungrycat_ops = {
	.init = hungrycat_init,
	.update = hungrycat_update,
	.render = sars_render,
	.dispatch = sars_dispatch,
	.enter = hungrycat_enter,
	.leave = hungrycat_leave,
};

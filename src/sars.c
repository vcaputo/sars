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

#include <play.h>
#include <stage.h>

#include "clear-node.h"
#include "glad.h"
#include "m4f-3dx.h"
#include "macros.h"
#include "sars.h"

#define SARS_DEFAULT_WIDTH	800
#define SARS_DEFAULT_HEIGHT	600
#define SARS_WINDOW_FLAGS	(SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI)


void sars_canvas_size(sars_t *sars, int *res_width, int *res_height)
{
	assert(sars);
	assert(res_width);
	assert(res_height);

	SDL_GL_GetDrawableSize(sars->window, res_width, res_height);
}


void sars_canvas_to_ndc(sars_t *sars, int x, int y, float *res_x, float *res_y)
{
	int	w, h;

	assert(sars);
	assert(res_x);
	assert(res_y);

	sars_canvas_size(sars, &w, &h);

	*res_x = (float)x / (float)w * 2.f - 1.f;
	*res_y = (1.f - (float)y / (float)h) * 2.f - 1.f;
}


void sars_canvas_from_ndc(sars_t *sars, float x, float y, int *res_x, int *res_y)
{
	int	w, h;

	assert(sars);
	assert(res_x);
	assert(res_y);

	sars_canvas_size(sars, &w, &h);

	*res_x = roundf(x * (float)w * .5f + .5f * (float)w);
	*res_y = roundf(y * (float)h * .5f + .5f * (float)h);
}


void sars_viewport_size(sars_t *sars, int *res_width, int *res_height)
{
	assert(sars);
	assert(res_width);
	assert(res_height);

	SDL_GetWindowSize(sars->window, res_width, res_height);
}


void sars_viewport_to_ndc(sars_t *sars, int x, int y, float *res_x, float *res_y)
{
	int	w, h;

	assert(sars);
	assert(res_x);
	assert(res_y);

	sars_viewport_size(sars, &w, &h);

	*res_x = (float)x / (float)w * 2.f - 1.f;
	*res_y = (1.f - (float)y / (float)h) * 2.f - 1.f;
}


void sars_viewport_from_ndc(sars_t *sars, float x, float y, int *res_x, int *res_y)
{
	int	w, h;

	assert(sars);
	assert(res_x);
	assert(res_y);

	sars_viewport_size(sars, &w, &h);

	*res_x = roundf(x * (float)w * .5f + .5f * (float)w);
	*res_y = roundf(y * (float)h * .5f + .5f * (float)h);
}


uint32_t sars_viewport_id(sars_t *sars)
{
	return SDL_GetWindowID(sars->window);
}


/* produce a boxed transformation matrix for the current screen/window dimensions */
static m4f_t sars_boxed_projection_x(sars_t *sars)
{
	static const float	desired_ratio = (float)SARS_DEFAULT_WIDTH / (float)SARS_DEFAULT_HEIGHT;
	v3f_t			scale = { .x = 1.f, .y = 1.f, .z = 1.f };
	float			display_ratio;
	int			w, h;

	sars_viewport_size(sars, &w, &h);
	assert(w > 0 && h > 0);

	display_ratio = (float)w / (float)h;

	if (desired_ratio <= display_ratio) {
		/* display is equal to or wider than desired, "pillarbox" / X-padded */
		scale.x = (float)h * desired_ratio / (float)w;
	} else {
		/* display is taller than desired, "letterbox" / Y-padded */
		scale.y = (float)w * (1.f / desired_ratio) / (float)h;
	}

	return m4f_scale(NULL, &scale);
}


static void sars_update_projection_x(sars_t *sars)
{
	assert(sars);

	if (sars->winmode == SARS_WINMODE_FILLSCREEN)
		sars->projection_x = m4f_identity();
	else
		sars->projection_x = sars_boxed_projection_x(sars);
}


sars_winmode_t sars_winmode_set(sars_t *sars, sars_winmode_t winmode)
{
	assert(sars);

	if (sars->winmode == winmode)
		return sars->winmode;

	switch (winmode) {
	case SARS_WINMODE_WINDOW:
		if (SDL_SetWindowFullscreen(sars->window, 0))
			return sars->winmode;
		break;

	case SARS_WINMODE_FULLSCREEN:
		if (sars->winmode == SARS_WINMODE_WINDOW) {
			if (SDL_SetWindowFullscreen(sars->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
				return sars->winmode;
		}
		break;

	case SARS_WINMODE_FILLSCREEN:
		if (sars->winmode == SARS_WINMODE_WINDOW) {
			if (SDL_SetWindowFullscreen(sars->window, SDL_WINDOW_FULLSCREEN_DESKTOP))
				return sars->winmode;
		}
		break;

	default:
		assert(0);
	}

	sars->winmode = winmode;

	sars_update_projection_x(sars);

	return sars->winmode;
}


static void * sars_init(play_t *play, int argc, char *argv[])
{
	sars_t	*sars;

	sars = calloc(1, sizeof(sars_t));
	fatal_if(!sars, "Unable to allocate sars_t");

	sars->stage = stage_new(&(stage_conf_t){.name = "sars", .active = 1, .alpha = 1.f}, NULL, NULL);
	fatal_if(!sars->stage, "Unable to create new stage");

	(void) clear_node_new(&(stage_conf_t){.parent = sars->stage, .name = "gl-clear-sars", .active = 1});

	sars->window_width = SARS_DEFAULT_WIDTH;
	sars->window_height = SARS_DEFAULT_HEIGHT;

	if (argc > 1) {
		/* for now just support --window [WxH] */
		if (!strcasecmp(argv[1], "--window")) {
			sars->winmode = SARS_WINMODE_WINDOW; /* this is kinda redundant since we default to windowed now */

			if (argc > 2)
				sscanf(argv[2], "%ux%u", &sars->window_width, &sars->window_height);
		}
		/* TODO: add --fullscreen? */
	}

	fatal_if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY) < 0,
		"Unable to set GL core profile attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2) < 0,
		"Unable to set GL major version attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) < 0,
		"Unable to set GL minor version attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0,
		"Unable to set GL doublebuffer attribute");

	fatal_if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) < 0,
		"Unable to set GL red size attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) < 0,
		"Unable to set GL green size attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) < 0,
		"Unable to set GL blue size attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8) < 0,	/* this in particular is required for cache-node.c to work w/alpha */
		"Unable to set GL alpha size attribute");

//#define MSAA_RENDER_TARGET
#ifdef MSAA_RENDER_TARGET
	fatal_if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4) < 0,
		"Unable to et GL multisample samples attribute");
	fatal_if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) < 0,
		"Unable to set GL multisample buffers attribute");
#endif

/* On older SDL versions, this define is missing at compile-time,
 * but we can just provide it here and still try the sethint call,
 * just in case the runtime SDL version knows how to handle it.
 */
#ifndef SDL_HINT_TOUCH_MOUSE_EVENTS
#define SDL_HINT_TOUCH_MOUSE_EVENTS "SDL_TOUCH_MOUSE_EVENTS"
#endif

	warn_if(!SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0"),
		"Unable to suppress synthetic mouse events on touch");

	sars->window = SDL_CreateWindow("sars",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				sars->window_width, sars->window_height,
				SARS_WINDOW_FLAGS | (sars->winmode == SARS_WINMODE_WINDOW ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP));

	if (!sars->window) {
		fatal_if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0) < 0,
			"Unable to clear GL multisample samples attribute");
		fatal_if(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0) < 0,
			"Unable to clear GL multisample buffers attribute");

		sars->window = SDL_CreateWindow("sars",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					sars->window_width, sars->window_height,
					SARS_WINDOW_FLAGS | (sars->winmode == SARS_WINMODE_WINDOW ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP));

		fatal_if(!sars->window,
			"Unable to create SDL window");
	}

	sars->gl = SDL_GL_CreateContext(sars->window);
	fatal_if(!sars->gl,
		"Unable to create GL context");

	fatal_if(SDL_GL_SetSwapInterval(1) < 0,
		"Unable to enable vsync");

	fatal_if(!gladLoadGL(),
		"Failed to initialize GLAD OpenGL extension loader");

	//This seems unnecessary now that the game grabs the mouse,
	//and it's undesirable with clickable UI elements outside the
	//gameplay - otherwise I'd have to draw a pointer.
	//SDL_ShowCursor(SDL_DISABLE);

#ifdef MSAA_RENDER_TARGET
	glEnable(GL_MULTISAMPLE);
#endif

	sars_update_projection_x(sars);

	return sars;
}


static void sars_update(play_t *play, void *context)
{
	play_context_enter(play, SARS_CONTEXT_HUNGRYCAT);
}


/* XXX: note render and dispatch are public and ignore the passed-in context,
 * so other contexts can use these as-is for convenience */
void sars_render(play_t *play, void *context)
{
	sars_t	*sars = play_context(play, SARS_CONTEXT_SARS);

	if (stage_render(sars->stage, play)) {
		SDL_GL_SwapWindow(sars->window);
	} else {
		SDL_Delay(100);	// FIXME: this should be computed
	}
}


void sars_dispatch(play_t *play, void *context, SDL_Event *event)
{
	sars_t	*sars = play_context(play, SARS_CONTEXT_SARS);

	if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
		int	w, h;

		/* on highdpi the window size and pixels are decoupled, so ignore what
		 * the event contains and query the canvas size.
		 */
		sars_canvas_size(sars, &w, &h);
		glViewport(0, 0, w, h);
		sars_update_projection_x(sars);
		stage_dirty(sars->stage);
	}

	/* cycle fullscreen/windowed winmodes */
	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_f)
		sars_winmode_set(sars, (sars->winmode + 1) % SARS_WINMODE_CNT);
}


const play_ops_t	sars_ops = {
	.init = sars_init,
	.update = sars_update,
//	.shutdown = sars_shutdown,
};

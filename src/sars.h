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

#ifndef _SARS_H
#define _SARS_H

#include <SDL.h>

#include <stage.h>

#include "m4f.h"

typedef enum sars_context_t {
	SARS_CONTEXT_SARS,
	SARS_CONTEXT_HUNGRYCAT,
	SARS_CONTEXT_GAME,
	SARS_CONTEXT_CNT
} sars_context_t;

typedef struct sars_t {
	SDL_Window	*window;
	SDL_GLContext	*gl;
	stage_t		*stage;
	unsigned	window_width, window_height;
	unsigned	windowed:1;
} sars_t;

void sars_canvas_size(sars_t *sars, int *res_width, int *res_height);
void sars_canvas_to_ndc(sars_t *sars, int x, int y, float *res_x, float *res_y);
void sars_canvas_from_ndc(sars_t *sars, float x, float y, int *res_x, int *res_y);
void sars_viewport_size(sars_t *sars, int *res_width, int *res_height);
void sars_viewport_to_ndc(sars_t *sars, int x, int y, float *res_x, float *res_y);
void sars_viewport_from_ndc(sars_t *sars, float x, float y, int *res_x, int *res_y);
uint32_t sars_viewport_id(sars_t *sars);
void sars_toggle_fullscreen(sars_t *sars);
void sars_render(play_t *play, void *context);
void sars_dispatch(play_t *play, void *context, SDL_Event *event);

#endif

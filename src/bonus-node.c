/*
 *  Copyright (C) 2022 - Vito Caputo - <vcaputo@pengaru.com>
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

/* Bonus nodes are for showing bonus quantities numerically which move along
 * with an entity, and when taken get released from the entity's position and
 * float away on their own.
 *
 * The current way this works is by providing a release counter pointer for
 * triggering the release, along with a release position pointer for storing
 * the last position when released.  So the entity needs some members for saving
 * these locations which point at members within the bonus_node_t.
 *
 * The release gets realised at render time by checking if the release counter
 * is non-zero.  Upon release, the render func takes over management of the
 * position and life-cycle of the node by returning STAGE_RENDER_FUNC_RET_FREE
 * when done.
 *
 * One particularly crufty aspect is the digit nodes are hung off a nested
 * stage within the bonus_node_t that's being explicitly rendered and freed,
 * rather than existing as direct descendants of the game stage.  This results
 * in some redundant stage_dirty()/stage_render()/stage_free() rigamarole here,
 * and is only being done because I want to vary the alpha in the render func
 * for fading the digits out as they decay - and the incoming stage_t on the
 * render func is a const so we can't set the alpha directly on the outer bonus
 * node before letting render naturally descend down to the children layers with
 * the adjusted alpha.
 *
 * It's no big deal for now, but it illustrates some of the unnecessary
 * friction remaining in the libstage API.  I'm not convinced constifying these
 * things is bringing enough value to justify these kinds of annoyances.  FIXME
 *
 * Another crufty point is for now I threw the release decay duration in
 * bonus-node.h so that the caller can accesss it when storing at the release
 * pointer, while making it available here for compile-time evaluation of (1.f
 * / BONUS_NODE_RELEASE_MS).  Also the counter isn't really applied in
 * normalized time, but is just a frame counter right now - the name lies.
 * TODO FIXME
 *
 * I'm not too concerned about these things since SARS is such a
 * silly thing, but it's a useful exercise to see how (un)usable these
 * libraries are.
 */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stage.h>

#include "bonus-node.h"
#include "digit-node.h"
#include "m4f.h"
#include "m4f-3dx.h"
#include "v2f.h"

typedef struct bonus_node_t {
	stage_t		*stage;
	v2f_t		*position;
	float		scale;
	unsigned	release;
	v2f_t		release_position;
	unsigned	n_digits;
	m4f_t		digits_x[];
} bonus_node_t;

static stage_render_func_ret_t bonus_node_render(const stage_t *stage, void *object, float alpha, void *render_ctxt)
{
	bonus_node_t	*bonus_node = object;
	v3f_t		scale;

	assert(stage);
	assert(bonus_node);

	scale.x = scale.y = scale.z = bonus_node->scale;

	if (bonus_node->release) {
		float	t = 1.f - (1.f / BONUS_NODE_RELEASE_MS * (float)bonus_node->release);

		bonus_node->release--;
		if (!bonus_node->release)
			return STAGE_RENDER_FUNC_RET_FREE;

		for (int i = 0; i < bonus_node->n_digits; i++) {
			bonus_node->digits_x[i] = m4f_translate(NULL, &(v3f_t){
								.x = bonus_node->release_position.x + bonus_node->scale*2.f + (float)i * -bonus_node->scale*2.f + (t * sinf((float)bonus_node->release * .1f) * .1f),
								.y = bonus_node->release_position.y + bonus_node->scale*2.f + t * .5f,
								.z = 0.f
							});
			bonus_node->digits_x[i] = m4f_scale(&bonus_node->digits_x[i], &scale);
		}

		stage_set_alpha(bonus_node->stage, 1.f - t);
		stage_dirty(bonus_node->stage);
		stage_render(bonus_node->stage, render_ctxt);

		return STAGE_RENDER_FUNC_RET_CONTINUE;
	}

	/* update tranforms for the digits before they render */
	for (int i = 0; i < bonus_node->n_digits; i++) {
		bonus_node->digits_x[i] = m4f_translate(NULL, &(v3f_t){
								.x = bonus_node->position->x + bonus_node->scale * 2.f + (float)i * -bonus_node->scale * 2.f,
								.y = bonus_node->position->y + bonus_node->scale*2.f,
								.z = 0.f
							});
		bonus_node->digits_x[i] = m4f_scale(&bonus_node->digits_x[i], &scale);
	}
	stage_dirty(bonus_node->stage);
	stage_render(bonus_node->stage, render_ctxt);

	return STAGE_RENDER_FUNC_RET_CONTINUE;
}


static void bonus_node_free(const stage_t *stage, void *object)
{
	bonus_node_t	*bonus_node = object;

	assert(stage);
	stage_free(bonus_node->stage);
	free(bonus_node);
}


static const stage_ops_t bonus_node_ops = {
	.render_func = bonus_node_render,
	.free_func = bonus_node_free,
};


stage_t * bonus_node_new(stage_conf_t *conf, unsigned value, m4f_t *projection_x, v2f_t *position, float scale, unsigned **release, v2f_t **release_position)
{
	unsigned	v = value, n_digits = 0, i = 0;
	bonus_node_t	*bonus_node;
	stage_t		*s;

	assert(release);
	assert(position);
	assert(release_position);
	assert(projection_x);

	do {
		n_digits++;
		v /= 10;
	} while (v);

	bonus_node = calloc(1, sizeof(bonus_node_t) + n_digits * sizeof(bonus_node->digits_x[0]));
	assert(bonus_node);
	bonus_node->n_digits = n_digits;
	bonus_node->position = position;
	bonus_node->scale = scale;
	bonus_node->stage = stage_new(&(stage_conf_t){.name = "bonus-container", .active = 1, .alpha = 1.f}, NULL, NULL); /* use a discrete container stage for render_func alpha control */
	*release = &bonus_node->release;
	*release_position = &bonus_node->release_position;

	s = stage_new(conf, &bonus_node_ops, bonus_node);
	assert(s);

	v = value;
	do {
		/* TODO: model_x positioning of the digits */
		(void) digit_node_new(&(stage_conf_t){ .parent = bonus_node->stage, .name = "bonus-digit", .active = 1, .alpha = 1.f },
			v % 10, projection_x, &bonus_node->digits_x[i++]);
		v /= 10;
	} while (v);

	return s;
}

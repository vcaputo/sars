/*
 *  Copyright (C) 2018-2020 - Vito Caputo - <vcaputo@pengaru.com>
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

/* TODO: this could totally be built upon shader-node.[ch], it would just supply
 * shaders that do texturing and a uniform func that binds in the texture...
 */

#include <assert.h>
#include <stdlib.h>

#include <stage.h>

#include "glad.h"
#include "m4f.h"
#include "macros.h"
#include "shader.h"
#include "tex-node.h"
#include "tex.h"

typedef struct tex_node_t {
	tex_t		*tex;
	m4f_t		*projection_x;
	m4f_t		*model_x;
} tex_node_t;


/* Render simply renders a texd texture onto the screen */
static void tex_node_render(const stage_t *stage, void *object, float alpha, void *render_ctxt)
{
	tex_node_t	*tex_node = object;

	assert(stage);
	assert(tex_node);

	tex_render(tex_node->tex, alpha, tex_node->projection_x, tex_node->model_x);
}


static void tex_node_free(const stage_t *stage, void *object)
{
	tex_node_t	*tex_node = object;

	assert(stage);
	assert(tex_node);

	tex_free(tex_node->tex);
}

static const stage_ops_t tex_node_ops = {
	.render_func = tex_node_render,
	.free_func = tex_node_free,
};


/* retun a tex node from a reusable refcounted tex instance */
stage_t * tex_node_new_tex(stage_conf_t *conf, tex_t *tex, m4f_t *projection_x, m4f_t *model_x)
{
	tex_node_t	*tex_node;
	stage_t		*s;

	assert(conf);

	tex_node = calloc(1, sizeof(tex_node_t));
	fatal_if(!tex_node, "Unable to allocate tex_node \"%s\"", conf->name);

	s = stage_new(conf, &tex_node_ops, tex_node);
	fatal_if(!s, "Unable to create stage \"%s\"", conf->name);

	tex_node->tex = tex_ref(tex);
	tex_node->projection_x = projection_x;
	tex_node->model_x = model_x;

	return s;

}


/* return a tex node from a pix array
 * the pixels are used in-place and no duplicate is made.
 */
stage_t * tex_node_new_mem(stage_conf_t *conf, int width, int height, const unsigned char *buf, m4f_t *projection_x, m4f_t *model_x)
{
	tex_t	*tex = tex_new(width, height, buf);
	stage_t	*stage = tex_node_new_tex(conf, tex_new(width, height, buf), projection_x, model_x);

	tex_free(tex);

	return stage;
}

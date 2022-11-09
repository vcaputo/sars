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

#include <assert.h>
#include <stdlib.h>

#include <stage.h>

#include "glad.h"
#include "m4f.h"
#include "macros.h"
#include "shader.h"
#include "shader-node.h"
#include "v2f.h"

typedef struct shader_node_t {
	shader_t			*shader;
	shader_node_uniforms_func_t	*uniforms_func;
	void				*uniforms_ctxt;
	const m4f_t			*transform;
} shader_node_t;

static unsigned	vbo, tcbo;

static const float	vertices[] = {
	+1.f, +1.f, 0.f,
	+1.f, -1.f, 0.f,
	-1.f, +1.f, 0.f,
	+1.f, -1.f, 0.f,
	-1.f, -1.f, 0.f,
	-1.f, +1.f, 0.f,
};


/* TODO: verify that this is OK, I recall tutorials stating these are always
 * in the range 0-1, but it seems perfectly OK to use -1..+1 which is more
 * convenient here where these shader-textured quads appreciate being fed 
 * unit square coordinates with 0,0 @ the center.
 */
static const float	texcoords[] = {
	1.f, 1.f,
	1.f, -1.f,
	-1.f, 1.f,
	1.f, -1.f,
	-1.f, -1.f,
	-1.f, 1.f,
};


static stage_render_func_ret_t shader_node_render(const stage_t *stage, void *object, float alpha, void *render_ctxt)
{
	shader_node_t	*shader_node = object;
	unsigned	n_uniforms;
	int		*uniforms, *attributes;

	assert(stage);
	assert(shader_node);

	shader_use(shader_node->shader, &n_uniforms, &uniforms, NULL, &attributes);

	if (shader_node->uniforms_func)
		shader_node->uniforms_func(shader_node->uniforms_ctxt, render_ctxt, n_uniforms, uniforms, shader_node->transform, alpha);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attributes[0], 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(attributes[0]);

	glBindBuffer(GL_ARRAY_BUFFER, tcbo);
	glVertexAttribPointer(attributes[1], 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(attributes[1]);

	/* XXX: this could be made optional, but since alpha is a constant throughout the stage
	 * integration I'm just always turning it on so stage_set_alpha() always works.  There
	 * are definitely full-screen full-opaque shader node situations where the pointless
	 * performance hit sucks though, especially on older hardware.
	 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return STAGE_RENDER_FUNC_RET_CONTINUE;
}


static void shader_node_free(const stage_t *stage, void *object)
{
	shader_node_t	*shader_node = object;

	assert(stage);
	assert(shader_node);

	/* XXX FIXME: hmm, maybe the caller should supply a shader_t ** instead */
	(void) shader_free(shader_node->shader);
	free(shader_node);
}


static const stage_ops_t shader_node_ops = {
	.render_func = shader_node_render,
	.free_func = shader_node_free,
};


/* return a new shader stage node from an already compiled and linked shader program */
stage_t * shader_node_new_shader(const stage_conf_t *conf, shader_t *shader, const m4f_t *transform, shader_node_uniforms_func_t *uniforms_func, void *uniforms_ctxt)
{
	shader_node_t	*shader_node;
	stage_t		*stage;

	assert(conf);
	assert(shader);

	if (!vbo) {
		/* common to all shader nodes */
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &tcbo);
		glBindBuffer(GL_ARRAY_BUFFER, tcbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	shader_node = calloc(1, sizeof(shader_node_t));
	fatal_if(!shader_node, "Unable to allocate shader_node");

	shader_ref(shader);
	shader_node->shader = shader;
	shader_node->uniforms_func = uniforms_func;
	shader_node->uniforms_ctxt = uniforms_ctxt;
	shader_node->transform = transform;

	stage = stage_new(conf, &shader_node_ops, shader_node);
	fatal_if(!stage, "Unable to create stage \"%s\"", conf->name);

	return stage;
}


/* return a new shader stage node from source */
stage_t * shader_node_new_src(const stage_conf_t *conf, const char *vs_src, const char *fs_src, const m4f_t *transform, shader_node_uniforms_func_t *uniforms_func, void *uniforms_ctxt, unsigned n_uniforms, const char **uniforms)
{
	stage_t		*stage;
	shader_t	*shader;

	assert(vs_src);
	assert(fs_src);

	shader = shader_pair_new(vs_src, fs_src, n_uniforms, uniforms,
			2,
			(const char *[]) {
			"vertex",
			"texcoord",
			});
	stage = shader_node_new_shader(conf, shader, transform, uniforms_func, uniforms_ctxt);
	shader_free(shader);

	return stage;
}

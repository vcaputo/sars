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
#include <stdlib.h>

#include "glad.h"
#include "m4f.h"
#include "macros.h"
#include "shader.h"
#include "tex.h"

typedef struct tex_t {
	unsigned	tex;
	unsigned	refcnt;
} tex_t;

static unsigned	vbo, tcbo;
static shader_t	*tex_shader;

static const float	vertices[] = {
	+1.f, +1.f, 0.f,
	+1.f, -1.f, 0.f,
	-1.f, +1.f, 0.f,
	+1.f, -1.f, 0.f,
	-1.f, -1.f, 0.f,
	-1.f, +1.f, 0.f,
};

static const float	texcoords[] = {
	1.f, 0.f,
	1.f, 1.f,
	0.f, 0.f,
	1.f, 1.f,
	0.f, 1.f,
	0.f, 0.f,
};


static const char	*tex_vs = ""
	"#version 120\n"

	"uniform mat4	model_x;"

	"attribute vec3	vertex;"
	"attribute vec2	texcoord;"

	"void main()"
	"{"
	"	gl_TexCoord[0].xy = texcoord;"
	"	gl_Position = model_x * vec4(vertex, 1.f);"
	"}"
"";


static const char	*tex_fs = ""
	"#version 120\n"

	"uniform sampler2D	tex0;"
	"uniform float		alpha;"

	"void main()"
	"{"
	"	gl_FragColor = texture2D(tex0, gl_TexCoord[0].st);"
	"	gl_FragColor.a *= alpha;"
	"}"
"";


/* Render simply renders a texd texture onto the screen */
void tex_render(tex_t *tex, float alpha, m4f_t *model_x)
{
	int		*uniforms, *attributes;

	assert(tex);
	assert(model_x);

	shader_use(tex_shader, NULL, &uniforms, NULL, &attributes);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attributes[0], 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(attributes[0]);

	glBindBuffer(GL_ARRAY_BUFFER, tcbo);
	glVertexAttribPointer(attributes[1], 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(attributes[1]);

	glBindTexture(GL_TEXTURE_2D, tex->tex);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUniform1f(uniforms[0], alpha);
	glUniformMatrix4fv(uniforms[1], 1, GL_FALSE, &model_x->m[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


tex_t * tex_new(int width, int height, const unsigned char *buf)
{
	tex_t	*tex;

	assert(buf);

	if (!vbo) {
		/* common to all tex instances */
		tex_shader = shader_pair_new(tex_vs, tex_fs,
					2,
					(const char *[]) {
						"alpha",
						"model_x",
					},
					2,
					(const char *[]) {
						"vertex",
						"texcoord",
					});

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &tcbo);
		glBindBuffer(GL_ARRAY_BUFFER, tcbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	tex = calloc(1, sizeof(tex_t));
	fatal_if(!tex, "Unable to allocate tex_t");

	glGenTextures(1, &tex->tex);
	glBindTexture(GL_TEXTURE_2D, tex->tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	glBindTexture(GL_TEXTURE_2D, 0);

	tex->refcnt = 1;

	return tex;
}


tex_t * tex_ref(tex_t *tex)
{
	assert(tex);

	tex->refcnt++;

	return tex;
}


tex_t * tex_free(tex_t *tex)
{
	if (!tex)
		return NULL;

	assert(tex->refcnt > 0);

	tex->refcnt--;
	if (!tex->refcnt) {
		glDeleteTextures(1, &tex->tex);
		free(tex);
	}

	return NULL;
}

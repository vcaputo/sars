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

#include <SDL.h>

#include <play.h>
#include <stage.h>

#include "glad.h"
#include "plasma-node.h"
#include "shader-node.h"
#include "macros.h"
#include "m4f.h"

typedef struct plasma_node_t {
	int		*maga;
	float		*gloom;
} plasma_node_t;

static const char	*plasma_vs = ""
#ifdef __EMSCRIPTEN__
	"#version 100\n"

	"varying vec2 UV;"
#else
	"#version 120\n"
#endif

	"uniform mat4	projection_x;"

	"attribute vec3 vertex;"
	"attribute vec2 texcoord;"

	"void main()"
	"{"
#ifdef __EMSCRIPTEN__
	"	UV = texcoord;"
#else
	"	gl_TexCoord[0].xy = texcoord;"
#endif
	"	gl_Position = projection_x * vec4(vertex, 1.0);"
	"}"
"";


// derived from https://www.bidouille.org/prog/plasma
static const char	*plasma_fs = ""
#ifdef __EMSCRIPTEN__
	"#version 100\n"
#else
	"#version 120\n"
#endif

	"#define PI 3.1415926535897932384626433832795\n"

#ifdef __EMSCRIPTEN__
	"precision mediump float;"
	"varying vec2 UV;"
#endif

	"uniform float alpha;"
	"uniform float time;"
	"uniform float gloom;"
	"uniform int maga;"

	"void main() {"
	"	float v;"
	"	float stime = sin(time * .01) * 100.0;"

#ifdef __EMSCRIPTEN__
	"	vec2 c = UV;"
#else
	"	vec2 c = gl_TexCoord[0].st;"
#endif

	// this zooms the texture coords in and out a bit with time
	"	c *= (sin(stime * .01) *.5 + .5) * 3.0 + 1.0;"

	// plasma calculations, stime instead of time directly to vary directions and speed
	"	v = sin((c.x + stime));"
	"	v += sin((c.y + stime) * .5);"
	"	v += sin((c.x + c.y +stime) * .5);"

	"	c += vec2(sin(stime * .33), cos(stime * .5)) * 3.0;"

	"	v += sin(sqrt(c.x * c.x + c.y * c.y + 1.0) + stime);"

	"	if (maga == 1) {"
	"		float	r, g, b;"

	"		r = smoothstep(.25, .8, cos(PI * v + sin(time)));"
	"		g = smoothstep(.25, .8, cos(1.333 * PI + PI * v + sin(time)));"
	"		b = smoothstep(.25, .8, cos(2.666 * PI + PI * v + sin(time)));"

		"	gl_FragColor = vec4(r + g, g, b + g, 1.);"
	"	} else {"
	"		vec3	col = vec3(cos(PI * v + sin(time)), sin(PI * v + cos(time * .33)), cos(PI * v + sin(time * .66)));"
		"	gl_FragColor = vec4((col * .5 + .5), 1.);"
	"	}"
	"}"
"";


static void plasma_uniforms(void *uniforms_ctxt, void *render_ctxt, unsigned n_uniforms, const int *uniforms, const m4f_t *model_x, float alpha)
{
	plasma_node_t	*plasma = uniforms_ctxt;
	play_t		*play = render_ctxt;

	glUniform1f(uniforms[0], alpha);
	glUniform1f(uniforms[1], play_ticks(play, PLAY_TICKS_TIMER0) * .001f); // FIXME KLUDGE ALERT
	glUniformMatrix4fv(uniforms[2], 1, GL_FALSE, &model_x->m[0][0]);
	glUniform1f(uniforms[3], *(plasma->gloom));
	glUniform1i(uniforms[4], *(plasma->maga));
}


/* create plasma rendering stage */
stage_t * plasma_node_new(const stage_conf_t *conf, m4f_t *projection_x, float *gloom, int *maga)
{
	plasma_node_t	*ctxt;

	/* FIXME TODO:
	 * shader-node doesn't cleanup uniforms_ctxt, so this is being leaked currently.
	 * but that doesn't really matter since plasma-node is only created once per game.
	 * The shortest path to cleaning this up would be to publicize the shader-node struct
	 * and implementation functions, and stitch it all together here, maybe with a nested
	 * shader_node_t in plasma_node_t.  That's rather annoying, and perhaps highlights a
	 * shortcoming in the existing stage_t composability.  It'd be nice if uniforms_ctxt
	 * could be typed with something embedding an optional free func, which shader_node_free()
	 * would then call to cleanup when set.  It's unclear if that should just be a bespoke
	 * shader-node type, or something more generally defined by libstage.  Either way, it's
	 * not terribly important for SARS in this singleton plasma-node case.
	 */
	ctxt = calloc(1, sizeof(plasma_node_t));
	fatal_if(!ctxt, "unable to allocate plasma_node_t");

	ctxt->gloom = gloom;
	ctxt->maga = maga;

	return	shader_node_new_src(conf, plasma_vs, plasma_fs, projection_x, plasma_uniforms, ctxt, 5,
			(const char *[]){
				"alpha",
				"time",
				"projection_x",
				"gloom",
				"maga",
			}
		);
}

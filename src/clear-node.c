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

#include <stage.h>

#include "clear-node.h"
#include "glad.h"
#include "macros.h"


static stage_render_func_ret_t clear_node_render(const stage_t *stage, void *object, float alpha, void *render_ctxt)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	return STAGE_RENDER_FUNC_RET_CONTINUE;
}


static const stage_ops_t clear_node_ops = {
	.render_func = clear_node_render,
};


stage_t * clear_node_new(stage_conf_t *conf)
{
	stage_t	*s;

	assert(conf);

	s = stage_new(conf, &clear_node_ops, NULL);
	fatal_if(!s, "Unable to create stage \"%s\"", conf->name);

	return s;
}

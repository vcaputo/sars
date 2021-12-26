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
#include <stage.h>

#include "tex.h"
#include "tex-node.h"

#include "gfx/gfx-zero.h"
#include "gfx/gfx-one.h"
#include "gfx/gfx-two.h"
#include "gfx/gfx-three.h"
#include "gfx/gfx-four.h"
#include "gfx/gfx-five.h"
#include "gfx/gfx-six.h"
#include "gfx/gfx-seven.h"
#include "gfx/gfx-eight.h"
#include "gfx/gfx-nine.h"

static const unsigned char	*digits_pixels[10] = {
	gfx_zero.pixel_data,
	gfx_one.pixel_data,
	gfx_two.pixel_data,
	gfx_three.pixel_data,
	gfx_four.pixel_data,
	gfx_five.pixel_data,
	gfx_six.pixel_data,
	gfx_seven.pixel_data,
	gfx_eight.pixel_data,
	gfx_nine.pixel_data,
};

static tex_t		*digits_tex[10];

#define DIGIT_WIDTH	184
#define DIGIT_HEIGHT	288

stage_t * digit_node_new(stage_conf_t *conf, unsigned digit, m4f_t *projection_x, m4f_t *model_x)
{
	assert(digit < 10);

	if (!digits_tex[digit])
		digits_tex[digit] = tex_new(DIGIT_WIDTH, DIGIT_HEIGHT, digits_pixels[digit]);

	return tex_node_new_tex(conf, digits_tex[digit], projection_x, model_x);
}

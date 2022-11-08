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

#include "ansr-tex.h"
#include "tex.h"
#include "tex-node.h"

static const char	*digits_assets[10] = {
	"assets/zero.ans",
	"assets/one.ans",
	"assets/two.ans",
	"assets/three.ans",
	"assets/four.ans",
	"assets/five.ans",
	"assets/six.ans",
	"assets/seven.ans",
	"assets/eight.ans",
	"assets/nine.ans",
};

static const char	*digits_masks_assets[10] = {
	"assets/zero.mask.ans",
	"assets/one.mask.ans",
	"assets/two.mask.ans",
	"assets/three.mask.ans",
	"assets/four.mask.ans",
	"assets/five.mask.ans",
	"assets/six.mask.ans",
	"assets/seven.mask.ans",
	"assets/eight.mask.ans",
	"assets/nine.mask.ans",
};

static tex_t		*digits_tex[10];

#define DIGIT_WIDTH	184
#define DIGIT_HEIGHT	288

stage_t * digit_node_new(stage_conf_t *conf, unsigned digit, m4f_t *projection_x, m4f_t *model_x)
{
	assert(digit < 10);

	if (!digits_tex[digit])
		digits_tex[digit] = ansr_tex_new(digits_assets[digit], digits_masks_assets[digit]);

	return tex_node_new_tex(conf, digits_tex[digit], projection_x, model_x);
}

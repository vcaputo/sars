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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ansr.h>

#include "ansr-tex.h"
#include "cp437.h"
#include "macros.h"
#include "tex.h"


/* I'm not bothering with a fully-featured SAUCE implementation, but it's
 * necessary to get at its width+lines since a lot of these art editors are
 * really relying on the wrapping at width even with non-80-char widths.
 * So here's a little ad-hoc extractor of just the size for character files.
 */
static int ansr_get_sauce_dimensions(FILE *f, unsigned *width, unsigned *height)
{
	char		buf[128];
	long		oldpos;
	unsigned	w, h;

	assert(f);

	oldpos = ftell(f);
	if (fseek(f, -128, SEEK_END) < 0)
		goto _err;

	if (fread(buf, sizeof(buf), 1, f) < 1)
		goto _err;

	if (buf[0] != 'S' ||
	    buf[1] != 'A' ||
	    buf[2] != 'U' ||
	    buf[3] != 'C' ||
	    buf[4] != 'E' ||
	    buf[5] != '0' ||
	    buf[6] != '0')
		goto _err;

	if (buf[94] != 1 ||
	    buf[95] != 1)
		goto _err;

	w = buf[96];
	w |= ((unsigned)buf[97]) << 8;

	h = buf[98];
	h |= ((unsigned)buf[99]) << 8;

	if (width)
		*width = w;
	if (height)
		*height = h;

	fseek(f, oldpos, SEEK_SET);

	return 0;

_err:
	fseek(f, oldpos, SEEK_SET);

	return -1;
}


/* foo.ans -> ansr_t * */
static ansr_t * ansr_from_file(const char *path)
{
	ansr_conf_t	conf = { .screen_width = 80 };
	char		buf[4096];
	size_t		len;
	FILE		*f;
	ansr_t		*a;

	f = fopen(path, "rb");
	if (!f)
		return NULL;

	debug_if(ansr_get_sauce_dimensions(f, &conf.screen_width, NULL) < 0,
		"No SAUCE metadata for \"%s\"", path);

	debugf("\"%s\" %u wide", path, conf.screen_width);

	a = ansr_new(&conf, NULL, 0);
	if (!a) {
		fclose(f);
		return NULL;
	}

	while ((len = fread(buf, 1, sizeof(buf), f)) > 0) {
		if (ansr_write(a, buf, len) < 0) {
			fclose(f);
			return ansr_free(a);
		}
	}

	fclose(f);

	return a;
}


typedef struct ansr_view_t {
	unsigned	width, height;
	uint32_t	pixels[];
} ansr_view_t;


static void cp437_draw_glyph(ansr_view_t *view, int x, int y, uint32_t bg, uint32_t fg, unsigned char c)
{
	int			CY = c / 32, CX = c % 32;
	const unsigned char	*src = &cp437.pixel_data[(8 + CY * 16) * cp437.width + (8 + CX * 9)];

	assert(view);
	assert(x >= 0 && y >= 0);

	/* the glyphs are 8x16 but within the cp437->pixel_data they're rendered as 9x16 with the horizontal
	 * separator, and for the glyphs where the right-most column was right-extended their 8th column is
	 * pre-duplicated out to the divider into the right.  Here I'll just treat everything as 8x16 and not
	 * render any separator into the surface - which might actually be visibly wrong vs. viewing ANSI on
	 * a DOS PC VGA console, we'll see.
	 * cp437->pixel_data also has 8 bytes of padding on all sides.
	 */
	for (int v = 0; v < 16; v++) {
		for (int u = 0; u < 8; u++) {
			uint32_t	*dest;

			dest = &view->pixels[(y * 16 + v) * view->width + (x * 8 + u)];
			if (src[u])
				*dest = fg;
			else
				*dest = bg;
		}
		src += cp437.width;
	}
}


/* ansr_t * -> cp437 -> ansr_view_t * */
static ansr_view_t * ansr_view_as_cp437(ansr_t *ansr)
{
	unsigned	width;
	ansr_view_t	*v;

	assert(ansr);
	assert(ansr->rows);

	width = ansr->conf.screen_width;

	for (size_t r = 0; r < ansr->height; r++) {
		if (ansr->rows[r] && ansr->rows[r]->width > width)
			width = ansr->rows[r]->width;
	}

	v = calloc(1, sizeof(ansr_view_t) + width * 8 * ansr->height * 16 * sizeof(uint32_t));
	if (!v)
		return NULL;

	v->width = width * 8;
	v->height = ansr->height * 16;

{
	uint32_t	dims[8] = {
				0xff000000,
				0xff0000aa,
				0xff00aa00,
				0xff0055aa,
				0xffaa0000,
				0xffaa00aa,
				0xffaaaa00,
				0xffaaaaaa,
			};
	uint32_t	brights[8] = {
				0xff555555,
				0xff5555ff,
				0xff55ff55,
				0xff55ffff,
				0xffff5555,
				0xffff55ff,
				0xffffff55,
				0xffffffff,
			};

	for (size_t r = 0; r < ansr->height; r++) {
		ansr_row_t	*row = ansr->rows[r];

		if (row) {
			for (size_t c = 0; c < row->width; c++) {
				uint32_t	fg, bg;

				if (!row->cols[c].code)
					continue;

				if (row->cols[c].disp_state.attrs.bold) {
					fg = brights[row->cols[c].disp_state.colors.fg];
				} else {
					fg = dims[row->cols[c].disp_state.colors.fg];
				}
				bg = dims[row->cols[c].disp_state.colors.bg];

				if (row->cols[c].disp_state.attrs.invert) {
					uint32_t	tmp;

					tmp = fg;
					fg = bg;
					bg = tmp;
				}

				cp437_draw_glyph(v, c, r, bg, fg, row->cols[c].code);
			}
		}
	}
}

	return v;
}


/* load an .ans file and render out to a texture returned as tex_t */
tex_t * ansr_tex_new(const char *path, const char *mask_path)
{
	tex_t		*tex;
	ansr_t		*a;
	ansr_view_t	*v;

	a = ansr_from_file(path);
	fatal_if(!a, "unable to create ansr from .ans \"%s\"", path);

	v = ansr_view_as_cp437(a);
	fatal_if(!v, "unable to create ansr_view from ansr \"%s\"", path);
	ansr_free(a);

	if (mask_path) {
		/* when a mask is provided, only black pixels in the mask will be
		 * transparent in the output - everything else will be made 100% opaque.
		 */
		ansr_view_t	*mv;

		a = ansr_from_file(mask_path);
		fatal_if(!a, "unable to create ansr from .ans \"%s\"", mask_path);
		mv = ansr_view_as_cp437(a);
		fatal_if(!mv, "unable to create ansr_view from ansr \"%s\"", mask_path);
		ansr_free(a);

		fatal_if(v->width != mv->width || v->height != mv->height,
			"\"%s\" <-> \"%s\" mask dimensions mismatch", path, mask_path);

		for (size_t i = 0; i < v->width * v->height; i++) {
			if (!(mv->pixels[i] & 0xffffff))
				v->pixels[i] = 0; /* transparent pixel, toss out any color info */
			else
				v->pixels[i] |= 0xff000000; /* opaque pixel, set the alpha bits */
		}
		free(mv);
	}

	tex = tex_new(v->width, v->height, (const unsigned char *)v->pixels);
	fatal_if(!tex, "unable to create tex from ansr_view \"%s\"", path);
	free(v);

	return tex;
}

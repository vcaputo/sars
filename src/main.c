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

#include <play.h>

extern const play_ops_t sars_ops;
extern const play_ops_t hungrycat_ops;
extern const play_ops_t game_ops;

static const play_ops_t	*ops[] = {
	&sars_ops,
	&hungrycat_ops,
	&game_ops,
	NULL
};

int main(int argc, char *argv[])
{
	play_t	*play;

	play = play_startup(argc, argv, ops);

	play_run(play);

	return play_shutdown(play);
}

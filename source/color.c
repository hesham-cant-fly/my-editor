#include "color.h"
#include <ncurses.h>

static int rgb_scale(int val) {
    return (val * 1000) / 255;
}

void setup_colors(void)
{
	if (has_colors() == FALSE || can_change_color() == FALSE) {
		endwin();
		return;
	}

	start_color();
	use_default_colors();

#define X(name_, r_, g_, b_) \
	init_extended_color(name_, rgb_scale(r_), rgb_scale(g_), rgb_scale(b_));
COLORS_DEF(X)
#undef X

#define X(name_, fg_, bg_) \
	init_pair(name_, fg_, bg_);
	FACES_DEF(X)
#undef X
}

#include "input.h"
#include "common.h"
#include "output.h"
#include <ncurses.h>

void editor_process_key_press(void)
{
	row_t *row = current_row();

	const int ch = getch();

	if (ch == KEY_RESIZE) {
		editor_refresh();
	}

	if (ch == CTRL('q')) {
		quit();
	}

	if (ch == CTRL('a')) {
		G.tabwidth += 1;
		editor_refresh();
	}

	if (ch == CTRL('e')) {
		G.tabwidth -= 1;
		editor_refresh();
	}

	if (row == NULL) return;

	if (ch == KEY_LEFT) {
		editor_move_cursor_left();
	}

	if (ch == KEY_RIGHT) {
		editor_move_cursor_right();
	}

	if (ch == KEY_DOWN) {
		editor_move_cursor_down();
	}

	if (ch == KEY_UP) {
		editor_move_cursor_up();
	}

	if (ch == KEY_HOME) {
		editor_set_cx(0);
	}

	if (ch == KEY_END) {
		editor_set_cx(row->chars.len - 1);
	}

	if (ch == KEY_PPAGE) {
		editor_set_cy(0);
	}

	if (ch == KEY_NPAGE) {
		editor_set_cy(G.rows.len - 1);
	}

	row = current_row();
	if ((size_t)G.cx > row->chars.len) {
		G.cx = row->chars.len;
	}
}

#include "input.h"
#include "common.h"
#include "output.h"
#include <ncurses.h>

void editor_process_key_press(void)
{
	row_t *row = current_row();

	const int ch = getch();

	switch (ch) {
	case KEY_RESIZE:
		editor_refresh();
		return;

	case CTRL('q'):
		quit();
		return;

	case CTRL('a'):
		G.tabwidth += 1;
		editor_refresh();
		return;

	case CTRL('s'):
		editor_save();
		return;

	case CTRL('e'):
		G.tabwidth -= 1;
		editor_refresh();
		return;
	}

	row = current_row();
	if (row == NULL) return;

	switch (ch) {
	case KEY_ENTER:
	case '\r':
	case '\n':
		editor_insert_newline();
		break;
	case KEY_LEFT:
		editor_move_cursor_left();
		break;
	case KEY_RIGHT:
		editor_move_cursor_right();
		break;
	case KEY_DOWN:
		editor_move_cursor_down();
		break;
	case KEY_UP:
		editor_move_cursor_up();
		break;
	case KEY_HOME:
		editor_set_cx(0);
		break;
	case KEY_END:
		editor_set_cx(row->chars.len - 1);
		break;
	case KEY_PPAGE:
		editor_set_cy(0);
		break;
	case KEY_NPAGE:
		editor_set_cy(G.rows.len - 1);
		break;
	case KEY_BACKSPACE:
		editor_delete_char();
		break;
	default:
		editor_insert_char(ch);
		break;
	}

	row = current_row();
	if ((size_t)G.cx > row->chars.len) {
		G.cx = row->chars.len;
	}
}

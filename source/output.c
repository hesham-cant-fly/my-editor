#include "output.h"
#include "common.h"
#include "cmylib.h"
#include "color.h"
#include <ncurses.h>

static void fill_background(void)
{
	const dim_t screen = get_screen_size();
	with_face (FACE_BACKGROUND) {
		forange (y, 0, (size_t)screen.height)
		forange (x, 0, (size_t)screen.width) {
			mvprint(x, y, " ");
		}
	}
}

static void editor_draw_status_bar(void)
{
	const dim_t screen = get_screen_size();
	with_face (FACE_STATUS_BAR) {
		forange (x, 0, (size_t)screen.width) {
			if (x == 1) {
				const int result = mvprint(x, screen.height, "{s} {sv}", G.filename, ft_str(G.file_type));
				x += result - 1;
			} else if ((size_t)screen.width - x <= 13) {
				const int printed = mvprint(x, screen.height, "{d:0w6}/{d:0w6}", G.cy + 1, G.rows.len);
				x += printed - 1;
			} else {
				mvprint(x, screen.height, " ");
			}
		}
	}
}

static void editor_draw_message(void)
{
	const dim_t screen = get_screen_size();
	mvprint(0, screen.height + 1, "{s}", G.message);
}

static void editor_draw_rows(void)
{
	const dim_t screen = get_screen_size();
	with_face (FACE_DEFAULT) {
		for (int y = 0; y < screen.height; y += 1) {
			int filerow = y + G.rowoff;
			if ((size_t)y >= G.rows.len || (size_t)filerow >= G.rows.len) {
				mvprint(0, y, "~");
				if (G.rows.len == 0 && y == screen.height / 3) {
					const int welcome_len = snsprint(NULL, 0, "My Editor -- version {sv}", G.version);
					const int remaining_col_len = screen.width - welcome_len;
					mvprint(remaining_col_len / 2, y, "My Editor -- version {sv}", G.version);
				}
			} else {
				string_t line = G.rows.items[filerow].render;
				line.data += G.coloff;
				if (line.len < (size_t)G.coloff) {
					line.len = 0;
				} else {
					line.len -= G.coloff;
				}
				mvprint(0, y, "{str}", line);
			}
		}
	}
}

static int editor_row_cx_to_rx(row_t *row, int cx)
{
	int rx = 0;
	for (int i=0; i<cx && (size_t)i < row->chars.len; i+=1) {
		if (row->chars.data[i] == '\t') {
			rx += (G.tabwidth - 1) - (rx % G.tabwidth);
		}
		rx += 1;
	}

	return rx;
}

static void editor_scroll(void) {
	G.rx = 0;
	if ((size_t)G.cy < G.rows.len) {
		G.rx = editor_row_cx_to_rx(current_row(), G.cx);
	}

	dim_t screen = get_screen_size();
	if (G.cy < G.rowoff) {
		G.rowoff = G.cy;
	}
	if (G.cy >= G.rowoff + screen.height) {
		G.rowoff = G.cy - screen.height + 1;
	}
	if (G.rx < G.coloff) {
		G.coloff = G.rx;
	}
	if (G.rx >= G.coloff + screen.width) {
		G.coloff = G.rx - screen.width + 1;
	}
}

void editor_refresh_screen(void)
{
	if (G.need_refresh) {
		iarreach (i, G.rows) {
			update_row(&G.rows.items[i]);
		}

		editor_scroll();
		erase();

		fill_background();
		editor_draw_rows();
		editor_draw_status_bar();
		editor_draw_message();

		move(G.cy - G.rowoff, G.rx - G.coloff);
		refresh();
		G.need_refresh = false;
	}
}

void editor_set_cy(int y)
{
	if (y < 0) {
		G.cy = 0;
	} else if ((size_t)y < G.rows.len) {
		G.cy = y;
	} else {
		G.cy = G.rows.len - 1;
	}
	editor_refresh();
}

void editor_set_cx(int x)
{
	if (x < 0) {
		G.cx = 0;
	}
	dim_t screen = get_screen_size();
	if (x >= screen.width) G.cx = screen.width - 1;
	G.cx = x;
	editor_refresh();
}

void editor_move_cursor_up(void)
{
	if (G.cy == 0) return;
	G.cy -= 1;
	editor_refresh();
}

void editor_move_cursor_down(void)
{
	if ((size_t)G.cy == G.rows.len) {
		editor_set_message("{d} {usize}", G.cy, G.rows.len);
		return;
	}
	G.cy += 1;
	editor_refresh();
}

void editor_move_cursor_left(void)
{
	if (G.cx == 0) return;
	G.cx -= 1;
	editor_refresh();
}

void editor_move_cursor_right(void)
{
	row_t *row = current_row();
	if (row == NULL) {
		editor_refresh();
		return;
	}
	if ((size_t)G.cx < row->chars.len) {
		G.cx += 1;
	}
	editor_refresh();
}

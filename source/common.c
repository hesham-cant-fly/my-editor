#include "common.h"
#include "output.h"
#include "terminal.h"
#include "cmylib.h"
#include "color.h"
#include <errno.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

editor_config_t G = {0};

static void free_row(row_t *row)
{
	delete(row->chars.data);
	delete(row->render.data);
	memset(row, 0, sizeof(*row));
}

void init_editor(void)
{
	initscr();
	enable_raw_mode();
	setup_colors();
	G.version = sv_from_chars(EDITOR_VERSION);
	G.rows = marrinit(rows_t, default_allocator_);
	G.tabwidth = 4;
	editor_refresh();
}

void editor_refresh(void)
{
	G.need_refresh = true;
}

void editor_open_buf(char *buf)
{
	string_t buffer = string_from_chars(buf);
	string_t line = {buffer.data, 0};
	for (size_t i=0; i < buffer.len; i += 1) {
		const char ch = buffer.data[i];
		if (ch == '\n' || ch == '\r') {
			marrpush(G.rows, ((row_t){.chars = (string_t){line.data, line.len}}));
			line.data = buffer.data + i;
			line.len = 0;
			continue;
		}
		line.len += 1;
	}

	if (G.filename != NULL) free(G.filename);
	G.filename = strdup("*buffer*");
	G.file_type = FT_UNKOWN;

	editor_refresh();
}

void editor_open_file(char *path)
{
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		die("fopen");
	}
	if (G.filename != NULL) free(G.filename);
	G.filename = strdup(path);
	{
		string_view_t s = sv_from_chars(path);
		if (sv_ends_with_cstr(s, ".c")) {
			G.file_type = FT_C;
		} else if (sv_ends_with_cstr(s, ".js")) {
			G.file_type = FT_JS;
		} else {
			G.file_type = FT_UNKOWN;
		}
	}

	char *line = NULL;
	size_t linecap = 0;
	ssize_t linelen  = 0;

	while ((linelen = getline(&line, &linecap, fp)) != -1) {
		while (linelen > 0 && (line[linelen - 1] == '\n' ||
		                       line[linelen - 1] == '\r')) {
			linelen--;
		}
		insert_row(G.rows.len, (string_t){ line, linelen });
	}
	free(line);
	fclose(fp);
}

void editor_set_message(const char *const fmt, ...)
{
	if (G.message.data != NULL) {
		delete(G.message.data);
	}

	va_list args;
	va_start(args, fmt);
	string_builder_t sb = sb_vformat(default_allocator_, fmt, args);
	string_t msg = sb_build(&sb);
	va_end(args);
	G.message = msg;
	editor_refresh();
}

void editor_insert_char(int c)
{
	if ((size_t)G.cy == G.rows.len) {
		insert_row(G.rows.len, string_from_chars(""));
	}
	row_insert_char(&G.rows.items[G.cy], G.cx, c);
	G.cx += 1;
	editor_refresh();
}

void editor_insert_newline(void)
{
	if (G.cx == 0) {
		insert_row(G.cy, string_from_chars(""));
	} else {
		row_t *row = current_row();
		insert_row(G.cy + 1, (string_t) {
			row->chars.data + G.cx,
			row->chars.len - G.cx,
		});
		row = current_row();
		row->chars.len = G.cx;
		row->chars.data[row->chars.len] = '\0';
	}
	G.cy += 1;
	G.cx = 0;
	editor_refresh();
}

void editor_delete_char(void)
{
	if ((size_t)G.cy == G.rows.len) return;
	if (G.cy == 0 && G.cx == 0) return;

	row_t *row = current_row();
	if (G.cx > 0) {
		row_delete_char(row, G.cx - 1);
		G.cx -= 1;
	} else {
		row_t *prev_row = &G.rows.items[G.cy - 1];
		G.cx = prev_row->chars.len;
		row_append_string(prev_row, row->chars);
		editor_delete_row(G.cy);
		editor_set_message("{d}", G.cx);
		G.cy -= 1;
	}
	editor_refresh();
}

string_t editor_rows_to_string(void)
{
	string_builder_t sb = sb_new(default_allocator_, 67);
	arreach (row_t, row, G.rows) {
		sb_pushf(&sb, "{str}\n", row.chars);
	}

	return sb_build(&sb);
}

void editor_save(void)
{
	if (G.filename == NULL) return;

	string_t content = editor_rows_to_string();

	stream_t f = sopen(G.filename, "w");
	if (f.data == NULL) die("sopen");

	if ((size_t)swrite(f, (unsigned char*)content.data, 1, content.len) != content.len) {
		editor_set_message("Can't save! I/O error: {s}", strerror(errno));
	}

	sclose(f);
	editor_set_message("{usize} bytes written to disk", content.len);
	delete(content.data);
}

void insert_row(size_t at, string_t content)
{
	if (at < 0 || at > G.rows.len) {
		return;
	}

	if (G.rows.len >= G.rows.cap) {
		marrgrow(G.rows);
	}
	memmove(G.rows.items + at + 1, G.rows.items + at, sizeof(row_t) * (G.rows.len - at));

	row_t row = {0};
	row.chars = (string_t){
		.data = make(content.len + 1),
		.len = content.len,
	};
	memcpy(row.chars.data, content.data, content.len + 1);
	row.chars.data[row.chars.len] = '\0';
	update_row(&row);
	G.rows.items[at] = row;
	G.rows.len += 1;
	editor_refresh();
}

void update_row(row_t *row)
{
	if (row->render.data != NULL) {
		delete(row->render.data);

		row->render.data = NULL;
		row->render.len = 0;
	}

	string_builder_t sb = sb_new(default_allocator_, row->chars.len);
	for (size_t i=0; i < row->chars.len; i+=1) {
		const char ch = row->chars.data[i];
		if (ch == '\t') {
			sb_pushf(&sb, "{s:w*}", "", (int)G.tabwidth);
		} else {
			sb_push_char(&sb, ch);
		}
	}

	row->render = sb_build(&sb);
}

void editor_delete_row(size_t at)
{
	if (at < 0 || at >= G.rows.len) {
		return;
	}
	free_row(G.rows.items + at);
	memmove(G.rows.items + at, G.rows.items + at + 1, sizeof(row_t) * (G.rows.len - at - 1));
	G.rows.len -= 1;
	editor_refresh();
}

row_t *current_row(void)
{
	if (G.rows.len == 0) return NULL;
	if ((size_t)G.cy >= G.rows.len) {
		return &G.rows.items[G.rows.len - 1];
	}
	return &G.rows.items[G.cy];
}

void row_insert_char(row_t *row, size_t at, int c)
{
	if (at < 0 || at > row->chars.len) at = row->chars.len;
	char *new_chars = renew(row->chars.len + 2, row->chars.data);
	memmove(&new_chars[at + 1], &new_chars[at], row->chars.len - at + 1);
	new_chars[at] = c;
	row->chars.len += 1;
	row->chars.data = new_chars;
}

void row_append_string(row_t *row, string_t string)
{
	string_builder_t sb = sb_from_chars_copy(default_allocator_, row->chars.data);
	sb_push_string(&sb, string);
	free_row(row);
	row->chars = sb_build(&sb);
	editor_refresh();
}

void row_delete_char(row_t *row, size_t at)
{
	memmove(row->chars.data + at, row->chars.data + at + 1, row->chars.len - at);
	row->chars.len -= 1;
}

int mvprint(int x, int y, const char *const fmt, ...)
{
	const dim_t screen = get_screen_size();

	if (x >= screen.width || y >= screen.height+2) return 0;

	const int available_width = screen.width - x;

	va_list args = {0};
	va_start(args, fmt);
	char *buffer = make(available_width + 1);
	const int result = vsnsprint(buffer, available_width + 1, fmt, args);
	va_end(args);

	for (int i=0; i < available_width;  i += 1) {
		if (buffer[i] == '\0') break;
		mvprintw(y, x + i, "%c", buffer[i]);
	}
	delete(buffer);

	return result;
}

dim_t get_screen_size(void)
{
	int height = 0, width = 0;
	getmaxyx(stdscr, height, width);
	height -= 2;
	return (dim_t){width, height};
}

void die(const char *s)
{
	editor_refresh_screen();

	perror(s);
	exit(1);
}

void quit(void)
{
	editor_refresh_screen();
	exit(0);
}

void message(const char *const fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	string_builder_t formated = sb_vformat(default_allocator_, fmt, args);
	va_end(args);

	string_builder_t sb = sb_new(default_allocator_, 0);
	sb_pushf(&sb, "echo ");
	sb_pushf(&sb, "{sb}", formated);
	sb_pushf(&sb, " >> /dev/pts/1");

	(void)system(sb.data);

	sb_delete(&sb);
	sb_delete(&formated);
}

string_view_t ft_str(file_type_t ft)
{
	switch (ft) {
	case FT_UNKOWN:  return sv_from_chars("text");
	case FT_C:       return sv_from_chars("C");
	case FT_JS:      return sv_from_chars("JavaScript");
	}
}

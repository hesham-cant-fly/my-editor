#include "common.h"
#include "output.h"
#include "terminal.h"
#include "cmylib.h"
#include "color.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

editor_config_t G = {0};

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
		push_row((string_t){ line, linelen });
	}
	free(line);
	fclose(fp);
}

void editor_set_message(string_t msg)
{
	G.message = msg;
}

void push_row(string_t content)
{
	row_t row = {0};
	row.chars = (string_t){
		.data = make(content.len + 1),
		.len = content.len,
	};
	memcpy(row.chars.data, content.data, content.len + 1);
	update_row(&row);
	marrpush(G.rows, row);
}

row_t *current_row(void)
{
	if (G.rows.len == 0) return NULL;
	if ((size_t)G.cy >= G.rows.len) {
		return &G.rows.items[G.rows.len - 1];
	}
	return &G.rows.items[G.cy];
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

	// FIXME: A TAB WIDTH! PROBLEM
	// mvaddnstr(y, x, buffer, available_width);
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

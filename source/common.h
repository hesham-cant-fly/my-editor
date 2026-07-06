/** Created in: 05/21/2026 16:07
  *
  */
#ifndef COMMON_H_
#define COMMON_H_

#ifndef _DEFAULT_SOURCE
#  define _DEFAULT_SOURCE
#endif // !_DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include "cmylib.h"

#define EDITOR_VERSION "0.0.67"

typedef struct row_t {
	string_t chars;
	string_t render;
} row_t;

typedef struct rows_t {
	allocator_t allocator;
	size_t len, cap;
	row_t *items;
} rows_t;

typedef enum file_type_t {
	FT_UNKOWN = 0,
	FT_C,
	FT_JS,
} file_type_t;

typedef struct editor_config_t {
	bool need_refresh;
	size_t tabwidth;
	int cx, cy;
	int rx;
	int rowoff, coloff;
	rows_t rows;
	char *filename;
	string_t message;
	string_view_t version;
	file_type_t file_type;
	struct termios orig_termios;
} editor_config_t;

typedef struct dim_t {
	int width, height;
} dim_t;

extern editor_config_t G;

void init_editor(void);
int mvprint(int x, int y, const char *const fmt, ...);
void editor_refresh(void);
void editor_open_buf(char *buf);
void editor_open_file(char *path);
void editor_set_message(string_t msg);
void push_row(string_t content);
row_t *current_row(void);
void update_row(row_t *row);
dim_t get_screen_size(void);
void die(const char *s);
void quit(void);
void message(const char *const fmt, ...);
string_view_t ft_str(file_type_t ft);

#endif /* !COMMON_H_ */

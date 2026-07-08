#include <stdio.h>
#include <ncurses.h>
#include "common.h"
#include "cmylib.h"
#include "input.h"
#include "output.h"

static int format_string_view(stream_t stream, modifier_stream_t mod, va_list args);
static int format_string_builder(stream_t stream, modifier_stream_t mod, va_list args);
static int format_string_t(stream_t stream, modifier_stream_t mod, va_list args);

int main(int argc, char **argv)
{
	setup_io_stream();
	define_format_specifier("sv", format_string_view);
	define_format_specifier("sb", format_string_builder);
	define_format_specifier("str", format_string_t);

	set_default_allocator(get_c_allocator());

	init_editor();
	if (argc >= 2) {
		editor_open_file(argv[1]);
	}

	editor_set_message("HELP: Ctrl-Q = quit | Ctrl-S = save");

	while (true) {
		editor_refresh_screen();
		editor_process_key_press();
	}

	return 0;
}

static int format_string_view(stream_t stream, modifier_stream_t mod, va_list args)
{
	(void)mod;
	const string_view_t sv = va_arg(args, string_view_t);
	return sprint(stream, "{s:*}", sv.data, (int)sv.len);
}

static int format_string_builder(stream_t stream, modifier_stream_t mod, va_list args)
{
	(void)mod;
	const string_builder_t sv = va_arg(args, string_builder_t);
	return sprint(stream, "{s:*}", sv.data, (int)sv.len);
}

static int format_string_t(stream_t stream, modifier_stream_t mod, va_list args)
{
	(void)mod;
	const string_t sv = va_arg(args, string_t);
	return sprint(stream, "{s:*}", sv.data, (int)sv.len);
}

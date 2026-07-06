#include "terminal.h"

#include <stdio.h>
#include <ncurses.h>

#include "cmylib.h"
#include "common.h"

void disable_raw_mode(void)
{
	endwin();
}

void enable_raw_mode(void)
{
	atexit(disable_raw_mode);

	raw();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(1);
}

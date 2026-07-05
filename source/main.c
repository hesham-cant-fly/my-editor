#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "cmylib.h"

struct termios g_original_termios = {0};

static void disable_raw_mode(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_original_termios);
}

static void enable_raw_mode(void)
{
	tcgetattr(STDIN_FILENO, &g_original_termios);
	atexit(disable_raw_mode);

	struct termios raw = g_original_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void)
{
	setup_io_stream();
	enable_raw_mode();

	while (true) {
		char c = '\0';
		sread(stin, &c, 1, 1);
		if (iscntrl(c)) {
			println("{d}\r", c);
		} else {
			println("{d} ({c})\r", c, c);
		}
		if (c == 'q') break;
	}

	return 0;
}

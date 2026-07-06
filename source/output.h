/** Created in: 05/43/2026 17:07
  *
  */
#ifndef OUTPUT_H_
#define OUTPUT_H_

void editor_refresh_screen(void);

void editor_set_cy(int y);
void editor_set_cx(int x);
void editor_move_cursor_up(void);
void editor_move_cursor_down(void);
void editor_move_cursor_left(void);
void editor_move_cursor_right(void);

#endif /* !OUTPUT_H_ */

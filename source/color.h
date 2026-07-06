/** Created in: 06/15/2026 16:07
  *
  */
#ifndef COLOR_H_
#define COLOR_H_

typedef enum color_t {
	COLOR_T_ = 16,
#define COLORS_DEF(X_) \
	X_(MY_WHITE, 255, 255, 255) \
	X_(MY_BLACK, 0, 0, 0) \
	X_(MY_RED, 255, 0, 0)

#define X(name_, ...) name_,
COLORS_DEF(X)
#undef X
} color_t;

typedef enum face_t {
	FACE_T_ = 0,
#define FACES_DEF(X_) \
	X_(FACE_BACKGROUND, -1, MY_BLACK) \
	X_(FACE_DEFAULT, MY_WHITE, MY_BLACK) \
	X_(FACE_STATUS_BAR, MY_BLACK, MY_WHITE)

#define X(name_, ...) name_,
FACES_DEF(X)
#undef X
} face_t;

#define with_face(face_) \
	for (int x = (attron(COLOR_PAIR(face_)), 1); x; x = 0, attroff(COLOR_PAIR(face_)))

void setup_colors(void);

#endif /* !COLOR_H_ */

/** Created in: 05/29/2026 16:07
  *
  */
#ifndef INPUT_H_
#define INPUT_H_

#include <stdbool.h>

#ifdef CTRL
#  undef CTRL
#endif // CTRL
#define CTRL(c_) ((c_) & 0x1F)

void editor_process_key_press(void);

#endif /* !INPUT_H_ */

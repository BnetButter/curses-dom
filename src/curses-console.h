#ifndef CURSES_CONSOLE_H
#define CURSES_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif


int console_init(int maxlen);
int console_toggle();
int console_resize(int size[2]);
int console_log(const char *, ...) __attribute__((format(printf, 1, 2)));



#ifdef __cplusplus
}
#endif
#endif
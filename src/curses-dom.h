#ifndef NCURSES_DOM_H
#define NCURSES_DOM_H
#include "gumbo.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DomElement {
    const short scrollHeight;
    const short scrollWidth;

    const short offsetHeight;
    const short offsetWidth;
    
    const short clientWidth;
    const short clientHeight;

    short scrollTop;
    short scrollLeft;
} DomElement;


typedef struct {
    char text;
} TPixel;

struct Display {
    const short width;
    const short height;
    TPixel **buffer;
};

extern DomElement HTML;
extern struct Display display;

int print_display();

int terminal_did_change_size(int size[2]);
int terminal_size_change_handled();








#ifdef __cplusplus
}
#endif
#endif
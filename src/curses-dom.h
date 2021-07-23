#ifndef NCURSES_DOM_H
#define NCURSES_DOM_H
#include "gumbo.h"
#include <gmodule.h>
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

    struct DomElement *parent;
    GArray *children;
} DomElement;

DomElement *DomElement_new(DomElement *parent);


typedef int (*HTML_Tag_Handler)(GumboNode *, void *);
extern HTML_Tag_Handler html_tag[GUMBO_TAG_LAST];



extern DomElement HTML;

int print_display();

int terminal_did_change_size(int size[2]);
int terminal_size_change_handled();


#ifdef NDEBUG
#define assert(expr)   do {} while (0)
#else

void _capture_assertion(const char *expr, const char * f, int line);
#define assert(expr)  ((expr) ? 0 : _capture_assertion(#expr, __FILE__, __LINE__))
#endif





#ifdef __cplusplus
}
#endif
#endif
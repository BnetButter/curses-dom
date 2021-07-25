#ifndef NCURSES_DOM_H
#define NCURSES_DOM_H
#include <curses.h>
#include <gmodule.h>
#include <panel.h>
#include "gumbo.h"
#include "_dom.h"

#ifdef __cplusplus
extern "C" {
#endif


DomElement *DomElement_new(DomElement *parent);
void DomElement_destroy(DomElement *self);
DomElement *domtree_new(const char *html);
void domtree_delete(DomElement *);

DomElement *getElementById(DomElement *root, const char *id);

typedef int (*HTML_Tag_Handler)(GumboNode *, void *);
extern HTML_Tag_Handler html_tag[GUMBO_TAG_LAST];



extern DomElement HTML;

#define TERM_HEIGHT     HTML.offsetHeight
#define TERM_WIDTH      HTML.offsetWidth

int print_display();

int terminal_did_change_size(int size[2]);
int terminal_size_change_handled();

/*
*/
int handle_node(GumboNode *node, DomElement *parent);





#ifdef __cplusplus
}
#endif
#endif
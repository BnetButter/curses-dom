#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "curses-dom.h"

#define set_member_attr(where, value) (assert(sizeof(where) == sizeof(value)), memcpy((void *) & where, & value, sizeof(value)))

static volatile sig_atomic_t window_size_changed;
HTML_Tag_Handler html_tag[GUMBO_TAG_LAST];
DomElement HTML;


static void
Handle_SIGWINCH(int signum)
{
    window_size_changed = 1;
}

int terminal_did_change_size(int size[2])
{
    if (window_size_changed) {
        struct winsize w;
        ioctl(0, TIOCGWINSZ, &w);
        size[0] = w.ws_row;
        size[1] = w.ws_col;
        window_size_changed = 0;
        set_member_attr(HTML.scrollHeight, w.ws_row);
        set_member_attr(HTML.scrollWidth, w.ws_col);
        return 1;
    }
    return window_size_changed;
}

DomElement *
DomElement_new(DomElement *parent)
{
    DomElement *child = malloc(sizeof(DomElement));
    child->children = g_array_new(0, 0, sizeof(DomElement *));
    child->parent = parent;
    g_array_append_val(parent->children, child);
    return child;
}

static int 
handle_div(GumboNode *node, void *args)
{
    mvprintw(0, 0, "Div");
}

static void __attribute__((constructor(101)))
curses_init() 
{
    initscr(); 
    atexit(endwin);
    struct sigaction sigwinch, old;
    sigwinch.sa_flags = 0;
    sigemptyset(& sigwinch.sa_mask);
    sigaddset(& sigwinch.sa_mask, SIGINT);
    sigwinch.sa_flags = 0;
    sigwinch.sa_handler = Handle_SIGWINCH;
    sigaction(SIGWINCH, & sigwinch, NULL);

    short row = getmaxy(stdscr);
    short col = getmaxx(stdscr);
    
    set_member_attr(HTML.offsetHeight, row);
    set_member_attr(HTML.offsetWidth, col);

    
    HTML.children = g_array_new(0, true, sizeof(DomElement *));
    html_tag[GUMBO_TAG_DIV] = handle_div;
    
}


static void __attribute__((destructor(101)))
curses_fini()
{
    endwin();
}

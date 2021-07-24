#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "curses-dom.h"
#include "_dom.h"

#define set_member_attr(where, value) (assert(sizeof(where) == sizeof(value)), memcpy((void *) & where, & value, sizeof(value)))

static volatile sig_atomic_t window_size_changed;
static volatile sig_atomic_t signum = 0;
static int assertion_lineno = 0;
static const char *assertion_stmt;
static const char *assertion_file;

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
        set_member_attr(HTML.offsetHeight, w.ws_row);
        set_member_attr(HTML.offsetWidth, w.ws_col);
        return 1;
    }
    return window_size_changed;
}



int handle_node(GumboNode *node, DomElement *parent)
{
    
    switch (node->type) {
        case GUMBO_NODE_DOCUMENT:
            handle_document((GumboDocument *) & node->v, parent);
            break;
        case GUMBO_NODE_ELEMENT:
            handle_element((GumboElement *) & node->v, parent); 
            break;
        case GUMBO_NODE_TEXT:
            handle_text((GumboText *) & node->v, parent);
            break;
        case GUMBO_NODE_WHITESPACE:
            return;
        default:
            assert(0 && "Unhandled node type");
            exit(1);
    }
}

static inline int
handle_document(const GumboDocument *node, DomElement *parent)
{

}

static inline int 
request_size(DomElement *parent, DomDim request, DomDim *received)
{
    if (! parent) {
        return;
    }
    
    short height = parent->clientHeight + request.row;
    short width = parent->clientWidth + request.col;
    set_member_attr(parent->clientHeight, height);
    set_member_attr(parent->clientWidth, width);
  
}


static inline int
handle_text(const GumboText *node, DomElement *parent)
{
    
}



static inline int
handle_element(const GumboElement *node, DomElement *parent)
{
    /*DomElement *new_node = DomElement_new(parent);
    
    if (html_tag[node->tag]) {
        void *args[] = {
            new_node,
        };
        html_tag[node->tag](node, args);
    }
    else {
        const GumboVector *children = & node->children;
        for (int i = 0; i < children->length; i++) {
            handle_node(children->data[i], new_node);
        }
    } */
}

static int 
handle_div(GumboElement *node, void **params)
{
    const GumboVector *children = & node->children;
    DomElement *thisnode = params[0];
    for (int i = 0; i < children->length; i++) {
        handle_node(children->data[i], thisnode);
    }
}

static int
handle_panic_signal(int n)
{
    signum = n;
    exit(1);
}

static void __attribute__((constructor(101)))
curses_init() 
{
    initscr(); 

    struct sigaction sigwinch; // handle term resize
    struct sigaction panic;    // cannot recover, but restore terminal
    sigwinch.sa_handler = Handle_SIGWINCH;
    sigwinch.sa_flags = SA_RESTART;
    sigemptyset(& sigwinch.sa_mask);
    sigaddset(& sigwinch.sa_mask, SIGINT);
    sigaction(SIGWINCH, & sigwinch, NULL);

    panic.sa_handler = handle_panic_signal;
    sigaction(SIGSEGV, & panic, NULL);
    sigaction(SIGABRT, & panic, NULL);
    
    short row = getmaxy(stdscr);
    short col = getmaxx(stdscr);
    
    set_member_attr(HTML.offsetHeight, row);
    set_member_attr(HTML.offsetWidth, col);

    HTML.children = g_array_new(0, true, sizeof(DomElement *));
    
    WINDOW *html_win = derwin(stdscr, HTML.offsetHeight, HTML.offsetWidth, 0, 0);
    HTML.panel = new_panel(html_win);
    html_tag[GUMBO_TAG_DIV] = handle_div;
}


void _capture_assertion(const char *exprstr, const char *file, int line)
{
    assertion_stmt = exprstr;
    assertion_file = file;
    assertion_lineno = line;
    raise(SIGABRT);
}

static void __attribute__((destructor(101)))
curses_fini()
{
    endwin();
    if (signum == SIGABRT && assertion_lineno) {
        fprintf(stderr, "%s:%d AssertionError: '%s'\n", assertion_file, assertion_lineno, assertion_stmt);
    }
    else if (signum) {
        fprintf(stderr, "%s\n", strsignal(signum));
    }
}

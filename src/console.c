#include <curses.h>
#include <panel.h>
#include <gmodule.h>
#include <stdarg.h>
#include "panic.h"
#include "curses-dom.h"
#include "curses-console.h"



static struct {
    PANEL *panel;
    WINDOW *content;
    int length;
    int maxlen;
    int hidden;

    GList *head;
    GList *tail;

} console;

#define CONSOLE_INITIALIZED     (console.panel)

int console_init(int maxlen)
{
    assert(stdscr);
    assert(! CONSOLE_INITIALIZED);

    
    WINDOW *win = derwin(stdscr, TERM_HEIGHT, TERM_WIDTH, 0, 0);
    console.panel = new_panel(win);
    console.content = derwin(win, TERM_HEIGHT - 2, TERM_WIDTH - 2, 1, 1);
    console.maxlen = maxlen;
    hide_panel(console.panel);
    console.hidden = true;
}

int console_log(const char *format, ...)
{
    char *message;
    va_list ap;
    va_start(ap, format);
    
    int size = vsnprintf(NULL, 0, format, ap);
    assert(size);
    panic_if_not((message = malloc(size + 1)), "malloc");
    vsnprintf(message, size + 1, format, ap);
    va_end(ap);

    if (! console.head) {
        console.tail = console.head = g_list_prepend(NULL, message);
    }
    else if (console.length < console.maxlen) {
        console.head = g_list_prepend(console.head, message);
    }
    else {
        // Hard to understand glib's documentation for removing last element
        GList *tail = console.tail->prev;
        free(console.tail->data);
        tail->next = NULL;
        g_list_free(console.tail);
        console.length -= 1;
        console.head = g_list_prepend(console.head, message);
        console.tail = tail;
    }
    console.length += 1;
}

int console_resize(int size[2])
{
    WINDOW *win = panel_window(console.panel);
    wresize(win, size[0], size[1]);
    wresize(console.content, size[0] - 2, size[1] - 2);

}

static int
print_messages(GList *node, int stop)
{
    if ((! node) || (! stop)) {
        return;
    }
    
    WINDOW *win = console.content;

    mvwprintw(win, getcury(win), 0, "%s", node->data);
    wmove(win, getcury(win) + 1, 0);
    for (int i = 0; i < getmaxx(win); i++) {
        waddch(win, '-');
    }
    return print_messages(node->next, stop - 1);
}

int console_toggle()
{
    WINDOW *win = panel_window(console.panel);
    if (console.hidden) {
        box(win, '|', '-');
        wmove(console.content, 0, 0);
        print_messages(console.head, TERM_HEIGHT - 2);
        show_panel(console.panel);
        console.hidden = 0;
    }
    else {
        wclear(win);
        werase(console.content);
        hide_panel(console.panel);
        console.hidden = 1;
    }
}


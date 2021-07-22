#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "curses-dom.h"

#define set_member_attr(where, value) (assert(sizeof(where) == sizeof(value)), memcpy((void *) & where, & value, sizeof(value)))

static volatile sig_atomic_t window_size_changed;

DomElement HTML;
struct Display display;


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

        
        TPixel **new_buffer = malloc(w.ws_row * sizeof(TPixel *));
        for (int i = 0; i < w.ws_row; i++) {
            new_buffer[i] = malloc(w.ws_col * sizeof(TPixel));
            memset(new_buffer[i], 0, w.ws_col * sizeof(TPixel));
            if (i < display.height) {
                memcpy(
                    new_buffer[i],
                    display.buffer[i], 
                    display.width > w.ws_col 
                        ? sizeof(TPixel) * w.ws_col 
                        : display.width * sizeof(TPixel)
                    );
            }
        }

        for (int i = 0; i < display.height; i++) {
            free(display.buffer[i]);
        }
        free(display.buffer);
        display.buffer = new_buffer;
        set_member_attr(display.height, w.ws_row);
        set_member_attr(display.width, w.ws_col);
        return 1;
    }
    return window_size_changed;
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
    sigwinch.sa_flags = SA_RESTART;
    sigwinch.sa_handler = Handle_SIGWINCH;
    sigaction(SIGWINCH, & sigwinch, NULL);

    short row = getmaxy(stdscr);
    short col = getmaxx(stdscr);   
    set_member_attr(display.height, row);
    set_member_attr(display.width, col);

    display.buffer = malloc(row * sizeof(TPixel *));
    for (int i = 0; i < row; i++) {
        display.buffer[i] = malloc(col * sizeof(TPixel));
        
        for (int j = 0; j < col; j++) {
            display.buffer[i][j].text = ' ';
        }
    }
}


static void __attribute__((destructor(101)))
curses_fini()
{ 
    for (int i = 0; i < display.height; i++) {
        free(display.buffer[i]);
    }
    free(display.buffer);
    endwin();
}

int print_display()
{
    for (int i = 0; i < display.height; i++) {
        for (int j = 0; j < display.width; j++) {
            mvaddch(i, j, display.buffer[i][j].text);
        }
    }
    refresh();
}
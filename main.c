#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <panel.h>
#include <gmodule.h>
#include <ctype.h>
#include "katana.h"
#include "gumbo.h"
#include "curses-dom.h"
#include "curses-console.h"
#include "panic.h"


static inline ssize_t
alloc_buffer(int fd, char *output[])
{
    struct stat filestat;
    ssize_t result;

    if (fstat(fd, &filestat) < 0) return -1;

    result = filestat.st_size;
    *output = calloc(1, result + 1);
    return result;
}

static inline ssize_t
read_file(FILE *fp, char *buffer[], ssize_t size)
{
    int start = 0;
    int bytes_read;
    while ((bytes_read = fread(*buffer + start, 1, size - start, fp))) {
        start += bytes_read;
    }
    return bytes_read;
}



int print_row = 0, print_col = 0;


int main(int argc, const char *argv[])
{
    assert(argc == 2);
    FILE *fp;
    char *file_contents;
    ssize_t file_size;

    panic_if_not((fp = fopen(argv[1], "r")), "fopen");
    panic_if_err((file_size = alloc_buffer(fileno(fp), & file_contents)), "alloc_buffer");
    
    console_init(128);
    console_log("hello                                                                                                              10");
    console_log("world");


    read_file(fp, &file_contents, file_size);
    
    DomElement *root = domtree_new(
        "<html>"
            "<div id='div1' style='border:1 red;'>"
                "Hello"
            "</div>"
        "</html>"
    );
    DomElement *elt = getElementById(root, "div1");
    while (1) {
        int term_size[2];
        if (terminal_did_change_size(term_size)) {
            erase();
            resizeterm(TERM_HEIGHT, TERM_WIDTH);

            console_resize(term_size);
        }
        doupdate();
        update_panels();
        refresh();

        getch();
        console_toggle();
    }
    domtree_delete(root);
    
    
    /*
    GumboOutput *out = gumbo_parse_with_options(
        & kGumboDefaultOptions,
        file_contents,
        file_size
    );


    refresh();
    
    

    
    handle_node(out->root, & HTML);
    while (1) {
        if (terminal_did_change_size(term_size)) {
            mvprintw(0, 0, "Term size: %d, %d\n", HTML.offsetWidth, HTML.offsetHeight);
        }

        print_row = 0;
        doupdate();
        update_panels();
        refresh();
        getch();

        assert(0 && "Hello");
    }

    gumbo_destroy_output(& kGumboDefaultOptions, out);
    free(file_contents);
    */
}

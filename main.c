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
#include "gumbo.h"
#include "curses-dom.h"

#define panic(stmt, msg) if (stmt) perror(msg), exit(1)


static inline int handle_node(GumboNode *node, DomElement *parent);

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

static inline int
handle_document(const GumboDocument *node, DomElement *parent)
{

}

static inline int
find_next_non_whitespace(char *head)
{
    int offset = 0;
    do {
        offset ++;
    } while (isspace(*(head + offset)));
    return offset;
}

static inline char
printed_char(const char **headptr, int *last_char_is_whitespace)
{   
    const char *head = *headptr;
    char c;
    
    if (! isspace(*head)) {
        *last_char_is_whitespace = 0;
        (*headptr)++;
        return c = *head;
    }
    else if (isspace(*head) && ! *last_char_is_whitespace) {
        (*headptr)++;
        *last_char_is_whitespace = 1;
        return c = ' ';
    }
    else {
        *headptr += find_next_non_whitespace(head);
        return 0;
    }
}

static inline int
strip_white_space(const char *text, char buffer[], size_t size)
{   
    int last_is_whitespace = isspace(*text);
    int i = 0;
    while (*text && i < size) {
        char c = printed_char(& text, & last_is_whitespace);
        buffer[i] = c;
        if (c) i++;
    }
    return i;
}

static inline int
handle_element(const GumboElement *node, DomElement *parent)
{
    const GumboVector *children = & node->children;
    DomElement *new_node = DomElement_new(parent);
    
    for (int i = 0; i < children->length; i++) {
        handle_node(children->data[i], new_node);
    }
}


int print_row = 0, print_col = 0;


static inline int
handle_text(const GumboText *node, DomElement *parent)
{
    const char *text = node->text;
    size_t textlen = strlen(text);
    char *buffer = malloc(textlen + 1);
    buffer[textlen] = 0;
    strip_white_space(text, buffer, textlen);
    mvprintw(print_row, 0, "%s", buffer);
    print_col = 0;
    print_row++;
    free(buffer);
}



static inline int
handle_node(GumboNode *node, DomElement *parent)
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

typedef struct Panel {
    WINDOW *window;
    PANEL *panel;
} Panel;


struct Panel *
Panel_new(WINDOW *orig, int nlines, int ncols, int begin_y, int begin_x)
{
    struct Panel *panel = malloc(sizeof(struct Panel));
    panel->window = derwin(orig, nlines, ncols, begin_y, begin_x);
    panel->panel = new_panel(panel->window);
    box(panel->window, '|', '-');
    return panel;
}


int main(int argc, const char *argv[])
{
    assert(argc == 2);
    FILE *fp;
    char *file_contents;
    ssize_t file_size;

    panic(! (fp = fopen(argv[1], "r")), "fopen");
    panic((file_size = alloc_buffer(fileno(fp), & file_contents)) < 0, "alloc_buffer");
    
    read_file(fp, &file_contents, file_size);
    GumboOutput *out = gumbo_parse_with_options(
        & kGumboDefaultOptions,
        file_contents,
        file_size
    );

    Panel *panel1 = Panel_new(stdscr, HTML.scrollHeight - 2, HTML.scrollWidth - 2, 1, 1);
    Panel *panel2 = Panel_new(panel1->window, HTML.scrollHeight - 4, HTML.scrollWidth - 4, 1, 1);
    Panel *panel3 = Panel_new(panel2->window, HTML.scrollHeight - 6, HTML.scrollWidth - 6, 1, 1);

    waddstr(panel3->window, "Hello");
    wrefresh(panel3->window);
    refresh();
    

    int term_size[2];
    handle_node(out->root, & HTML);
    while (1) {
        if (terminal_did_change_size(term_size)) {
            mvprintw(0, 0, "Term size: %d, %d\n", HTML.scrollWidth, HTML.scrollHeight);
        }

        print_row = 0;
        doupdate();
        update_panels();
        refresh();
        getch();
    
    }

    gumbo_destroy_output(& kGumboDefaultOptions, out);
    free(file_contents);
}

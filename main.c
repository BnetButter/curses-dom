#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include <gmodule.h>
#include <ctype.h>
#include "gumbo.h"
#include "curses-dom.h"

#define panic(stmt, msg) if (stmt) perror(msg), exit(1)




static inline int handle_node(GumboNode *node);

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
handle_document(const GumboDocument *node)
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
handle_element(const GumboElement *node)
{
    const GumboVector *children = & node->children;
    for (int i = 0; i < children->length; i++) {
        handle_node(children->data[i]);
    }
}


int print_row = 0, print_col = 0;


static inline int
handle_text(const GumboText *node)
{
    const char *text = node->text;
    char *buffer = malloc(strlen(text) + 1);
    
    strip_white_space(text, buffer, strlen(text));
    for (int i =  0; i < strlen(buffer) + 1; i++) {
        display.buffer[print_row][print_col].text = buffer[print_col];
        print_col ++;
    }
    print_col = 0;
    print_row++;
    free(buffer);
}

typedef void (*tag_handler)(GumboNode *, void *);
typedef tag_handler TagTable[GUMBO_TAG_LAST];


static inline int
handle_node(GumboNode *node)
{

    switch (node->type) {
        case GUMBO_NODE_DOCUMENT:
            handle_document((GumboDocument *) & node->v);
            break;
        case GUMBO_NODE_ELEMENT:
            handle_element((GumboElement *) & node->v);
            break;
        case GUMBO_NODE_TEXT:
            handle_text((GumboText *) & node->v);
            break;
        case GUMBO_NODE_WHITESPACE:
            return;
        default:
            assert(0 && "Unhandled node type");
            exit(1);
    }
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
    

    int term_size[2];

    int tmp = 0;
    while (1) {
        if (terminal_did_change_size(term_size)) {
            mvprintw(0, 0, "Term size: %d, %d\n", HTML.scrollWidth, HTML.scrollHeight);
        }
        print_row = 0;
        handle_node(out->root);
        print_display();
    }

    gumbo_destroy_output(& kGumboDefaultOptions, out);
    free(file_contents);
}

#include <panel.h>
#include <curses.h>
#include <stdlib.h>
#include "katana.h"
#include "_dom.h"
#include "panic.h"



static inline struct DomNode *
DomNode_new(struct DomNode *parent, size_t size, DomNodeType type)
{
    struct DomNode *self = malloc(size);
    self->type = type;
    if (parent) {
        struct DomElement *p_elt = DomNode_cast(parent, DOM_NODE_ELT);
        assert(p_elt && "Invalid parent node type");
        g_ptr_array_add(p_elt->children, self);
    }
    self->parent = parent;
    return self;
}

static inline void
DomNode_delete(struct DomNode *self)
{
    if (DomNode_parent(self)) {
        struct DomElement *parent = DomNode_cast(self->parent, DOM_NODE_ELT);
        g_ptr_array_remove(parent->children, self);
    }
    free(self);
}

static inline GHashTable *
style_new(const char *value)
{
    KatanaOutput *output = katana_parse(value, strlen(value), KatanaParserModeDeclarationList);
    GHashTable *table = g_hash_table_new(g_str_hash, g_str_equal);

    KatanaArray *decls = output->declarations;
    for (int i = 0; i < decls->length; i++) {
        KatanaDeclaration *decl = decls->data[i];
        
        for (int j = 0; j < decl->values->length; j++) {
            KatanaValue *value = decl->values->data[j];
            assert(value);
        }
        assert(decl);
    }    
    katana_destroy_output(output);
}

static inline void
style_delete()
{

}

static inline void
DomText_init(struct DomText *self, const char *text)
{
    size_t slen = strlen(text);
    char *buffer = malloc(slen + 1);
    strip_white_space(text, buffer, slen);
    self->text = g_string_new(buffer);
    free(buffer);
}

static inline void
DomText_dtor(struct DomText *self)
{
    g_string_free(self->text, true);
}

static inline void
DomElement_init(struct DomElement *self, GumboElement *gumbo_node, struct DomRoot *root)
{
    assert(DomNode_cast(self, DOM_NODE_ELT));
    self->children = g_ptr_array_new();
    self->attributes = g_hash_table_new(g_str_hash, g_str_equal);
    for (int i = 0; i < gumbo_node->attributes.length; i++) {
        GumboAttribute *attr = gumbo_node->attributes.data[i];
        assert(attr->attr_namespace == GUMBO_ATTR_NAMESPACE_NONE && "Invalid attribute namespace");
        
        // insert hash table for style look up
        if (g_str_equal("style", attr->name)) {
            g_hash_table_insert(self->attributes, attr->name, style_new(attr->value));
        }
        else {
            g_hash_table_insert(self->attributes, attr->name, attr->value);
        }
        
        // cache element id
        if (g_str_equal("id", attr->name)) {
            g_hash_table_insert(root->elementsById, attr->value, self);
        }
    }

    self->tag = gumbo_node->tag;
    WINDOW *win = derwin(DomNode_parent(self) ? WINDOW_of(DomNode_parent(self)): stdscr, 1, 1, 0, 0);
    self->panel = new_panel(win);
    return self;
}

static inline void
DomElement_dtor(struct DomElement *self)
{   

    /* Create copy so we can mutate original */
    GPtrArray *copy = g_ptr_array_copy(self->children, NULL, NULL);

    for (int i = copy->len - 1; i >= 0; i--) {
        struct DomNode *node = g_ptr_array_index(copy, i);
        if (node->type & DOM_NODE_TEXT_BIT) {
            DomText_dtor(node);
        }
        else if (node->type & DOM_NODE_ELT_BIT) {
            // Recursively destroy children
            DomElement_dtor(node);
        }
        else {
            assert(0 && "Unknown DomNode type");
        }
        DomNode_delete(node);
    }
    g_ptr_array_free(copy, true);
    g_ptr_array_free(self->children, true), self->children = NULL;
    
    GHashTable *style = g_hash_table_lookup(self->attributes, "style");
    if (style) {
        g_hash_table_destroy(style);
    }

    g_hash_table_destroy(self->attributes), self->attributes = NULL;
    delwin(WINDOW_of(self));
    del_panel(PANEL_of(self)), self->panel = NULL;
}


static inline void handle_gumbo_node(GumboNode *node, DomElement *parent, struct DomRoot *root);

static inline void
DomRoot_init(struct DomRoot *self, const char *text)
{
    size_t textlen = strlen(text);
    self->html_buffer = malloc(textlen + 1);
    memcpy(self->html_buffer, text, textlen);
    self->output = gumbo_parse_with_options(
        &kGumboDefaultOptions, 
        self->html_buffer,
        textlen
    );
    self->elementsById = g_hash_table_new(g_str_hash, g_str_equal);
    
    GumboNode *gumbo_root = self->output->root;
    switch (gumbo_root->type) {
        case GUMBO_NODE_ELEMENT:
            DomElement_init(self, & gumbo_root->v.element, self);
            break;
        default:
            assert(0 && "Output is not GUMBO_NODE_ELEMENT");
    }
    
    GumboVector *vec = & gumbo_root->v.element.children;
    for (int i = 0; i < vec->length; i++) {
        handle_gumbo_node(vec->data[i], self, self);
    }
}

static inline void
DomRoot_dtor(struct DomRoot *self)
{
    assert(DomNode_cast(self, DOM_NODE_ROOT));
    gumbo_destroy_output(& kGumboDefaultOptions, self->output);
    free(self->html_buffer);
    DomElement_dtor(self);
    g_hash_table_destroy(self->elementsById);
    
}

static inline void
handle_gumbo_element(GumboElement *gumbonode, struct DomElement *parent, struct DomRoot *root)
{
    struct DomElement *domnode = DomNode_new(parent, sizeof(struct DomElement), DOM_NODE_ELT);
    DomElement_init(domnode, gumbonode, root);
    for (int i = 0; i < gumbonode->children.length; i++) {
        handle_gumbo_node(gumbonode->children.data[i], domnode, root);
    }
}

static inline void
handle_gumbo_text(GumboText *gumbonode, struct DomElement *parent)
{
    struct DomText *text = DomNode_new(parent, sizeof(struct DomText), DOM_NODE_TEXT);
    DomText_init(text, gumbonode->text);
}

static inline void
handle_gumbo_node(GumboNode *node, DomElement *parent, struct DomRoot *root)
{
    switch (node->type) {
        case GUMBO_NODE_DOCUMENT:
            return;
        case GUMBO_NODE_ELEMENT:
            return handle_gumbo_element((GumboElement *) & node->v, parent, root);
        case GUMBO_NODE_TEXT:
            return handle_gumbo_text((GumboText *) & node->v, parent);
        case GUMBO_NODE_WHITESPACE:
            return;
        default:
            assert(0 && "Unhandled node type");
            exit(1);
    }
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

struct DomElement *
domtree_new(const char *html)
{
    struct DomRoot *tree = DomNode_new(NULL, sizeof(struct DomRoot), DOM_NODE_ROOT);
    DomRoot_init(tree, html);
    assert(((struct DomNode *) tree)->type == DOM_NODE_ROOT);
    return tree;
}

void domtree_delete(struct DomElement *_self)
{
    DomRoot_dtor(DomNode_cast(_self, DOM_NODE_ROOT));
    DomNode_delete(_self);
}

DomElement *
getElementById(DomElement *root, const char *id)
{
    struct DomRoot *elt = DomNode_cast(root, DOM_NODE_ROOT);
    if (elt) {
        return g_hash_table_lookup(elt->elementsById, id);
    }
    else {
        return NULL;
    }
}

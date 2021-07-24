#ifndef _DOM_H
#define _DOM_H

// For internal use. Do not include directly

#ifdef __cplusplus
extern "C" {
#endif
#include <gmodule.h>
#include <panel.h>
#include <assert.h>
#include "gumbo.h"

typedef struct DomElement DomElement;

#define DOM_NODE_ELT_BIT   (1 << 0)
#define DOM_NODE_ROOT_BIT   (1 << 1)
#define DOM_NODE_TEXT_BIT   (1 << 2)

typedef enum {
    DOM_NODE_ELT = DOM_NODE_ELT_BIT,
    DOM_NODE_ROOT = DOM_NODE_ELT_BIT | DOM_NODE_ROOT_BIT,
    DOM_NODE_TEXT = DOM_NODE_TEXT_BIT,
} DomNodeType;

struct DomNode {
    DomNodeType type;
    struct DomElement *parent;
};

struct DomText {
    struct DomNode _;
    GString *text;
};

struct DomElement {
    struct DomNode _;
    GumboTag tag;

    const short scrollHeight;
    const short scrollWidth;

    const short offsetHeight;
    const short offsetWidth;
    
    const short clientWidth;
    const short clientHeight;

    const short top;
    const short left;
    
    short scrollTop;
    short scrollLeft;

    GHashTable *attributes;
    GPtrArray *children;
    PANEL *panel;
};

struct DomRoot {
    struct DomElement _;
    GumboOutput *output;
    char *html_buffer;
    GHashTable *elementsById;
};

typedef struct {
    short row;
    short col;
} DomDim;

/**
 * @brief Downcast to DomNode and retrieve type
*/
#define DomNode_type(node)  (((struct DomNode *) (node))->type)

/**
 * @brief Downcast to DomNode and retrieve parent
*/
#define DomNode_parent(node)    (((struct DomNode *) (node))->parent)

/**
 * @brief Dynamic cast node to type. If type does not match node type, eval to NULL.
*/
#define DomNode_cast(node, type) ((DomNode_type(node) & (type##_BIT) ? (node) : NULL))



static inline PANEL *
PANEL_of(struct DomNode *node)
{
    if (node->type & DOM_NODE_TEXT_BIT) {
        struct DomElement *parent = DomNode_cast(node->parent, DOM_NODE_ELT);
        assert(parent);
        return parent->panel;
    }
    return ((struct DomElement *) DomNode_cast(node, DOM_NODE_ELT))->panel;
}

/**
 * @brief Get the window of DomElement's interior panel
*/
#define WINDOW_of(dom)    (panel_window(PANEL_of(dom)))

/**
 * @brief Request parent DOM element for additional size. Propagate request to
 *  top level Element
 * @param parent The parent element of this child
 * @param request_size Request parent element for additional size
 * @param received Size granted by parent element
*/
static inline int request_size(DomElement *parent, DomDim request, DomDim *received);


/**
 * @brief Reduce all whitespace to at most one. 
 * @param text Source text
 * @param buffer output buffer
 * @param size  sizeof output buffer
 * @return Number of bytes written to buffer, or -1 if buffer is too small
*/
static inline int strip_white_space(const char *text, char buffer[], size_t size);
static inline int find_next_non_whitespace(char *head);
static inline char printed_char(const char **headptr, int *last_char_is_whitespace);

/**
 * @brief Callback for GUMBO_ELEMENT
 */
static inline int handle_element(const GumboElement *node, DomElement *parent);

/**
 * @brief Callback for GUMBO_TEXT
 */
static inline int handle_text(const GumboText *node, DomElement *parent);

/**
 * @brief Callback for GUMBO_DOCUMENT
 */
static inline int handle_document(const GumboDocument *node, DomElement *parent);


#ifdef __cplusplus
}
#endif
#endif
#ifndef _DOM_H
#define _DOM_H

// For internal use. Do not include directly

#ifdef __cplusplus
extern "C" {
#endif

#include <panel.h>
#include "curses-dom.h"

/**
 * @brief Get the window of DomElement's interior panel
*/
#define WINDOW_of(dom)      (panel_window(dom->panel))

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
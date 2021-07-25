#ifndef _STYLE_HANDLERS_H
#define _STYLE_HANDLERS_H

#include <gmodule.h>
#include "katana.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*f_StyleHandler)(GHashTable *styletable, const char *key, KatanaValue *value);

extern GHashTable *CSS_Attribute_Handlers;

#ifdef __cplusplus
}
#endif
#endif
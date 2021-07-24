#ifndef PANIC_H
#define PANIC_H
#include <stdio.h>
#include <stdlib.h>

#define panic_if(stmt, msg) if ((stmt)) perror(msg), exit(EXIT_FAILURE)
#define panic_if_not(stmt, msg) if (! (stmt)) perror(msg), exit(EXIT_FAILURE)
#define panic_if_err(stmt, msg) if ((stmt < 0)) perror(msg), exit(EXIT_FAILURE)

#endif
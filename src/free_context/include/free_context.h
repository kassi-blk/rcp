#ifndef _FREE_CONTEXT_H

#include "consts.h"
#include "struct.h"

void fc_add (void * object, int type, struct free_context * fc);
void exit_free (struct free_context * fc, int status);

#define _FREE_CONTEXT_H
#endif

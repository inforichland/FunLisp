#ifndef __READER_H__
#define __READER_H__

#include "types.h"
#include <stdlib.h>

void initialize_reader();
void cleanup_reader();

object_t *read(FILE*);

bool nilp(object_t*);
bool truep(object_t*);
bool falsep(object_t*);

#endif

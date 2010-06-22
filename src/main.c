/************************************************
 * Title: main.c
 * Author: Tim Wawrzynczak
 * License: BSD 3-clause
 * Date: 6/15/10
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"
#include "writer.h"
#include "eval.h"
#include "types.h"
#include "primitives.h"

int main() {
  initialize_types();
  create_primitives();
  
  printf("This is the REPL! have fun\n\n");

  while (1) {
    printf("repl> ");
    object_t *r = read(stdin);
    if (r->type == t_symbol && !strcmp(r->values.symbol.value, "quit"))
      break;
    else{
      object_t *result = eval(r, get_global_env());
      if (result == NULL) {
	printf("Unbound symbol!\n");
      } else {
	write(result);
      }
    }
    printf("\n");
  }

  cleanup_types();

  printf("Goodbye!\n");
  return 0;
}

/*************************************************
 * Title: writer.c
 * Author: Tim Wawrzynczak
 * License: BSD 3-clause
 * Date: 6/15/10
 ************************************************/

#include <stdio.h>
#include "types.h"
#include "reader.h"

void write(object_t *obj);

void write_cons(object_t *obj) {
  object_t *car = obj->values.cons.car,
    *cdr = obj->values.cons.cdr;
  write(car);

  if (nilp(cdr)) {
    return;
  } else if (cdr->type == t_cons) {
    printf(" ");
    write_cons(cdr);
  } else {
    printf(" . ");
    write(cdr);
  }
}

void write(object_t *obj) {
  switch (obj->type) {
    case t_fixnum:
      printf("%ld", obj->values.fixnum.value);
      break;
    case t_character:
      printf("#\\");
      switch (obj->values.character.value) {
	case '\n':
	  printf("newline");
	  break;
	case ' ':
	  printf("space");
	  break;
	case '\t':
	  printf("tab");
	  break;
	default:
	  printf("%c", obj->values.character.value);
	  break;
      }
      break;
    case t_boolean:
      if (truep(obj))
	printf("#t");
      else if (falsep(obj))
	printf("#f");
      else
	die("wtf boolean!");
      break;
    case t_string:
      printf("\"%s\"\n", obj->values.string.value);
      break;
    case t_symbol:
      printf("%s", obj->values.symbol.value);
      break;
    case t_cons:
      if (nilp(obj))
	printf("()");
      else {
	printf("(");
	write_cons(obj);
	printf(")");
      }
      break;
    default:
      die("Unknown type (can't write)");
  }
}
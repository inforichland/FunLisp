#include <stdio.h>
#include "types.h"

/* for primitives, 'arguments' is a list of arguments,
   hence a cons'ed up list of object_t* 's */

object_t *prim_car(object_t *arguments) {
  object_t *first = car(arguments);
  return car(first);
}

object_t *prim_cdr(object_t *arguments) {
  object_t *first = car(arguments);
  return cdr(first);
}

object_t *prim_cons(object_t *arguments) {
  object_t *first = car(arguments);
  object_t *second = car(cdr(arguments));
  return create_cons(first, second);
}

object_t *prim_add(object_t *arguments) {
  long result = 0;

  while (!nilp(arguments)) {
    result += (car(arguments))->values.fixnum.value;
    arguments = cdr(arguments);
  }

  return create_fixnum(result);
}

object_t *prim_sub(object_t *arguments) {
  long result = 0;

  if (!nilp(arguments)) {
    result = (car(arguments))->values.fixnum.value;
    arguments = cdr(arguments);
    
    while (!nilp(arguments)) {
      result -= (car(arguments))->values.fixnum.value;
      arguments = cdr(arguments);
    }
  }
  
  return create_fixnum(result);
}

object_t *prim_fixnump(object_t *arguments) {
  if (car(arguments)->type == t_fixnum)
    return get_true();
  else
    return get_false();
}

object_t *prim_symbolp(object_t *arguments) {
  if (car(arguments)->type == t_symbol)
    return get_true();
  else
    return get_false();
}

object_t *prim_stringp(object_t *arguments) {
  if (car(arguments)->type == t_string)
    return get_true();
  else
    return get_false();
}

object_t *prim_functionp(object_t *arguments) {
  if (car(arguments)->type == t_primitive)
    return get_true();
  else
    return get_false();
}

void create_primitives() {
  make_primitive(create_symbol("car"), prim_car);
  make_primitive(create_symbol("cdr"), prim_cdr);
  make_primitive(create_symbol("cons"), prim_cons);
  make_primitive(create_symbol("+"), prim_add);
  make_primitive(create_symbol("-"), prim_sub);
  make_primitive(create_symbol("fixnum?"), prim_fixnump);
  make_primitive(create_symbol("symbol?"), prim_symbolp);
  make_primitive(create_symbol("string?"), prim_stringp);
  make_primitive(create_symbol("function?"), prim_functionp);
}

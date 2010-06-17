#include "types.h"

/**********************************
 * evaluation 
 *********************************/

bool evaluate_to_self(type_t t) {
  switch (t) {
    case t_boolean:
    case t_fixnum:
    case t_character:
    case t_string:
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}

bool quoted(object_t *exp) {
  return (is_cons(exp) && quotep(car(exp)));
}

bool definition(object_t *exp) {
  return (is_cons(exp) && definep(car(exp)));
}

object_t *eval(object_t *exp, object_t *env) { 
  if (evaluate_to_self(exp->type)) 
    return exp;
  
  if (quoted(exp)) 
    return car(cdr(exp));
  
  if (definition(exp)) {
    object_t *symb = car(cdr(exp)),
      *val = car(cdr(cdr(exp)));
    
    if (val == NULL) {
      create_new_variable(symb, get_nil(), env);
    } else {
      create_new_variable(symb,
			  eval(val, env),
			  env);
    }
    return symb;
  }

  if (is_symbol(exp)) {
    return find_variable_value(exp, env);
  }
  
  die("Can't eval!\n");
  
  return NULL;
}

#include "types.h"

/**********************************
 * evaluation 
 *********************************/

object_t *eval(object_t*, object_t*);

bool evaluate_to_self(object_t *exp) {
  switch (exp->type) {
    case t_boolean:
    case t_fixnum:
    case t_character:
    case t_string:
      return TRUE;
    default:
      if (nilp(exp)) return TRUE;
  }
  return FALSE;
}

bool quoted(object_t *exp) {
  return (is_cons(exp) && quotep(car(exp)));
}

bool definition(object_t *exp) {
  return (is_cons(exp) && definep(car(exp)));
}

bool function(object_t *exp) {
  return (is_cons(exp) && !nilp(exp) &&
	  car(exp)->type == t_symbol);
}

object_t *make_arguments(object_t *args, object_t *env) {
  if (nilp(args)) { 
    return get_nil();
  } else {
    return create_cons(eval(car(args), env),
		       make_arguments(cdr(args), env));
  }
}

bool maybe_eval_to_function(object_t *exp) {
  return (is_cons(car(exp)));
}

object_t *eval(object_t *exp, object_t *env) { 
  object_t *ret = NULL;
  
  if (evaluate_to_self(exp)) {
    ret = exp;
  }
  else if (quoted(exp)) {
    ret = car(cdr(exp));
  }
  else if (definition(exp)) {
    object_t *symb = car(cdr(exp)),
      *val = car(cdr(cdr(exp)));
    
    if (val == NULL) {
      create_new_variable(symb, get_nil(), env);
    } else {
      create_new_variable(symb,
			  eval(val, env),
			  env);
    }
    ret = symb;
  }
  else if (is_symbol(exp)) {
    //printf("\nfound symbol: %s\n\n", exp->values.symbol.value);
    ret = find_variable_value(exp, env);
  }
  else if (function(exp)) {
    object_t *arguments = make_arguments(cdr(exp), env);
    object_t *func = eval(car(exp), env);

    if (func == NULL || func == get_nil() ||
	func->type != t_primitive) {
      fprintf(stderr, "func: %d\n", (unsigned int)func);
      //fprintf(stderr, "type: %d\n", func->type);
      die("Not a primitive!\n");
    } else {
      ret = (func->values.primitive.function)(arguments);
    }
  } else if (maybe_eval_to_function(exp)) {
    object_t *c = car(exp);
    object_t *func = eval(c, env);
    if (!(func == NULL || nilp(func))) {
      object_t *arguments = make_arguments(cdr(exp), env);
      ret = (func->values.primitive.function)(arguments);
    }
    else {
      die("Not a function!\n");
    }
  }
  else {
    die("Can't eval!\n");
  }
  
  return ret;
}

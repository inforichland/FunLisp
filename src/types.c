#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

unsigned int MAX_SYMBOLS = 100;

object_t *create_object(type_t type) {
  object_t *obj = malloc(object_size);
  if (obj == NULL) die("Unable to allocate memory!\n");
  
  obj->type = type;
  return obj;
}

/** static objects **/
object_t *true_obj, *false_obj;
object_t *nil;
object_t *quote;
object_t *define;
object_t *lambda;
object_t **symbol_table;

/* environments */
object_t *empty_env;
object_t *global_env;

object_t *get_global_env() { return global_env; }

#define is_p(name,obj) bool name(object_t *o) { return (o == obj); }

is_p(nilp,nil);
is_p(truep,true_obj);
is_p(falsep,false_obj);
is_p(quotep,quote);
is_p(definep,define);

#define get_obj(name,obj) object_t *name() { return obj; }

get_obj(get_nil,nil);
get_obj(get_true,true_obj);
get_obj(get_false,false_obj);
get_obj(get_quote,quote);
get_obj(get_define,define);

object_t *create_fixnum(long val) { 
  object_t *obj = create_object(t_fixnum);
  obj->values.fixnum.value = val;
  return obj;
}

object_t *create_boolean(char val) {
  object_t *obj = create_object(t_boolean);
  obj->values.boolean.value = val;
  return obj;
}

object_t *create_character(char val) {
  object_t *obj = create_object(t_character);
  obj->values.character.value = val;
  return obj;
}

object_t *lookup_symbol(char *val) {
  if (symbol_table == NULL) return NULL;
  
  int i;
  for (i = 0; i < MAX_SYMBOLS; i++) {
    if (symbol_table[i] != NULL && 
	!strcmp(symbol_table[i]->values.symbol.value, val)) 
      return symbol_table[i];
  }

  return NULL;
}

object_t *create_symbol(char *val) {
  object_t *tmp = lookup_symbol(val);
  if (tmp == NULL) {
    /* create the object */
    object_t *obj = create_object(t_symbol);
    obj->values.symbol.value = malloc(strlen(val) + 1);
    if (obj->values.symbol.value == NULL)
      die("Out of memory in create_symbol!\n");
    strcpy(obj->values.symbol.value, val);

    int i;
    if (symbol_table == NULL) {
      symbol_table = (object_t**) malloc(MAX_SYMBOLS * sizeof(object_t*));
      if (symbol_table == NULL) {
	die("Out of memory in create_symbol!\n");
      }

      for (i = 0; i < MAX_SYMBOLS; i++)
	symbol_table[i] = NULL;
    }

    bool added = FALSE;
    for (i = 0; i < MAX_SYMBOLS; i++) {
      if (symbol_table[i] == NULL) {
	added = TRUE;
	symbol_table[i] = obj;
	break;
      }
    }

    if (!added) {
      // die("Symbol table full!");
      unsigned int old = MAX_SYMBOLS;
      MAX_SYMBOLS *= 2;
      if (MAX_SYMBOLS < old) die("Symbol table full!\n");

      symbol_table = (object_t**) realloc(symbol_table, MAX_SYMBOLS * sizeof(object_t*));
      if (symbol_table == NULL)
	die("Error realloc()-ing symbol table!\n");
      return create_symbol(val);
    } 
    return obj;
  } else 
    return tmp;
}

object_t *create_string(char *val) {
  object_t *obj = create_object(t_string);
  obj->values.string.value = malloc(strlen(val) + 1);
  if (obj->values.string.value == NULL)
    die("Out of memory in create_string!\n");

  strcpy(obj->values.string.value, val);
  return obj;
}

object_t *create_cons(object_t *car, object_t *cdr) {
  object_t *obj = create_object(t_cons);
  obj->values.cons.car = car;
  obj->values.cons.cdr = cdr;
  return obj;
}

object_t *create_primitive(primitive p) {
  object_t *obj = create_object(t_primitive);
  obj->values.primitive.function = p;
  return obj;
}

object_t *car(object_t *obj) { return obj->values.cons.car; }
object_t *cdr(object_t *obj) { return obj->values.cons.cdr; }

/* environments */

object_t *parent_env(object_t *env) { return cdr(env); }

object_t *frame_vars(object_t *frame) { return car(frame); }
object_t *frame_vals(object_t *frame) { return cdr(frame); }

void add_new_binding(object_t *variable, object_t *value,
		     object_t *frame) {
  frame->values.cons.car = create_cons(variable, car(frame));
  frame->values.cons.cdr = create_cons(value, cdr(frame));
}

object_t *find_variable_value(object_t *var, object_t *env) {
  object_t *frame, *vars, *vals;
  while (env != get_nil()) {
    frame = car(env);
    vars = frame_vars(frame);
    vals = frame_vals(frame);
    while (vars != get_nil()) {
      if (var == car(vars)) {
	return car(vals);
      }
      
      vars = cdr(vars);
      vals = cdr(vals);
    }
    env = parent_env(env);
  }

  return NULL;
}

object_t *new_frame(object_t *variables,
		    object_t *values) {
  return create_cons(variables, values);
}

object_t *extend(object_t *variables,
		 object_t *values,
		 object_t *env) {
  return create_cons(new_frame(variables, values), env);
}

void change_variable_value(object_t *var, object_t *val,
			   object_t *env) {
  object_t *frame, *vars, *vals;

  while (env != get_nil()) {
    frame = car(env);
    vars = frame_vars(frame);
    vals = frame_vals(frame);
    while (vars != get_nil()) {
      if (var == car(vars)) {
	vals->values.cons.car = val;
	return;
      }
      vars = cdr(vars);
      vals = cdr(vals);
    }
    env = parent_env(env);
  }
  die("Variable not bound!\n");
}

void create_new_variable(object_t *var, object_t *val,
			 object_t *env) {
  object_t *frame = car(env);
  object_t *vars = frame_vars(frame);
  object_t *vals = frame_vals(frame);

  while (vars != get_nil()) {
    if (var == car(vars)) {
      vals->values.cons.car = val;
      return;
    }
    vars = cdr(vars);
    vals = cdr(vals);
  }
  add_new_binding(var, val, frame);
}

/* initialization */

void initialize_types() {  
  false_obj = create_boolean(FALSE);
  true_obj = create_boolean(TRUE);
  nil = create_cons(NULL,NULL);
  quote = create_symbol("quote");
  define = create_symbol("define");
  lambda = create_symbol("lambda");

  nil->values.cons.car = nil;
  nil->values.cons.cdr = nil;

  empty_env = nil;
  
  global_env = extend(nil, nil, empty_env);
}

void make_primitive(object_t *name, primitive p) {
  create_new_variable(name, create_primitive(p), global_env);
}

void die(const char *msg) {
  fprintf(stderr, msg);
  exit(1);
}

void cleanup_types() {
  /* clean up symbol table */
  int i;
  for(i = 0; i < MAX_SYMBOLS; i++) {
    if (symbol_table[i] != NULL) {
      free(symbol_table[i]);
    }
  }
  free(symbol_table);
}

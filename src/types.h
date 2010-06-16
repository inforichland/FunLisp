#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdlib.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1
#define MAX_STRING_LENGTH 1024
#define MAX_SYMBOL_LENGTH 256

typedef char bool;

typedef enum {
  t_fixnum,
  t_character,
  t_boolean,
  t_symbol,
  t_string,
  t_cons,
} type_t;

typedef struct object_t {
  type_t type;
  union {
    struct { long value; } fixnum;
    struct { char value; } character;
    struct { bool value; } boolean;
    struct { char *value; } symbol;
    struct { char *value; } string;
    struct { 
      struct object_t *car;
      struct object_t *cdr;
    } cons;
  } values;
} object_t;

object_t *create_object(type_t);
object_t *create_fixnum(long);
object_t *create_boolean(bool);
object_t *create_character(char);
object_t *create_symbol(char*);
object_t *create_string(char*);
object_t *create_cons(object_t*, object_t*);


#define is_type(type,val) ((val)->type == (type))
#define is_fixnum(val)    (is_type(t_fixnum,(val)))
#define is_boolean(val)   (is_type(t_boolean,(val)))
#define is_character(val) (is_type(t_character,(val)))
#define is_symbol(val)    (is_type(t_symbol,(val)))
#define is_string(val)    (is_type(t_string,(val)))
#define is_cons(val)      (is_type(t_cons,(val)))

#define object_size sizeof(object_t)

void die(const char *msg);


#endif

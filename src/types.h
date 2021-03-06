#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdlib.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1
#define MAX_STRING_LENGTH 1024
#define MAX_SYMBOL_LENGTH 256

typedef char bool;

typedef enum
{
	t_nil,
        t_fixnum,
        t_character,
        t_boolean,
        t_symbol,
        t_string,
        t_cons,
        t_primitive,
        t_function,
} type_t;

typedef struct object_t
{
        type_t type;
        union
        {
		struct { } nil;
                struct { long value; } fixnum;
                struct { char value; } character;
                struct { bool value; } boolean;
                struct { char *value; } symbol;
                struct { char *value; } string;
                struct
                { 
                        struct object_t *car;
                        struct object_t *cdr;
                } cons;
                struct { struct object_t *(*function)(struct object_t *arguments); } primitive;
                struct
                {
                        struct object_t *body;
                        struct object_t *env;
                        struct object_t *args;
                } function;
        };
} object_t;

/* function type */
typedef object_t *(*primitive)(object_t *arguments);

/* creating new objects */
object_t *create_fixnum(long);
object_t *create_boolean(bool);
object_t *create_character(char);
object_t *create_symbol(const char*);
object_t *create_string(char*);
object_t *create_cons(object_t*, object_t*);
object_t *create_primitive(primitive);

object_t *car(object_t*);
object_t *cdr(object_t*);

void make_primitive(object_t*, primitive);

#define is_nil(val)       ((val)->type == t_nil)
#define is_fixnum(val)    ((val)->type == t_fixnum)
#define is_boolean(val)   ((val)->type == t_boolean)
#define is_character(val) ((val)->type == t_character)
#define is_symbol(val)    ((val)->type == t_symbol)
#define is_string(val)    ((val)->type == t_string)
#define is_cons(val)      ((val)->type == t_cons)

bool nilp(object_t*);
bool truep(object_t*);
bool falsep(object_t*);
bool quotep(object_t*);
bool definep(object_t*);

object_t *get_nil(void);
object_t *get_true(void);
object_t *get_false(void);
object_t *get_quote(void);
object_t *get_define(void);

#define object_size sizeof(object_t)

/* miscellaneous */
void initialize_types(void);
void cleanup_types(void);
void die(const char *msg);

/* variables */
void create_new_variable(object_t*, object_t*, object_t*);
object_t *find_variable_value(object_t*, object_t*);

#endif

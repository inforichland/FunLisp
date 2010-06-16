#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "types.h"

/*********************************
 * dealing w/ heap objects
 ********************************/

unsigned int MAX_SYMBOLS = 100;

/* prototype */
object_t *read(FILE *f);

object_t *create_object(type_t type) {
  object_t *obj = malloc(object_size);
  if (obj == NULL) die("Unable to allocate memory!");
  
  obj->type = type;
  return obj;
}

/** static objects **/
object_t *false_obj;
object_t *true_obj;
object_t *nil;
object_t **symbol_table;

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
      for (i = 0; i < MAX_SYMBOLS; i++)
	symbol_table[i] = NULL;
    }

    char added = FALSE;
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
      if (MAX_SYMBOLS < old) die("Symbol table full!");

      symbol_table = (object_t**) realloc(symbol_table, MAX_SYMBOLS * sizeof(object_t*));
      if (symbol_table == NULL)
	die("Error realloc()-ing symbol table!");
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

void initialize_reader() {  
  false_obj = create_boolean(FALSE);
  true_obj = create_boolean(TRUE);
  nil = create_cons(NULL,NULL);
}

void cleanup_reader() {
  /* clean up symbol table */
  int i;
  for(i = 0; i < MAX_SYMBOLS; i++) {
    if (symbol_table[i] != NULL) {
      free(symbol_table[i]);
    }
  }
  free(symbol_table);
}

#define is_p(name,obj) bool name(object_t *o) { return (o == obj); }

is_p(nilp,nil);
is_p(truep,true_obj);
is_p(falsep,false_obj);

/* bool nilp(object_t *obj) { return (obj == nil); } */
/* bool truep(object_t *obj) { return (obj == true_obj); } */
/* bool falsep(object_t *obj) { return (obj == false_obj); } */

/************************************
 * reader 
 ***********************************/

void ignore_ws(FILE *f) {
  int c;
  while ((c = getc(f)) && !feof(f)) {
    if (isspace(c)) continue;
    else if (c == ';') { /* a comment */
      while (((c = getc(f)) && !feof(f)) && (c != '\n')) ;
      continue; 
    }
    ungetc(c,f);
    break;
  }
}

bool delimiter(int c) {
  return (c == EOF || c == '(' || c == ')' ||
	  c == ';' || c == '"' || isspace(c));
}

int peek(FILE *f) { 
  int c = getc(f);
  ungetc(c,f);
  return c;
}

void ensure_delimiter(FILE *f) {
  if (!delimiter(peek(f)))
    die("Expected delimiter!\n");
}

void ensure_expected(FILE *f, char *str) {
  int c;

  while (*str != '\0') {
    c = getc(f);
    if (c != *str) die("Expected different character!\n");
    ++str;
  }
}

object_t *read_number(FILE *f) {
  int c = getc(f);
  long number = 0;
  int positive = 1;

  if (isdigit(c) || (isdigit(peek(f)) && c == '-')) {
    if (c == '-') positive = -1;
    else ungetc(c,f);

    while (isdigit(c = getc(f))) 
      number = (number * 10) + (c - '0');
    number *= positive;
    if (delimiter(c)) {
      ungetc(c,f);
      return create_fixnum(number);
    } else {
      die("No delimiter following number!");
    }
  }
  return NULL;
}

object_t *read_character(FILE *f) {
  int c = getc(f);

  if (c == '#') {
    c = getc(f);
    switch (c) {
      case 'f': 
	return false_obj;
      case 't':
	return true_obj;
      case '\\':
	c = getc(f);
	switch (c) {
	  case EOF:
	    die("Characater literal not complete!\n");
	  case 's':
	    if (peek(f) == 'p') {
	      ensure_expected(f,"pace");
	      ensure_delimiter(f);
	      return create_character(' ');
	    }
	    else return create_character('s');
	    break;
	  case 'n':
	    if (peek(f) == 'e') {
	      ensure_expected(f,"ewline");
	      ensure_delimiter(f);
	      return create_character('\n');
	    }
	    else return create_character('n');
	    break;
	  case 't':
	    if (peek(f) == 'a') {
	      ensure_expected(f,"ab");
	      ensure_delimiter(f);
	      return create_character('\t');
	    }
	    else return create_character('t');
	    break;
	  default:
	    return create_character(c);
	}
	break;
      default:
	die("Invalid character literal!\n");
    }
  }
  return NULL;
}

object_t *read_string(FILE *f) {
  int c = getc(f), i = 0;
  char buffer[MAX_STRING_LENGTH];

  if (c == '"') {
    while ((c = getc(f)) != '"') {
      // extract escape sequences first
      if (c == '\\') {
	c = getc(f);
	if (c == 'n') c = '\n';
	else if (c == 'r') c = '\r';
	else if (c == 't') c = '\t';
	else die("Invalid escape sequence!");
      }
      if (c == EOF) die("EOF in the middle of a string!\n");

      if (i < 1024)
	buffer[i++] = c;
      else
	die("String constant is too long!\n");
    }
    buffer[i] = '\0';
    return create_string(buffer);
  }
  else die("Invalid string!");

  return NULL;
}

object_t *read_symbol(FILE *f) {
  int c = getc(f), i = 0;
  char buffer[MAX_SYMBOL_LENGTH];

  if (isalpha(c)) {
    while (isalpha(c) || c == '_') {
      if (i < MAX_SYMBOL_LENGTH - 1) {
	buffer[i++] = c;
	c = getc(f);
      } else {
	die("Symbol too long!");
      }
    }

    if (delimiter(c)){ 
      ungetc(c,f);
      buffer[i] = '\0';
      return create_symbol(buffer);
    }
    else
      die("Symbol not followed by delimiter!\n");
  }
  else die("Invalid symbol!");

  return NULL;
}

object_t *read_cons(FILE *f) {
  int c = getc(f);
  object_t *car, *cdr;

  if (c == ')') return nil;
  ungetc(c,f);

  car = read(f);
  ignore_ws(f);
  c = getc(f);

  /* check for dotted list */
  if (c == '.') {
    if (!delimiter(peek(f))) die("No delimiter after dot in dotted list!\n");
    cdr = read(f);
    ignore_ws(f);
    c = getc(f);
    if (c != ')') die("Missing right parenthesis in dotted list!\n");
    return create_cons(car,cdr);
  }
  else {
    ungetc(c,f);
    cdr = read_cons(f);
    return create_cons(car,cdr);
  }
    
  return NULL;
}

object_t *read(FILE *f) {
  ignore_ws(f);
  int c = getc(f);

  if (isdigit(c) || (c == '-' && isdigit(peek(f)))) {
    ungetc(c,f);
    return read_number(f);
  } else if (c == '#') {
    ungetc(c,f);
    return read_character(f); /* also booleans */
  } else if (c == '"') {
    ungetc(c,f);
    return read_string(f);
  } else if (isalpha(c)) {
    ungetc(c,f);
    return read_symbol(f);
  } else if (c == '(') {
    return read_cons(f);
  } else {
    die("Bad input!");
  }
  die("Invalid read state!\n");
  return NULL;
}


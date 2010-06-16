#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "types.h"

/*********************************
 * dealing w/ heap objects
 ********************************/

/* prototype */
object_t *read(FILE *f);

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
	return get_false();
      case 't':
	return get_true();
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

  if (c == ')') return get_nil();
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
  } else if (c == '\'') {
    return create_cons(get_quote(), create_cons(read(f), get_nil()));
  } else {
    die("Bad input!\n");
  }
  die("Invalid read state!\n");
  return NULL;
}


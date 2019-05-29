/*************************************************
 * Title: writer.c
 * Author: Tim Wawrzynczak
 * License: BSD 3-clause
 * Date: 6/15/10
 ************************************************/

#include <stdio.h>
#include "types.h"
#include "reader.h"
#include "writer.h"

void write_cons(object_t *obj)
{
        object_t *car = obj->cons.car;
        object_t *cdr = obj->cons.cdr;

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

void write(object_t *obj)
{
        switch (obj->type) {
	case t_nil:
		printf("nil");
		break;
        case t_fixnum:
                printf("%ld", obj->fixnum.value);
                break;
        case t_character:
                printf("#\\");
                switch (obj->character.value) {
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
                        printf("%c", obj->character.value);
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
                printf("\"%s\"\n", obj->string.value);
                break;
        case t_symbol:
                printf("%s", obj->symbol.value);
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
        case t_primitive:
                printf("<primitive>");
                break;
        default:
                fprintf(stderr, "Unknown type (can't write) (type: %d)\n", obj->type);
                exit(1);
        }
}

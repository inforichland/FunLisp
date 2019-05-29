#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "writer.h"

/* http://home.pipeline.com/~hbaker1/LinearLisp.html  */

enum ops {
	op_swap,	/* r1 <-> r2 */
        op_car,		/* r1 <-> CAR(r2) */
        op_cdr,		/* r1 <-> CDR(r2) */
        op_nilp,	/* NULL(r1) */
        op_atomp,	/* ATOM(r1) */
        op_eq,		/* EQ(r1, r2) */
        op_assignsym,	/* r1 := 'foo */
        op_assign,	/* r1 := r2 */
        op_cons,	/* r1 <-> CAR(fr);  r2 <-> fr;  fr <-> CDR(r2); r1 := NIL */
        op_push,	/* CONS(r1,r2) */
        op_pop,		/* if NULL(r1) & !ATOM(r2) then {fr <-> CDR(r2); r2 <-> fr; r1 <-> CAR(fr); } else die */
};

struct reg {
        object_t *value;
};

enum regs {
        reg0,
        reg1,
        reg2,
        reg3,
        reg4,
        reg5,
        reg6,
        reg7,
        fr,
        sp,
        REG_COUNT
};

static struct reg regs[REG_COUNT];

struct instruction {
        enum ops op;
        union {

                struct { enum regs r1; enum regs r2; }  swap;
                struct { enum regs r1; enum regs r2; }  car;
                struct { enum regs r1; enum regs r2; }  cdr;
                struct { enum regs r1; }                nilp;
                struct { enum regs r1; }                atomp;
                struct { enum regs r1; enum regs r2; }  eq;
                struct { enum regs r1; object_t *sym; } assignsym;
                struct { enum regs r1; enum regs r2; }  assign;
                struct { enum regs r1; enum regs r2; }  cons;
                struct { enum regs r1; enum regs r2; }  push;
                struct { enum regs r1; enum regs r2; }  pop;
        };
};

void execute(const struct instruction *i)
{
        struct reg tmp;
        object_t *o1;
        object_t *o2;
        
        switch (i->op) {
        case op_swap:
                tmp.value = regs[i->swap.r1].value;
                regs[i->swap.r1].value = regs[i->swap.r2].value;
                regs[i->swap.r2].value = tmp.value;
                break;
        case op_assignsym:
                regs[i->assignsym.r1].value = i->assignsym.sym;
                break;
        case op_car:
                tmp.value = regs[i->car.r1].value;
		regs[i->car.r1].value = regs[i->car.r2].value->cons.car;
		regs[i->car.r2].value->cons.car = tmp.value;
                break;
	case op_cdr:
                tmp.value = regs[i->cdr.r1].value;
		regs[i->cdr.r1].value = regs[i->cdr.r2].value->cons.cdr;
		regs[i->cdr.r2].value->cons.cdr = tmp.value;
                break;
        case op_cons:
                o1 = regs[i->cons.r1].value;
                o2 = regs[i->cons.r2].value;
                regs[i->cons.r2].value = create_cons(o1, o2);
                regs[i->cons.r1].value = get_nil();
                break;
        default:
                break;
        }
}

/* gcc -std=gnu11 vm.c writer.c types.c -g */
#define ASSIGNSYM(r_, sym_) { .op = op_assignsym, .assignsym.r1 = (r_), .assignsym.sym = (sym_) }
#define SWAP(r1_, r2_) { .op = op_swap, .swap.r1 = (r1_), .swap.r2 = (r2_) }
#define CAR(r1_, r2_) { .op = op_car, .car.r1 = (r1_), .car.r2 = (r2_) }
#define CDR(r1_, r2_) { .op = op_cdr, .cdr.r1 = (r1_), .cdr.r2 = (r2_) }
#define CONS(r1_, r2_) { .op = op_cons, .cons.r1 = (r1_), .cons.r2 = (r2_) }
int main(void)
{
        initialize_types();

	for (int i = 0; i < REG_COUNT; ++i)
		regs[i].value = get_nil();
	
        object_t *foo = create_symbol("foo");
        object_t *bar = create_symbol("bar");
        object_t *quux = create_symbol("quux");

        struct instruction ops[] = {
                [0] = ASSIGNSYM(reg0, foo),
                [1] = ASSIGNSYM(reg1, bar),
                [2] = SWAP(reg0, reg1),
                [3] = ASSIGNSYM(reg3, get_nil()),
                [4] = ASSIGNSYM(reg2, quux),
                [5] = CONS(reg2, reg3),
		[6] = CONS(reg1, reg3),
		[7] = CONS(reg0, reg3),
		[8] = CAR(reg0, reg3),
		[9] = CDR(reg1, reg3),
		[10] = CDR(reg2, reg1),
        };

        for (int i = 0; i < 9; ++i) {
                execute(&ops[i]);
        }

        for (int i = 0; i < REG_COUNT; ++i) {
                printf("reg%d: ", i);
                write(regs[i].value);
                printf("\n");
        }

        cleanup_types();
	free(foo);
	free(bar);
	free(quux);
}

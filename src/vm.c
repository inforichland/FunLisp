#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "writer.h"

enum ops {
        op_swap,
        op_car,
        op_cdr,
        op_nilp,
        op_atomp,
        op_eq,
        op_assignsym,
        op_assign,
        op_cons,
        op_push,
        op_pop,
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
                struct { enum regs r1; enum regs r2; };
                struct { enum regs r3; object_t *sym; };
                /*
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
                */
        };
};

void execute(const struct instruction *i)
{
        struct reg tmp;
        
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
                tmp.value = regs[i->car.r2].value;
        default:
                die("Unhandled op");
                break;
        }
}

/* gcc -std=gnu11 vm.c writer.c types.c -g*/
int main(void)
{
        initialize_types();
        
        object_t *foo = create_symbol("foo");
        object_t *bar = create_symbol("bar");

        struct instruction ops[3] = {
                [0] = { .op = op_assignsym, .assignsym.r1 = reg0, .assignsym.sym = foo },
                [1] = { .op = op_assignsym, .assignsym.r1 = reg1, .assignsym.sym = bar },
                [2] = { .op = op_swap, .swap.r1 = reg0, .swap.r2 = reg1 },
        };

        for (int i = 0; i < 3; ++i) {
                execute(&ops[i]);
        }

        printf("reg0: ");
        write(regs[reg0].value);
        printf("\nreg1: ");
        write(regs[reg1].value);
        printf("\n");
        
        cleanup_types();
}

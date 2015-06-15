#include "object.h"

void assemblecode(InterCodes *st, FILE *fp) {
	while (st != NULL) {
		int kind = st->code.kind;
		if (kind == LABEL) {
			char *s = change(st->code.u.op);
			fprintf(fp, "%s:\n", s);	
		}

		if (kind == ASSIGN) {
			reg *regl = get_reg(st->code.u.assign.left);
			int opkind = st->code.u.assign.right->kind;
			if (opkind == CONSTINT) 
				fprintf(fp, "li %s, %d\n", regl->name, st->code.u.assign.right->u.value.Int);
			else {
				ret *regr = get_reg(st->code.u.assign.right);
				fprintf(fp, "move %s, %s\n", regl->name, regr->name);
			}
		}

		if (kind >= ADD && kind <= DIV) {
			reg *regf = get_reg(st->code.u.binop.result);
			reg *regl = get_reg(st->code.u.binop.op1);
			int opkind = st->code.u.binop.op2->kind;
			char *s = malloc(100);
			if (opkind == CONSTINT) {
				int value = st->code.u.binop.op2->u.value.Int;
				sprintf(s, "%d", value);
			} else {
				reg *regr = get_reg(st->code.u.binop.op2);
				sprintf(s, "%s", regr->name);
			}
			switch (kind) {
				case ADD: 
					if (opkind == CONSTINT) 
						fprintf(fp, "addi %s, %s, %s\n", regf->name, regl->name, s);
					else 
						fprintf(fp, "add %s, %s, %s\n", regf->name, regl->name, s);
					break;
				case SUB: 
					if (opkind == CONSTINT) 
						fprintf(fp, "addi %s, %s, -%s\n", regf->name, regl->name, s);
					else 
						fprintf(fp, "sub %s, %s, %s\n", regf->name, regl->name, s);
					break;
				case MUL:
					fprintf(fp, "mul %s, %s, %s\n", regf->name, regl->name, s);
					break;
				case DIV:
					fprintf(fp, "div %s, %s\n", regl->name, s);
					fprintf(fp, "mflo %s\n", regf->name); 
					break;
			}
		}

		if (kind == 
		st = st->next;
	}
}

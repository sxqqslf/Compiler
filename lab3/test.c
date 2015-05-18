#include "intercode.h"

int main() {
	Operand op1 = (Operand)malloc(sizeof(struct Operand_));
	Operand op2 = (Operand)malloc(sizeof(struct Operand_));
	Operand op3 = (Operand)malloc(sizeof(struct Operand_));

	op1->kind = VARIABLE;
	op1->u.var_no = 1;

	op2->kind = CONSTANT;
	op2->u.value = 1;

	struct InterCodes *ans = (struct InterCodes *)malloc(sizeof(struct InterCodes));
	ans->prev = ans->next = NULL;
	ans->code = createcode(ASSIGN, op1, op2, NULL);

	op3->kind = VARIABLE;
	op3->u.var_no = 2;

	op2->u.value = 2;

	ans->next = (struct InterCodes *)malloc(sizeof(struct InterCodes));
	ans->next->code = createcode(ADD, op1, op2, op3);
	ans->next->next = NULL;
	ans->next->prev = ans;

	printcode(ans);
}

#include "object.h"
#define FRAMESIZE 256

int offset;

void print_label(InterCodes *c, FILE *fp) {
	char *s = change(c->code.u.op);
	fprintf(fp, "%s:\n", s);	
}

void print_assign(InterCodes *c, FILE *fp) {
	Operand left = c->code.u.assign.left;
	Operand right = c->code.u.assign.right;

	var *op1 = findvar(varList, left);
	var *op2 = findvar(varList, right);
	if (op1 == NULL) {
		op1 = createvar(left, fp);
		insertvar(op1);
	}
	
	int oplkind = left->kind;
	int oprkind = right->kind;
	if (oplkind == TEMPVAR_) {
		if (oprkind != CONSTINT) {
			assert(op2 != NULL);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, op2->offset);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op1->offset);
			fprintf(fp, "  sw $%s, 0($%s)\n", regi[8].name, regi[9].name);
		} else {
			int value = right->u.value.Int;
			fprintf(fp, "  li $%s, %d\n", regi[9].name, value);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, op1->offset);
			fprintf(fp, "  sw $%s, ($%s)\n", regi[9].name, regi[8].name);
		}
	} else if (oprkind == CONSTINT) {
		int value = right->u.value.Int;
		fprintf(fp, "  li $%s, %d\n", regi[8].name, value);
		fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, op1->offset);
	} else if (oprkind == TEMPVAR_) {
		assert(op2 != NULL);
		fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op2->offset);
		fprintf(fp, "  lw $%s, 0($%s)\n", regi[8].name, regi[9].name);
		fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, op1->offset);
	} else {
		fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op2->offset);
		fprintf(fp, "  move $%s, $%s\n", regi[8].name, regi[9].name);
		fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, op1->offset);
	}
}

void print_binop(int kind, InterCodes *c, FILE *fp) {
	Operand opl = c->code.u.binop.op1;
	Operand opr = c->code.u.binop.op2;
	int oplkind = opl->kind, oprkind = opr->kind;
	int valuel = -1, valuer = -1;
	if (oplkind == CONSTINT) 
		valuel = opl->u.value.Int;
	if (oprkind == CONSTINT)
		valuer = opr->u.value.Int;

	var *res = findvar(varList, c->code.u.binop.result);
	var *op1 = findvar(varList, opl);
	var *op2 = findvar(varList, opr);

	if (res == NULL) {
		res = createvar(c->code.u.binop.result, fp);
		insertvar(res);
	}

	if (kind == ADD || kind == SUB || kind == MUL) {
		if (oplkind == CONSTINT) 
			fprintf(fp, "  li $%s, %d\n", regi[9].name, valuel);
		else {
			assert(op1 != NULL);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op1->offset);
			if (oplkind == TEMPVAR_) 
				fprintf(fp, "  lw $%s, ($%s)\n", regi[9].name, regi[9].name);
		}

		if (oprkind == CONSTINT) 
			fprintf(fp, "  li $%s, %d\n", regi[10].name, valuer);
		else {
			assert(op2 != NULL);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[10].name, op2->offset);
			if (oprkind == TEMPVAR_)
				fprintf(fp, "  lw $%s, ($%s)\n", regi[10].name, regi[10].name);
		}

		if (kind == ADD)
			fprintf(fp, "  add $%s, $%s, $%s\n", regi[8].name, regi[9].name, regi[10].name);
		else if (kind == SUB) 
			fprintf(fp, "  sub $%s, $%s, $%s\n", regi[8].name, regi[9].name, regi[10].name);
		else if (kind == MUL)
			fprintf(fp, "  mul $%s, $%s, $%s\n", regi[8].name, regi[9].name, regi[10].name);
		fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, res->offset);
	}

	if (kind == DIV) {
		if (oprkind == CONSTINT) {
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op1->offset);
			fprintf(fp, "  li $%s, %d\n", regi[10].name, valuer);
			fprintf(fp, "  div $%s, $%s\n", regi[9].name, regi[10].name);
		} else { 
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op1->offset);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[10].name, op2->offset);
			fprintf(fp, "  div $%s, $%s\n", regi[9].name, regi[10].name);
		}
		fprintf(fp, "  mflo $%s\n", regi[8].name);
		fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, res->offset);
	}
}

void print_GOTOS(InterCodes *c, FILE *fp) {
	fprintf(fp, "  j %s\n", change(c->code.u.op)); 
}

void print_RETURN(InterCodes *c, FILE *fp) {
	var *retvalue = findvar(varList, c->code.u.op);
	if (c->code.u.op->kind != CONSTINT) {
		assert(retvalue != NULL);	
		fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, retvalue->offset);
	} else 
		fprintf(fp, "  li $%s, %d\n", regi[8].name, c->code.u.op->u.value.Int);

	fprintf(fp, "  subu $sp, $fp, 8\n");
	fprintf(fp, "  lw $fp, 0($sp)\n");
	fprintf(fp, "  addi $sp, $sp, 4\n");
	fprintf(fp, "  lw $ra, 0($sp)\n");
	fprintf(fp, "  addi $sp, $sp, 4\n");

	fprintf(fp, "  move $v0, $%s\n", regi[8].name);
	fprintf(fp, "  jr $ra\n");
}

void print_COND(InterCodes *c, FILE *fp) {
	var *op1 = findvar(varList, c->code.u.relop.op1);
	var *op2 = findvar(varList, c->code.u.relop.op2);
	int kind1 = c->code.u.relop.op1->kind;
	int kind2 = c->code.u.relop.op2->kind;

	if (kind1 == CONSTINT) 
		fprintf(fp, "  li $%s, %d\n", regi[8].name, c->code.u.relop.op1->u.value.Int);
	else {
		if (op1 == NULL) {
			op1 = createvar(c->code.u.relop.op1, fp);
			insertvar(op1);
		}
		fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, op1->offset);
		if (kind1 == TEMPVAR_) 
			fprintf(fp, "  lw $%s, 0($%s)\n", regi[8].name, regi[8].name);
	}

	if (kind2 == CONSTINT)
		fprintf(fp, "  li $%s, %d\n", regi[9].name, c->code.u.relop.op2->u.value.Int);
	else {
		if (op2 == NULL) {
			op2 = createvar(c->code.u.relop.op1, fp);
			insertvar(op2);
		}
		fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op2->offset);
		if (kind2 == TEMPVAR_)
			fprintf(fp, "  lw $%s, 0($%s)\n", regi[9].name, regi[9].name);
	}
	char *s = change(c->code.u.relop.label);
	char relop[3];
	memset(relop, 0, sizeof(relop));
	strcpy(relop, c->code.u.relop.rel->u.value.relop); 

	if (strcmp(relop, "==") == 0) 
		fprintf(fp, "  beq ");
	else if (strcmp(relop, "!=") == 0) 
		fprintf(fp, "  bne ");
	else if (strcmp(relop, ">") == 0)
		fprintf(fp, "  bgt ");
	else if (strcmp(relop, "<") == 0) 
		fprintf(fp, "  blt ");
	else if (strcmp(relop, ">=") == 0) 
		fprintf(fp, "  bge ");
	else fprintf(fp, "  ble ");

	fprintf(fp, "$%s, $%s, %s\n", regi[8].name, regi[9].name, s);
}

void initial(FILE *fp) {
	offset = 0;

	int i;
	for (i=0; i<32; i++) {
		memset(regi[i].name, 0, sizeof(regi[i].name));
		regi[i].variable = NULL;
	}
	strcpy(regi[0].name, "0");
	strcpy(regi[1].name, "at");
	strcpy(regi[2].name, "v0");
	strcpy(regi[3].name, "v1");
	for (i=0; i<4; i++) {
		regi[4+i].name[0] = 'a';
		regi[4+i].name[1] = i+48;
	}
	for (i=0; i<8; i++) {
		regi[8+i].name[0] = 't';
		regi[8+i].name[1] = i+48;
	}
	for (i=0; i<8; i++) {
		regi[16+i].name[0] = 's';
		regi[16+i].name[1] = i+48;
	}
	strcpy(regi[24].name, "t8");
	strcpy(regi[25].name, "t9");
	strcpy(regi[26].name, "k0");
	strcpy(regi[27].name, "k1");
	strcpy(regi[28].name, "gp");
	strcpy(regi[29].name, "sp");
	strcpy(regi[30].name, "fp");
	strcpy(regi[31].name, "ra");

	fprintf(fp, ".data\n_prompt: .asciiz \"Enter an integer: \"\n");
	fprintf(fp, "_ret: .asciiz \"\\n\"\n");
	fprintf(fp, ".globl main\n.text\n");

	fprintf(fp, "read:\n  li $v0, 4\n  la $a0, _prompt\n  syscall\n  li $v0, 5\n  syscall\n  jr $ra\n\n");
	fprintf(fp, "write:\n  li $v0, 1\n  syscall\n  li $v0, 4\n  la $a0, _ret\n  syscall\n  move $v0, $0\n  jr $ra\n\n");
}

InterCodes *print_FUNCTION(InterCodes *c, FILE *fp) {
	fprintf(fp, "\n%s:\n", c->code.name);	

	while (varList != NULL) {
		var *tmp = varList;
		varList = varList->next;
		free(tmp);
	}
	offset = 12;
	fprintf(fp, "  subu $sp, $sp, 4\n");
	fprintf(fp, "  sw $ra, 0($sp)\n");
	fprintf(fp, "  subu $sp, $sp, 4\n");
	fprintf(fp, "  sw $fp, 0($sp)\n");
	fprintf(fp, "  addi $fp, $sp, 8\n");
	fprintf(fp, "  subu $sp, $sp, %d\n", FRAMESIZE);

	InterCodes *t = c->next;
	if (t == NULL) return ;
	int num = 0;
	while (t->code.kind == PARAM && num < 4) {
		var *param = findvar(varList, t->code.u.op);
		if (param == NULL) {
			param = createvar(t->code.u.op, fp);
			insertvar(param);
		}
		fprintf(fp, "  sw $a%d, -%d($fp)\n", num ++, param->offset);
		t = t->next;
	}

	int delta = 0;
	while (t->code.kind == PARAM) {
		var *param = findvar(varList, t->code.u.op);
		if (param == NULL) {
			param = createvar(t->code.u.op, fp);
			insertvar(param);
		}
		fprintf(fp, "  lw $%s, %d($fp)\n", regi[8].name, delta);
		fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, param->offset); 
		delta += 4;
		t = t->next;
		num ++;
	}
	return t;
}

void insertvar(var *newvar) {
	if (varList == NULL) {
		varList = newvar; return ;
	}
	var *h = varList;
	while (h->next != NULL) h = h->next;
	h->next = newvar;
}

var *findvar(var *h, Operand op) {
	char *s = change(op);
	while (h != NULL) {
		if (strcmp(h->name, s) == 0) 
			return h;
		h = h->next;
	}
	return NULL;
}

void print_CALL(InterCodes *c, FILE *fp) {
	InterCodes *ori = c;
	c = c->prev;
	int count = 0;
	Operand arg[100];
	if (c->code.kind == ARG) {
		Operand op = c->code.u.op;
		while (op != NULL) {
			arg[count ++] = op;
			op = op->next;
		}
	}

	int i = 0;
	while (i < 4 && count > 0) {
		Operand op = arg[--count];
		var *tmp = findvar(varList, op);
		if (op->kind == CONSTINT) 
			fprintf(fp, "  li $%s, %d\n", regi[8].name, op->u.value.Int);
		else {
			assert(tmp != NULL);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, tmp->offset);
		}

		fprintf(fp, "  move $a%d, $%s\n", i, regi[8].name);
		i ++;
	}

	for (i=0; i<count; i++) {
		Operand op = arg[i];
		var *tmp = findvar(varList, op);
		if (op->kind == CONSTINT) 
			fprintf(fp, "  li $%s, %d\n", regi[8].name, op->u.value.Int);
		else {
			assert(tmp != NULL);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, tmp->offset);
		}
		fprintf(fp, "  subu, $sp, $sp, 4\n");
		fprintf(fp, "  sw $%s, ($sp)\n", regi[8].name);

		c = c->next;
	}

	if (strcmp(ori->code.name, "write") == 0) {
		var *arg = findvar(varList, ori->code.u.op);
		int kind = ori->code.u.op->kind;
		if (kind == CONSTINT) {
			fprintf(fp, "  li $%s, %d\n", regi[8].name, ori->code.u.op->u.value.Int);
			fprintf(fp, "  move $a0, $%s\n", regi[8].name);
		} else { 
			assert(arg != NULL);
			fprintf(fp, "  lw $%s, -%d($fp)\n", regi[8].name, arg->offset);
			fprintf(fp, "  move $a0, $%s\n", regi[8].name);
		}
		fprintf(fp, "  jal %s\n", ori->code.name);
		return ;
	}
	fprintf(fp, "  jal %s\n", ori->code.name);
	fprintf(fp, "  move $%s, $v0\n", regi[8].name);
	var *retvalue = findvar(varList, ori->code.u.op);
	if (retvalue == NULL) {
		retvalue = createvar(ori->code.u.op, fp);
		insertvar(retvalue);
	}
	fprintf(fp, "  sw $%s, -%d($fp)\n", regi[8].name, retvalue->offset);

	//fprintf(fp, "addi $sp, $sp, %d\n", count*4);
}

var *createvar(Operand op, FILE *fp) {
	InterCodes *p = st;	
	int flag = -1;
	char *s = change(op);
	while (p != NULL) {
		if (p->code.kind == DEC && strcmp(change(p->code.u.array.ID), s) == 0) {
			flag = p->code.u.array.addr->u.value.Int;
			break;
		} else p = p->next;
	}
	var *ret = (var *)malloc(sizeof(var));
	memset(ret->name, 0, sizeof(ret->name));
	strcpy(ret->name, s);
	ret->offset = offset;
	if (flag == -1)
		offset += 4;
	else 
		offset += flag*4;
	ret->next = NULL;
	
	return ret;
}

void print_ARRAY(InterCodes *c, FILE *fp) {
	var *op1 = findvar(varList, c->code.u.binop.op1);
	var *op2 = findvar(varList, c->code.u.binop.op2);
	var *pos = findvar(varList, c->code.u.binop.result);

	if (op1 == NULL) {
		op1 = createvar(c->code.u.binop.op1, fp);
		insertvar(op1);
	}
	if (op2 == NULL) {
		op2 = createvar(c->code.u.binop.op2, fp);
		insertvar(op2);
	}
	if (pos == NULL) {
		pos = createvar(c->code.u.binop.result, fp);
		insertvar(pos);
	}

	fprintf(fp, "  addi $%s, $fp, %d\n", regi[8].name, op1->offset);
	fprintf(fp, "  lw $%s, -%d($fp)\n", regi[9].name, op2->offset);
	fprintf(fp, "  add $%s, $%s, $%s\n", regi[10].name, regi[8].name, regi[9].name);
	fprintf(fp, "  sw $%s, -%d($fp)\n", regi[10].name, pos->offset);
}

void assemblecode(InterCodes *st, FILE *fp) {
	initial(fp);
	while (st != NULL) {
		int kind = st->code.kind;
		if (kind == LABEL) 
			print_label(st, fp);
	
		if (kind == ASSIGN) 
			print_assign(st, fp);

		if (kind >= ADD && kind <= DIV) 
			print_binop(kind, st, fp);

		if (kind == GOTOL) 
			print_GOTOS(st, fp);

		if (kind == CALL) 
			print_CALL(st, fp);

		if (kind == RETURN) 
			print_RETURN(st, fp);

		if (kind == CONDITION) 
			print_COND(st, fp);

		if (kind == FUNCTION) {
			st = print_FUNCTION(st, fp);
			continue;
		}

		if (kind == ARRAY) 
			print_ARRAY(st, fp);

		st = st->next;
	}
}

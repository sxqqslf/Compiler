#include "intercode.h"

char OPT[20] = {' ', '+', '-', '*', '/'};
int varcount = 1;
int tmpvarcount = 1;
int labelcount = 1;
struct count {
	char name[100];
	int count;
	struct count *next;
};
struct count *hash[HASHSIZE];

void INIT() {
	int i;
	for (i=0; i<HASHSIZE; i++) hash[i] = NULL;
}

extern char *change(Operand x) ;

InterCodes *translate(struct node *root) {
	if (root == NULL) return ;

	if (strcmp(root->type, "Stmt") == 0) 
		return translate_Stmt(root);

	if (strcmp(root->type, "CompSt") == 0) 
		return translate_Compst(root);

	if (strcmp(root->type, "VarDec") == 0) 
		return translate_VarDec(root);

	InterCodes *code = NULL;
	if (strcmp(root->type, "FunDec") == 0) {
		code = (InterCodes *)malloc(sizeof(InterCodes));
		code->code.kind = FUNCTION;
		strcpy(code->code.name, root->child->value);
		FunctionMessage *func = funcHashtable[hash_pjw(root->child->value)];
		FieldList arg = func->argList;
		while (arg != NULL) {
			InterCodes *tmp = (InterCodes *)malloc(sizeof(InterCodes));
			tmp->code.kind = PARAM;
			tmp->code.u.op = newOperand(VARIABLE, arg->name);
			tmp->prev = tmp->next = NULL;
			code = bindCode(code, tmp);
			arg = arg->down;
		}
	}
/*	
	if (strcmp(root->type, "VarList") == 0) {
		code = (InterCodes *)malloc(sizeof(InterCodes));
		code->code.kind = PARAM;
		code->code.u.op = newOperand(VARIABLE, root->child->child->next->child->value);
		code->prev = code->next = NULL;
		return code;
	}
*/

	struct node *child = root->child;
	if (child != NULL) {
		code = bindCode(code, translate(child));
		while (child->next != NULL) {
			code = bindCode(code, translate(child->next));
			child = child->next;
		}
	}
	
	return code;
}

InterCodes *boolExp(struct node *root) {
	Operand label1 = newOperand(OPLABEL, NULL);
	Operand label2 = newOperand(OPLABEL, NULL); 
	InterCodes *code1 = translate_Cond(root, label1, label2);
	InterCodes *code2 = newInterCode(LABEL, label1, NULL, NULL);
	InterCodes *code3 = newInterCode(LABEL, label2, NULL, NULL);
	return bindCode(bindCode(code1, code2), code3);
}

int CALC(char *name) {
	int index = hash_pjw(name);
	if (hash[index] == NULL) {
		hash[index] = (struct count *)malloc(sizeof(struct count));
		hash[index]->next = NULL;
		hash[index]->count = varcount ++;
		strcpy(hash[index]->name, name);
		return hash[index]->count;
	} else {
		struct count *tmp = hash[index];
		while (tmp != NULL) {
			if (strcmp(tmp->name, name) == 0) break;
			tmp = tmp->next;
		}
		if (tmp == NULL) {
			struct count *new = (struct count *)malloc(sizeof(struct count));
			new->next = hash[index];
			new->count = varcount ++;
			strcpy(new->name, name);
			hash[index] = new;
			return new->count;
		} else 
			return tmp->count;
	}
}

InterCodes *translate_Exp(struct node *root, Operand op) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	//INT
	if (strcmp(child->type, "INT") == 0) {
		Operand opr = newOperand(CONSTINT, child->value);
		InterCodes *code = newInterCode(ASSIGN, op, opr, NULL);
		return code;
	}
	//FLOAT
	if (strcmp(child->type, "FLOAT") == 0) {
		Operand opr = newOperand(CONSTFLOAT, child->value);
		InterCodes *code = newInterCode(ASSIGN, op, opr, NULL);
		return code;
	}
	//ID
	if (strcmp(child->type, "ID") == 0 && child->next == NULL) {
		Operand var = newOperand(VARIABLE, child->value);
		//op->kind = VARIABLE;
		//op->u.var_no = CALC(child->value);
		//return NULL;
		return newInterCode(ASSIGN, op, var, NULL);
	}
	//函数调用
	if (strcmp(child->type, "ID") == 0 && child->next != NULL) {
		struct node *now = child->next->next;
		char *name = child->value;
		if (strcmp(now->type, "RP") == 0) {
			InterCodes *retcode = (InterCodes *)malloc(sizeof(InterCodes));
			retcode->code.kind = CALL;
			retcode->code.u.op = op;
			strcpy(retcode->code.name, name);

			return retcode;
		}

		if (strcmp(now->type, "Args") == 0) {

			Operand argList = NULL;
			InterCodes *code1 = translate_Args(now, &argList);
			InterCodes *code2 = (InterCodes *)malloc(sizeof(InterCodes));
			InterCodes *code3 = (InterCodes *)malloc(sizeof(InterCodes));

			code2->code.kind = ARG;
			code2->code.u.op = argList;
			strcpy(code2->code.name, name);

			if (strcmp(name, "write") == 0) {
				code2->code.kind = CALL;
				return bindCode(code1, code2);
			}

			/*if (argList->next != NULL) {
				argList = argList->next;
				InterCodes *code = (InterCodes *)malloc(sizeof(InterCodes));
				code->code.kind = ARG;
				code->code.u.op = argList;
				strcpy(code->code.name, "");
				bindCode(code2, code);
			}
			*/

			code3->code.kind = CALL;
			code3->code.u.op = op;
			strcpy(code3->code.name, name);

			return bindCode(bindCode(code1, code2), code3);
		}
	}
	//(Exp)
	if (strcmp(child->type, "LP") == 0) 
		return translate_Exp(child->next, op);	
	//-Exp	
	if (strcmp(child->type, "MINUS") == 0) {
		Operand tmp = newOperand(TEMPVAR, NULL); 
		Operand con = newOperand(CONSTINT, "0");
		InterCodes *code1 = translate_Exp(child->next, tmp);
		InterCodes *code2 = newInterCode(SUB, con, tmp, op);
		return bindCode(code1, code2);
	}
	//Not
	if (strcmp(child->type, "NOT") == 0) 
		return boolExp(root);
	//算术表达式，逻辑表达式，赋值
	if (strcmp(child->type, "Exp") == 0) {
		child = child->next;

		if (strcmp(child->type, "AND") == 0 || 
			strcmp(child->type, "OR") == 0 ||
			strcmp(child->type, "RELOP") == 0) return boolExp(root);

		if (strcmp(child->type, "ASSIGNOP") == 0) {
			Operand tmp = newOperand(TEMPVAR, NULL);	
			InterCodes *code1 = translate_Exp(child->next, tmp);
			InterCodes *code2 = NULL; 
			
			struct node *now = root->child;
			if (strcmp(now->child->type, "ID") == 0) {
				Operand OP = newOperand(VARIABLE, now->child->value);
				/*Operand OP = (Operand)malloc(sizeof(Operand_));
				OP->kind = VARIABLE;
				OP->u.var_no = CALC(now->child->value);
				OP->next = NULL;*/
				code2 = newInterCode(ASSIGN, OP, tmp, NULL);
				if (op != NULL) {
					InterCodes *code3 = newInterCode(ASSIGN, op, OP, NULL);
					code2 = bindCode(code2, code3);
				}
			} else {				//目前只考虑一维数组，不考虑结构体
				Operand arr = newOperand(TEMPVAR_, NULL);
				code2 = translate_Exp(now, arr);
				InterCodes *code3 = newInterCode(ASSIGN, arr, tmp, NULL);
				code2 = bindCode(code2, code3);
			}

			return bindCode(code1, code2);
		}

		if (strcmp(child->type, "LB") == 0) {
			int size = sizeof(int); 
			char s[20]; sprintf(s, "%d", size);
			Operand op1 = newOperand(TEMPVAR, NULL);
			Operand op2 = newOperand(TEMPVAR, NULL);
			Operand op3 = newOperand(CONSTINT, s);
			Operand op4 = newOperand(VARIABLE, root->child->child->value);
			InterCodes *code1 = translate_Exp(child->next, op1);
			InterCodes *code2 = newInterCode(MUL, op1, op3, op2);
			InterCodes *code3 = newInterCode(ARRAY, op4, op2, op);
			op->kind = TEMPVAR_;
			return bindCode(bindCode(code1, code2), code3);
		}

		Operand op1 = newOperand(TEMPVAR, NULL);
		Operand op2 = newOperand(TEMPVAR, NULL);
		InterCodes *code1 = translate_Exp(root->child, op1);
		InterCodes *code2 = translate_Exp(child->next, op2);
		int type;
		switch (child->type[0]) {
			case 'P': type = ADD; break;
			case 'M': type = SUB; break;
			case 'S': type = MUL; break;
			case 'D': type = DIV; break;
		}
		InterCodes *code3 = newInterCode(type, op1, op2, op);
		return bindCode(bindCode(code1, code2), code3);
	}
}

InterCodes *translate_VarDec(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	if (child->next != NULL) {
		Operand name = newOperand(VARIABLE, child->child->value);
		Operand size = newOperand(CONSTINT, child->next->next->value); 
		return newInterCode(DEC, name, size, NULL);
	}
}

InterCodes *translate_Cond(struct node *root, Operand labeltrue, Operand labelfalse) {
	if (root == NULL) return NULL;

	struct node *exp1 = root->child;
	if (strcmp(exp1->type, "NOT") == 0) 
		return translate_Cond(exp1->next, labelfalse, labeltrue);

	struct node *opt = exp1->next;
	struct node *exp2 = opt->next;
	if (strcmp(opt->type, "AND") == 0) {
		Operand label1 = newOperand(OPLABEL, NULL);
		InterCodes *code1 = translate_Cond(exp1, label1, labelfalse);
		InterCodes *code2 = translate_Cond(exp2, labeltrue, labelfalse);
		InterCodes *code3 = newInterCode(LABEL, label1, NULL, NULL);
		return bindCode(bindCode(code1, code3), code2);
	}

	if (strcmp(opt->type, "OR") == 0) {
		Operand label1 = newOperand(OPLABEL, NULL);
		InterCodes *code1 = translate_Cond(exp1, labeltrue, label1);
		InterCodes *code2 = translate_Cond(exp2, labeltrue, labelfalse);
		InterCodes *code3 = newInterCode(LABEL, label1, NULL, NULL);
		return bindCode(bindCode(code1, code3), code2);
	}

	char oper[3]; strcpy(oper, "");
	if (strcmp(opt->type, "GT") == 0) strcpy(oper, ">");
	else if (strcmp(opt->type, "LT") == 0) strcpy(oper, "<");
	else if (strcmp(opt->type, "GE") == 0) strcpy(oper, ">=");
	else if (strcmp(opt->type, "LE") == 0) strcpy(oper, "<=");
	else if (strcmp(opt->type, "EQ") == 0) strcpy(oper, "==");
	else if (strcmp(opt->type, "NQ") == 0) strcpy(oper, "!=");
	

	if (strcmp(oper, "") != 0) {
		Operand op1 = newOperand(TEMPVAR, NULL);
		Operand op2 = newOperand(TEMPVAR, NULL);
		Operand relop = newOperand(OPCONDITION, oper);
		InterCodes *code1 = translate_Exp(exp1, op1);
		InterCodes *code2 = translate_Exp(exp2, op2);
		InterCodes *code3 = (InterCodes *)malloc(sizeof(InterCodes));
		InterCodes *code4 = newInterCode(GOTOL, NULL, labelfalse, NULL);
		code3->code.kind = CONDITION; code3->code.u.relop.op1 = op1; 
		code3->code.u.relop.op2 = op2; code3->code.u.relop.rel = relop; 
		code3->code.u.relop.label = labeltrue; code3->prev = code3->next = NULL;
		return bindCode(bindCode(code1, code2), bindCode(code3, code4));
	}
}

InterCodes *translate_Stmt(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	if (strcmp(child->type, "Exp") == 0) 
		return translate_Exp(child, NULL);

	if (strcmp(child->type, "CompSt") == 0) 
		return translate_Compst(child);

	if (strcmp(child->type, "RETURN") == 0) {
		Operand op = newOperand(TEMPVAR, NULL);
		InterCodes *code1 = translate_Exp(child->next, op);
		InterCodes *code2 = newInterCode(RETURN, op, NULL, NULL);
		return bindCode(code1, code2);
	}

	if (strcmp(child->type, "IF") == 0) {
		struct node *stmt1 = child->next->next->next->next;
		Operand label1 = newOperand(OPLABEL, NULL);
		Operand label2 = newOperand(OPLABEL, NULL);
		InterCodes *code1 = translate_Cond(child->next->next, label1, label2);
		InterCodes *code2 = translate_Stmt(stmt1);
		InterCodes *LABEL1 = newInterCode(LABEL, label1, NULL, NULL);
		InterCodes *LABEL2 = newInterCode(LABEL, label2, NULL, NULL);
		if (stmt1->next != NULL) {
			struct node *stmt2 = stmt1->next->next;
			Operand label3 = newOperand(OPLABEL, NULL);
			InterCodes *code3 = translate_Stmt(stmt2);
			InterCodes *LABEL3 = newInterCode(LABEL, label3, NULL, NULL);
			InterCodes *GOTO = newInterCode(GOTOL, NULL, label3, NULL);
			return bindCode(bindCode(bindCode(code1, LABEL1), bindCode(code2, GOTO)), 
							bindCode(bindCode(LABEL2, code3), LABEL3));
		} else 
			return bindCode(bindCode(code1, LABEL1), bindCode(code2, LABEL2));
	}

	if (strcmp(child->type, "WHILE") == 0) {
		struct node *exp = child->next->next;
		struct node *stmt = exp->next->next;
		Operand label1 = newOperand(OPLABEL, NULL);
		Operand label2 = newOperand(OPLABEL, NULL);
		Operand label3 = newOperand(OPLABEL, NULL);
		InterCodes *code1 = translate_Cond(exp, label2, label3);
		InterCodes *code2 = translate_Stmt(stmt);
		InterCodes *LABEL1 = newInterCode(LABEL, label1, NULL, NULL);
		InterCodes *LABEL2 = newInterCode(LABEL, label2, NULL, NULL);
		InterCodes *GOTO = newInterCode(GOTOL, NULL, label1, NULL);
		InterCodes *LABEL3 = newInterCode(LABEL, label3, NULL, NULL);
		return bindCode(bindCode(bindCode(LABEL1, code1), bindCode(LABEL2, code2)), 
						bindCode(GOTO, LABEL3));
	}
}

InterCodes *translate_Compst(struct node *root) {
	if (root == NULL) return NULL;	

	if (strcmp(root->type, "Stmt") == 0) 
		return translate_Stmt(root);

	if (strcmp(root->type, "Dec") == 0) {
		if (root->child->next != NULL) {
			struct node *exp = root->child->next->next;
			Operand op1 = newOperand(TEMPVAR, NULL);
			Operand op2 = newOperand(VARIABLE, root->child->child->value);
			InterCodes *code1 = translate_Exp(exp, op1);
			InterCodes *code2 = newInterCode(ASSIGN, op2, op1, NULL);
			return bindCode(code1, code2);
		} else 
			return translate_VarDec(root->child);
	}

	struct node *child = root->child;	
	InterCodes *code = NULL;
	if (child != NULL) {
		code = bindCode(code, translate_Compst(child));
		while (child->next != NULL) {
			code = bindCode(code, translate_Compst(child->next));
			child = child->next;
		}
	}
	return code;
}

InterCodes *translate_Args(struct node *root, Operand *argList) {
	if (root == NULL) return NULL;

	struct node *exp = root->child;
	Operand op = newOperand(TEMPVAR, NULL);
	InterCodes *code1 = translate_Exp(exp, op);
	*argList = bindOperand(op, *argList);
	if (exp->next == NULL) 
		return code1;
	else {
		InterCodes *code2 = translate_Args(exp->next->next, argList);
		return bindCode(code1, code2);
	}
}

InterCodes *bindCode(InterCodes *code1, InterCodes *code2) {
	if (code1 == NULL) return code2;

	InterCodes *ret = code1;
	while (code1->next != NULL) code1 = code1->next;
	code1->next = code2;
	if (code2 != NULL) code2->prev = code1;

	return ret;
}

Operand bindOperand(Operand op1, Operand op2) {
	Operand ret = op1;
	while (op1->next != NULL) op1 = op1->next;
	op1->next = op2;

	return ret;
}

Operand newOperand(int kind, char *value) {
	Operand op = (Operand)malloc(sizeof(Operand_));
	op->kind = kind;
	if (kind == VARIABLE) op->u.var_no = CALC(value);
	if (kind == CONSTINT) op->u.value.Int = atoi(value);
	if (kind == CONSTFLOAT) op->u.value.Float = atof(value);
	if (kind == TEMPVAR || kind == TEMPVAR_) op->u.var_no = tmpvarcount ++;
	if (kind == OPLABEL) op->u.var_no = labelcount ++;
	if (kind == OPCONDITION) strcpy(op->u.value.relop, value);
	op->next = NULL;

	return op;
}

InterCodes *newInterCode(int kind, Operand op1, Operand op2, Operand res) {
	InterCodes *retcode = (InterCodes *)malloc(sizeof(InterCodes));
	retcode->prev = retcode->next = NULL;
	retcode->code.kind = kind;
	if (kind == ASSIGN) {
		retcode->code.u.assign.left = op1;
		retcode->code.u.assign.right = op2;
	} else if (kind >= ADD && kind <= DIV) {
		retcode->code.u.binop.result = res;
		retcode->code.u.binop.op1 = op1;
		retcode->code.u.binop.op2 = op2;
	} else if (kind == ARRAY) {
		retcode->code.u.binop.result = res;
		retcode->code.u.binop.op1 = op1;
		retcode->code.u.binop.op2 = op2;
	} else if (kind == LABEL) {
		retcode->code.u.op = op1;
	} else if (kind == GOTOL) {
		retcode->code.u.op = op2;
	} else if (kind == RETURN) {
		retcode->code.u.op = op1;
	} else if (kind == DEC) {
		retcode->code.u.array.ID = op1;
		retcode->code.u.array.addr = op2;
	}

	return retcode;
}

char *change(Operand op) {
	if (op == NULL) return NULL;

	char *ret = malloc(100), str[20];
	memset(ret, 0, 100);
	if (op->kind == VARIABLE) {
		sprintf(str, "%d", op->u.var_no);
		ret[0] = 'v'; strcat(ret+1, str);
	} else if (op->kind == TEMPVAR) {
		sprintf(str, "%d", op->u.var_no);
		ret[0] = 't'; strcat(ret+1, str);
	} else if (op->kind == TEMPVAR_) {
		sprintf(str, "%d", op->u.var_no);
		ret[0] = '*'; ret[1] = 't';
		strcat(ret+2, str);
	} else if (op->kind == OPLABEL) {
		sprintf(str, "%d", op->u.var_no);
		strcpy(ret, "label"); strcat(ret+5, str);
	} else if (op->kind == CONSTINT || op->kind == CONSTFLOAT) {
		if (op->kind == CONSTINT) sprintf(str, "%d", op->u.value.Int);
		if (op->kind == CONSTFLOAT) sprintf(str, "%lf", op->u.value.Float);
		ret[0] = '#'; strcat(ret+1, str);
	}
	
	return ret;
}

void printCode(InterCodes *st) {
	while (st != NULL) {
		if (st->code.kind == ASSIGN) {
			char *s1 = change(st->code.u.assign.left), 
				 *s2 = change(st->code.u.assign.right);
			printf("%s := %s\n", s1, s2);
		} else if (st->code.kind >= ADD && st->code.kind <= DIV) {
			char *s1 = change(st->code.u.binop.op1),
				 *s2 = change(st->code.u.binop.op2),
				 *s3 = change(st->code.u.binop.result);
			if (s1 == NULL) 
				printf("%s := -%s\n", s3, s2);
			else 
				printf("%s := %s %c %s\n", s3, s1, OPT[st->code.kind], s2);
		} else if (st->code.kind == DEC) {
			char *s = change(st->code.u.array.ID);
			int size = st->code.u.array.addr->u.value.Int;
			printf("DEC %s %d\n", s, sizeof(int)*size);
		} else if (st->code.kind == ARRAY) {
			int kind = st->code.u.binop.result->kind;
			st->code.u.binop.result->kind = TEMPVAR;
			char *s1 = change(st->code.u.binop.op1),
				 *s2 = change(st->code.u.binop.op2),
				 *s3 = change(st->code.u.binop.result);
			printf("%s := &%s + %s\n", s3, s1, s2);
			st->code.u.binop.result->kind = kind;
		} else if (st->code.kind == LABEL) {
			char *s = change(st->code.u.op);
			printf("LABEL %s :\n", s);
		} else if (st->code.kind == FUNCTION) {
			printf("FUNCTION %s :\n", st->code.name);							
		} else if (st->code.kind == ARG) {
			Operand op = st->code.u.op;			
			while (op != NULL) {
				char *s = change(op);
				printf("ARG %s\n", s);
				op = op->next;
			}
		} else if (st->code.kind == CONDITION) {
			char *s1 = change(st->code.u.relop.op1),
				 *s2 = change(st->code.u.relop.op2),
				 *s3 = change(st->code.u.relop.label);
			printf("IF %s %s %s GOTO %s\n", s1, st->code.u.relop.rel->u.value.relop, s2, s3);
		} else if (st->code.kind == GOTOL) {
			char *s = change(st->code.u.op);
			printf("GOTO %s\n", s);
		} else if (st->code.kind == RETURN) {
			char *s = change(st->code.u.op);
			printf("RETURN %s\n", s);
		} else if (st->code.kind == CALL) {
			char *s = change(st->code.u.op);
			if (strcmp(st->code.name, "read") == 0) 
				printf("READ %s\n", s);
			else if (strcmp(st->code.name, "write") == 0) 
				printf("WRITE %s\n", s);
			else 
				printf("%s := CALL %s\n", s, st->code.name);
		} else if (st->code.kind == PARAM) {
			char *s = change(st->code.u.op);
			printf("PARAM %s\n", s);
		}
		
		st = st->next;
	}
}

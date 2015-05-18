#include "intercode.h"
#include "node.h"
#include <string.h>
#include <stdio.h>

void translate(struct node *root) {
	if (root == NULL) return ;

	if (strcmp(root->type, "Stmt") == 0) {
		translate_Stmt(root);
		return ;
	}
	if (strcmp(root->type, "Compst") == 0) {
		translate_Compst(root);
		return ;
	}

	struct node *child = root->child;
	if (child != NULL) {
		translate(child);
		while (child->next != NULL) {
			translate(child->next);
			child = child->next;
		}
	}

}

Operand newOperand(int kind, char value) {
	Operand ret = (Operand)malloc(sizeof(Operand_));
	ret->kind = kind;
	if (kind == CONSTINT)
		ret->u.value.Int = atoi(value);
	if (kind == CONSTFLOAT)
		ret->u.value.Float = atof(value);
	if (kind == VARIABLE)
		ret->u.var_no = varcount ++;
	if (kind == TEMPVAR)
		ret->u.var_no = tmpvarcount ++;

	return ret;
}


InterCodes *translate_Exp(struct node *root, Operand op) {
	if (root = NULL) return NULL;

	struct node *child = root;
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
		op->kind = VARIABLE;
		op->u.var_no = varcount ++;
		return NULL;
	}
	//函数调用
	if (strcmp(child->type, "ID") == 0 && child->next != NULL) {
		struct node *now = child->next->next;
		char *name = child->value;
		if (strcmp(now->type, "RP") == 0) {
			InterCodes *code = (InterCodes *)malloc(sizeof(InterCodes));
			code->kind = FUNCTION;
			code->op = op;
			strcpy(code->name, name);

			return code;
		}

		if (strcmp(now->type, "Args") == 0) {
			Operand argList = NULL;
			InterCodes *code1 = translate_Args(now, argList);
			InterCodes *code2 = (InterCodes *)malloc(sizeof(InterCodes));
			InterCodes *code3 = (InterCodes *)malloc(sizeof(InterCodes));

			code2->kind = ARG;
			code2->op = argList;
			strcpy(code2->name, name);

			if (strcmp(name, "write") == 0) 
				return bindCode(code1, code2);

			if (argList->next != NULL) {
				argList = argList->next;
				InterCodes *code = (InterCodes *)malloc(sizeof(InterCodes));
				code->kind = ARG;
				code->op = argList;
				code->name = NULL;
				bindCode(code2, code);
			}

			code3->kind = FUNCTION;
			code3->op = op;
			strcpy(code3->name, name);

			return bindCode(bindCode(code1, code2), code3);
		}
	}
	//(Exp)
	if (strcmp(child->type, "LP") == 0) 
		return translate_Exp(child->next, op);	
	//-Exp	
	if (strcmp(child->type, "MINUS") == 0) {
		Operand tmp = newOperand(TEMPVAR, NULL); 
		InterCodes *code1 = translate_Exp(child->next, tmp);
		InterCodes *code2 = newInterCode(SUB, NULL, tmp, op);
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
			InterCodes *code2; 
			
			struct node *now = root->child;
			if (strcmp(now->child->type, "ID") == 0) 
				code2 = newInterCode(ASSIGN, op, tmp, NULL);
			else {				//目前只考虑一维数组，不考虑结构体
				Operand arr = newOperand(TEMPVAR, NULL);
				code2 = translate_Exp(now, arr);
			}

			return bindCode(code1, code2);
		}

		if (strcmp(child->type, "LB") == 0) {
			Operand op1 = (Operand)malloc(sizeof(Operand_));
			Operand op2 = (Operand)malloc(sizeof(Operand_));
			InterCodes *code1 = translate_Exp(root->child, op1);
			InterCodes *code2 = translate_Exp(child->next, op2);
			InterCodes *code3 = newInterCode(ARRAY, op1, op2, NULL);
			return bindCode(bindCode(code1, code2), code3);
		}

		Operand op1 = (Operand)malloc(sizeof(Operand_));
		Operand op2 = (Operand)malloc(sizeof(Operand_));
		InterCodes *code1 = translate_Exp(root->child, op1);
		InterCodes *code2 = translate_Exp(child->next, op2);
		int type;
		switch (child->type[0]) {
			case 'A': type = ADD; break;
			case 'S': type = SUN; break;
			case 'M': type = MUL; break;
			case 'D': type = DIV; break;
		}
		InterCodes *code3 = newInterCode(type, op1, op2, op);
		return bindCode(bindCode(code1, code2), code3);
	}
}

InterCodes *translate_Cond(struct node *root, Operand op1, Operand op2) {
	
}

InterCodes *translate_Stmt(struct node *root) {
	
}

InterCodes *translate_Compst(struct node *root) {

}

InterCodes *translate_Args(struct node *root, FieldList argList) {
}

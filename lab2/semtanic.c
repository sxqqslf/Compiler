#include <stdio.h>
#include <string.h>
#include "node.h"
#include "syntax.tab.h"

void init() {
	int i;
	for (i = 0; i < HASHSIZE; i ++)
		varHashtable[i] = stack[i] = NULL;
		funcHashtable[i] = NULL;
	level = 0;
}

void copyField(FieldList dst, FieldList src) {
	FieldList last = dst;
	while (src != NULL) {
		last->name = malloc(strlen(src->name)+1);
		strcpy(last->name, src->name);
		last->lineno = src->lineno;
		last->level = src->level;
		last->type = (Type)malloc(sizeof(Type_));
		copyType(last->type, src->type);
		last->down = last->tail = last->head = NULL;
		src = src->down;
		if (src != NULL) {
			last->down = (FieldList)malloc(sizeof(FieldList_));
			last = last->down;
		}
	}
}

void copyType(Type dst, Type src) {
	dst->kind = src->kind;
	switch (dst->kind) {
		case 0: dst->u.basic.intValue = src->u.basic.intValue; break;
		case 1: dst->u.basic.floatValue = src->u.basic.floatValue; break;
		case 2: dst->u.array.elem = (Type)malloc(sizeof(Type_)); 
				copyType(dst->u.array.elem, src->u.array.elem);
				dst->u.array.size = src->u.array.size; break;
		case 3: dst->u.structure = (FieldList)malloc(sizeof(FieldList_));
				copyField(dst->u.structure, src->u.structure); break;
	}
}

int insertVar(FieldList var, int lvl) {
	unsigned int index = hash_pjw(var->name);	
	if (varHashtable[index] == NULL) {
		varHashtable[index] = var;
		varHashtable[index]->level = level;
	} else {
		if (varHashtable[index]->level == level && 
			strcmp(varHashtable[index]->name, var->name) == 0) {
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", var->lineno, var->name);
			return 0;
		}
		var->tail = varHashtable[index]->tail;
		varHashtable[index]->tail = var;
		var->tail->head = var;
	}
	return 1;
}

int insertFunc(FunctionMessage *func) {
	unsigned int index = hash_pjw(func->name);
	if (funcHashtable[index] == NULL) {
		funcHashtable[index] = func;
		return 1;
	} else {
		printf("Error type 4 at Line %d: Redefined function \"%s\".\n", func->lineno, func->name);
		return 0;
	}
}

void travel(struct node *root) {
	if (root == NULL) return ;

	ExtDefList(root->child);
}


void ExtDefList(struct node *root) {
	if (root == NULL) return ;

	ExtDef(root->child);
	ExtDefList(root->child->next);
}

void ExtDef(struct node *root) {
	if (root == NULL) return ;

	struct node *child = root->child;
	Type type = Specifier(child);
	
	if (strcmp(child->next->type, "ExtDecList") == 0) 
		ExtDecList(child->next, type);
	else if (strcmp(child->next->type, "FunDec") == 0) {
		level ++;
		FunctionMessage *funType = FunDec(child->next, type);
		if (strcmp(child->next->next->type, "SEMI") == 0) 
			funType->visitedTag = 0;
		else { 
			funType->visitedTag = 1;
			FieldList index = funType->argList;
			if (index != NULL) {
				stack[top] = (FieldList)malloc(sizeof(FieldList_));
				FieldList last = stack[top ++];
				while (index != NULL) {							//将函数的参数插入符号表
					FieldList tmp = (FieldList)malloc(sizeof(FieldList_));
					tmp->name = malloc(strlen(index->name) + 1);
					strcpy(tmp->name, index->name);
					tmp->lineno = index->lineno;
					tmp->type = (Type)malloc(sizeof(Type_));
					copyType(tmp->type, index->type);
					tmp->tail = tmp->head = tmp->down = NULL;
					if (last != NULL) last->down = tmp;
					last = tmp;
					//insertVar(index, level);
					index = index->down;
				}
				Compst(child->next->next, stack[top-1], type);
				//函数定义里的Compst，把参数传入，和第一层的Compst中的变量比较，不能有相同, 并且把返回类型传入
			} else 
				Compst(child->next->next, NULL, type);
				//Compst结束后，会把函数体内的变量都free，包括函数的参数
		}
		level --;
	}
}

void ExtDecList(struct node *root, Type type) {
	if (root == NULL) return ;

	struct node *child = root->child;
	FieldList tmp = VarDec(child, type, 0, 1);
	//insertVar(tmp, level);

	if (child->next != NULL)
		ExtDecList(child->next->next, type);
}

Type Specifier(struct node *root) {
	if (root == NULL) return NULL;
	
	struct node *child = root->child;
	Type ret;

	if (strcmp(child->type, "TYPE") == 0) {
		ret = (Type)malloc(sizeof(Type_));
		if (strcmp(child->value, "int") == 0)
			ret->kind = 0;			
		else if (strcmp(child->value, "float") == 0)
			ret->kind = 1;
	} else 
		ret = StructSpecifier(child);
	
	return ret;
}

Type StructSpecifier(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child->next;
	if (strcmp(child->type, "LC") == 0 || child->next != NULL) {				//结构体的定义,放入结构体定义表中
		int flag = strcmp(child->type, "LC");
		FieldList tmp = (FieldList)malloc(sizeof(FieldList_));
		if (flag) {
			tmp->name = malloc(strlen(child->child->value) + 1);
			strcpy(tmp->name, child->child->value);
		}
		tmp->type = (Type)malloc(sizeof(Type_)); 
		tmp->type->kind = 3;
		if (flag) 
			tmp->type->u.structure = DefList(child->next->next, 1, NULL);
		else 
			tmp->type->u.structure = DefList(child->next, 1, NULL);
		int ret = insertStruct(tmp);
		if (ret || (flag && varHashtable[hash_pjw(child->child->value)] != NULL)) 	//已经定义了这个结构体，或者结构体名称和已有变量冲突
			printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", child->line, child->child->value);
		return tmp->type;
	} else {				//用结构体定义一个变量
		unsigned int index = hash_pjw(child->child->value); 
		if (structHashtable[index] == NULL) { 	//这个结构体并没有定义过
			printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", child->line, child->child->value);
			return NULL;
		} else {				
			Type ret = (Type)malloc(sizeof(Type_));
			ret->kind = 3;
			ret->u.structure = structHashtable[index]->type->u.structure;
			return ret;
		}
	}
}

FieldList DefList(struct node *root, int isStruct, FieldList first) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	FieldList ret = Def(child, isStruct);

	FieldList tmp = first;
	int flag = 1;
	while (tmp != NULL && ret != NULL) {
		if (strcmp(tmp->name, ret->name) == 0) {
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n", child->line, ret->name);
			flag = 0; break;
		}
		tmp = tmp->down;
	}

	if (first == NULL) first = ret;
	else if (flag) first->down = ret;

	tmp = first;
	while (tmp->down != NULL) tmp = tmp->down;
	if (child != NULL) 
		tmp->down = DefList(child->next, isStruct, first);

	return ret;
}

FieldList Def(struct node *root, int isStruct) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	Type type = Specifier(child);
	
	FieldList ret = DecList(NULL, child->next, type, isStruct);

	return ret;
}

FieldList DecList(FieldList first, struct node *root, Type type, int isStruct)  {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	FieldList ret = Dec(child, type, isStruct);

	FieldList tmp = first; 
	int flag = 1;
	while (tmp != NULL && ret != NULL) {
		if (strcmp(tmp->name, ret->name) == 0) {
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n", child->line, ret->name);
			flag = 0; break;
		}
		tmp = tmp->down;
	}

	if (first == NULL) first = ret;
	else first->down = ret;
	child = child->next; 
	if (child != NULL) {
		tmp = first;
		if (tmp == NULL) tmp = DecList(first, child->next, type, isStruct);
		else {
			while (tmp->down != NULL) tmp = tmp->down;
			tmp->down = DecList(first, child->next, type, isStruct);
		}
	}

	return ret;
}

FieldList Dec(struct node *root, Type type, int isStruct) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	FieldList ret = VarDec(child, type, isStruct, 1);	

	if (child->next != NULL) {
		Type tmp = Exp(child->next->next);
		if (tmp != NULL && compareType(tmp, type) != 0) {
			printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child->line);
			return NULL;
		}
	}
	
	return ret;
}

void Compst(struct node *root, FieldList arg, Type returnType) {
	if (root == NULL) return ;

	struct node *child = root->child->next;		//DefList
	if (returnType != NULL) {					//函数定义部分的Compst
		if (strcmp(child->type, "DefList") == 0) {
			FieldList varIn = DefList(child, 0, NULL);			//change	
			FieldList store = varIn, tmp = arg;

			if (arg == NULL) { 
				arg = varIn; stack[top ++] = arg; 
			} else {
				//把和函数参数同一层的变量链表接到参数表的后方
				while (arg->down!= NULL) arg = arg->down;
				arg->down = varIn;
			}
		}
	} else {					//StmtList部分的Compst
		level ++;
		if (strcmp(child->type, "DefList") == 0) {
			FieldList varIn = DefList(child, 0, arg);	
			stack[top ++] = varIn;
		}
		level --;
	}

	if (strcmp(child->type, "DefList") == 0) 
		StmtList(child->next, returnType);
	else 
		StmtList(child, returnType);

	del(stack[top-1]); top --;
}

void StmtList(struct node *root, Type returnType) {
	if (root == NULL) return ;
	
	struct node *child = root->child;
	Stmt(child, returnType);
	if (child != NULL) StmtList(child->next, returnType);
}

void Stmt(struct node *root, Type returnType) {
	if (root == NULL) return ;

	struct node *child = root->child;
	if (strcmp(child->type, "Exp") == 0) {
		Exp(child);
	}
	else if (strcmp(child->type, "Compst") == 0) 
		Compst(child, NULL, returnType);
	else if (strcmp(child->type, "RETURN") == 0) {
		Type tmp = Exp(child->next);
		if (compareType(tmp, returnType)) 
			printf("Error type 8 at Line %d: Type mismatched for return.\n", child->next->line);
	} else if (strcmp(child->type, "IF") == 0) {
		Exp(child->next->next);
		child = child->next->next->next->next;
		Stmt(child, returnType);
		if (child->next != NULL) 
			Stmt(child->next->next, returnType);
	} else {
		Exp(child->next->next);
		child = child->next->next;
		Stmt(child->next->next, returnType);
	}
}

FunctionMessage *FunDec(struct node *root, Type type) {
	if (root == NULL) return NULL;

	FunctionMessage *ret = (FunctionMessage *)malloc(sizeof(FunctionMessage));
	struct node *child = root->child;

	ret->name = malloc(strlen(child->value)+1);
	strcpy(ret->name, child->value);
	ret->lineno = child->line;
	ret->returnType = type;
	ret->next = NULL;

	if (strcmp(child->next->next->type, "RP") == 0) 
		ret->argList = NULL;
	else  {
		ret->argList = (FieldList)malloc(sizeof(FieldList_));

		copyField(ret->argList, VarList(child->next->next));
	assert(ret->argList != NULL);
	}
	
	int flag = insertFunc(ret);
	return (flag)?ret:NULL;
}

FieldList VarList(struct node *root) {
	if (root == NULL) return NULL;

	FieldList ret;
	struct node *child = root->child;

	ret = ParamDec(child);
	child = child->next;

	if (child != NULL) {
		FieldList tmp = VarList(child->next);
		if (ret == NULL) ret = tmp;
		else {
			FieldList iter = ret;
			int flag = 0;
			while (iter->down != NULL) {	
				if (compare(iter, tmp) == 0) flag = 1;
				iter = iter->down;
			}
			iter->down= tmp;
			if (flag)			//函数定义时，参数重复定义
				printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", child->line, tmp->name);
		}
	}

	return ret;
}

FieldList ParamDec(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	Type type = Specifier(child);
	
	return VarDec(child->next, type, 0, 1);
}

FieldList VarDec(struct node *root, Type type, int isStruct, int top) {
	if (root == NULL) return NULL;

	FieldList ret;
	struct node *child = root->child;
	if (child->next == NULL) {				//ID
		ret = (FieldList)malloc(sizeof(FieldList_));
		ret->name = malloc(strlen(child->value) + 1);
		strcpy(ret->name, child->value);
		ret->lineno = child->line;
		ret->type = (Type)malloc(sizeof(Type_));
		copyType(ret->type, type);
		ret->tail = ret->head = ret->down = NULL;
		if (top && !isStruct && !insertVar(ret, level)) return NULL;
	} else {
		ret = VarDec(child, type, isStruct, 0);
		Type temp = (Type)malloc(sizeof(Type_));
		temp->kind = 2;
		temp->u.array.elem = ret->type;
		temp->u.array.size = atoi(child->next->next->value);
		ret->type = temp;
		if (top && !isStruct && !insertVar(ret, level)) return NULL;
	}

	return ret;
}

char *getargList(FieldList arg) {
	int flag = 0;
	char *argl = malloc(100);
	
	memset(argl, 0, 100);
	while (arg != NULL) {
		int type = arg->type->kind;
		if (flag) strcat(argl, ", ");
		switch (type) {
			case 0: strcat(argl, "int"); break;
			case 1: strcat(argl, "float"); break;
			case 2: strcat(argl, "[]");
					Type tmp = arg->type->u.array.elem;
					while (tmp->kind == 2) {
						strcat(argl, "[]");
						tmp = tmp->u.array.elem;
					}
					break;
			case 3: strcat(argl, "struct "); break;
		}
		flag = 1;
		arg = arg->down;
	}

	return argl;
}

Type Exp(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	if (strcmp(child->type, "Exp") == 0) {
		Type tmp1 = Exp(child);
		if (strcmp(child->next->next->type, "Exp") == 0) {
			Type tmp2 = Exp(child->next->next);
			if (strcmp(child->next->type, "ASSIGNOP") == 0) {
				struct node *tmp = child->child;
				int flag = 0;
				if (strcmp(tmp->type, "ID") == 0 && tmp->next == NULL) flag = 1;
				if (strcmp(tmp->type, "Exp") == 0)
					if (strcmp(tmp->next->type, "LB") == 0 || strcmp(tmp->next->type, "DOT") == 0) flag = 1;
				if (!flag) {
					printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", child->line);
					return NULL;
				}	
				if (tmp1 != NULL && tmp2 != NULL && compareType(tmp1, tmp2)) {
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child->line);
					return NULL;
				}
				return tmp1;
			} else if (strcmp(child->next->type, "LB") == 0) {
				if (tmp1->kind != 2) {
					printf("Error type 10 at Line %d: \"%s\" is not an array.\n", child->line, child->child->value);
					return NULL;
				}
				if (tmp2->kind != 0) {
					printf("Error type 12 at Line %d: \"%.2lf\" is not an integer.\n", child->line, tmp2->u.basic.floatValue);
					return NULL;
				}
				/******--------------***********/
			} else if (strcmp(child->next->type, "DOT") == 0) {
				if (tmp1->kind != 3) {
					printf("Error type 13 at Line %d: Illegal use of \".\".\n", child->line);
					return NULL;
				}
			} else {
				int flag = 0;
				if (tmp1 != NULL && tmp2 != NULL) { 
					if (compareType(tmp1, tmp2)) flag = 1;
					if ((tmp1->kind != 0 && tmp1->kind != 1) || 
						(tmp2->kind != 0 && tmp2->kind != 1)) flag = 1;
					if (flag) {
						printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->line);
						return NULL;
					}
				}
			}
			return tmp2;
		} else if (child->next->next->type, "ID") {
			Type type = Exp(child);
			if (type->kind != 3) {
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", child->line);
				return NULL;
			}
			//取出‘.’后面的域的类型
			char *structvar = malloc(strlen(child->next->next->value) + 1);
			strcpy(structvar, child->next->next->value);
			FieldList st = type->u.structure;
			while (st != NULL) {
				if (strcmp(st->name, structvar) == 0) return st->type;
				st = st->down;
			}
			printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", child->line, structvar);
			return NULL;
		}
		return tmp1;
	} else if (strcmp(child->type, "LP") == 0) {
		return Exp(child->next);
	} else if (strcmp(child->type, "MINUS") == 0 || strcmp(child->type, "NOT") == 0) {
		Type tmp = Exp(child->next);
		if (tmp->kind != 0 && tmp->kind != 1) 
			printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->line);
		return tmp;
	} else if (strcmp(child->type, "ID") == 0 && child->next != NULL) {
		int index = hash_pjw(child->value);
		FieldList var = varHashtable[index];
		if (var != NULL) {
			printf("Error type 11 at Line %d: \"%s\" is not a function.\n", child->line, child->value);
			return NULL;
		}
		FunctionMessage *func = funcHashtable[index];
		if (func == NULL) {
			printf("Error type 2 at Line %d: Undefined function \"%s\".\n", child->line, child->value);
			return NULL;
		}
		if (func->visitedTag == 0) {
			printf("Error type 18 at Line %d: Undefined function \"%s\".\n", child->line, child->value);
			return NULL;
		}
		struct node *arg;
		if (strcmp(child->next->next->type, "Args") == 0) 
			arg = child->next->next;
		else 
			arg = NULL;

		FieldList argList = Args(arg);
		char *argl1 = getargList(func->argList), *argl2 = getargList(argList);
		if (compare(argList, func->argList) != 0) {
			printf("Error type 9 at Line %d: Function \"%s(%s)\" is not applicable for arguments \"(%s)\".\n", child->line, func->name, argl1, argl2);
			return NULL;
		}	
		return func->returnType;
	} else if (strcmp(child->type, "ID") == 0) {
		FieldList ret = varHashtable[hash_pjw(child->value)];
		if (ret == NULL) {
			printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", child->line, child->value);
			return NULL;
		}
		return ret->type;
	} else if (strcmp(child->type, "INT") == 0 || strcmp(child->type, "FLOAT") == 0) {
		int flag = 0;
		if (strcmp(child->type, "FLOAT") == 0) flag = 1;
		FieldList ret = (FieldList)malloc(sizeof(FieldList_));
		ret->name = NULL;
		ret->lineno = child->line;
		ret->type = (Type)malloc(sizeof(Type_));
		ret->type->kind = flag;
		if (flag == 0)
			ret->type->u.basic.intValue = atoi(child->value);
		else 
			ret->type->u.basic.floatValue = atof(child->value);
		ret->tail = ret->head = ret->down = NULL;

		return ret->type;
	}
}

FieldList Args(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child;
	Type type = Exp(child);
	FieldList ret = (FieldList)malloc(sizeof(FieldList_));

	ret->name = NULL;
	ret->lineno = child->line;
	ret->type = (Type)malloc(sizeof(Type_));
	copyType(ret->type, type);
	ret->tail = ret->head = ret->down = NULL;

	if (child->next != NULL) 
		ret->down = Args(child->next->next);
	
	return ret;
}

int compare(FieldList v1, FieldList v2) {	
	//同为结构体，且相同
	while ( 1 ) {
		if (v1 == NULL && v2 != NULL || v1 != NULL && v2 == NULL) return 1;
		if (v1 == NULL && v2 == NULL) return 0;
		if (v1->type->kind == v2->type->kind) {
			if (v1->type->kind == 3) {			//结构体
				if (compareType(v1->type, v2->type)) return 1;
			} else if (v1->type->kind == 2) {	//数组
				if (compareType(v1->type->u.array.elem, v2->type->u.array.elem)) return 1;
			} 
			v1 = v1->down;
			v2 = v2->down;
		} else 
			return 1;
	}
}

int compareType(Type t1, Type t2) {
	while ( 1 ) {
		if (t1->kind != t2->kind) return 1;				//类型不同，直接返回
		if (t1->kind == 0 || t1->kind == 1) return 0;	//基本类型，直接返回
		if (t1->kind == 2) {
			t1 = t1->u.array.elem;
			t2 = t2->u.array.elem;
			continue;
		}
		return compare(t1->u.structure, t2->u.structure);
	}
}

int insertStruct(FieldList struc) {
	int index = hash_pjw(struc->name);	

	if (structHashtable[index] == NULL) {
		structHashtable[index] = struc;
		return 0;
	}
	FieldList tmp = structHashtable[index];
	while (tmp != NULL) {
		if (compare(tmp, struc) == 0) return 1;
		tmp = tmp->tail;
	}
	struc->tail = structHashtable[index]->tail;
	structHashtable[index]->tail->head = struc;
	structHashtable[index] = struc;
	return 0;
}

void del(FieldList obj) {
	if (obj == NULL) return ;
	while (obj != NULL) {
		FieldList tmp = obj;
		obj = obj->down;
		free(tmp->name);
		dell(tmp->type);	
		free(tmp);
	}
}

void dell(Type t) {
	if (t == NULL) return ;
	if (t->kind == 0 || t->kind == 1) {
		free(t); return ;
	}
	if (t->kind == 2) 
		dell(t->u.array.elem);
	else 
		del(t->u.structure);
}

int main(int argc, char **argv) {
	init();
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]); return 1;
	}
	root = NULL; 
	yyrestart(f); yyparse();
	//printTree(root, 0);	
	travel(root);
	return 0;
}

yyerror(char *msg) {
	printf("syntax error!\n");
}

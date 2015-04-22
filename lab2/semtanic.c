#include <stdio.h>
#include <string.h>
#include "node.h"
#include "syntax.tab.h"

void init() {
	int i;
	for (i = 0; i < HASHSIZE; i ++)
		varHashtable[i] = funcHashtable[i] = stack[i] = NULL;
}

void insertVar(FieldList var) {
	unsigned int index = hash_pjw(var->name);	
	if (varHashtable[index] == NULL)
		varHashtable[index] = var;
	else {
		var->tail = varHashtable[index]->tail;
		varHashtable[index]->tail = var;
		var->tail->head = var;
	}
}

void travel(struct node *root) {
	if (root == NULL) return ;

	ExtDefList(root->child);
}

void ExtDefList(struct node *root) {
	if (root == NULL) return ;

	ExtDef(root->child);
	ExtDefList(root->child);
}

void ExtDef(struct node *root) {
	if (root == NULL) return ;

	struct node *child = root->child;
	Type type = Specifier(child);
	
	if (strcmp(child->next->type, "ExtDecList") == 0) 
		ExtDecList(child->next, type);
	else if (strcmp(child->next->type, "FunDec") == 0) {
		FunctionMessage *funType = FunDec(child->next, type);
		if (strcmp(child->next->next->type, "SEMI") == 0) 
			funType->visitedTag = 0;
		else { 
			funType->visitedTag = 1;
			FieldList index = funType->argList;
			FieldList tmp = stack[top++], last = NULL;  
			while (index != NULL) {							//将函数的参数插入符号表
				tmp = (FieldList)malloc(sizeof(FieldList));
				tmp->name = malloc(strlen(index->name) + 1);
				strcpy(tmp->name, index->name);
				tmp->lineno = index->lineno;
				tmp->type = index->type;					//这里不再malloc
				tmp->tail = tmp->head = tmp->down = NULL;
				if (last != NULL) last->down = tmp;
				last = tmp;
				insertVar(tmp);
				index = index->tail;
			}
			//函数定义里的Compst，把参数传入，和第一层的Compst中的变量比较，不能有相同
		 	Compst(child->next->next, stack[top-1]);	
		}
	}
}

void Compst(struct node *root, FieldList var) {
	if (root == NULL) return ;

	struct node *child = root->child->next;		//DefList
	FieldList varIn = DefList(child);	

	if (var != NULL && varIn != NULL) {			//函数定义部分的Compst
		FieldList store = varIn;
		while (varIn != NULL) {
			FieldList tmp = var;
			int flag = 0;
			while (tmp != NULL) {
				if (compare(varIn, tmp) == 0) {
					printf("Error Type 3 at Line %d: Redefined variable %s\n", tmp->lineno, tmp->name);
					flag = 1;
				}
				tmp = tmp->down;
			}
			if (!flag) insertVar(varIn);
			varIn = varIn->down;
		}
		//把和函数参数同一层的变量链表接到参数表的后方
		varIn = store;
		while (var->down!= NULL) var = var->down;
		var->down = varIn;
	} else {					//StmtList部分的Compst
		FieldList st = stack[top ++];
		st = varIn;
		while (varIn != NULL) {
			insertVar(varIn);
			varIn = varIn->down;
		}
	}
}

FunctionMessage *FunDec(struct node *root, Type_ type) {
	if (root == NULL) return NULL;

	FunctionMessage *ret = (FunctionMessage *)malloc(sizeof(FunctionMessage));
	struct node *child = root->child;

	ret->name = malloc(strlen(child->name)+1);
	strcpy(ret->name, child->name);
	ret->lineno = child->line;
	ret->returnType = type;
	ret->next = NULL;

	if (strcmp(child->next->next->type, "RP") == 0) 
		ret->argList = NULL;
	else 
		ret->argList = VarList(child->next->next);
	
	return ret;
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
			while (iter->tail != NULL) {	
				if (compare(iter, tmp) == 0) flag = 1;
				iter = iter->tail;
			}
			iter->tail = tmp;
			if (flag)			//函数定义时，参数重复定义
				printf("Error Type 3 at Line %d: Redefined variable \"%s\".\n", child->line, tmp->name);
		}
	}

	return ret;
}

FieldList ParamDec(struct node *root) {
	if (root == NULL) return NULL;

	struct node *child = root->child
	Type type = Specifier(child);
	
	return VarDec(child->next, type);
}

FieldList VarDec(struct node *root, Type type) {
	if (root == NULL) return NULL;

	FieldList ret;
	struct node *child = root->child;
	if (child->next == NULL) {				//ID
		ret = (FieldList)malloc(sizeof(FieldList));
		ret->name = malloc(strlen(child->name) + 1);
		strcpy(ret->name, child->name);
		ret->type = type;
		ret->tail = NULL;
	} else {
		ret = VarDec(child, type);
		Type temp = (Type)malloc(sizeof(Type));
		temp->kind = 1;
		temp->u.array.elem = ret->type;
		temp->u.array.size = atoi(child->next->next->value);
		ret->type = temp;
	}

	return ret;
}

int main() {
	init();
	travel(root);
	return 0;
}


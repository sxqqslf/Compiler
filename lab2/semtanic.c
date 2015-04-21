#include <stdio.h>
#include <string.h>
#include "node.h"
#include "syntax.tab.h"

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
		else 
			funType->visitedTag = 1;
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
		argList = NULL;
	else 
		argList = VarList(child->next->next);

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
				printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", child->line, tmp->name);
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
	travel(root);
	return 0;
}


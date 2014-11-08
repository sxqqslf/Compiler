#include "node.h"

struct node *newNode(int isTerminal, int line, char* type, char* value) {
	struct node *ret = (struct node *)malloc(sizeof(struct node));
	ret->isTerminal = isTerminal;
	ret->line = line;
	ret->type = malloc(strlen(type)+1);
	strcpy(ret->type, type);
	ret->value = malloc(strlen(type)+1);
	strcpy(ret->value, value);
	ret->child = ret->next = NULL;
	return ret;
}

struct node *createNode(int arity, ...) {
	va_list	ap;
	va_start(ap, arity);

	struct node *root = (struct node *)malloc(sizeof(struct node));
	//assert(root != NULL);
	root->child = root->next = NULL;
	root->isTerminal = 0;
	char *s = va_arg(ap, char *);
	root->type = malloc(strlen(s)+1);
	strcpy(root->type, s);

	while (arity --) {
		struct node *t = va_arg(ap, struct node *);
		//assert(t != NULL);
		if (t == NULL) continue;
		root->line = t->line;
		t->next = root->child;
		root->child = t;
	}	
	va_end(ap);
	return root;
}

//打印语法分析树，root为树节点，indent为缩进量
void printTree(struct node* root, int indent) {
	if (root == NULL) return ;

	int len = 0; 
	for (; len < indent; len ++) printf(" ");		//打印缩进

	printf("%s", root->type);
	if (root->isTerminal) {
		if (strcmp(root->type, "ID") == 0) 
			printf(": %s\n", root->value);
		else if (strcmp(root->type, "TYPE") == 0)
			printf(": %s\n", root->value);
		else if (strcmp(root->type, "INT") == 0)
			printf(": %d\n", atoi(root->value));
		else if (strcmp(root->type, "FLOAT") == 0)
			printf(": %.5f\n", atof(root->value));
		else
			printf("\n");
	} else 
		printf(" (%d)\n", root->line);

	struct node* child = root->child;
	while (child != NULL) {
		printTree(child, indent+2);
		child = child->next;
	}
}






#include "node.h"

struct node* newNode(int isTerminal, int line, char* type, char* value) {
	struct node *ret = (struct node *)malloc(sizeof(struct node));
	ret->isTerminal = isTerminal;
	ret->line = line;
	ret->type = type;
	ret->value = value;
	ret->child = ret->next = NULL;
	return ret;
}

struct node* createNode(int arity, ...) {
	va_list	ap;
	va_start(ap, arity);

	struct node* root = (struct node*)malloc(sizeof(struct node));
	char* s = va_arg(ap, char*);
	root->type = malloc(sizeof(s)+1);
	printf("%s\n", s);
	root->isTerminal = 0;
	strcpy(root->type, s);
	int flag = 0;
	while (arity --) {
		struct node* t = va_arg(ap, struct node*);
		if (flag == 0) {
			root->line = t->line;
			flag = 1;
		}
		if (root->child == NULL) 
			root->child = t;
		else {
			t->next = root->child; root->child = t;
		}
	}	
	return root;
}

//打印语法分析树，root为树节点，indent为缩进量
void printTree(struct node* root, int indent) {
	if (root == NULL) return ;

	int len = 0; 
	for (; len < indent; len ++) printf(" ");		//打印缩进

	printf("%s", root->type);
	if (root->isTerminal) {
		if (strcpy(root->type, "ID") == 0) 
			printf(": %s\n", root->value);
		else if (strcpy(root->type, "TYPE") == 0)
			printf(": %s\n", root->value);
		else if (strcpy(root->type, "INT") == 0)
			printf(": %d\n", atoi(root->value));
		else if (strcpy(root->type, "FLOAT") == 0)
			printf(": %.5f\n", atof(root->value));
		else
			printf("%s\n", root->value);
	} else 
		printf(" (%d)\n", root->line);

	struct node* child = root->child;
	while (child != NULL) {
		printTree(child, indent+2);
		child = child->next;
	}
}






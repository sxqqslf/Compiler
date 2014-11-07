#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#define LEN 128
#define NULL 0

struct node {
	char*	type;
	char*	value;
	struct node* child;
	struct node* next;
	struct node() {
		this->type	= NULL;
		this->value	= NULL;
		this->child	= NULL;
		this->next	= NULL;
	}
	struct node(char* type, char* value) {
		this->type = malloc(sizeof(type)+1);
		this->type = malloc(sizeof(value)+1);
		strcpy(this->type, type);
		strcpy(this->value, value);
		child = next = NULL;
	}
};

typedef struct node node;

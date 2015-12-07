#include <stdio.h>
#include <stdlib.h>
#include "minic_ast.h"

char *nodeName[];
Node *valueStack[STACK_SIZE];   
int rightLength[NODE_NUM + 1];  
int sp;

void printTree(Node *ptr, int indent);
void printNode(Node *pt, int indent);

char *nodeName[] = {
	"ACTUAL_PARAM",	"ADD",			"ADD_ASSIGN",	"ARRAY_VAR",	"ASSIGN_OP",
	"CALL",			"COMPOUND_ST",	"CONST_NODE",	"DCL",			"DCL_ITEM",
	"DCL_LIST",		"DCL_SPEC",		"DIV",			"DIV_ASSIGN",	"EQ",
	"ERROR_NODE",	"EXP_ST",		"FORMAL_PARA",	"FUNC_DEF",		"FUNC_HEAD",
	"GE",			"GT",			"IDENT",		"IF_ELSE_ST",	"IF_ST",
	"INDEX",		"INT_NODE",		"LE",			"LOGICAL_AND",	"LOGICAL_NOT",
	"LOGICAL_OR",	"LT",			"MOD",			"MOD_ASSIGN",	"MUL",
	"MUL_ASSIGN",	"NE",			"NUMBER",		"PARAM_DCL",	"POST_DEC",
	"POST_INC",		"PRE_DEC",		"PRE_INC",		"PROGRAM",		"RETURN_ST",
	"SIMPLE_VAR",	"STAT_LIST",	"SUB",			"SUB_ASSIGN",	"UNARY_MINUS",
	"VOID_NODE",	"WHILE_ST"		
};

int ruleName[] = {
		0,	PROGRAM, 0, 0, 0,
		0, FUNC_DEF, FUNC_HEAD, DCL_SPEC, 0,
		0, 0, 0, CONST_NODE, INT_NODE,
		VOID_NODE, 0, FORMAL_PARA, 0, 0, 
		0, 0, PARAM_DCL, COMPOUND_ST, DCL_LIST,
		DCL_LIST, 0, 0, DCL, 0,
		0, DCL_ITEM, DCL_ITEM, SIMPLE_VAR, ARRAY_VAR,
		0, 0, STAT_LIST, 0, 0, 
		0, 0, 0, 0, 0, 
		0, EXP_ST, 0, 0, IF_ST,
		IF_ELSE_ST, WHILE_ST, RETURN_ST, 0, 0, 
		ASSIGN_OP, ADD_ASSIGN, SUB_ASSIGN, MUL_ASSIGN, DIV_ASSIGN,
		MOD_ASSIGN, 0, LOGICAL_OR, 0, LOGICAL_AND,
		0, EQ, NE, 0, GT,
		LT, GE, LE, 0, ADD, 
		SUB, 0, MUL, DIV, MOD,
		0, UNARY_MINUS, LOGICAL_NOT, PRE_INC, PRE_DEC,
		0, INDEX, CALL, POST_INC, POST_DEC,
		0, 0, ACTUAL_PARAM, 0, 0, 
		0, 0, 0
};

Node* buildNode(int tokenNumber, char* tokenValue)
{
	Node *ptr;

	ptr = (Node *) malloc(sizeof(Node));

	if (!ptr) {
		printf("ERROR : memory allocation error in buildNode()\n");
		exit(1);
	}

	ptr->token.tokenNumber = tokenNumber;
	ptr->token.tokenValue = tokenValue;
	ptr->noderep = TERMINAL;
	ptr->son = ptr->next = NULL;
	return ptr;
}


Node* buildTree(int tokenNumber, Node* son, Node* next)
{
	Node *ptr;

	ptr = (Node*) malloc(sizeof(Node));
	if(!ptr)
	{
		printf("ERROR : memory allocation error in buildTree()\n");
		exit(1);
	}

	ptr->token.tokenNumber = tokenNumber;
	ptr->token.tokenValue = NULL;
	ptr->noderep = NONTERM;
	ptr->son = son;
	ptr->next = next;

	return ptr;
}

void appendNext(Node* node, Node* next)
{
	Node *ptr = node;
	int i = 1;
	while (ptr->next != NULL) { // 계속 뒤쪽으로 연결해서 붙도록...
		ptr = ptr->next;
		i++;
	}
//	printf("append next %dth ... \n", i);
	ptr->next = next;
}

// Node* buildTree(int nodeNumber, Token)
// {
// 	int start, i, j;
// 	Node *first, *ptr;

// 	i = sp - rhsLength + 1;

// 	if(!nodeNumber && i > sp)
// 	{
// 		ptr = NULL;
// 	}
// 	else
// 	{
// 		start = i;

// 		while(i <= sp - 1)
// 		{
// 			j = i + 1;
// 			while(j <= sp && valueStack[j] == NULL) j++;
// 			if(j <= sp)
// 			{
// 				ptr = valueStack[i];
// 				while(ptr->next) ptr = ptr->next;
// 				ptr->next = valueStack[j];
// 			}
// 			i = j;
// 		}
// 		first = (start > sp) ? NULL : valueStack[start];

// 		if(nodeNumber)
// 		{
// 			ptr = (Node*) malloc(sizeof(Node));
// 			if(!ptr)
// 			{
// 				printf("ERROR : memory allocation error in buildTree()\n");
// 				exit(1);
// 			}

// 			ptr->token.tokenNumber = nodeNumber;
// 			ptr->token.tokenValue = NULL;
// 			ptr->noderep = nonterm;
// 			ptr->son = first;
// 			ptr->next = NULL;
// 		} else {
// 			ptr = first;
// 		}
// 	}

// 	sp = sp - rhsLength + 1;
// 	return ptr;
// }


void printNode(Node *pt, int indent)
{
	int i;
	for (i = 1; i <= indent; i++) {
		printf("    ");
	}

	if (pt->noderep == TERMINAL)	{
		printf("Terminal: %s\n", pt->token.tokenValue);
	} else {
		int i;
		i = (int) (pt->token.tokenNumber);
		printf("Nonterminal: %s\n", nodeName[i]);
	}
}


void printTree(Node *ptr, int indent)
{
	Node *p = ptr;
	while (p != NULL) {
		printNode(p, indent);
		if (p->noderep == NONTERM) {
			printTree(p->son, indent + 1);
		}
		p = p->next;
	}
}

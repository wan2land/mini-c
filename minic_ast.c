#include <stdio.h>
#include "minic_ast.h"

extern char *nodeName[];

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

Node *valueStack[STACK_SIZE];	
int rightLength[NODE_NUM + 1];	
int sp;							
int i;

void buildNode(int tokenNumber, char *tokenValue)
{
	Node *ptr;
	Token token;

	token.tokenNumber = tokenNumber;
	token.tokenValue = tokenValue;

	if(++sp > STACK_SIZE) {
		printf("ERROR : parsing stack overflow\n");
		exit(1);
	}

	ptr = (Node *) malloc(sizeof(Node));
	if(!ptr)
	{
		printf("ERROR : memory allocation error in buildNode()\n");
		exit(1);
	}

	ptr->token = token;
	ptr->noderep = terminal;
	ptr->son = ptr->brother = NULL;

	valueStack[sp] = ptr;
}


void buildTree(int nodeNumber, int rhsLength)
{
	int start, i, j;
	Node *first, *ptr;

	i = sp - rhsLength + 1;

	if(!nodeNumber && i > sp)
	{
		ptr = NULL;
	}
	else
	{
		start = i;

		while(i <= sp - 1)
		{
			j = i + 1;
			while(j <= sp && valueStack[j] == NULL) j++;
			if(j <= sp)
			{
				ptr = valueStack[i];
				while(ptr->brother) ptr = ptr->brother;
				ptr->brother = valueStack[j];
			}
			i = j;
		}
		first = (start > sp) ? NULL : valueStack[start];

		if(nodeNumber)
		{
			ptr = (Node*) malloc(sizeof(Node));
			if(!ptr)
			{
				printf("ERROR : memory allocation error in buildTree()\n");
				exit(1);
			}

			ptr->token.tokenNumber = nodeNumber;
			ptr->token.tokenValue = NULL;
			ptr->noderep = nonterm;
			ptr->son = first;
			ptr->brother = NULL;
		}
		else
			ptr = first;
	}

	sp = sp - rhsLength + 1;
	valueStack[sp] = ptr;
	rightLength[nodeNumber] = 0;
}


void printNode(Node *pt, int indent)
{
	int i;

	for(i = 1; i <= indent; i++)
		printf(" ");

	if(pt->noderep == terminal)		
	{
		printf(" Terminal: %s", pt->token.tokenValue);
	}
	else							
	{
		int i;
		i = (int) (pt->token.tokenNumber);
		printf(" Nonterminal: %s", nodeName[i]);
	}
	printf("\n");
}


void printTree(Node *pt, int indent)
{
	Node *p = pt;
	while(p != NULL)
	{
		printNode(p, indent);
		if(p->noderep == nonterm)
			printTree(p->son, indent + 4);
		
		p = p->brother;
	}
}

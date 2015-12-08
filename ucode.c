#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minic_ast.h"

#define LABEL_SIZE			8
#define SYMTAB_ARRAY_SIZE	512

int checkPredefined(Node *ptr);
void processCondition(Node *ptr);

FILE* file;

typedef enum {
    NON_SPECIFIER, VOID_TYPE, INT_TYPE
} TypeSpecifier;

typedef enum {
    NON_QUALIFIER, FUNC_TYPE, PARAM_TYPE, CONST_TYPE, VAR_TYPE
} TypeQuailfier;

typedef struct {
	char *id;
	TypeSpecifier typeSpecifier;
	TypeQuailfier typeQualifier;
	int offset;
	int width;
	int base;
	int initialValue;
	struct symtab *link;
} symtabArray;

typedef struct symtab
{
	char *name;
	int num;
	int base;
	symtabArray st[SYMTAB_ARRAY_SIZE];
	struct symtab *parent;
} SymTab;

int labelNum = 0, width = 1, lvalue;
int stackTop, offsetStack[5], symtabBase;
int flag_returned, returnWithValue;
struct symtab *tblStack[5];
symtabArray rtSymbol;

void dd(Node* ptr)
{
	printTree(ptr, 1, stdout);
}

void icg_error(int i)
{
    fprintf(stderr, "ERROR: %d\n", i);
}

void emit0(char *opcode)
{
    fprintf(file, " \t %s \n", opcode);
}

void emit1(char *opcode, int operand)
{
    fprintf(file, " \t %s \t %d\n", opcode, operand);
}

void emit2(char *opcode, int operand1, int operand2)
{
    fprintf(file, " \t %s \t %d \t %d\n", opcode, operand1, operand2);
}

void emitJump(char *opcode, char *operand)
{
    fprintf(file, " \t %s \t %s\n", opcode, operand);
}

void emitSym(char *opcode, int operand1, int operand2, int operand3)
{
    fprintf(file, " \t %s \t %d \t %d \t %d \t\n", opcode, operand1, operand2, operand3);
}

void emitProc(char *label, int proc1, int proc2, int proc3)
{
    fprintf(file, "%s \t proc \t %d \t %d \t %d\n", label, proc1, proc2, proc3);
}

void emitLabel(char *label)
{
    fprintf(file, "%s \t nop\n", label);
}

void emitComment(char *label)
{
	fprintf(file, "/*%s */\n", label);
}


int typeSize(TypeSpecifier type)
{
    return 1;
}


SymTab* makeSymtab(char *name)
{
	SymTab* ptr;

	ptr = (SymTab*)malloc(sizeof(struct symtab));

	ptr->name=name;
	ptr->num=0;
	ptr->base=symtabBase++;

	return ptr;
}

void initSymbolTable()
{
	stackTop = 0;
	symtabBase = 1; // ?

	tblStack[stackTop] = makeSymtab(NULL);
	tblStack[stackTop]->parent = NULL;
	offsetStack[stackTop] = 1;
}

int lookup(char *id, int global)
{
	int i, symID = -1;

	for(i = 0; i < tblStack[stackTop]->num; i++)
	{
		if(strcmp(tblStack[stackTop]->st[i].id, id) == 0)
		{
			symID = i;
			rtSymbol = tblStack[stackTop]->st[i];
			break;
		}
	}

	if(global && stackTop > 0)
	{
		for(i = 0; i < tblStack[stackTop-1]->num; i++)
		{
			if(strcmp(tblStack[stackTop-1]->st[i].id, id) == 0)
			{
				symID = i;
				rtSymbol = tblStack[stackTop-1]->st[i];
				break;
			}
		}
	}
	
	return symID;
}

int insert(char *id, TypeSpecifier typeSpecifier, TypeQuailfier typeQualifier, int base, int offset, int width, int initialValue)
{
	SymTab* symtab;
	int symID;
	
	symtab = tblStack[stackTop];
	if((symID = lookup(id,0)) != -1)
	{
		return symID;
	}
	symtab->st[symtab->num].id = id;
	symtab->st[symtab->num].typeSpecifier = typeSpecifier;
	symtab->st[symtab->num].typeQualifier = typeQualifier;
	symtab->st[symtab->num].offset = offset;
	symtab->st[symtab->num].width = width;
	symtab->st[symtab->num].initialValue = initialValue;
	symtab->st[symtab->num].link = NULL;
	symtab->st[symtab->num].base = symtab->base;
	symtab->num++;

	return symtab->num-1;
	return 0;
}

int insertFuncName(char *id, int returnType, int noArguments)
{
	SymTab* tbl;

	stackTop++;

	tblStack[stackTop] = makeSymtab(id);
	tblStack[stackTop]->parent = tblStack[stackTop-1];
	offsetStack[stackTop] = 1;

	tbl=tblStack[stackTop-1];
	tbl->st[tbl->num].id = id;
	tbl->st[tbl->num].typeSpecifier = returnType;
	tbl->st[tbl->num].typeQualifier = FUNC_TYPE;
	tbl->st[tbl->num].offset = 0;
	tbl->st[tbl->num].width = noArguments;
	tbl->st[tbl->num].initialValue = 0;
	tbl->st[tbl->num].link = tblStack[stackTop];
	tbl->st[tbl->num].base = tbl->base;

	tbl->num++;
	stackTop--;

	return tbl->num - 1;
}

void genLabel(char *label)
{
	sprintf(label, "$$%d", labelNum);
	labelNum++;
}

void rv_emit(Node *ptr)
{
	int stIndex;

	if(ptr->token.tokenNumber == NUMBER)			
	{
		emit1("ldc", atoi(ptr->token.tokenValue));
	}
	else											
	{
		stIndex = lookup(ptr->token.tokenValue, 1);
		if(stIndex == -1)
		{
			return;
		}
		if(rtSymbol.typeQualifier == CONST_TYPE)	
		{
			emit1("ldc", rtSymbol.initialValue);
		}
		else if(rtSymbol.width > 1)				
		{
			emit2("lda", rtSymbol.base, rtSymbol.offset);
		}
		else									
		{
			emit2("lod", rtSymbol.base, rtSymbol.offset);
		}
	}
}

void processSimpleVariable(Node *ptr, TypeSpecifier typeSpecifier, TypeQuailfier typeQualifier)
{
    int stIndex;
    int sign = 1;

    Node *p = ptr->son;
    Node *q = ptr->next;

    if (ptr->token.tokenNumber != SIMPLE_VAR) {
        fprintf(stderr, "error in SIMPLE_VAR\n");
    }

    if (typeQualifier == CONST_TYPE) {
    	int initialValue;
        if (q == NULL) {
            fprintf(stderr, "%s must have a constant value\n", ptr->token.tokenValue);
            return;
        }
        if (q->token.tokenNumber == UNARY_MINUS) {
            sign = -1;
            q = q->son;
        }

        initialValue = sign * atoi(q->token.tokenValue);
      	stIndex = insert(p->token.tokenValue, typeSpecifier, typeQualifier, 0, 0, 0, initialValue);

    } else { // variable type
    	int size, base;
		size = typeSize(typeSpecifier);

		stIndex = insert(p->token.tokenValue, typeSpecifier, typeQualifier,
							base, offsetStack[stackTop], width, 0);
		offsetStack[stackTop] += size;
    }
}

void processArrayVariable(Node *ptr, int typeSpecifier, int typeQualifier)
{
	Node *p = ptr->son;			
	int stIndex, size, base;

	if(ptr->token.tokenNumber != ARRAY_VAR)
	{
		fprintf(file, "error in ARRAY_VAR\n");
		return;
	}

	if(p->next == NULL)	
	{
		fprintf(file, "array size must be specified\n");
	}
	else
	{
		size = atoi(p->next->token.tokenValue);
	}

	size *= typeSize(typeSpecifier);

	stIndex = insert(p->token.tokenValue, typeSpecifier, typeQualifier,
						base, offsetStack[stackTop], size, 0);
	offsetStack[stackTop] += size;
}

void processDeclaration(Node *ptr)
{
    TypeSpecifier typeSpecifier;
    TypeQuailfier typeQualifier;
    Node *p, *q;

    if (ptr->token.tokenNumber != DCL_SPEC) 
    {
        icg_error(4);
    }

    // step 1: process DCL_SPEC
    typeSpecifier = INT_TYPE;        // default type
    typeQualifier = VAR_TYPE;
    
    p = ptr->son;
    while (p) {
        if (p->token.tokenNumber == INT_NODE) {
            typeSpecifier = INT_TYPE;
        } else if (p->token.tokenNumber == CONST_NODE) {
            typeQualifier = CONST_TYPE;
        } else { // AUTO, EXTERN, REGISTER, FLOAT, DOUBLE, SIGNED, UNSIGNED
            printf("not yet implemented\n");
            return;
        }
        p = p->next;
    }

    // step 2: process DCL_ITEM
    p = ptr->next;
    if (p->token.tokenNumber != DCL_ITEM) {
        icg_error(5);
    }

    while (p) {
        q = p->son;
        switch (q->token.tokenNumber) {
            case SIMPLE_VAR:            // simple variable
                processSimpleVariable(q, typeSpecifier, typeQualifier);
                break;

            case ARRAY_VAR:                // array variable
                processArrayVariable(q, typeSpecifier, typeQualifier);
                break;

            default:
                printf("error in SIMPLE_VAR or ARRAY_VAR\n");
                break;
        }
        p = p->next;
    }
}

void processFuncHeader(Node *ptr)
{
	TypeSpecifier returnType;
    int noArguments;
    int stIndex;
    Node *p;

    if (ptr->token.tokenNumber != FUNC_HEAD) {
        printf("error in processFuncHeader\n");
    }

    // step 1: process the function return type
    p = ptr->son->son;
    while(p)
    {
        if (p->token.tokenNumber == INT_NODE) {
            returnType = INT_TYPE;
        } else if (p->token.tokenNumber == VOID_NODE) {
            returnType = VOID_TYPE;
        } else {
            printf("invalid function return type\n");
        }
        p = p->next;
    }

    // // step 2: count the number of formal parameters
    p = ptr->son->next->next; // FORMAL_PARA
    p = p->son; // PARAM_DCL

    noArguments = 0;
    while(p) {
        noArguments++;
        p = p->next;
    }

    // step 3: insert the function name
    // stIndex = insert(ptr->son->next->token.tokenValue, returnType, FUNC_TYPE, 1, 0, noArguments, 0);
	stIndex = insertFuncName(ptr->son->next->token.tokenValue, returnType, noArguments);
}



void processOperator(Node *ptr)
{
	int stIndex;

	switch(ptr->token.tokenNumber)
	{
		case ASSIGN_OP:
		{
			Node *lhs = ptr->son, *rhs = ptr->son->next;

			if(lhs->noderep == NONTERM)
			{
				lvalue = 1;
				processOperator(lhs);
				lvalue = 0;
			}

			if(rhs->noderep == NONTERM)
			{
				processOperator(rhs);
			}
			else
			{
				rv_emit(rhs);
			}

			if(lhs->noderep == TERMINAL)	
			{
				stIndex = lookup(lhs->token.tokenValue, 1);
				if(stIndex == -1)
				{
					fprintf(stderr, "undefined variable: %s\n", lhs->token.tokenValue);
					return;
				}
				emit2("str", rtSymbol.base, rtSymbol.offset);
			}
			else							
			{
				emit0("sti");
			}
			break;
		}

		case ADD_ASSIGN: case SUB_ASSIGN: case MUL_ASSIGN:
		case DIV_ASSIGN: case MOD_ASSIGN:
		{
			Node *lhs = ptr->son, *rhs = ptr->son->next;
			int nodeNumber = ptr->token.tokenNumber;

			ptr->token.tokenNumber = ASSIGN_OP;

			if(lhs->noderep == NONTERM)
			{
				lvalue = 1;
				processOperator(lhs);
				lvalue = 0;
			}

			ptr->token.tokenNumber = nodeNumber;

			if(lhs->noderep == NONTERM)
			{
				processOperator(lhs);
			}
			else
			{
				rv_emit(lhs);
			}

			if(rhs->noderep == NONTERM)
			{
				processOperator(rhs);
			}
			else
			{
				rv_emit(rhs);
			}

			switch(ptr->token.tokenNumber)
			{
				case ADD_ASSIGN:	emit0("add");	break;
				case SUB_ASSIGN:	emit0("sub");	break;
				case MUL_ASSIGN:	emit0("mult");	break;
				case DIV_ASSIGN:	emit0("div");	break;
				case MOD_ASSIGN:	emit0("mod");	break;
			}

			if(lhs->noderep == TERMINAL)
			{
				stIndex = lookup(lhs->token.tokenValue, 1);
				if(stIndex == -1)
				{
					fprintf(file, "undefined variable: %s\n", lhs->son->token.tokenValue);
					return;
				}
				emit2("str", rtSymbol.base, rtSymbol.offset);
			
			}
			else
			{
				emit0("sti");
			}
			break;
		}

		case ADD:	case SUB:	case MUL:	case DIV:	case MOD:
		case EQ:	case NE:	case GT:	case LT:	case GE:	case LE:
		case LOGICAL_AND:	case LOGICAL_OR:
		{
			Node *lhs = ptr->son, *rhs = ptr->son->next;

			if(lhs->noderep == NONTERM) 
			{
				processOperator(lhs);
			}
			else
			{
				rv_emit(lhs);
			}

			if(rhs->noderep == NONTERM) 
			{
				processOperator(rhs);
			}
			else
			{
				rv_emit(rhs);
			}

			switch(ptr->token.tokenNumber)
			{
				case ADD:			emit0("add");		break;		
				case SUB:			emit0("sub");		break;
				case MUL:			emit0("mult");		break;
				case DIV:			emit0("div");		break;
				case MOD:			emit0("mod");		break;
				case EQ:			emit0("eq");		break;		
				case NE:			emit0("ne");		break;
				case GT:			emit0("gt");		break;
				case LT:			emit0("lt");		break;
				case GE:			emit0("ge");		break;
				case LE:			emit0("le");		break;
				case LOGICAL_AND:	emit0("and");		break;		
				case LOGICAL_OR:	emit0("or");		break;
			}
		}
		break;

		case UNARY_MINUS:	case LOGICAL_NOT:
		{
			Node *p = ptr->son;

			if(p->noderep == NONTERM)
			{
				processOperator(p);
			}
			else
			{
				rv_emit(p);
			}

			switch(ptr->token.tokenNumber)
			{
				case UNARY_MINUS:	emit0("neg");		break;
				case LOGICAL_NOT:	emit0("not");		break;
			}
			break;
		}

		case INDEX:
		{
			Node *indexExp = ptr->son->next;

			if(indexExp->noderep == NONTERM) processOperator(indexExp);
			else rv_emit(indexExp);

			stIndex = lookup(ptr->son->token.tokenValue, 1);
			if(stIndex == -1)
			{
				fprintf(file, "undefined variable: %s\n", ptr->son->token.tokenValue);
				return;
			}
			emit2("lda", rtSymbol.base, rtSymbol.offset);
			emit0("add");
			if(!lvalue)			
			{
				emit0("ldi");
			}
			break;
		}

		case PRE_INC:	case PRE_DEC:	case POST_INC:	case POST_DEC:
		{
			Node *p = ptr->son;
			Node *q;
			int stIndex;
			int amount = 1;

			if(p->noderep == NONTERM)
			{
				processOperator(p);
			}
			else
			{
				rv_emit(p);
			}

			q = p;
			while(q->noderep != TERMINAL) 
			{
				q = q->son;
			}

			if(!q || (q->token.tokenNumber != IDENT))
			{
				fprintf(file, "increment/decrement operators can not be applied in expression\n");
				return;
			}
			stIndex = lookup(q->token.tokenValue, 1);
			if(stIndex == -1)
			{
				return;
			}

			switch(ptr->token.tokenNumber)
			{
				case PRE_INC:	emit0("inc");	break;
				case PRE_DEC:	emit0("dec");	break;
				case POST_INC:	emit0("inc");	break;
				case POST_DEC:	emit0("dec");	break;
			}

			if(p->noderep == TERMINAL)
			{
				stIndex = lookup(p->token.tokenValue, 1);
				if(stIndex == -1) 
				{
					return;
				}

				emit2("str", rtSymbol.base, rtSymbol.offset);
			}
			else if(p->token.tokenNumber == INDEX)
			{
				lvalue = 1;
				processOperator(p);
				lvalue = 0;
				emit0("swp");
				emit0("sti");
			}
			else fprintf(file, "error in increment/decrement operators\n");
			break;
		}

		case CALL:
		{
			Node *p = ptr->son;		
			char *functionName;
			int stIndex;
			int noArguments;

			if(checkPredefined(p))
			{
				break;
			}

			functionName = p->token.tokenValue;

			stIndex = lookup(functionName, 1);
			
			if(stIndex == -1)			
			{
				fprintf(file, "%s: undefined function\n", functionName);
				break; 
			}
			noArguments = rtSymbol.width;
			
			emit0("ldp");
			p = p->next->son; 
			while(p)			
			{
				if(p->noderep == NONTERM)
				{
					processOperator(p);
				}
				else
				{
					rv_emit(p);
				}
				noArguments--;
				p = p->next;
			}

			if(noArguments > 0)
			{
				fprintf(file, "%s: too few actual arguments\n", functionName);
			}

			if(noArguments < 0)
			{
				fprintf(file, "%s: too many actual arguments\n", functionName);
			}

			emitJump("call", ptr->son->token.tokenValue);
			break;
		}
	}
}


int checkPredefined(Node *ptr)
{
	Node *p=ptr;
	char *functionName;
	int noArguments;
	int stIndex;

	functionName = p->token.tokenValue;

	if(strcmp(functionName, "read") == 0)
	{
		noArguments = 1;

		emit0("ldp");
		p = p->next->son; 
		while(p)
		{
			if(p->noderep == NONTERM)
			{
				processOperator(p);
			}
			else {
				stIndex = lookup(p->token.tokenValue, 1);
				if(stIndex == -1)
				{
					return 0;
				}
				emit2("lda", rtSymbol.base, rtSymbol.offset);
			}
			noArguments--;
			p = p->next;
		}

		if(noArguments > 0)
		{
			fprintf(file, "%s: too few actual arguments\n", functionName);
		}

		if(noArguments < 0)
		{
			fprintf(file, "%s: too many actual arguments\n", functionName);
		}

		emitJump("call", functionName);
		return 1;
	}
	else if(strcmp(functionName, "write") == 0)
	{
		noArguments = 1;

		emit0("ldp");
		p = p->next->son; 
		while(p)
		{
			if(p->noderep == NONTERM) {
				processOperator(p);
			}
			else {
				stIndex = lookup(p->token.tokenValue, 1);
				if(stIndex == -1) return 0;
				emit2("lod", rtSymbol.base, rtSymbol.offset);
			}
			noArguments--;
			p=p->next;
		}

		if(noArguments > 0)
		{
			fprintf(file, "%s: too few actual arguments\n", functionName);
		}

		if(noArguments < 0)
		{
			fprintf(file, "%s: too many actual arguments\n", functionName);
		}

		emitJump("call", functionName);
		return 1;
	}
	else if(strcmp(functionName, "lf") == 0)
	{
		emitJump("call", functionName);
		return 1;
	}

	return 0;
}


void processStatement(Node *ptr)
{
	Node *p;

	switch(ptr->token.tokenNumber)
	{
		case COMPOUND_ST:
			p = ptr->son->next;
			p = p->son;
			while(p)
			{
				processStatement(p);
				p = p->next;
			}
			break;

		case EXP_ST:
			if(ptr->son != NULL)
			{
				processOperator(ptr->son);
			}
			break;

		case RETURN_ST:
			if(ptr->son != NULL)
			{
				returnWithValue = 1;
				p = ptr->son;
				if(p->noderep == NONTERM)
				{
					processOperator(p);
				}
				else
				{
					rv_emit(p);
				}
				emit0("retv");
			}
			else
				emit0("ret");
			flag_returned=1;
			break;

		case IF_ST:
		{
			char label[LABEL_SIZE]={0};

			genLabel(label);
			processCondition(ptr->son);
			emitJump("fjp", label);
			processStatement(ptr->son->next);
			emitLabel(label);
		}
		break;

		case IF_ELSE_ST:
		{
			char label1[LABEL_SIZE]={0}, label2[LABEL_SIZE]={0};

			genLabel(label1);
			genLabel(label2);
			processCondition(ptr->son);
			emitJump("fjp", label1);
			processStatement(ptr->son->next);
			emitJump("ujp", label2);
			emitLabel(label1);
			processStatement(ptr->son->next->next);
			emitLabel(label2);
		}
		break;

		case WHILE_ST:
		{
			char label1[LABEL_SIZE]={0}, label2[LABEL_SIZE]={0};

			genLabel(label1);
			genLabel(label2);
			emitLabel(label1);
			processCondition(ptr->son);
			emitJump("fjp", label2);

			processStatement(ptr->son->next);

			emitJump("ujp", label1);
			emitLabel(label2);
		}
		break;

		default:
			fprintf(file, "not yet implemented.\n");
			break;
	}
}

void processCondition(Node *ptr)
{
	if(ptr->noderep == NONTERM) {
		processOperator(ptr);
	} else {
		rv_emit(ptr);
	}
}




void processFunction(Node *ptr)
{
	int stIndex, i;
	char *functionName;
	Node *p;

	functionName = ptr->son->son->next->token.tokenValue;
	flag_returned = 0;

	stIndex = lookup(functionName, 1);
	stackTop++;
	tblStack[stackTop] = tblStack[stackTop-1]->st[stIndex].link;
	offsetStack[stackTop] = 1;
	for(p = ptr->son->next->son->son; p; p = p->next)
	{
		if(p->token.tokenNumber == DCL) {
			processDeclaration(p->son);
		}
		else
		{
			icg_error(3);
		}
	}

	emitProc(functionName, offsetStack[stackTop]-1, tblStack[stackTop]->base, 2);

	for(i = 0; i < tblStack[stackTop]->num; i++)
	{
		emitSym("sym", tblStack[stackTop]->base, 
			tblStack[stackTop]->st[i].offset, tblStack[stackTop]->st[i].width);
	}
	
	for(p = ptr->son; p; p = p->next)
	{
		if(p->token.tokenNumber == COMPOUND_ST)
		{
			processStatement(p);
		}
	}

	if(!flag_returned)
	{
		emit0("ret");
	}
	emit0("end");
	
	stackTop--;
}




void display()
{
	int i, j;
	for (i = 0; i <= 1; i++) {
		//	symtab->st[symtab->num].id = id;
		printf("Stack Top [%d]\n", i);
		printf("  Name             Spec        Qual  Of  Wd  Ba  Init\n");
		printf("  ---------------------------------------------------\n");
		for (j = 0; j < tblStack[i]->num; j++) {
			printf("  %-*.*s", 10, 10, tblStack[i]->st[j].id);
			if (tblStack[i]->st[j].typeSpecifier == VOID_TYPE) {
				printf("  void_type");
			} else if (tblStack[i]->st[j].typeSpecifier == INT_TYPE) {
				printf("   int_type");
			} else {
				printf("    unknown");
			}
			if (tblStack[i]->st[j].typeQualifier == FUNC_TYPE) {
				printf("   func_type");
			} else if (tblStack[i]->st[j].typeQualifier == PARAM_TYPE) {
				printf("  param_type");
			} else if (tblStack[i]->st[j].typeQualifier == CONST_TYPE) {
				printf("  const_type");
			} else if (tblStack[i]->st[j].typeQualifier == VAR_TYPE) {
				printf("    var_type");
			} else {
				printf("     unknown");
			}

			printf(" %3d", tblStack[i]->st[j].offset);
			printf(" %3d", tblStack[i]->st[j].width);
			printf(" %3d", tblStack[i]->st[j].base);
			printf(" %5d", tblStack[i]->st[j].initialValue);
			printf("\n");
		}
		printf("  ---------------------------------------------------\n");
	}
}


void codeGen(Node *root, FILE *ucoFile)
{
    Node *p;            // pointer for Node
    int globalSize;        // the size of global variables

	file = ucoFile; // 
    
    initSymbolTable();

    // step 1: process the declaration part
    for (p = root->son; p; p = p->next) {
        if (p->token.tokenNumber == DCL) {
            processDeclaration(p->son);
        } else if (p->token.tokenNumber == FUNC_DEF) {
            processFuncHeader(p->son);
        } else {
            icg_error(3);
        }
    }

    globalSize = offsetStack[stackTop]-1;

    // step 2: process the function part
    for (p = root->son; p; p = p->next) {
        if (p->token.tokenNumber == FUNC_DEF) {
            processFunction(p);
        }
    }

    display();

    // step 3: generate codes for starting routine
    //                bgn        globalSize
    //                ldp
    //                call    main
    //                end
    emit1("bgn", globalSize);
    emit0("ldp");
    emitJump("call", "main");
    emit0("end");
}

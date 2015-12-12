#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ucode.h"
#include "minic_ast.h"

void processOperator(SymbolTable *table, Node *ptr);

FILE* file;

int lvalue;
int flag_returned;

SymbolTable* rootTable;

void dd(Node* ptr)
{
    printTree(ptr, 1, stdout);
}

void display(SymbolTable* table, int indent)
{
    int i, j;

    if (table->count == 0) {
        printf("No Data");
        return;
    }
    for (j = indent * 4; j; j--) printf(" ");
    printf("---------------------------------------------------\n");
    for (j = indent * 4; j; j--) printf(" ");
    printf("Name             Spec        Qual  Of  Wd  Ba  Init\n");
    for (j = indent * 4; j; j--) printf(" ");
    printf("---------------------------------------------------\n");
    for (i = 0; i < table->count; i++) {
        for (j = indent * 4; j; j--) printf(" ");
        printf("%-*.*s", 10, 10, table->rows[i].id);
        if (table->rows[i].spec == SPEC_VOID) {
            printf("  void_type");
        } else if (table->rows[i].spec == SPEC_INT) {
            printf("   int_type");
        } else {
            printf("    unknown");
        }
        if (table->rows[i].qual == QUAL_FUNC) {
            printf("   func_type");
        } else if (table->rows[i].qual == QUAL_PARA) {
            printf("  param_type");
        } else if (table->rows[i].qual == QUAL_CONST) {
            printf("  const_type");
        } else if (table->rows[i].qual == QUAL_VAR) {
            printf("    var_type");
        } else {
            printf("     unknown");
        }

        printf(" %3d", table->rows[i].offset);
        printf(" %3d", table->rows[i].width);
        printf(" %3d", table->rows[i].base);
        printf(" %5d", table->rows[i].init);
        printf("\n");
        if (table->rows[i].qual == QUAL_FUNC) {
            display(table->rows[i].table, indent + 1);
        }
    }
    for (j = indent * 4; j; j--) printf(" ");
    printf("---------------------------------------------------\n");
}

void icg_error(int i)
{
    fprintf(stderr, "ERROR: %d\n", i);
}

void emit0(char *opcode)
{
    fprintf(file, "           %s\n", opcode);
}

void emit1(char *opcode, int operand)
{
    fprintf(file, "           %-7s %d\n", opcode, operand);
}

void emit2(char *opcode, int operand1, int operand2)
{
    fprintf(file, "           %-7s %-7d %d\n", opcode, operand1, operand2);
}

void emitJump(char *opcode, char *operand)
{
    fprintf(file, "           %-7s %s\n", opcode, operand);
}

void emitSym(char *opcode, int operand1, int operand2, int operand3)
{
    fprintf(file, "           %-7s %-7d %-7d %d\n", opcode, operand1, operand2, operand3);
}

void emitProc(char *label, int proc1, int proc2, int proc3)
{
    fprintf(file, "%-10s %-7s %-7d %-7d %d\n", label, "proc", proc1, proc2, proc3);
}

void emitLabel(char *label)
{
    fprintf(file, "%-10s nop\n", label);
}

int typeSize(Specifier type)
{
    return 1; // always 1. (because of int.)
}

void genLabel(char *label)
{
    static int labelNum = 0;
    sprintf(label, "$$%d", labelNum++);
}


SymbolTable* createSymbolTable(char *name)
{
    static int symbolTableBase = 1;

    SymbolTable* table;

    table = (SymbolTable*)malloc(sizeof(struct _SymbolTable));

    table->name=name;
    table->count = 0;
    table->offset = 1;
    table->base = symbolTableBase++;

    return table;
}

SymbolTable* initSymbolTable()
{
    return createSymbolTable(NULL);
}

int lookup(SymbolTable* table, char *id)
{
    int i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->rows[i].id, id) == 0) {
            return i;
        }
    }

    return -1;
}

int insert(SymbolTable *table, char *id, Specifier spec, Qualifier qual, int offset, int width, int init)
{
    int symID;
    SymbolTable* children = NULL;

    if (qual == QUAL_FUNC) {
        children = createSymbolTable(id);
    } else {
        if ((symID = lookup(table, id)) != -1) {
            return symID;
        }
    }
    int top = table->count;

    table->rows[top].id = id;
    table->rows[top].spec = spec;
    table->rows[top].qual = qual;
    table->rows[top].offset = offset;
    table->rows[top].width = width;
    table->rows[top].init = init;
    table->rows[top].table = children;
    table->rows[top].base = table->base;
    table->count++;

    return top;
}


void rv_emit(SymbolTable *table, Node *ptr)
{
    int stIndex;

    if (ptr->token.tokenNumber == NUMBER) {
        emit1("ldc", atoi(ptr->token.tokenValue));
    } else {
        SymbolRow* foundRow;
        stIndex = lookup(table, ptr->token.tokenValue);
        if (stIndex != -1) {
            foundRow = &table->rows[stIndex];
        }

        if (stIndex == -1) {
            stIndex = lookup(rootTable, ptr->token.tokenValue);
            if (stIndex != -1) {
                foundRow = &(rootTable->rows[stIndex]);
            } else {
                return;
            }
        }

        if (foundRow->qual == QUAL_CONST) {
            emit1("ldc", foundRow->init);
        } else if (foundRow->width > 1) {
            emit2("lda", foundRow->base, foundRow->offset);
        } else {
            emit2("lod", foundRow->base, foundRow->offset);
        }
    }
}

void processSimpleVariable(SymbolTable *table, Node *ptr, Specifier spec, Qualifier qual)
{
    int sign = 1;

    Node *p = ptr->son;
    Node *q = ptr->next;

    if (ptr->token.tokenNumber != SIMPLE_VAR) {
        fprintf(stderr, "error in SIMPLE_VAR\n");
    }

    if (qual == QUAL_CONST) {
        int init;
        if (q == NULL) {
            fprintf(stderr, "%s must have a constant value\n", ptr->token.tokenValue);
            return;
        }
        if (q->token.tokenNumber == UNARY_MINUS) {
            sign = -1;
            q = q->son;
        }
        init = sign * atoi(q->token.tokenValue);
        insert(table, p->token.tokenValue, spec, qual, 0, 0, init);

    } else { // variable type
        int size;
        size = typeSize(spec);
        insert(table, p->token.tokenValue, spec, qual, table->offset, 1, 0);
        table->offset += size;
    }
}

void processArrayVariable(SymbolTable *table, Node *ptr, int spec, int qual)
{
    Node *p = ptr->son;
    int size;

    if (ptr->token.tokenNumber != ARRAY_VAR) {
        fprintf(file, "error in ARRAY_VAR\n");
        return;
    }

    if (p->next == NULL) {
        fprintf(file, "array size must be specified\n");
    } else {
        size = atoi(p->next->token.tokenValue);
    }

    size *= typeSize(spec);

    insert(table, p->token.tokenValue, spec, qual, table->offset, size, 0);
    table->offset += size;
}

void processDeclaration(SymbolTable *table, Node *ptr)
{
    Specifier spec;
    Qualifier qual;
    Node *p, *q;

    if (ptr->token.tokenNumber != DCL_SPEC) {
        icg_error(4);
    }

    // step 1: process DCL_SPEC
    spec = SPEC_INT;
    qual = QUAL_VAR;

    p = ptr->son;
    while (p) {
        if (p->token.tokenNumber == INT_NODE) {
            spec = SPEC_INT;
        } else if (p->token.tokenNumber == CONST_NODE) {
            qual = QUAL_CONST;
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
                processSimpleVariable(table, q, spec, qual);
                break;

            case ARRAY_VAR:                // array variable
                processArrayVariable(table, q, spec, qual);
                break;

            default:
                printf("error in SIMPLE_VAR or ARRAY_VAR\n");
                break;
        }
        p = p->next;
    }
}

void processFuncHeader(SymbolTable *table, Node *ptr)
{
    Specifier returnType;
    int noArguments;
    Node *p;

    if (ptr->token.tokenNumber != FUNC_HEAD) {
        printf("error in processFuncHeader\n");
    }

    // step 1: process the function return type
    p = ptr->son->son;
    while (p) {
        if (p->token.tokenNumber == INT_NODE) {
            returnType = SPEC_INT;
        } else if (p->token.tokenNumber == VOID_NODE) {
            returnType = SPEC_VOID;
        } else {
            printf("invalid function return type\n");
        }
        p = p->next;
    }

    // // step 2: count the countber of formal parameters
    p = ptr->son->next->next; // FORMAL_PARA
    p = p->son; // PARAM_DCL

    noArguments = 0;
    while (p) {
        noArguments++;
        p = p->next;
    }

    // step 3: insert the function name
    insert(table, ptr->son->next->token.tokenValue, returnType, QUAL_FUNC, 0, noArguments, 0);
}

int checkPredefined(SymbolTable *table, Node *ptr)
{
    Node *p=ptr;
    char *functionName;
    int noArguments;
    int stIndex;

    functionName = p->token.tokenValue;
    if (strcmp(functionName, "read") == 0) {
        noArguments = 1;

        emit0("ldp");
        p = p->next->son;
        while (p) {
            if (p->noderep == NONTERM) {
                processOperator(table, p);
            } else {
                stIndex = lookup(table, p->token.tokenValue);
                if (stIndex == -1) {
                    return 0;
                }
                emit2("lda", table->rows[stIndex].base, table->rows[stIndex].offset);
            }
            noArguments--;
            p = p->next;
        }

        if (noArguments > 0) {
            fprintf(file, "%s: too few actual arguments\n", functionName);
        }

        if (noArguments < 0) {
            fprintf(file, "%s: too many actual arguments\n", functionName);
        }

        emitJump("call", functionName);
        return 1;
    } else if (strcmp(functionName, "write") == 0) {
        noArguments = 1;

        emit0("ldp");
        p = p->next->son;
        while (p) {
            if (p->noderep == NONTERM) {
                processOperator(table, p);
            } else {
                stIndex = lookup(table, p->token.tokenValue);
                if (stIndex == -1) return 0;
                emit2("lod", table->rows[stIndex].base, table->rows[stIndex].offset);
            }
            noArguments--;
            p=p->next;
        }

        if (noArguments > 0) {
            fprintf(file, "%s: too few actual arguments\n", functionName);
        }

        if (noArguments < 0) {
            fprintf(file, "%s: too many actual arguments\n", functionName);
        }

        emitJump("call", functionName);
        return 1;
    } else if (strcmp(functionName, "lf") == 0) {
        emitJump("call", functionName);
        return 1;
    }

    return 0;
}

void processOperator(SymbolTable *table, Node *ptr)
{
    int stIndex;

    switch(ptr->token.tokenNumber) {
        case ASSIGN_OP: {
            Node *lhs = ptr->son, *rhs = ptr->son->next;

            if (lhs->noderep == NONTERM) {
                lvalue = 1;
                processOperator(table, lhs);
                lvalue = 0;
            }

            if (rhs->noderep == NONTERM) {
                processOperator(table, rhs);
            } else {
                rv_emit(table, rhs);
            }

            if (lhs->noderep == TERMINAL)     {
                stIndex = lookup(table, lhs->token.tokenValue);
                if (stIndex == -1) {
                    fprintf(stderr, "undefined variable: %s\n", lhs->token.tokenValue);
                    return;
                }
                emit2("str", table->rows[stIndex].base, table->rows[stIndex].offset);
            } else                             {
                emit0("sti");
            }
            break;
        }

        case ADD_ASSIGN: case SUB_ASSIGN: case MUL_ASSIGN:
        case DIV_ASSIGN: case MOD_ASSIGN: {
            Node *lhs = ptr->son, *rhs = ptr->son->next;
            int nodeNumber = ptr->token.tokenNumber;

            ptr->token.tokenNumber = ASSIGN_OP;

            if (lhs->noderep == NONTERM) {
                lvalue = 1;
                processOperator(table, lhs);
                lvalue = 0;
            }

            ptr->token.tokenNumber = nodeNumber;

            if (lhs->noderep == NONTERM) {
                processOperator(table, lhs);
            } else {
                rv_emit(table, lhs);
            }

            if (rhs->noderep == NONTERM) {
                processOperator(table, rhs);
            } else {
                rv_emit(table, rhs);
            }

            switch(ptr->token.tokenNumber) {
                case ADD_ASSIGN: emit0("add"); break;
                case SUB_ASSIGN: emit0("sub"); break;
                case MUL_ASSIGN: emit0("mult"); break;
                case DIV_ASSIGN: emit0("div"); break;
                case MOD_ASSIGN: emit0("mod"); break;
            }

            if (lhs->noderep == TERMINAL) {
                stIndex = lookup(table, lhs->token.tokenValue);
                if (stIndex == -1) {
                    fprintf(file, "undefined variable: %s\n", lhs->son->token.tokenValue);
                    return;
                }
                emit2("str", table->rows[stIndex].base, table->rows[stIndex].offset);
            } else {
                emit0("sti");
            }
            break;
        }

        case ADD: case SUB: case MUL:  case DIV:  case MOD:
        case EQ:  case NE:  case GT:   case LT:   case GE:    case LE:
        case LOGICAL_AND:   case LOGICAL_OR: {
            Node *lhs = ptr->son, *rhs = ptr->son->next;

            if (lhs->noderep == NONTERM)  {
                processOperator(table, lhs);
            } else {
                rv_emit(table, lhs);
            }

            if (rhs->noderep == NONTERM)  {
                processOperator(table, rhs);
            } else {
                rv_emit(table, rhs);
            }

            switch(ptr->token.tokenNumber) {
                case ADD:         emit0("add");       break;
                case SUB:         emit0("sub");       break;
                case MUL:         emit0("mult");      break;
                case DIV:         emit0("div");       break;
                case MOD:         emit0("mod");       break;
                case EQ:          emit0("eq");        break;
                case NE:          emit0("ne");        break;
                case GT:          emit0("gt");        break;
                case LT:          emit0("lt");        break;
                case GE:          emit0("ge");        break;
                case LE:          emit0("le");        break;
                case LOGICAL_AND: emit0("and");       break;
                case LOGICAL_OR:  emit0("or");        break;
            }
            break;
        }
        case UNARY_MINUS:    case LOGICAL_NOT: {
            Node *p = ptr->son;

            if (p->noderep == NONTERM) {
                processOperator(table, p);
            } else {
                rv_emit(table, p);
            }

            switch(ptr->token.tokenNumber) {
                case UNARY_MINUS:    emit0("neg");        break;
                case LOGICAL_NOT:    emit0("not");        break;
            }
            break;
        }
        case INDEX: {
            Node *indexExp = ptr->son->next;

            if (indexExp->noderep == NONTERM) processOperator(table, indexExp); else rv_emit(table, indexExp);

            stIndex = lookup(table, ptr->son->token.tokenValue);
            if (stIndex == -1) {
                fprintf(file, "undefined variable: %s\n", ptr->son->token.tokenValue);
                return;
            }
            emit2("lda", table->rows[stIndex].base, table->rows[stIndex].offset);
            emit0("add");
            if (!lvalue)             {
                emit0("ldi");
            }
            break;
        }
        case PRE_INC:    case PRE_DEC:    case POST_INC:    case POST_DEC: {
            Node *p = ptr->son;
            Node *q;
            int stIndex;
            int amount = 1;

            if (p->noderep == NONTERM) {
                processOperator(table, p);
            } else {
                rv_emit(table, p);
            }

            q = p;
            while (q->noderep != TERMINAL)  {
                q = q->son;
            }

            if (!q || (q->token.tokenNumber != IDENT)) {
                fprintf(file, "increment/decrement operators can not be applied in expression\n");
                return;
            }
            stIndex = lookup(table, q->token.tokenValue);
            if (stIndex == -1) {
                return;
            }

            switch(ptr->token.tokenNumber) {
                case PRE_INC:    emit0("inc");    break;
                case PRE_DEC:    emit0("dec");    break;
                case POST_INC:   emit0("inc");    break;
                case POST_DEC:   emit0("dec");    break;
            }

            if (p->noderep == TERMINAL) {
                stIndex = lookup(table, p->token.tokenValue);
                if (stIndex == -1)  {
                    return;
                }

                emit2("str", table->rows[stIndex].base, table->rows[stIndex].offset);
            } else if (p->token.tokenNumber == INDEX) {
                lvalue = 1;
                processOperator(table, p);
                lvalue = 0;
                emit0("swp");
                emit0("sti");
            } else fprintf(file, "error in increment/decrement operators\n");
            break;
        }
        case CALL: {
            Node *p = ptr->son;
            char *functionName;
            int stIndex;
            int noArguments;

            if (checkPredefined(table, p)) {
                break;
            }

            functionName = p->token.tokenValue;

            stIndex = lookup(rootTable, functionName);
      
            if (stIndex == -1) {
                fprintf(file, "%s: undefined function\n", functionName);
                break;
            }
            noArguments = rootTable->rows[stIndex].width;
      
            emit0("ldp");
            p = p->next->son;
            while (p)             {
                if (p->noderep == NONTERM) {
                    processOperator(table, p);
                } else {
                    rv_emit(table, p);
                }
                noArguments--;
                p = p->next;
            }

            if (noArguments > 0) {
                fprintf(file, "%s: too few actual arguments\n", functionName);
            }

            if (noArguments < 0) {
                fprintf(file, "%s: too many actual arguments\n", functionName);
            }

            emitJump("call", ptr->son->token.tokenValue);
            break;
        }
    }
}

void processCondition(SymbolTable *table, Node *ptr)
{
    if (ptr->noderep == NONTERM) {
        processOperator(table, ptr);
    } else {
        rv_emit(table, ptr);
    }
}

void processStatement(SymbolTable *table, Node *ptr)
{
    Node *p;
    char label1[LABEL_SIZE]={0}, label2[LABEL_SIZE]={0};

    switch(ptr->token.tokenNumber) {
        case COMPOUND_ST:
            p = ptr->son->next;
            p = p->son;
            while (p) {
                processStatement(table, p);
                p = p->next;
            }
            break;

        case EXP_ST:
            if (ptr->son != NULL) {
                processOperator(table, ptr->son);
            }
            break;

        case RETURN_ST:
            if (ptr->son != NULL) {
                p = ptr->son;
                if (p->noderep == NONTERM) {
                    processOperator(table, p);
                } else {
                    rv_emit(table, p);
                }
                emit0("retv");
            } else
                emit0("ret");
            flag_returned=1;
            break;

        case IF_ST:
            genLabel(label1);
            processCondition(table, ptr->son);
            emitJump("fjp", label1);
            processStatement(table, ptr->son->next);
            emitLabel(label1);
        	break;

        case IF_ELSE_ST:
            genLabel(label1);
            genLabel(label2);
            processCondition(table, ptr->son);
            emitJump("fjp", label1);
            processStatement(table, ptr->son->next);
            emitJump("ujp", label2);
            emitLabel(label1);
            processStatement(table, ptr->son->next->next);
            emitLabel(label2);
        	break;

        case WHILE_ST:
            genLabel(label1);
            genLabel(label2);
            emitLabel(label1);
            processCondition(table, ptr->son);
            emitJump("fjp", label2);

            processStatement(table, ptr->son->next);

            emitJump("ujp", label1);
            emitLabel(label2);
	        break;

        default:
            fprintf(file, "not yet implemented.\n");
            break;
    }
}

void processFunction(SymbolTable *table, Node *ptr)
{
    int stIndex, i;
    char *functionName;
    Node *p;
    int width = 0;

    functionName = ptr->son->son->next->token.tokenValue;
    flag_returned = 0;

    stIndex = lookup(table, functionName);

    SymbolTable *nextTable = table->rows[stIndex].table;

    // parameters
    if (ptr->son->son->next->next->son) {
        for (p = ptr->son->son->next->next->son; p; p = p->next) {
            if (p->token.tokenNumber == PARAM_DCL) {
                processDeclaration(nextTable, p->son); // todo
            } else {
                icg_error(9);
            }
        }
    }
    for (p = ptr->son->next->son->son; p; p = p->next) {
        if (p->token.tokenNumber == DCL) {
            processDeclaration(nextTable, p->son); // todo
        } else {
            icg_error(3);
        }
    }

    emitProc(functionName, nextTable->offset - 1, nextTable->base, 2);

    // 지역변수
    for (i = 0; i < nextTable->count; i++) {
        emitSym("sym", nextTable->base, nextTable->rows[i].offset, nextTable->rows[i].width);
    }

    // Run Statement
    for (p = ptr->son; p; p = p->next) {
        if (p->token.tokenNumber == COMPOUND_ST) {
            processStatement(nextTable, p);
        }
    }

    if (!flag_returned) {
        emit0("ret");
    }
    emit0("end");
}

void codeGen(Node *root, FILE *ucoFile)
{
    Node *p;            // pointer for Node
    int globalSize;        // the size of global variables

    file = ucoFile; //

    rootTable = initSymbolTable();

    // step 1: process the declaration part
    for (p = root->son; p; p = p->next) {
        if (p->token.tokenNumber == DCL) {
            processDeclaration(rootTable, p->son);
        } else if (p->token.tokenNumber == FUNC_DEF) {
            processFuncHeader(rootTable, p->son);
        } else {
            icg_error(3);
        }
    }

    globalSize = rootTable->offset - 1;

    // step 2: process the function part
    for (p = root->son; p; p = p->next) {
        if (p->token.tokenNumber == FUNC_DEF) {
            processFunction(rootTable, p);
        }
    }

    display(rootTable, 0);

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

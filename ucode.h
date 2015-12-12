
#ifndef __UCODE_HEADER
#define __UCODE_HEADER

#include "minic_ast.h"

#define LABEL_SIZE           8
#define SYMTAB_ARRAY_SIZE    512

typedef enum {
    SPEC_NONE, SPEC_VOID, SPEC_INT
} Specifier;

typedef enum {
    QUAL_NONE, QUAL_FUNC, QUAL_PARA, QUAL_CONST, QUAL_VAR
} Qualifier;

typedef struct _SymbolRow {
    char *id;
    Specifier spec;
    Qualifier qual;
    int offset;
    int width;
    int base;
    int init;
    struct _SymbolTable *table;
} SymbolRow;

typedef struct _SymbolTable
{
    char *name;
    int count;
    int offset;
    int base;
    SymbolRow rows[SYMTAB_ARRAY_SIZE];
} SymbolTable;


void codeGen(Node *ptr, FILE *ucoFile);

#endif

%{
    #include <stdio.h>
    #include <string.h>
    #include "minic_ast.h"
    #include "minic.h"
    #include "ucode.h"

    extern FILE *yyin;

    int yylex();
    void yyerror(const char *s);

    Node* root;


%}
%union {
    struct nodeType *ast;
    int ival;
    char* string;
}
%token TCONST TELSE TIF TINT TRETURN TVOID TWHILE TEQUAL TNOTEQU
    TLESSE TGREATE TAND TOR TINC TDEC TADDASSIGN TSUBASSIGN TMULASSIGN
    TDIVASSIGN TMODASSIGN
%token <string> TIDENT TNUMBER
%type <ast> function_def translation_unit external_dcl function_name function_header compound_st
    dcl_spec formal_param opt_dcl_list declaration_list declaration dcl_specifiers dcl_specifier
    type_specifier type_qualifier opt_formal_param formal_param_list param_dcl init_dcl_list init_declarator
    declarator opt_number opt_stat_list statement_list statement expression_st opt_expression if_st
    while_st return_st expression assignment_exp actual_param actual_param_list unary_exp postfix_exp primary_exp
    logical_or_exp logical_and_exp equality_exp relational_exp additive_exp multiplicative_exp opt_actual_param

%nonassoc LOWER_THAN_TELSE
%nonassoc TELSE

%%

mini_c: translation_unit {
        root = buildTree(PROGRAM, $1, NULL);
    }
    ;

translation_unit: external_dcl {
        $$ = $1;
    }
    | translation_unit external_dcl {
        appendNext($1, $2);
        $$ = $1;
    }
    ;

external_dcl: function_def {
        $$ = $1;
    }
    | declaration {
        $$ = $1;
    }
    ;

function_def: function_header compound_st {
        appendNext($1, $2);
        $$ = buildTree(FUNC_DEF, $1, NULL);
    }
    ;

function_header: dcl_spec function_name formal_param {
        appendNext($1, $2);
        appendNext($2, $3);
        $$ = buildTree(FUNC_HEAD, $1, NULL);
    }
    ;

dcl_spec: dcl_specifiers {
        $$ = buildTree(DCL_SPEC, $1, NULL);
    }
    ;

dcl_specifiers: dcl_specifier {
        $$ = $1;
    }
    | dcl_specifiers dcl_specifier {
        appendNext($1, $2);
        $$ = $1;
    }
    ;

dcl_specifier: type_qualifier { $$ = $1; }
    | type_specifier { $$ = $1; }
    ;

type_qualifier: TCONST {
        $$ = buildTree(CONST_NODE, NULL, NULL);
    }
    ;

type_specifier: TINT {
        $$ = buildTree(INT_NODE, NULL, NULL);
    }
    | TVOID {
        $$ = buildTree(VOID_NODE, NULL, NULL);
    }
    ;

function_name: TIDENT { $$ = buildNode(IDENT, $1); }
    ;

formal_param: '(' opt_formal_param ')' {
        $$ = buildTree(FORMAL_PARA, $2, NULL);
    }
    ;

opt_formal_param: formal_param_list { $$ = $1; }
    | { $$ = NULL; }
    ;

formal_param_list: param_dcl {
        $$ = $1;
    }
    | formal_param_list ',' param_dcl {
        appendNext($1, $3);
        $$ = $1;
    }
    ;

param_dcl: dcl_spec declarator {
        appendNext($1, $2);
        $$ = buildTree(PARAM_DCL, $1, NULL);
    }
    ;

compound_st: '{' opt_dcl_list opt_stat_list '}' {
        appendNext($2, $3);
        $$ = buildTree(COMPOUND_ST, $2, NULL);
    }
    ;

opt_dcl_list: declaration_list {
        $$ = buildTree(DCL_LIST, $1, NULL);
    }
    | {
        $$ = buildTree(DCL_LIST, NULL, NULL);
    }
    ;

declaration_list: declaration {
        $$ = $1;
    }
    | declaration_list declaration {
        appendNext($1, $2);
        $$ = $1;
    }
    ;

declaration: dcl_spec init_dcl_list ';' {
        appendNext($1, $2);
        $$ = buildTree(DCL, $1, NULL);
    }
    ;

init_dcl_list: init_declarator {
        $$ = $1;
    }
    | init_dcl_list ',' init_declarator {
        appendNext($1, $3);
        $$ = $1;
    }
    ;

init_declarator: declarator {
        $$ = buildTree(DCL_ITEM, $1, NULL);
    }
    | declarator '=' TNUMBER {
        appendNext($1, buildNode(IDENT, $3)); // @todo 이거 구조 맞나?
        $$ = buildTree(DCL_ITEM, $1, NULL);
    }
    ;

declarator: TIDENT {
        $$ = buildTree(SIMPLE_VAR, buildNode(IDENT, $1), NULL);
    }
    | TIDENT '[' opt_number ']' {
        Node* ptr = buildNode(IDENT, $1);
        appendNext(ptr, $3);
        $$ = buildTree(ARRAY_VAR, ptr, NULL);
    }
    ;

opt_number: TNUMBER {
        $$ = buildNode(NUMBER, $1);
    }
    | {
        $$ = NULL;
    }
    ;

opt_stat_list: statement_list {
        $$ = buildTree(STAT_LIST, $1, NULL);
    }
    | { $$ = NULL; }
    ;

statement_list: statement {
        $$ = $1;
    }
    | statement_list statement {
        appendNext($1, $2);
        $$ = $1;
    }
    ;

statement: compound_st { $$ = $1; }
    | expression_st { $$ = $1; }
    | if_st { $$ = $1; }
    | while_st { $$ = $1; }
    | return_st { $$ = $1; }
    ;

expression_st: opt_expression ';' {
        $$ = buildTree(EXP_ST, $1, NULL);
    }
    ;

opt_expression: expression { $$ = $1; }
    | { $$ = NULL }
    ;

if_st: TIF '(' expression ')' statement %prec LOWER_THAN_TELSE {
        appendNext($3, $5);
        $$ = buildTree(IF_ST, $3, NULL);
    }
    | TIF '(' expression ')' statement TELSE statement {
        appendNext($3, $5);
        appendNext($5, $7);
        $$ = buildTree(IF_ELSE_ST, $3, NULL);
    }
    ;

while_st: TWHILE '(' expression ')' statement {
        appendNext($3, $5);
        $$ = buildTree(WHILE_ST, $3, NULL);
    }
    ;

return_st: TRETURN opt_expression ';' {
        $$ = buildTree(RETURN_ST, $2, NULL);
    }
    ;

expression: assignment_exp { $$ = $1; }
    ;

assignment_exp: logical_or_exp { $$ = $1; }
    | unary_exp '=' assignment_exp {
        appendNext($1, $3);
        $$ = buildTree(ASSIGN_OP, $1, NULL);
    }
    | unary_exp TADDASSIGN assignment_exp {
        appendNext($1, $3);
        $$ = buildTree(ADD_ASSIGN, $1, NULL);
    }
    | unary_exp TSUBASSIGN assignment_exp {
        appendNext($1, $3);
        $$ = buildTree(SUB_ASSIGN, $1, NULL);
    }
    | unary_exp TMULASSIGN assignment_exp {
        appendNext($1, $3);
        $$ = buildTree(MUL_ASSIGN, $1, NULL);
    }
    | unary_exp TDIVASSIGN assignment_exp {
        appendNext($1, $3);
        $$ = buildTree(DIV_ASSIGN, $1, NULL);
    }
    | unary_exp TMODASSIGN assignment_exp {
        appendNext($1, $3);
        $$ = buildTree(MOD_ASSIGN, $1, NULL);
    }
    ;

logical_or_exp: logical_and_exp { $$ = $1; }
    | logical_or_exp '||' logical_and_exp {
        appendNext($1, $3);
        $$ = buildTree(LOGICAL_OR, $1, NULL);
    }
    ;

logical_and_exp: equality_exp { $$ = $1; }
    | logical_and_exp '&&' equality_exp {
        appendNext($1, $3);
        $$ = buildTree(LOGICAL_AND, $1, NULL);
    }
    ;

equality_exp: relational_exp { $$ = $1; }
    | equality_exp TEQUAL relational_exp {
        appendNext($1, $3);
        $$ = buildTree(EQ, $1, NULL);
    }
    | equality_exp TNOTEQU relational_exp {
        appendNext($1, $3);
        $$ = buildTree(NE, $1, NULL);
    }
    ;

relational_exp: additive_exp { $$ = $1; }
    | relational_exp '>' additive_exp {
        appendNext($1, $3);
        $$ = buildTree(GT, $1, NULL);
    }
    | relational_exp '<' additive_exp {
        appendNext($1, $3);
        $$ = buildTree(LT, $1, NULL);
    }
    | relational_exp TGREATE additive_exp {
        appendNext($1, $3);
        $$ = buildTree(GE, $1, NULL);
    }
    | relational_exp TLESSE additive_exp {
        appendNext($1, $3);
        $$ = buildTree(LE, $1, NULL);
    }
    ;

additive_exp: multiplicative_exp { $$ = $1; }
    | additive_exp '+' multiplicative_exp {
        appendNext($1, $3);
        $$ = buildTree(ADD, $1, NULL);
    }
    | additive_exp '-' multiplicative_exp {
        appendNext($1, $3);
        $$ = buildTree(SUB, $1, NULL);
    }
    ;

multiplicative_exp: unary_exp { $$ = $1; }
    | multiplicative_exp '*' unary_exp {
        appendNext($1, $3);
        $$ = buildTree(MUL, $1, NULL);
    }
    | multiplicative_exp '/' unary_exp {
        appendNext($1, $3);
        $$ = buildTree(DIV, $1, NULL);
    }
    | multiplicative_exp '%' unary_exp {
        appendNext($1, $3);
        $$ = buildTree(MOD, $1, NULL);
    }
    ;

unary_exp: postfix_exp { $$ = $1; }
    | '-' unary_exp {
        $$ = buildTree(UNARY_MINUS, $2, NULL);
    }
    | '!' unary_exp {
        $$ = buildTree(LOGICAL_NOT, $2, NULL);
    }
    | TINC unary_exp {
        $$ = buildTree(PRE_INC, $2, NULL);
    }
    | TDEC unary_exp {
        $$ = buildTree(PRE_DEC, $2, NULL);
    }
    ;

postfix_exp: primary_exp { $$ = $1; }
    | postfix_exp '[' expression ']' {
        appendNext($1, $3);
        $$ = buildTree(INDEX, $1, NULL);
    }
    | postfix_exp '(' opt_actual_param ')' {
        appendNext($1, $3);
        $$ = buildTree(CALL, $1, NULL);
    }
    | postfix_exp TINC {
        $$ = buildTree(POST_INC, $1, NULL);
    }
    | postfix_exp TDEC {
        $$ = buildTree(POST_DEC, $1, NULL);
    }
    ;

opt_actual_param: actual_param { $$ = $1; }
    | { $$ = NULL; }
    ;

actual_param: actual_param_list {
        $$ = buildTree(ACTUAL_PARAM, $1, NULL);
    }
    ;

actual_param_list: assignment_exp {
        $$ = $1;
    }
    | actual_param_list ',' assignment_exp {
        appendNext($1, $3);
        $$ = $1;
    }
    ;

primary_exp: TIDENT {
        $$ = buildNode(IDENT, $1);
    }
    | TNUMBER {
        $$ = buildNode(NUMBER, $1);
    }
    | '(' expression ')' { // ex. (30 * 20) + 4
        $$ = $2;
    }
    ;

%%

char* toString(char* string)
{
    char* str;
    str = (char*)malloc(strlen(string) + 1);
    strcpy(str, string);
    return str;
}

Node* parse(FILE *sourceFile)
{
    yyin = sourceFile;
    do{
        yyparse();
    } while(!feof(yyin));

    return root;
}

int main(int argc, char *argv[]){
    FILE *sourceFile;
    FILE *astFile, *ucoFile;

    char filename[100];
    Node *root;

    if (argc != 2) {
        fprintf(stderr, "arguments not valid.");
        return -1;
    }

    strcpy(filename, argv[1]);

    sourceFile = fopen(filename, "r");
    astFile = fopen(strcat(strtok(filename, "."), ".ast"), "w");
    ucoFile = fopen(strcat(strtok(filename, "."), ".uco"), "w");

    if(!sourceFile) {
        fprintf(stderr, "source file not open!\n");
        return -1;
    }
    if(!astFile) {
        fprintf(stderr, "ast file not not open!\n");
        return -1;
    }
    if(!ucoFile) {
        fprintf(stderr, "uco file not not open!\n");
        return -1;
    }

    printf("Start Parsing..\n");
    root = parse(sourceFile);
    printTree(root, 0, astFile);
    printf("End Parsing!\n");

    printf("Start Code Generate..\n");
    codeGen(root, ucoFile);
    printf("End Code Generate!\n");

    fclose(sourceFile);
    fclose(astFile);
    fclose(ucoFile);
    return 1;       
}


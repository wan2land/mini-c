%{
    #include <stdio.h>
    #include <string.h>
    #include "minic_ast.h"
    #include "minic.h"
    int yylex();
    void yyerror(const char *s);

    Node* root;

//    char *nodeName[];
//    Node *valueStack[STACK_SIZE];   
//    int rightLength[NODE_NUM + 1];  
    int sp;
    int blabla;
    char filename[30];
%}
%union {
    struct nodeType *ast;
    int ival;
    char* string;
}
%token TCONST TELSE TIF TINT TRETURN TVOID TWHILE TEQUAL TNOTEQU
    TLESSE TGREATE TAND TOR TINC TDEC TADDASSIGN TSUBASSIGN TMULASSIGN
    TDIVASSIGN TMODASSIGN TNUMBER
%type <ast> function_def translation_unit external_dcl function_name function_header compound_st
    dcl_spec formal_param opt_dcl_list declaration_list declaration dcl_specifiers dcl_specifier
    type_specifier type_qualifier opt_formal_param formal_param_list param_dcl init_dcl_list init_declarator
    declarator
%token <string> TIDENT
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
        // @todo
        // $$ = $1;
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

type_qualifier: TCONST { $$ = buildTree(CONST_NODE, NULL, NULL); }
    ;

type_specifier: TINT { $$ = buildTree(INT_NODE, NULL, NULL); }
    | TVOID { $$ = buildTree(VOID_NODE, NULL, NULL); }
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
        $$ = buildNode(IDENT, "Hello"); // @todo 여기는 function (int x, int y) 이런 예시가 없어서 확인 불가능. 만들어야 함.
    }
    | formal_param_list ',' param_dcl {
        appendNext($1, $3);
        $$ = $1;
    }
    ;

param_dcl: dcl_spec declarator // @todo
    ;

compound_st: '{' opt_dcl_list opt_stat_list '}' {
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
        appendNext($1, $3); // 왜 애러나지?
        $$ = $1;
    }
    ;

init_declarator: declarator {
        $$ = buildTree(DCL_ITEM, $1, NULL);
    }
    | declarator '=' TNUMBER {
        $$ = buildTree(DCL_ITEM, NULL, NULL); // @todo DCLITEM
    }
    ;

declarator: TIDENT {
        $$ = buildTree(SIMPLE_VAR, buildNode(TIDENT, $1), NULL);
    }
    | TIDENT '[' opt_number ']' {
        $$ = buildTree(SIMPLE_VAR, buildNode(TIDENT, $1), NULL);//buildTree(SIMPLE_VAR, $1, NULL);
    }
    ;

opt_number: TNUMBER
    |
    ;

opt_stat_list: statement_list
    |
    ;

statement_list: statement
    | statement_list statement
    ;

statement: compound_st
    | expression_st
    | if_st
    | while_st
    | return_st
    ;

expression_st: opt_expression ';'
    ;

opt_expression: expression
    |
    ;

if_st: TIF '(' expression ')' statement
    | TIF '(' expression ')' statement TELSE statement
    ;

while_st: TWHILE '(' expression ')' statement
    ;

return_st: TRETURN opt_expression ';'
    ;

expression: assignment_exp
    ;

assignment_exp: logical_or_exp
    | unary_exp '=' assignment_exp
    | unary_exp TADDASSIGN assignment_exp
    | unary_exp TSUBASSIGN assignment_exp
    | unary_exp TMULASSIGN assignment_exp
    | unary_exp TDIVASSIGN assignment_exp
    | unary_exp TMODASSIGN assignment_exp
    ;

logical_or_exp: logical_and_exp
    | logical_or_exp '||' logical_and_exp
    ;

logical_and_exp: equality_exp
    | logical_and_exp '&&' equality_exp
    ;

equality_exp: relational_exp
    | equality_exp TEQUAL relational_exp
    | equality_exp TNOTEQU relational_exp
    ;

relational_exp: additive_exp
    | relational_exp '>' additive_exp
    | relational_exp '<' additive_exp
    | relational_exp TGREATE additive_exp
    | relational_exp TLESSE additive_exp
    ;

additive_exp: multiplicative_exp
    | additive_exp '+' multiplicative_exp
    | additive_exp '-' multiplicative_exp
    ;

multiplicative_exp: unary_exp
    | multiplicative_exp '*' unary_exp
    | multiplicative_exp '/' unary_exp
    | multiplicative_exp '%' unary_exp
    ;

unary_exp: postfix_exp
    | '-' unary_exp
    | '!' unary_exp
    | TINC unary_exp
    | TDEC unary_exp
    ;

postfix_exp: primary_exp
    | postfix_exp '[' expression ']'
    | postfix_exp '(' opt_actual_param ')'
    | postfix_exp TINC
    | postfix_exp TDEC
    ;

opt_actual_param: actual_param
    |
    ;

actual_param: actual_param_list
    ;

actual_param_list: assignment_exp
    | actual_param_list ',' assignment_exp
    ;

primary_exp: TIDENT
    | TNUMBER
    | '(' expression ')'
    ;

%%

extern FILE *yyin;


char* toString(char* string)
{
    char* str;
    str = (char*)malloc(strlen(string) + 1);
    strcpy(str, string);
    return str;
}

int main(int argc, char *argv[]){
    FILE *sourceFile;
    strcpy(filename, argv[1]);
    sourceFile = fopen(filename, "r");

    if(!sourceFile)
        fprintf(stderr, "not open!\n");
    yyin = sourceFile;

    do{
        yyparse();
    } while(!feof(yyin));

    printTree(root, 0);

    fclose(sourceFile);
    return 1;       
}


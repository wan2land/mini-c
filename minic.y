%{
    #include <stdio.h>

    int yylex();
    void yyerror(const char *s);
%}
%token TCONST TELSE TIF TINT TRETURN TVOID TWHILE TEQUAL TNOTEQU
    TLESSE TGREATE TAND TOR TINC TDEC TADDASSIGN TSUBASSIGN TMULASSIGN
    TDIVASSIGN TMODASSIGN TIDENT TNUMBER
%%

mini_c: translation_unit
    ;

translation_unit: external_dcl
    | translation_unit external_dcl
    ;

external_dcl: function_def
    | declaration
    ;

function_def: function_header compound_st
    ;

function_header: dcl_spec function_name formal_param
    ;

dcl_spec: dcl_specifiers
    ;

dcl_specifiers: dcl_specifier
    | dcl_specifiers dcl_specifier
    ;

dcl_specifier: type_qualifier
    | type_specifier
    ;

type_qualifier: TCONST
    ;

type_specifier: TINT
    | TVOID
    ;

function_name: TIDENT
    ;

formal_param: '(' opt_formal_param ')'
    ;

opt_formal_param: formal_param_list
    |
    ;

formal_param_list: param_dcl
    | formal_param_list ',' param_dcl
    ;

param_dcl: dcl_spec declarator
    ;

compound_st: '{' opt_dcl_list opt_stat_list '}'
    ;

opt_dcl_list: declaration_list
    |
    ;

declaration_list: declaration
    | declaration_list declaration
    ;

declaration: dcl_spec init_dcl_list ';'
    ;

init_dcl_list: init_declarator
    | init_dcl_list ',' init_declarator
    ;

init_declarator: declarator
    | declarator '=' TNUMBER
    ;

declarator: TIDENT
    | TIDENT '[' opt_number ']'
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

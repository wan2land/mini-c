#define NODE_NUM    58
#define STACK_SIZE  512

typedef struct tokenType {
    int tokenNumber;
    char *tokenValue;
} Token;

typedef struct nodeType {
    Token token;
    enum {TERMINAL, NONTERM} noderep;
    struct nodeType *son;
    struct nodeType *next;
    char *description;
} Node;

enum nodeNumber {
    ACTUAL_PARAM,   ADD,            ADD_ASSIGN,     ARRAY_VAR,      ASSIGN_OP,
    CALL,           COMPOUND_ST,    CONST_NODE,     DCL,            DCL_ITEM,
    DCL_LIST,       DCL_SPEC,       DIV,            DIV_ASSIGN,     EQ,
    ERROR_NODE,     EXP_ST,         FORMAL_PARA,    FUNC_DEF,       FUNC_HEAD,
    GE,             GT,             IDENT,          IF_ELSE_ST,     IF_ST,
    INDEX,          INT_NODE,       LE,             LOGICAL_AND,    LOGICAL_NOT,
    LOGICAL_OR,     LT,             MOD,            MOD_ASSIGN,     MUL,
    MUL_ASSIGN,     NE,             NUMBER,         PARAM_DCL,      POST_DEC,
    POST_INC,       PRE_DEC,        PRE_INC,        PROGRAM,        RETURN_ST,
    SIMPLE_VAR,     STAT_LIST,      SUB,            SUB_ASSIGN,     UNARY_MINUS,
    VOID_NODE,      WHILE_ST,           
};

Node* buildNode(int tokenNumber, char* tokenValue);

Node* buildTree(int tokenNumber, Node* son, Node* next);

void appendNext(Node* node, Node* next);

void printTree(Node *ptr, int indent);

extern char *nodeName[];
extern Node *valueStack[STACK_SIZE];
extern int rightLength[NODE_NUM + 1];
extern int sp;

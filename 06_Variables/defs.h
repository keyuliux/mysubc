#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Structure and enum definitions

#define TEXTLEN 512 // Length of symbols in input
// Tokens
enum{
    T_EOF, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT, 
    T_SEMI, T_PRINT, T_INT, T_EQUALS, T_IDENT
};

// Token structure
struct token {
    int token;
    int intvalue;
};

// AST node types
enum {
    A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE, A_INTLIT,
    A_IDENT, A_LVIDENT, A_ASSIGN
};

// Abstract Sytax Tree structure
struct ASTnode
{
    int op; // "Operation" to be performed on this tree
    struct ASTnode *left;
    struct ASTnode *right;
    union {
        int intvalue; // For A_INTLIT,  the integer value
        int id;       // For A_IDENT, the symbol slot number        
    } v;
    
};

// Symbol table structure
struct symtable
{
    char *name;
};
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Structure and enum definitions

#define TEXTLEN 512 // Length of symbols in input
// Tokens
enum{
    T_EOF, 
    T_PLUS, T_MINUS, 
    T_STAR, T_SLASH, 
    T_EQ, T_NE, //==, !=, <, >, <= and >=
    T_LT, T_GT, T_LE, T_GE,
    // Type keywords
    T_VOID, T_CHAR, T_INT, T_LONG,
    // Structural tokens
    T_INTLIT, T_SEMI, T_ASSIGN, T_IDENT,
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, 
    T_AMPER, T_LOGAND,
    // Other Keywords
    T_PRINT, T_IF, T_ELSE, T_WHILE, T_FOR, 
    T_RETURN, T_COMMA, A_SCALE
};

// Token structure
struct token {
    int token;
    int intvalue;
};

// AST node types
enum {
  A_ADD = 1, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
  A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,
  A_INTLIT,
  A_IDENT, A_LVIDENT, A_ASSIGN, A_PRINT, A_GLUE, A_IF,
  A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN, A_FUNCCALL,
  A_DEREF, A_ADDR
};

// Primitive types
enum {
  P_NONE, P_VOID, P_CHAR, P_INT, P_LONG,
  P_VOIDPTR, P_CHARPTR, P_INTPTR, P_LONGPTR
};

// Abstract Sytax Tree structure
struct ASTnode
{
    int op; // "Operation" to be performed on this tree
    int type;// Type of any expression this tree generates
    struct ASTnode *left;
    struct ASTnode *mid;
    struct ASTnode *right;
    union {
        int intvalue; // For A_INTLIT,  the integer value
        int id;       // For A_IDENT, the symbol slot number 
        int size;     // For A_SCALE, the size to scale by       
    } v;
    
};

#define NOREG	-1		// Use NOREG when the AST generation
				// functions have no register to return

// Structural types
enum {
  S_VARIABLE, S_FUNCTION
};

// Symbol table structure
struct symtable
{
    char *name;
    int type;			// Primitive type for the symbol
    int stype;			// Structural type for the symbol
    int endlabel;  // For S_FUNCTION, the end label
};
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Structure and enum definitions

#define TEXTLEN 512 // Length of symbols in input
// Tokens
enum{
    T_EOF, 
    // Binary operators
    T_ASSIGN, T_LOGOR, T_LOGAND,
    T_OR, T_XOR, T_AMPER,
    T_EQ, T_NE, //==, !=, <, >, <= and >=
    T_LT, T_GT, T_LE, T_GE,
    T_LSHIFT, T_RSHIFT,
    T_PLUS, T_MINUS, T_STAR, T_SLASH, 
    T_INC, T_DEC, T_INVERT, T_LOGNOT,
    // Type keywords
    T_VOID, T_CHAR, T_INT, T_LONG,

    // Other Keywords
    T_IF, T_ELSE, T_WHILE, T_FOR, T_RETURN,
        // Structural tokens
    T_INTLIT, T_STRLIT, T_SEMI, T_IDENT,
    T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN,
    T_LBRACKET, T_RBRACKET
};

// Token structure
struct token {
    int token;
    int intvalue;
};

// AST node types
enum {
  A_ASSIGN = 1, A_LOGOR, A_LOGAND, A_OR, A_XOR, A_AND,
  A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE, A_LSHIFT, A_RSHIFT,
  A_ADD, A_SUBTRACT, A_MULTIPLY, A_DIVIDE,
  A_INTLIT, A_STRLIT,  A_IDENT, A_GLUE, 
    A_IF, A_WHILE, A_FUNCTION, A_WIDEN, A_RETURN, 
  A_FUNCCALL, A_DEREF, A_ADDR, A_SCALE,
  A_PREINC, A_PREDEC, A_POSTINC, A_POSTDEC,
  A_NEGATE, A_INVERT, A_LOGNOT, A_TOBOOL
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
    int rvalue; // True if the node is an rvalue
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
#define NOLABEL	 0		// Use NOLABEL when we have no label to
				// pass to genAST()

// Structural types
enum {
  S_VARIABLE, S_FUNCTION, S_ARRAY
};

// Storage classes
enum {
  C_GLOBAL = 1,  // Globally visible symbol
  C_LOCAL       // Locally visible symbol
};

// Symbol table structure
struct symtable
{
    char *name;
    int type;			// Primitive type for the symbol
    int stype;			// Structural type for the symbol
    int class;      // Storage class for the symbol
    int endlabel;  // For S_FUNCTION, the end label
    int size;      // Number of elements in the symbol
    int posn;      // from the stack base pointer
};
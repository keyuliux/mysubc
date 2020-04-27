#include "defs.h"
#include "data.h"
#include "decl.h"

// Parsing of declarations

// Parse the current token and
// return a primitive type enum value
int parse_type(int t)
{
    if (t == T_CHAR) return (P_CHAR);
    if (t == T_INT) return (P_INT);
    if (t == T_VOID) return (P_VOID);
    fatald("Illegal type, token %d", t);
}

// Parse the declaration of variable
void var_declaration(void)
{
    int id, type;

    type = parse_type(Token.token);
    scan(&Token);
    ident();
    id = addglob(Text, type, S_VARIABLE);
    genglobsym(id);
    semi();
}
//function_declaration: 'void' identifier '(' ')' compount_statement ;
// Parse the declaration of a simplistic function
struct ASTnode *function_declaration(void)
{
    struct ASTnode *tree;
    int nameslot;
    
    match(T_VOID, "void");
    ident();
    nameslot = addglob(Text, P_VOID, S_FUNCTION);
    lparen();
    rparen();

    tree = compound_statement();

    return (mkastunary(A_FUNCTION, P_VOID, tree, nameslot));
}


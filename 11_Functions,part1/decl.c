#include "defs.h"
#include "data.h"
#include "decl.h"

// Parsing of declarations

// Parse the declaration of variable
void var_declaration(void)
{
    // Ensure we have an 'int' token followed by an identifier
    // and a semicolon. Text now has the identifier's name.
    // Add it as a known identifier
    match(T_INT, "int");
    ident();
    addglob(Text);
    genglobsym(Text);
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
    nameslot = addglob(Text);
    lparen();
    rparen();

    tree = compound_statement();

    return (mkastunary(A_FUNCTION, tree, nameslot));
}


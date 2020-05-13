#include "defs.h"
#include "data.h"
#include "decl.h"

// Parsing of statements
/* 
compound_statement: '{' '}'
	| '{' statement'}'
	| '{' statement statements'}'
	;
statements: statement
	| statement statements
	;

statement: print_statement
	| declaration
	| assignment_statement
	| if_statement
	;

print_statement:	'print' expression ';' ;
declaration: 'int' identifier ';' ;
assignment_statement: identifier '=' expression ';' ;

if_statement: if_head
	| if_head 'else' compound_statement
	;
if_head: 'if' '(' true_false_expression ')' compound_statement

*/
static struct ASTnode *single_statement(void);


static struct ASTnode *if_statement(void) {
    struct ASTnode *condAST, *trueAST, *falseAST = NULL;

    // Ensure we have 'if' '('
    match(T_IF, "if");
    lparen();

    
    // Parse the following expression
    // and the ')' following. Ensure
    // the tree's operation is a comparison
    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);
    rparen();

    // Get the AST for the compound statement
    trueAST = compound_statement();

    // If we have an 'else', skip it
    // and get the AST for the coumpoud statement
    if (Token.token == T_ELSE) {
        scan(&Token);
        falseAST = compound_statement();
    }

    // Build and return the AST for this statement
    return (mkastnode(A_IF, P_NONE, condAST, trueAST, falseAST, 0));
}



//while_statement: 'while' '(' true_false_expression ')' compound_statement ;
static struct ASTnode *while_statement(void) {
    struct ASTnode *condAST, *bodyAST;

    // Ensure we have 'while' '('
    match(T_WHILE, "while");
    lparen();


    // Parse the following expression
    // and the ')' following. Ensure
    // the tree's operation is a comparison
    condAST = binexpr(0);

    if (condAST->op < A_EQ || condAST->op > A_GE)
    condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);
    rparen();
    // Get the AST for the compound statement
    bodyAST = compound_statement();

    // Build and return the AST for this statement
    return (mkastnode(A_WHILE, P_NONE, condAST, NULL, bodyAST, 0));
}







static struct ASTnode *for_statement(void) {
    struct ASTnode *condAST, *bodyAST;
    struct ASTnode *preopAST, *postopAST;
    struct ASTnode *tree;

    // Ensure we have 'for' '('
    match(T_FOR, "for");
    lparen();

    preopAST = single_statement();
    semi();







    condAST = binexpr(0);
    if (condAST->op < A_EQ || condAST->op > A_GE)
        condAST = mkastunary(A_TOBOOL, condAST->type, condAST, 0);
    semi();

    postopAST = single_statement();
    rparen();

    bodyAST = compound_statement();

    tree = mkastnode(A_GLUE, P_NONE, bodyAST, NULL, postopAST, 0);
    tree = mkastnode(A_WHILE, P_NONE, condAST, NULL, tree, 0);

    return (mkastnode(A_GLUE, P_NONE, preopAST, NULL, tree, 0));
}














// Parse a return statement and return its AST
// return_statement: 'return' '(' expression ')' ;
static struct ASTnode *return_statement(void)
{
    struct ASTnode *tree;

    // Can't return a value if function returns P_VOID
    if (Gsym[Functionid].type == P_VOID)
    fatal("Can't return from a void function");

    // Ensure we have 'return' '('
    match(T_RETURN, "return");
    lparen();

    // Parse the following expression
    tree = binexpr(0);

    tree = modify_type(tree, Gsym[Functionid].type, 0);
    if (tree == NULL)
        fatal("Incompatible types in return_statement");
    
    // Add on the A_RETURN node
    tree = mkastunary(A_RETURN, P_NONE, tree, 0);

    // Get the ')'
    rparen();
    return (tree);
}
// Pasrse a single statement
// and return its AST
static struct ASTnode *single_statement(void)
{
    int type;
    switch (Token.token)
        {
        case T_CHAR:
        case T_INT:
        case T_LONG:
            type = parse_type();
            ident();
            var_declaration(type, 1);
            return (NULL);
        case T_IF:
            return ( if_statement());
        case T_WHILE:
            return ( while_statement());
        case T_FOR:
            return (for_statement());
        case T_RETURN:
            return (return_statement());
        default:
           return (binexpr(0));
        }
    return (NULL); // Keep -Wall happy
}









struct ASTnode *compound_statement(void)
{
    struct ASTnode *left = NULL;
    struct ASTnode *tree;

    // Require a left curly bracket
    lbrace();

    while (1)
    {
        // Parse a single statement
        tree = single_statement();

        // some statement must be followed by a semicolon
        if (tree != NULL && 
                (tree->op == A_ASSIGN ||
                tree->op == A_RETURN || tree->op == A_FUNCCALL))
                semi();
        // For each new tree, either save it in left
        // if left is empty, or glue the left and the
        // new tree together
        if (tree != NULL) 
        {
            if (left == NULL)
            {
                left = tree;
            }else
            {
                left = mkastnode(A_GLUE, P_NONE, left, NULL, tree, 0);
            }

        }
        // When we hit a right curly bracket,
        // skip past it and return the AST
        if (Token.token == T_RBRACE)
        {
            rbrace();
            return (left);
        }
    }
}
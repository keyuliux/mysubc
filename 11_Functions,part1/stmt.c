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

static struct ASTnode *print_statement(void)
{
    struct ASTnode *tree;
    int reg;

    // Match a 'print' as the first token
    match(T_PRINT, "print");

    // Parse the following expression and
    // generate the assembly code
    tree = binexpr(0);

    // Make an print AST tree
    tree = mkastunary(A_PRINT, tree, 0);

    return (tree);
}

static struct ASTnode *assignment_statement(void)
{
    struct ASTnode *left, *right, *tree;
    int id;

    // Ensure we have an identifier
    ident();

    // Check it's been defied then make a leaf node for it
    if((id = findglob(Text)) == -1) {
        fatals("Undeclared variable", Text);
    }
    right = mkastleaf(A_LVIDENT, id);

    // Ensure we have an equals sign
    match(T_ASSIGN, "=");

    // Parse the following expression
    left = binexpr(0);
    // Make an assignment AST tree
    tree = mkastnode(A_ASSIGN, left, NULL, right, 0);

    return (tree);
}

//while_statement: 'while' '(' true_false_expression ')' compound_statement ;
static struct ASTnode *while_statement(void)
{
    struct ASTnode *condAST, *bodyAST;

    // Ensure we have 'while' '('
    match(T_WHILE, "while");
    lparen();

    // Parse the following expression
    // and the ')' following. Ensure
    // the tree's operation is a comparison
    condAST = binexpr(0);

    if (condAST->op < A_EQ || condAST->op > A_GE)
    fatal("Bad comparison operator");
    rparen();

    // Get the AST for the compound statement
    bodyAST = compound_statement();

    // Build and return the AST for this statement
    return (mkastnode(A_WHILE, condAST, NULL, bodyAST, 0));
}

/* 
if_statement: if_head
	| if_head 'else' compound_statement
	;
if_head: 'if' '(' true_false_expression ')' compound_statement

*/
static struct ASTnode *if_statement(void)
{
    struct ASTnode *condAST, *trueAST, *falseAST = NULL;

    // Ensure we have 'if' '('
    match(T_IF, "if");
    lparen();

    // Parse the following expression
    // and the ')' following. Ensure
    // the tree's operation is a comparison
    condAST = binexpr(0);

    if (condAST->op < A_EQ || condAST->op > A_GE)
        fatal("Bad comparison operator");
    rparen();

    // Get the AST for the compound statement
    trueAST = compound_statement();

    // If we have an 'else', skip it
    // and get the AST for the coumpoud statement
    if (Token.token == T_ELSE)
    {
        scan(&Token);
        falseAST = compound_statement();
    }

    // Build and return the AST for this statement
    return (mkastnode(A_IF, condAST, trueAST, falseAST, 0));
}

//for_statement: 'for' '(' preop_statement ';'
//			true_false_expression ';'
//			postop_statement ';' compound_statement ;
/*         A_GLUE
        /     \
   preop     A_WHILE
             /    \
        decision  A_GLUE
                  /    \
            compound  postop */
static struct ASTnode *for_statement(void)
{
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
        fatal("Bad comparison operator");
    semi();

    postopAST = single_statement();
    rparen();

    bodyAST = compound_statement();

    tree = mkastnode(A_GLUE, bodyAST, NULL, postopAST, 0);
    tree = mkastnode(A_WHILE, condAST, NULL, tree, 0);

    return (mkastnode(A_GLUE, preopAST, NULL, tree, 0));
}
// Pasrse a single statement
// and return its AST
static struct ASTnode *single_statement(void)
{
    switch (Token.token)
        {
        case T_PRINT:
            return (print_statement());
        case T_INT:
            var_declaration();
            return (NULL);
        case T_IDENT:
            return ( assignment_statement());
        case T_IF:
            return ( if_statement());
        case T_WHILE:
            return ( while_statement());
        case T_FOR:
            return (for_statement());
        default:
           fatald("Syntax error, token", Token.token);
        }
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
                (tree->op == A_PRINT || tree->op == A_ASSIGN))
                semi();
        // For each new tree, either save it in left
        // if left is empty, or glue the left and the
        // new tree together
        if (tree) 
        {
            if (left == NULL)
            {
                left = tree;
            }else
            {
                left = mkastnode(A_GLUE, left, NULL, tree, 0);
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
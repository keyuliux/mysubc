#include "defs.h"
#include "data.h"
#include "decl.h"

// AST tree interpreter

// List of AST operators
static char *ASTop[] = { "+", "-", "*", "/"};

// Given an AST, interpret the 
// operator in it and return
// a final value.
int interpretAST(struct ASTnode *n)
{
    int leftval, rightval;

    // Get the left and right sub-tree values
    if(n->left)
        leftval = interpretAST(n->left);
    if(n->right)
        rightval = interpretAST(n->right);
    
    // Debug: Print what we are about to do 
    if (n->op == A_INTLIT)
        printf("int %d\n", n->intvalue);
    else
        printf("%d %s %d\n", leftval, ASTop[n->op], rightval);

    switch (n->op)
    {
        case A_ADD:
            return (leftval + rightval);
            break;
        case A_SUBTRACT:
            return (leftval - rightval);
            break;
        case A_MULTIPLY:
            return (leftval * rightval);
            break;
        case A_DIVIDE:
            return (leftval / rightval);
            break;
        case A_INTLIT:
            return (n->intvalue);
            break;    
        default:
            fprintf(stderr, "Unknow AST operation %d\n", n->op);
            exit(1);
    }
    
}
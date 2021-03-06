#include "defs.h"
#include "data.h"
#include "decl.h"

// Parsing of expressions
static struct ASTnode *array_access(void);
// Parse a function call with a single expression
// argument and return its AST
struct ASTnode *funccall(void)
{
    struct ASTnode *tree;
    int id;
    // Check that the identifier has been defined,
    // then make a leaf node for it. XXX add structural type test
    if ((id = findglob(Text)) == -1)
    {
        fatals("Undeclared function", Text);
    }

    // Get the '('
    lparen();

    // Parse the following expression
    tree = binexpr(0);

    tree = mkastunary(A_FUNCCALL, Gsym[id].type, tree, id);
    
    // Get the ')'
    rparen();

    return (tree);
}
// Parse a primar factor and return an 
// AST node representing it.
static struct ASTnode *primary(void)
{
    struct ASTnode *n;
    int id;
    
    // For an INTLIT token, make a leaf AST node for it
    // and scan in the next token. Otherwise, a syntax error
    // for any other token type.
    switch (Token.token)
    {
    case T_STRLIT:
        // For a STRLIT token, generate the assembly for it.
        // Then make a leaf AST node for it. id is the string's label.
        id = genglobstr(Text);
        n = mkastleaf(A_STRLIT, P_CHARPTR, id);
        break;
    case T_INTLIT:
        if ((Token.intvalue >=0) && (Token.intvalue < 256))
            n = mkastleaf(A_INTLIT, P_CHAR, Token.intvalue);
        else
            n = mkastleaf(A_INTLIT, P_INT, Token.intvalue);
        break;
    case T_IDENT:
        scan(&Token);
        // It's a '(', so a function call
        if(Token.token == T_LPAREN)
        return (funccall());
        if (Token.token == T_LBRACKET)
        return (array_access());
        // Not a function call , so reject the new token
        reject_token(&Token);
        
        id = findglob(Text);
        if(id == -1)
            fatals("Unknown variable", Text);
        n = mkastleaf(A_IDENT, Gsym[id].type, id);
        break;
    case T_LPAREN:
    // Beginning of a parenthesised expression, skip the '('.
    // Scan in the expression and the right parentesis
        scan(&Token);
        n = binexpr(0);
        rparen();
        return (n);
    default:
        fatald("syntax error, token", Token.token);
    }
    scan(&Token);
    return (n);
}

// Conver a binary operator token into a binary AST operation.
// We rely on a 1:1 mapping form token to AST operation
int binastop(int tokentype)
{
    if (tokentype > T_EOF && tokentype < T_INTLIT)
        return (tokentype);
    fatald("Syntax error, token", tokentype);
    return (0);
}

// Return true if a token is right-associative,
// false otherwise
static int rightassoc(int tokentype) {
    if( tokentype == T_ASSIGN)
        return (1);
    return (0);
}

// Operator precedence for each token
static int OpPrec[] = { 0, 10, // T_EOF, T_PLUS, T_MINUS
                        20, 20,    // T_STAR, T_SLASH
                        30, 30,    // T_EQ, T_NE
                        40, 40, 
                        50, 50, 50, 50 // T_LT, T_GT, T_LE, T_GE
                        };

// Check that we have a binary operator and
// return its precedence
static int op_precedence(int tokentype)
{
  int prec;
  if (tokentype >= T_VOID)
    fatald("Token with no precedence in op_precedence:", tokentype);
  
  prec = OpPrec[tokentype];
  if(prec == 0)
  {
    fprintf(stderr, "syntax error on line %d, token %d\n", Line, tokentype);
    exit(1);
  }
  return (prec);
}  
// Parse a prefix expression and return
// a sub-tree representing it
struct ASTnode *prefix(void)
{
    struct ASTnode *tree;
    switch (Token.token)
    {
    case T_AMPER:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();

        // Ensure that it's an identifier
        if(tree->op != A_IDENT)
            fatal("& operator must be followed by an identifier");
        // Now change the operator to A_ADDR and the type to
        // a pointer to the original type
        tree->op = A_ADDR; 
        tree->type = pointer_to(tree->type);
        break;
    case T_STAR:
        // Get the next token and parse it
        // recursively as a prefix expression
        scan(&Token);
        tree = prefix();
        // For now, ensure it's either another deref or an 
        // identifier
        if(tree->op != A_IDENT && tree->op != A_DEREF)
            fatal("* -operator must be followed by an identifier or *");
        // Prepend an A_DEREF operation to the tree
        tree = mkastunary(A_DEREF, value_at(tree->type), tree, 0);
        break;
    default:
        tree = primary();
    }
    return (tree);
}

// Parse the index into an array and
// return an AST tree for it
static struct ASTnode *array_access(void) {
    struct ASTnode *left, *right;
    int id;

    // Check that the identifier has been defined as an array
    // then make a leaf node for it that points at the base
    if ((id = findglob(Text)) == -1 || Gsym[id].stype != S_ARRAY) {
        fatals("Undeclared array", Text);
    }
    left = mkastleaf(A_ADDR, Gsym[id].type, id);

    // Get the '['
    scan(&Token);

    // Parse the following expression
    right = binexpr(0);

    // Get the ']'
    match(T_RBRACKET, "]");

    // Ensure that this is of int type
    if (!inttype(right->type))
        fatal("Array index is not of integer type");
    
    // Scale the index by the size of the element's type
    right = modify_type(right, left->type, A_ADD);

    // Return an AST tree where the array's base has the offset
    // added to it, and dereference the element. Still an lvalue
    // at this point.
    left = mkastnode(A_ADD, Gsym[id].type, left, NULL, right, 0);
    left = mkastunary(A_DEREF, value_at(left->type), left, 0);

    return (left);
}

// Return an AST tree whose root is a binary operator
// Parameter ptp is the previous token's precedence.

struct ASTnode *binexpr(int ptp)
{
    
    struct ASTnode *left, *right;
    struct ASTnode *ltemp, *rtemp;
    int ASTop;
    int tokentype;

    // Get the integer literal on the left
    // Fetch the next token at the same time.
    left = prefix();
    // if we hit a semicolon, return just the left node
    tokentype = Token.token;
    if( tokentype == T_SEMI || tokentype == T_RPAREN 
    || tokentype == T_RBRACKET){
        left->rvalue = 1; return (left);
    }
         

    // While the precedence of this token is
    // more that of the previous taken precedence
    while((op_precedence(tokentype) > ptp) ||
                (rightassoc(tokentype) && op_precedence(tokentype) == ptp))
    {
        // Fetch in the nex integer literal
        scan(&Token);

        // Recursively call binexpr() with the
        // precedence of our token to build a sub-tree
        right = binexpr(OpPrec[tokentype]);
        ASTop = binastop(tokentype);

        if (ASTop == A_ASSIGN) {
            // Assignment
            // Make the right tree into an rvalue
            right->rvalue = 1;

            // Ensure the right's type matches the left
            right = modify_type(right, left->type, 0);
            if( left == NULL)
                fatal("Incompatible expression in assignment");

            // Make an assignment AST tree. However, switch
            // left and right around, so that the right expression's
            // code will be generated before the left expression
            ltemp = left; left = right; right = ltemp; 

        } else {
            // We are not doing an assignment, so both trees shou be rvalues
            // Convert both trees into rvalue if they are lvalue trees
            left->rvalue = 1;
            right->rvalue = 1;

            ltemp = modify_type(left, right->type, ASTop);
            rtemp = modify_type(right, left->type, ASTop);

            if (ltemp == NULL && rtemp == NULL)
                fatal("Incompatible types in binary expression");

            if (ltemp != NULL)
                left = ltemp;
                if (rtemp != NULL)
                right = rtemp; 
        }
        left = mkastnode(binastop(tokentype), left->type, left, NULL, right, 0);


        // Update the details of the current token.
        // If we hit a semicolon, return just the left node
        tokentype = Token.token;
        if (tokentype == T_SEMI || tokentype == T_RPAREN 
        || tokentype == T_RBRACKET) {
            left->rvalue=1; return (left);
        }
    }
    // Return the tree we have when the precedence
    // is the same or lower
    left->rvalue=1; return (left);
    
}



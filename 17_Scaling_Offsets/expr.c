#include "defs.h"
#include "data.h"
#include "decl.h"

// Parsing of expressions

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
        // Not a function call , so reject the new token
        reject_token(&Token);
        
        id = findglob(Text);
        if(id == -1)
            fatals("Unknown variable", Text);
        n = mkastleaf(A_IDENT, Gsym[id].type, id);
        break;
    default:
        fatald("syntax error, token", Token.token);
    }
    scan(&Token);
    return (n);
}

// Conver a token into an AST operation.
// we rely on a 1:1 mapping from token to AST operation
int arithop(int tokentype)
{
    if (tokentype > T_EOF && tokentype < T_INTLIT)
        return (tokentype);
}

// Operator precedence for each token
static int OpPrec[] = { 0, 10, 10, // T_EOF, T_PLUS, T_MINUS
                        20, 20,    // T_STAR, T_SLASH
                        30, 30,    // T_EQ, T_NE
                        40, 40, 40, 40 // T_LT, T_GT, T_LE, T_GE
                        };

// Check that we have a binary operator and
// return its precedence
static int op_precedence(int tokentype)
{
  int prec = OpPrec[tokentype];
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
    if( Token.token == T_SEMI || Token.token == T_RPAREN)
        return (left);

    // While the precedence of this token is
    // more that of the previous taken precedence
    while(op_precedence(tokentype) > ptp)
    {
      // Fetch in the nex integer literal
      scan(&Token);

      // Recursively call binexpr() with the
      // precedence of our token to build a sub-tree
      right = binexpr(OpPrec[tokentype]);
      ASTop = arithop(tokentype);
      ltemp = modify_type(left, right->type, ASTop);
      rtemp = modify_type(right, left->type, ASTop);

      if (ltemp == NULL && rtemp == NULL)
        fatal("Incompatible types in binary expression");

      left = mkastnode(arithop(tokentype), left->type, left, NULL, right, 0);
        if (ltemp != NULL)
        left = ltemp;
        if (rtemp != NULL)
        right = rtemp;

      // Update the details of the current token.
      // If we hit a semicolon, return just the left node
      tokentype = Token.token;
      if (tokentype == T_SEMI || tokentype == T_RPAREN)
        return (left);

    }
    // Return the tree we have when the precedence
    // is the same or lower
    return (left);
    
}

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
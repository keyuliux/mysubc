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
        n = mkastleaf(A_INTLIT, Token.intvalue);
        break;
    case T_IDENT:
        id = findglob(Text);
        if(id == -1)
            fatals("Unknown variable", Text);
        n = mkastleaf(A_IDENT, id);
        break;
    default:
        fprintf(stderr, "syntax error on line %d\n", Line);
        exit(1);
    }
    scan(&Token);
    return (n);
}

// Conver a token into an AST operation.
int arithop(int tok)
{
  switch (tok) 
  {
    case T_PLUS:
      return (A_ADD);
    case T_MINUS:
      return (A_SUBTRACT);
    case T_STAR:
      return (A_MULTIPLY);
    case T_SLASH:
      return (A_DIVIDE);
    default:
      fprintf(stderr, "unknown token in arithop() on line %d, tok %d\n", Line, tok);
      exit(1);
  }
}

// Operator precedence for each token
static int OpPrec[] = { 0, 10, 10, 20, 20, 0};
                      //EOF, +, -, *, /, INTLIT
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

// Return an AST tree whose root is a binary operator
// Parameter ptp is the previous token's precedence.
struct ASTnode *binexpr(int ptp)
{
    struct ASTnode *n, *left, *right;
    int tokentype;

    // Get the integer literal on the left
    // Fetch the next token at the same time.
    left = primary();

    // if we hit a semicolon, return just the left node
    tokentype = Token.token;
    if( Token.token == T_SEMI)
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

      // Join that sub-tree with ours. Convert the token
      // into an AST operation at the same time.
      left = mkastnode(arithop(tokentype), left, right, 0);

      // Update the details of the current token.
      // If we hit a semicolon, return just the left node
      tokentype = Token.token;
      if (tokentype == T_SEMI)
        return (left);

    }
    // Return the tree we have when the precedence
    // is the same or lower
    return (left);
    
}
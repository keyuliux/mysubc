#include "defs.h"
#include "data.h"
#include "decl.h"
// Given an AST, generate

// Generate and return a new label number
int genlabel(void) {
  static int id = 1;
  return (id++);
}
// Generate the code for an IF statement
// and an optional ELSE clause
static int genIFAST(struct ASTnode *n) 
{
  int Lfalse, Lend;

  // Generate two labels: one for the 
  // false compound statement, and one
  // for the end of then overall IF statement.
  // When there is no Else clause, Lfalse _is_
  // the ending label!
  Lfalse = genlabel();
  if(n->right)
  {
    Lend = genlabel();
  }

  // Generate the condition code followed
  // by a zero jump to the false label
  // we cheat by sending the Lfalse label as a register
  genAST(n->left, Lfalse, n->op);
  genfreeregs();

  // Generate the true compound statement
  genAST(n->mid, NOLABEL, n->op);
  genfreeregs();

  // If there is an optional ELSE clause,
  // generate the jump to skip to the end
  if(n->right)
      cgjump(Lend);
  
  // Now the false label
  cglabel(Lfalse);

  // Optional ELSE clause: generate the 
  // false compound statement and the 
  // end label
  if (n->right)
  {
    genAST(n->right, NOLABEL, n->op);
    genfreeregs();
    cglabel(Lend);
  }

  return (NOREG);
}

static int genWHILE(struct ASTnode *n)
{
  int Lstart, Lend;

  Lstart = genlabel();
  Lend = genlabel();

  cglabel(Lstart);
  genAST(n->left, Lend, n->op);
  genfreeregs();


  genAST(n->right, NOLABEL, n->op);
  genfreeregs();

  cgjump(Lstart);
  cglabel(Lend);

  return (NOREG);
}


// assembly code recursively
int genAST(struct ASTnode *n, int reg, int parentASTop)
{
    int leftreg, rightreg;
    switch (n->op)
    {
        case A_IF: return (genIFAST(n));
        case A_WHILE: return (genWHILE(n));
        case A_GLUE:
          genAST(n->left, NOLABEL, n->op);
          genfreeregs();
          genAST(n->right, NOLABEL, n->op);
          genfreeregs();
          return (NOREG);
        case A_FUNCTION:
          // Generate the function 's preamble before the code
          cgfuncpreamble(n->v.id);
          genAST(n->left, NOLABEL, n->op);
          cgfuncpostamble(n->v.id);
          return (NOREG);
    }

    // Get the left and right sub-tree values
    if (n->left) leftreg = genAST(n->left, NOLABEL, n->op);
    if (n->right) rightreg = genAST(n->right, NOLABEL, n->op);

    switch (n->op)
    {
        case A_ADD: return (cgadd(leftreg, rightreg));
        case A_SUBTRACT: return (cgsub(leftreg, rightreg));
        case A_MULTIPLY: return (cgmul(leftreg, rightreg));
        case A_DIVIDE: return (cgdiv(leftreg, rightreg));

        case A_EQ: 
        case A_NE:
        case A_LT: 
        case A_GT:
        case A_LE:
        case A_GE: 
          if (parentASTop == A_IF || parentASTop == A_WHILE)
              return (cgcompare_and_jump(n->op, leftreg, rightreg, reg));
          else
              return(cgcompare_and_set(n->op, leftreg, rightreg));
        case A_INTLIT: return (cgloadint(n->v.intvalue, n->type));
        case A_IDENT: 
          // Load our value if we are an rvalue
          // or we are being dereferenced
          if (n->rvalue || parentASTop == A_DEREF)
            return (cgloadglob(n->v.id));
          else
            return (NOREG);        
        case A_ASSIGN: 
        // As we assigning to an identifier or through a pointer?
        switch (n->right->op)
        {
        case A_IDENT: return (cgstorglob(leftreg, n->right->v.id));
        case A_DEREF: return (cgstorderef(leftreg, rightreg, n->right->type));
        default:
          fatald("Can't A_ASSIGN in genAST(), op", n->op);
        }
        return (rightreg);
        case A_WIDEN:
          return (cgwiden(leftreg, n->left->type, n->type));
        case A_RETURN:
          cgreturn(leftreg, Functionid);
          return (NOREG);
        case A_FUNCCALL:
          return (cgcall(leftreg, n->v.id));
        case A_ADDR:
        return (cgaddress(n->v.id));
        case A_DEREF:
        // If we are an rvalue, dereference to get the value we point at
        // otherwise leave it for A_ASSIGN to store through the pointer
        if(n->rvalue)
          return (cgderef(leftreg, n->left->type));
        else
          return (leftreg);
        case A_SCALE:
          switch (n->v.size)
          {
          case 2: return(cgshlconst(leftreg, 1));
          case 4: return(cgshlconst(leftreg, 2));
          case 8: return(cgshlconst(leftreg, 3));
          default:
            // Load a register with the size and
            // multiply the leftreg by this size
            rightreg = cgloadint(n->v.size, P_INT);
            return (cgmul(leftreg, rightreg));
          }
        default:
            fatald("Unknown AST operator %d\n", n->op);
    }
    return (NOREG);		// Keep -Wall happy
}

void genpreamble() {
  cgpreamble();
}

void genpostamble() {
  cgpostamble();
}

void genfreeregs() {
  freeall_registers();
}

void genprintint(int reg) {
  cgprintint(reg);
}

void genglobsym(int id)
{
  cgglobsym(id);
}

int genprimsize(int type)
{
  return (cgprimsize(type));
}

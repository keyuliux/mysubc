#include "defs.h"
#include "data.h"
#include "decl.h"

// Symbol table functions

// Determin if the symbol s is in the global symbol table.
// Return its slot position or -1 if not found.
int findglob(char *s)
{
    int i;

    for (i = 0; i < Globs; i++)
    {
        if(*s == *Gsym[i].name && !strcmp(s, Gsym[i].name))
            return (i);
    }
    return (-1);

}

// Get the position of a new global symbol slot, or die
static int newglob(void)
{
    int p;
    if((p = Globs++) >= Locls)
        fatal("Too many global symbols");
    return (p);
}

// Determine if the symbols is in the local symbol table
// Return its slot position or -1 if not found.
int findlocl(char *s) {
    int i;

    for (i = Locls+1; i < NSYMBOLS; i++){
        if (*s == *Gsym[i].name && !strcmp(s, Gsym[i].name))
            return (i);
    }
    return (-1);
}

// Get the position of a new local symbol slot, or die
// if we've run out of positions
static int newlocl(void) {
    int p;
    if ((p = Locls--) <= Globs) 
    fatal("Too many local symbols");

    return (p);
}

// Update a symbol at the given slot number in the symboltable.
// Set up its:
// + type: char, int etc.
// + stuctural type: var, function, array etc
// + size: number of elements
// + endlabel: if this is a function
// + posn: Position in formation for local symbols
static void updatesym(int slot, char *name, int type, int stype,
                int class, int endlabel, int size, int posn) {
    if (slot < 0 || slot >= NSYMBOLS)
        fatal("Invalid symbol slot number in updatesym()");
    Gsym[slot].name = strdup(name);
    Gsym[slot].type = type;
    Gsym[slot].stype = stype;
    Gsym[slot].class = class;
    Gsym[slot].endlabel = endlabel;
    Gsym[slot].size = size;
    Gsym[slot].posn = posn;
}

// Add a global symbol to the symbol table
// Return the slot number in the symbol table
int addglob(char *name, int type, int stype, int endlabel, int size)
{ 
    int slot;

    // If this is already in the symbol table, return the existing slot
    if((slot = findglob(name)) != -1)
        return (slot);

    // Otherwise get a new slot, fill it in and
    // return the slot number
    slot = newglob();
    updatesym(slot, name, type, stype, C_GLOBAL, endlabel, size, 0);
    genglobsym(slot);
    return (slot);
}

// Add a local symbol to the symbol table. Set up its:
// + type: char, int etc.
// + stuctural type: var, function, array etc
// + size: number of elements
// + endlabel: if this is a function
// Return the slot number in the symbol table
int addlocal(char *name, int type, int stype, int endlabel, int size) {
    int slot, posn;

    // If this is already in the symbol table, return the exist slot
    if ((slot = findlocl(name)) != -1)
        return (slot);
    
    // Otherwise get a new symbol slot and a position for this local.
    // Update the symbol table entry and return the slot number
    slot = newlocl();
    posn = gengetlocaloffset(type, 0);
    updatesym(slot, name, type, stype, C_LOCAL, endlabel, size, posn);
    return (slot);
}

// Determin if the symbol it in the symbol table
int findsymbol(char *s) {
    int slot;

    slot = findlocl(s);
    if (slot == -1)
        slot = findglob(s);
    return (slot);
}
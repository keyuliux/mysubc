#ifndef extern_
    #define extern_ extern
#endif

//Global variables

extern_ int Line;
extern_ int Putback;
extern_ FILE *Infile;
extern_ FILE *Outfile;
extern_ struct token Token;
extern_ char Text[TEXTLEN + 1];

// Global symbol table
#define NSYMBOLS 1024
extern_ struct symtable Gsym[NSYMBOLS];
static int Globs = 0; // Position of next free global symbol slot

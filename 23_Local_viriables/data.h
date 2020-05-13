#ifndef extern_
    #define extern_ extern
#endif

//Global variables

extern_ int Line;
extern_ int Putback;
extern_ int Functionid; // Symbol id of the current function
extern_ int Globs; // Position of next free global symbol slot
extern_ FILE *Infile;
extern_ FILE *Outfile;
extern_ struct token Token;
extern_ char Text[TEXTLEN + 1];

// Global symbol table
#define NSYMBOLS 1024
extern_ struct symtable Gsym[NSYMBOLS];
extern_ int Globs; // Position of next free global symbol slot
extern_ int Locls; // Position of next free local symbol slot
extern_ int O_dumpAST;
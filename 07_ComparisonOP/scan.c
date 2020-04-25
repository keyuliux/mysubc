#include "defs.h"
#include "data.h"
#include "decl.h"

// Lexical scanning

// Return the position of character c
// in string s, or -1 if c not found
static int chrpos(char *s, int c)
{
    char *p;

    p = strchr(s, c);
    return (p ? p - s: -1);

}


// Get the next character from the input file.
static int next(void)
{
    int c;
    if (Putback)
    {
        c = Putback; // use the character putback 
        Putback = 0; //if there is one
        return c;
    }
    c = fgetc(Infile); // read from input file
    //printf("next c %d \n", c);
    if('\n' == c)
        Line++;         // increment line count
    return c;
}

// Put back an unwanted character
static void putback(int c)
{
    Putback = c;
}

// Skip past input that we don't need to deal with,
// i.e. whitespace, newlines, Return the first
// character we do need to deal with.
static int skip(void)
{
    int c;
    c = next();
    while (' '==c || '\t'==c || '\n'==c || '\r'==c || '\f'==c)
    {
        c = next();
    }
    return (c);
    
}

// Scan and return an integer literal
// value from the inout file. Store
// the value as a string in Text.
static int scanint(int c)
{
    int k, val = 0;
    // Convert each character into an int value
    while ((k = chrpos("0123456789", c)) >= 0)
    {
        val = val * 10 + k;
        c = next();
    }
    // we hit a non-integer character, put it back.
    putback(c);
    return val;

}

// Scan an indentifier from the input file and
// store in in buf[]. Return the indentifier's length
static int scanident(int c, char *buf, int lim)
{
    int i = 0;

    // Allow digits, alpha and underscores
    while (isalpha(c) || isdigit(c) || '_'==c)
    {
        // Error if we hit the indentifier length limit,
        // else append to buf[] and get next character
        if(lim - 1 == i)
        {
            printf( "identifier too long on line %d\n", Line);
            exit(1);
        } else if (i < lim -1)
        {
            buf[i++] = c;
        }
        c = next();
    }
    // We hit a non-valid character, put it back.
    // NUL-terminate the buf[] and return the length
    putback(c);
    buf[i] = '\0';
    return (i);
    
} 
// Given a word from the input, return the matching
// keyword token number or 0 if it's not a keyword.
// Switch on the first letter so that we don't have
// to waste time strcmp()ing against all the keywords.
static int keyword(char *s)
{
    switch (*s)
    {
        case 'p':
            if (!strcmp(s, "print"))
            return (T_PRINT);
            break;
        case 'i':
            if (!strcmp(s, "int"))
            return (T_INT);
            break;
    }
    return (0);

}
// Scan and return the next token found int the input.
// Return 1 if token valid, 0 if no tokens left.
int scan (struct token *t)
{
    int c, tokentype;

    // Skip whitespace
    c = skip();
    //printf("scan c %d \n", c);
    // Determine the token based on 
    // the input character
    switch (c)
    {
    case EOF:
        t->token = T_EOF;
        return (0);
    case '+':
        t->token = T_PLUS;
        break;
    case '-':
        t->token = T_MINUS;
        break;
    case '*':
        t->token = T_STAR;
        break;
    case '/':
        t->token = T_SLASH;
        break;
    case ';':
        t->token = T_SEMI;
        break;
    case '=':
        if ((c = next()) == '=')
        {
            t->token = T_EQ;
        }else{
            putback(c);
            t->token = T_ASSIGN;    
        }
        break;
    case '!':
        if((c = next())== '=')
        {
            t->token = T_NE;
        } else
        {
            fatalc("Unrecognised character", c);
        }
        break;
    case '<':
        if ((c = next()) == '=')
        {
            t->token = T_LE;
        }else{
            putback(c);
            t->token = T_LT;    
        }
        break;
    case '>':
        if ((c = next()) == '=')
        {
            t->token = T_GE;
        }else{
            putback(c);
            t->token = T_GT;    
        }
        break;
    default:
        // if it is a digit, scan the
        // literal integer value in
        if(isdigit(c))
        {
            t->intvalue = scanint(c);
            t->token = T_INTLIT;
            break;
        }else if (isalpha(c) || '_'==c)
        {
            // Read in a keyword or indentifier
            scanident(c, Text, TEXTLEN);
            // If it's recognised keyword, return that token
            if (tokentype = keyword(Text))
            {
                t->token = tokentype;
                break;
            }
            // Not a recognised keyword, so it must be an indentifier
            t->token = T_IDENT;
            break;
        }

        printf("Unrecognised character %c on line %d\n", c, Line);
        exit(1);
    }

    // we found 1 token
    return (1);

}
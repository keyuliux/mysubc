#include "defs.h"
#include "data.h"
#include "decl.h"

// Types and type handling

// Given two primitive types, return true if they are compatible,
// false otherwise. Also return either zero or an A_WIDEN
// operation if one has to be widened to match the other.
// If onlyright is true, only widen left to right.
int type_compatible(int *left, int *right, int onlyright)
{
    int leftsize, rightsize;

    // Same types, they are compatible
    if(*left == *right) { *left = *right = 0; return (1);}

    leftsize = genprimsize(*left);
    rightsize = genprimsize(*right);

    if ((leftsize == 0) || (rightsize == 0))
        return (0);
    // Widen P_CHARs to P_INTs as required
    if (leftsize < rightsize )
    {
        *left = A_WIDEN; *right = 0; return (1);
    }
    if (rightsize < leftsize)
    {
        if (onlyright) 
        return (0);
        *left = 0; 
        *right = A_WIDEN;
        return (1);
    }

    // Anything remaining is compatible
     *left = *right = 0; 
     return (1);
}
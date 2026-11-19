/*
*   $Id: get.h 525 2007-05-28 01:50:41Z elliotth $
*
*   Copyright (c) 1998-2002, Darren Hiebert
*
*   This source code is released for free distribution under the terms of the
*   GNU General Public License.
*
*   External interface to get.c
*/
#ifndef _GET_H
#define _GET_H

/*
*   INCLUDE FILES
*/
#include "general.h"  /* must always come first */

#include "ctags.h"  /* to define langType */

/*
*   MACROS
*/
/*  Is the character the start of a universal character name?  The gcc
 *  preprocessor rewrites extended identifiers into \uXXXX / \UXXXXXXXX form,
 *  so the leading backslash must be accepted as part of the identifier.
 *  The 'u' and the hex digits already satisfy isalnum().
 */
#define isUcnEscape(c)  ((c) == '\\')

/*  Is the character valid as a character of a C identifier?
 *  VMS allows '$' in identifiers.
 */
#define isident(c)  (isalnum(c) || (c) == '_' || (c) == '$' || isUcnEscape(c))

/*  Is the character valid as the first character of a C identifier?
 *  C++ allows '~' in destructors.
 *  VMS allows '$' in identifiers.
 */
#define isident1(c)  (isalpha(c) || (c) == '_' || (c) == '~' || (c) == '$' || isUcnEscape(c))

/*
*   FUNCTION PROTOTYPES
*/
extern boolean isBraceFormat (void);
extern unsigned int getDirectiveNestLevel (void);
extern void cppInit (const boolean state, const boolean hasAtLiteralStrings);
extern void cppTerminate (void);
extern void cppBeginStatement (void);
extern void cppEndStatement (void);
extern void cppUngetc (const int c);
extern int cppGetc (void);
extern int skipOverCComment (void);

#endif  /* _GET_H */

/* vi:set tabstop=4 shiftwidth=4: */

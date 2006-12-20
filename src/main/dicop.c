/*!
 * @file
 * @ingroup workerframe
 * @brief Some handy routines for char conversion.
 * 
 * @copydoc copyrighttext
*/

#include <dicop.h>

void a2h (char * a, const unsigned int len)
  { 
  unsigned int j;
  char b[512];

  for (j=0; j < len; j++)
    {
    sprintf (&b[j*2],"%.2x",(unsigned char)a[j]);
    }
  b[len*2] = 0;		/* zero terminate */
  strncpy (a,b,len*2+1);
  }

void h2a (char * h)
  {
  int i,j;
  char ab[3]; 

  ab[2] = 0;
  i = strlen(h); 
  for (j=0; j<i; j += 2)
    {
    ab[0] = h[j]; ab[1] = h[j+1];
    h[j >> 1] = strtol (ab,0,16);
    }
  h[i >> 1] = 0;
  }

/** Compare string a with length la and string b with lenght lb.
 * Return codes:
 * - -1 if la < lb or la == lb and "a" < "b"
 * -  0 if la == lb and "a" equals "b"
 * - +1 if la > lb or la == lb and "a" > "b"
*/

int cmpStrings (const char* a, const char* b, const int la, const int lb)
  {
  int i;
  
  if (la < lb) { return -1; }
  if (la > lb) { return +1; }

  /* la == lb */
  for (i = 0; i < la; i++)
    {
    if (a[i] < b[i]) { return -1; }
    if (a[i] > b[i]) { return +1; }
    }
  return 0; /* a equals b */
  }


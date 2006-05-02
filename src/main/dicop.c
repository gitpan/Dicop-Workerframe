/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

  Convert "65" to "A" and "A" to "65" 

*/ 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void a2h (unsigned char * a, const unsigned int len)
  { 
  unsigned int j;
  unsigned char b[512];
  
  for (j=0; j < len; j++)
    {
    sprintf (&b[j*2],"%.2x",a[j]);
    }
  b[len*2] = 0;		/* zero terminate */
  strncpy (a,b,len*2+1);
  }

void h2a (unsigned char * h)
  {
  int i,j;
  unsigned char ab[3]; 

  ab[2] = 0;
  i = strlen((char*)h); 
  for (j=0; j<i; j += 2)
    {
    ab[0] = h[j]; ab[1] = h[j+1];
    h[j >> 1] = (unsigned char)strtol (ab,0,16);
    }
  h[i >> 1] = 0;
  }

int cmpStrings (unsigned char* a, unsigned char* b, int la, int lb)
  {
  // compare a with la = length(a) and b with lb = length(b)
  // -1 if a < b, 0 if a == b and +1 if (a > b)
  int i;
  
  if (la < lb) { return -1; }
  if (la > lb) { return +1; }
  // la == lb from here

  for (i = 0; i < la; i++)
    {
    if (a[i] < b[i]) { return -1; }
    if (a[i] > b[i]) { return +1; }
    }
  return 0; // a equals b
  }


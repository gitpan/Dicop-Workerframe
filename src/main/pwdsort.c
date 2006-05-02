/*

  Copyright (c) 2003-2006
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

//#define DEBUG 		1
//#define DEBUG_SORT		1
//#define DEBUG_MERGE		1
//#define DEBUG_COUNTER		1

/* don't sort pair of 2 with merge_sort, use manual sort for speed */
#define OPTIMIZED_MERGESORT 	1

#include "../include/pwdsort.h"
#include "../include/dicop.h"

#ifdef DEBUG_COUNTER
unsigned long swapped = 0;
unsigned long copied = 0;
unsigned long compared = 0;
#endif

/* memcmp - oddly enough faster than both memcmp and strncmp (probably
   only for small ranges, but we use them a lot */
int _compare (const unsigned char* a, const unsigned char* b, const unsigned int len)
  {
  /* return 0 if equal, or <0 or >0 if not */
  unsigned int i, d;

#ifdef DEBUG_COUNTER
  compared++;
#endif
  
  for (i = 0; i < len; i++)
    {
    d = a[i] - b[i];
    if (d != 0)
      {
      return d;
      }
    }
  return 0;
  }

/* memcpy - oddly enough faster than both memcpy and strncpy (probably
   only for small ranges, but we use them a lot */
void _copy (unsigned char* a, const unsigned char* b, unsigned int len)
  {
  unsigned int i;
  unsigned int len1;

  len1 = len >> 2;
  len = len & 3;
  
#ifdef DEBUG_COUNTER
  copied++;
#endif
  for (i = 0; i < len1; i++)
    {
    *(unsigned int*)a = *(unsigned int*)b;
    a += sizeof(unsigned int); b += sizeof(unsigned int);
    }
  for (i = 0; i < len; i++)
    {
    *a = *b; a++; b++;
    }
  }

/* memswap - swap two memory ranges in place 
   faster than copy the first to a temp, then second over first and
   then temp back */
void _swap (unsigned char* a, unsigned char* b, unsigned int len)
  {
  unsigned int i;
  unsigned int j;
  unsigned int len1;

#ifdef DEBUG_COUNTER
  swapped++;
#endif
  len1 = len >> 2;
  len = len & 3;
  
  for (i = 0; i < len1; i ++)
    {
    j = *(unsigned int*)a;
    *(unsigned int*)a = *(unsigned int*)b;
    *(unsigned int*)b = j;
    a += sizeof(unsigned int); b += sizeof(unsigned int);
    }
  for (i = 0; i < len; i ++)
    {
    j = *a; *a = *b; *b = j; a++; b++;
    }

  }

void pwdgen_img_merge_sort (
  unsigned char* pwd_list,
  unsigned char* temp,
  unsigned long first,
  unsigned long last,
  unsigned long len		/* len including zero termination or padding! */
  )
  {
  unsigned long median;			/* half */
  unsigned long tr1, tr2, index;	/* index into temp */

#ifdef DEBUG_SORT
  printf ("Sorting from %i to %i\n",first,last);
#endif

#ifdef OPTIMIZED_MERGESORT
  if ((last - first) == 1)
    {
    /* if there are only two, we can sort them faster */
    if (_compare( pwd_list + first * len, pwd_list + last * len, len) > 0)
      {
      _swap( pwd_list + first * len, pwd_list + last * len, len);
      }
    return;
    }
#endif

  /* have at least 2 to sort ? */
#ifdef OPTIMIZED_MERGESORT
  if ((last - first) > 1)
#else
  if ((last - first) > 0)
#endif
    {
    median = (first + last) / 2;	/* /2 and trunc to int */
    pwdgen_img_merge_sort (pwd_list, temp, first, median, len);
    pwdgen_img_merge_sort (pwd_list, temp, median + 1, last, len);

#ifdef DEBUG_SORT
    printf ("Merging from %i to %i\n",first,last);
#endif
  
    /* merge sorted halves back together by using a temp array */

#ifdef DEBUG_SORT
    printf ("Before merge we have: \n");
    for (index = first; index <= last; index++)
      {
      printf ("  '%s' %i\n", pwd_list + index * len, index);
      }
#endif
  
    index = 0;
    /* until one of the halves is empty */
    tr1 = first; tr2 = median + 1;
    while ((tr1 <= median) && (tr2 <= last))
      {
#ifdef DEBUG_SORT
      printf ("At %i (tr1 %i tr2 %i) ",index, tr1, tr2); 
#endif
      if (_compare( pwd_list + tr1 * len, pwd_list + tr2 * len, len) <= 0)
        {
#ifdef DEBUG_SORT
      printf ("use tr1 %i\n", tr1); 
#endif

        _copy( temp + index * len, pwd_list + tr1 * len, len);
        tr1++;
        }
      else
        {
#ifdef DEBUG_SORT
      printf ("use tr2 %i\n", tr2); 
#endif
	_copy( temp + index * len, pwd_list + tr2 * len, len);
        tr2++;
        }
      index++;
      }

    /* now copy anything that is leftover from the non-empty half */

#ifdef DEBUG_SORT
    printf ("Now copy remaining... index %i (tr2 %i <=> last %i <=> tr1 %i)\n",index, tr2,last,tr1);
#endif

    /* right half empty? */
    if (tr2 <= last)
      {
      /* use memcpy since the block is likely large and this is faster */
      memcpy( temp + index * len, pwd_list + tr2 * len, len * (last - tr2 + 1) );
#ifdef DEBUG_SORT
      index += (last - tr2 + 1);
#endif
      }

    /* left half empty? */
    if (tr1 <= median)
      {
      /* use memcpy since the block is likely large and this is faster */
      memcpy( temp + index * len, pwd_list + tr1 * len, len * (median - tr1 + 1));
#ifdef DEBUG_SORT
      index += (median - tr1 + 1);
#endif
      }

#ifdef DEBUG_SORT
    printf ("index is now %i. Now copy temp back\n", index);
#endif

    /* now copy from temp back to pwd_list */
    memcpy( pwd_list + first * len, temp, len * (last - first + 1) );

#ifdef DEBUG_SORT
    printf ("After merge we have: \n");
    for (index = first; index <= last; index++)
      {
      printf ("  '%s' %i\n", pwd_list + index * len, index);
      }
#endif
    }

  return;					/* sorted now */ 
  }

/* implementation of shell sort, from "Algorithmns", Robert Sedgewick 
   currently not finished and unused
*/

void pwdgen_img_shell_sort (
  unsigned char* pwd_list,
  unsigned char* temp,
  unsigned long first,
  unsigned long last,
  unsigned long len		/* len including zero termination or padding! */
  )
  {
  unsigned long* index;			/* ptr to index array */
  unsigned long cnt;			/* number of items to sort */
  unsigned long h;			/* distance(s) */
  unsigned long i,j;			/* temp vars */
  unsigned char* v;			/* compare element */
//  unsigned char* temp;			/* temp array for re-arange */

#ifdef DEBUG_SORT
  printf ("Sorting from %i to %i\n",first,last);
#endif

  cnt = last - first + 1;
  if (2 == cnt)
    {
    /* if there are only two elements, we can sort them very efficient */
    if (_compare( pwd_list + first * len, pwd_list + last * len, len) > 0)
      {
      _swap( pwd_list + first * len, pwd_list + last * len, len);
      }
    return;
    }

  /* allocate memory for the index array */
  index = malloc (sizeof(unsigned long) * cnt);
  if (NULL == index) 
    {
    printf ("Warning: Could not allocate %li bytes for sort.\n", 
     cnt * sizeof(unsigned long));
    /* fail gracefully */
    return;
    }

  /* initialize index going from 1 .. cnt */
  for (i = 0; i < cnt; i++)
    {
    index[i] = i;
    }

  /* calculate first distance */
  h = 1; while (h < cnt) { h = 3 * h + 1; }

  do
    {
    h = h / 3;
    for (i = h+1; i <= cnt; i++)
      {
      v = pwd_list + index[i] * len; j = i;
      while ( _compare( pwd_list + index[j-h] * len, v, len) > 0)
        {
        index[j] = index[j-h]; j = j - h;
        if (j <= h) { break; }
        }
      index[j] = index[i];
      }
    } while (h > 1);

  /* after having sorted the index array, we need to re-arrange the string so
     that they match the sorte order */

  /*	nr 	index[nr] 	contents	arange to:
	0	5		F		A
 	1	1		B		B
	2	0		A		C
	3	4		E		D
	4	2		C		E
	5	3		D		F
  */

  /* use a scratch aray */
//  temp = malloc ( len * cnt);
//  if (NULL == temp)
//    {
//    printf ("Warning: Could not allocate %i bytes for sort.\n", 
//     cnt * len);
//    /* fail gracefully */
//    return;
//    }

  /* sort them into temp in the right order as dictated by index[] */
  for (i = 0; i < cnt; i++)
    {
    _copy ( temp + len * index[i], pwd_list + len * i, len);
    }
  /* copy whole lot back */
  _copy ( pwd_list, temp, cnt * len);

  /* free scratch memory */
  free(index); // free(temp);

  /* all done */
  return;
  }


/* ************************************************************************ */
/* merge_sort operating on a index array */

void pwdgen_idx_merge_sort (
  unsigned char* pwd_list,
  unsigned int* idx,
  unsigned int* temp,
  unsigned long first,
  unsigned long last,
  unsigned long len	/* len of strings including zero termination and/or padding! */
  )
  {
  unsigned long median;			/* half */
  unsigned long t, tr1, tr2, index;	/* index into temp */

#ifdef DEBUG_SORT
  printf ("Sorting from %i to %i\n",first,last);
#endif

#ifdef OPTIMIZED_MERGESORT
  if ((last - first) == 1)
    {
    /* if there are only two, we can sort them faster */
    if (_compare( pwd_list + idx[first] * len, pwd_list + idx[last] * len, len) > 0)
      {
      t = idx[first]; idx[first] = idx[last]; idx[last] = t;
      }
    return;
    }
#endif

  /* have at least 2 to sort ? */
#ifdef OPTIMIZED_MERGESORT
  if ((last - first) > 1)
#else
  if ((last - first) > 0)
#endif
    {
    median = (first + last) / 2;	/* /2 and trunc to int */
    pwdgen_idx_merge_sort (pwd_list, idx, temp, first, median, len);
    pwdgen_idx_merge_sort (pwd_list, idx, temp, median + 1, last, len);

#ifdef DEBUG_SORT
    printf ("Merging from %i to %i\n",first,last);
#endif
  
    /* merge sorted halves back together by using a temp array */

#ifdef DEBUG_SORT
    printf ("Before merge we have: \n");
    for (index = first; index <= last; index++)
      {
      printf ("  '%s' i %i = idx %i\n", pwd_list + idx[index] * len, index, idx[index]);
      }
#endif
  
    index = 0;
    /* until one of the halves is empty */
    tr1 = first; tr2 = median + 1;
    while ((tr1 <= median) && (tr2 <= last))
      {
#ifdef DEBUG_SORT
      printf ("At %i (tr1 %i tr2 %i) ",index, tr1, tr2); 
#endif
      if (_compare( pwd_list + idx[tr1] * len, pwd_list + idx[tr2] * len, len) <= 0)
        {
#ifdef DEBUG_SORT
      printf ("use tr1 %i\n", tr1); 
#endif

        temp[index] = idx[tr1++];
        }
      else
        {
#ifdef DEBUG_SORT
      printf ("use tr2 %i\n", tr2); 
#endif
        temp[index] = idx[tr2++];
        }
      index++;
      }

    /* now copy anything that is leftover from the non-empty half */

#ifdef DEBUG_SORT
    printf ("Now copy remaining... index %i (tr2 %i <=> last %i <=> tr1 %i)\n",index, tr2,last,tr1);
#endif

    /* right half empty? */
    if (tr2 <= last)
      {
      /* use memcpy since the block is likely large and this is faster */
      memcpy( &temp[index], &idx[tr2], sizeof(unsigned int) * (last - tr2 + 1) );
#ifdef DEBUG_SORT
      index += (last - tr2 + 1);
#endif
      }

    /* left half empty? */
    if (tr1 <= median)
      {
      /* use memcpy since the block is likely large and this is faster */
      memcpy( &temp[index], &idx[tr1], sizeof(unsigned int) * (median - tr1 + 1) );
#ifdef DEBUG_SORT
      index += (median - tr1 + 1);
#endif
      }

#ifdef DEBUG_SORT
    printf ("index is now %i. Now copy temp back\n", index);
#endif

    /* now copy from temp back to pwd_list */
    memcpy( idx, temp, sizeof(unsigned int) * (last - first + 1) );

#ifdef DEBUG_SORT
    printf ("After merge we have: \n");
    for (index = first; index <= last; index++)
      {
      printf ("  '%s' i %i = idx %i\n", pwd_list + idx[index] * len, index, idx[index]);
      }
#endif
    }

  return;					/* sorted now */ 
  }

/***************************************************************************/

/* go through a list, remove all doubles and return nr of leftover strings */
/* slightly faster variant, and working in place */

unsigned long pwdgen_img_remove_doubles_in_place (
  unsigned char* pwd_list,
  unsigned long count,
  unsigned long len		/* len including zero termination or padding! */
  )
  {
  unsigned long x, y;			/* index1, index2 */

  /* zero or one: no doubles possible */
  if (count <= 1)
    {
    return count;
    }

  x = 0; y = 1;			/* first, and second */
  while (y < count)
    {
    if (_compare(&pwd_list[x*len], &pwd_list[y*len], len) != 0)
      {
      /* are not equal, so copy it */
      x++;
      _copy (&pwd_list[ x * len ], &pwd_list[y * len], len);
      }
    y++;
    }
  /* x points to last, so increment to return nr of left over unique strings */
  return x + 1;
  }

/* merge two lists of strings */

/* first: count of strings in first, second: count of strings in second list 
   both lists are in pwd_list directly after another
*/

void pwdgen_img_merge (
  unsigned char* pwd_list,
  unsigned long fh,
  unsigned long sh,
  unsigned long len		/* len including zero termination or padding! */
  )
  {
  unsigned long median, first, last;			/* half */
  unsigned long tr1, tr2, index;	/* index into temp */
  unsigned char* temp;
#ifdef DEBUG_MERGE
  unsigned int j;
#endif

#ifdef DEBUG_MERGE
  printf ("Merging %li and %li\n", first, second);
#endif

  /* nothing to merge? */
  if ((fh + sh) <= 1)
    {
    return;
    }
 
  if ((fh + sh) == 2)
    {
    /* if there are only two, we can merge/sort them faster */
    if (_compare( pwd_list, pwd_list + len, len) > 0)
      {
      _swap( pwd_list, pwd_list + len, len);
      }
    return;
    }

  /* we have at least 3 to merge */
  first = 0;
  median = fh - 1;
  last = fh + sh - 1;

#ifdef DEBUG_MERGE
    printf ("Merging from %li to %li (median at %li)\n", first, last, median);
#endif
  
  /* merge sorted halves back together by using a temp array */

  temp = malloc (len * (fh + sh));
  if (NULL == temp)
    {
    printf ("Error: Cannot allocate %li bytes of memory for merging.\n", 
     len * (fh + sh));
    return;
    }

#ifdef DEBUG_MERGE1
    printf ("Before merge we have: \n");
    for (index = first; index <= last; index++)
      {
      printf ("  '");
      for (j = 0; j < len; j++)
        {
        printf ("%c", pwd_list[index*len+j]);
        }
      printf ("' %li\n", index);
      }
#endif
  
  index = 0;
  /* until one of the halves is empty */
  tr1 = first; tr2 = median + 1;
  while ((tr1 <= median) && (tr2 <= last))
    {
#ifdef DEBUG_MERGE
    printf ("At %li (tr1 %li tr2 %li) ",index, tr1, tr2); 
#endif
    if (_compare( pwd_list + tr1 * len, pwd_list + tr2 * len, len) <= 0)
      {
#ifdef DEBUG_MERGE1
      printf ("use tr1 %li\n", tr1); 
#endif

      _copy( temp + index * len, pwd_list + tr1 * len, len);
      tr1++;
      }
    else
      {
#ifdef DEBUG_MERGE1
      printf ("use tr2 %li\n", tr2); 
#endif
      _copy( temp + index * len, pwd_list + tr2 * len, len);
      tr2++;
      }
    index++;
    }

  /* now copy anything that is leftover from the non-empty half */

#ifdef DEBUG_MERGE
  printf ("Now copy remaining... index %li (tr2 %li <=> last %li <=> tr1 %li)\n",index, tr2,last,tr1);
#endif

  /* right half empty? */
  if (tr2 <= last)
    {
    /* use memcpy since the block is likely large and this is faster */
    memcpy( temp + index * len, pwd_list + tr2 * len, len * (last - tr2 + 1) );
#ifdef DEBUG_MERGE
    index += (last - tr2 + 1);
#endif
    }

  /* left half empty? */
  if (tr1 <= median)
    {
    /* use memcpy since the block is likely large and this is faster */
    memcpy( temp + index * len, pwd_list + tr1 * len, len * (median - tr1 + 1));
#ifdef DEBUG_MERGE
    index += (median - tr1 + 1);
#endif
    }

#ifdef DEBUG_MERGE
    printf ("index is now %li. Now copy temp back\n", index);
#endif

  /* now copy from temp back to pwd_list */
  memcpy( pwd_list, temp, len * (last - first + 1) );

#ifdef DEBUG_MERGE1
  printf ("After merge we have: \n");
  for (index = first; index <= last; index++)
    {
    printf ("  '%s' %li\n", pwd_list + index * len, index);
    }
#endif

  /* free the scratch memory */
  free (temp);
  return;					/* merged now */ 
  }



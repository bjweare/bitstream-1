#ifndef _IFUNCTIONS_H_
#define _IFUNCTIONS_H_

#include <math.h>
#include <limits.h>

static inline int imin(int a, int b)
{
  return ((a) < (b)) ? (a) : (b);
}

static inline int imax(int a, int b)
{
  return ((a) > (b)) ? (a) : (b);
}

static inline int iabs(int a)
{
	return ((a) >= 0) ? (a) : ((a)*(-1));
}

#endif

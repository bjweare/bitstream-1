#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <stdlib.h>

void error(char *text, int code)
{
  fprintf(stderr, "%s\n", text);

  exit(code);
}


#endif

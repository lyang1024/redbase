#include <unistd.h>
#include <sys/types.h>
#include "pf.h"
#include <stdlib.h>
#include <cstdio>
#include <cstring>

// return 0 if not overlap, 1 if overlap
static int overlap_rect(void *value1, void *value2){
	Rect *rt1 = *(Rect*)value1;
	Rect *rt2 = *(Rect*)value2;
	if (rt1->x1 > rt2->x2 || rt2->x1 > rt1->x2) return 0;
	if (rt1->y2 > rt2->y1 || rt2->y2 > rt1->y1) return 0;
	return 1;
}

static int compare_string(void *value1, void* value2, int attrLength){
  return strncmp((char *) value1, (char *) value2, attrLength);
}

static int compare_int(void *value1, void* value2, int attrLength){
  if((*(int *)value1 < *(int *)value2))
    return -1;
  else if((*(int *)value1 > *(int *)value2))
    return 1;
  else
    return 0;
}

static int compare_float(void *value1, void* value2, int attrLength){
  if((*(float *)value1 < *(float *)value2))
    return -1;
  else if((*(float *)value1 > *(float *)value2))
    return 1;
  else
    return 0;
}

static bool print_string(void *value, int attrLength){
  char * str = (char *)malloc(attrLength + 1);
  memcpy(str, value, attrLength+1);
  str[attrLength] = '\0';
  printf("%s ", str);
  free(str);
  return true;
}

static bool print_int(void *value, int attrLength){
  int num = *(int*)value;
  printf("%d ", num);
  return true;
}

static bool print_float(void *value, int attrLength){
  float num = *(float *)value;
  printf("%f ", num);
  return true;
}

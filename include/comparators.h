#include <unistd.h>
#include <sys/types.h>
#include "pf.h"
#include "redbase.h"
#include "parser.h"
#include "mbr.h"
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

// return 0 if not overlap, positive number if overlap

float get_overlap(void *value1, void *value2){
	struct MBR rt1 = *(struct MBR*)value1;
	struct MBR rt2 = *(struct MBR*)value2;
	if (rt1.x1 > rt2.x2 || rt2.x1 > rt1.x2) return 0;
	if (rt1.y2 < rt2.y1 || rt2.y2 < rt1.y1) return 0;
    float e1 = min(rt1.x2, rt2.x2) - max(rt1.x1, rt2.x1);
	float e2 = min(rt1.y2, rt2.y2) - max(rt1.y1, rt2.y2);
	return e1*e2;
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

static bool print_mbr(void *value, int attrLength){
  struct MBR temp = *(struct MBR*)value;
  printf("[%f,%f]-[%f,%f]",temp.x1,temp.y1,temp.x2,temp.y2);
  return true;
}


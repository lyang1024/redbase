#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <string>
#include <cfloat>
#include "mbr.h"
using namespace std;
ostream &operator<<(ostream &s, const MBR &m)
{
    s << "[" << m.x1 << "," << m.y1 <<"]\n";
	s << "[" << m.x2 << "," << m.y2 <<"]\n";
	return s;
}

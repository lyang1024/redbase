#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <cfloat>

struct MBR{
	float position[4];
	friend std::ostream &operator<<(std::ostream &s, const MBR &mbr);
	friend std::sstream &operator>>(std::sstream &s, const MBR &mbr);
}

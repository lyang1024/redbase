#ifndef MBR_H
#define MBR_H

struct MBR{
	float x1;
	float x2;
	float y1;
	float y2;
	//friend std::ostream &operator<<(std::ostream &s, const MBR &mbr);
	//float getArea();
	//friend std::sstream &operator>>(std::sstream &s, const MBR &mbr);
};

#endif

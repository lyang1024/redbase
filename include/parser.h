//
// parser.h
//   Parser Component Interface
//

#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "redbase.h"
#include "pf.h"

//
// Structure declarations and output functions
//
struct AttrInfo{
    char     *attrName;   /* attribute name       */
    AttrType attrType;    /* type of attribute    */
    int      attrLength;  /* length of attribute  */
};

struct RelAttr{
    char     *relName;    // Relation name (may be NULL)
    char     *attrName;   // Attribute name

    // Print function
    friend std::ostream &operator<<(std::ostream &s, const RelAttr &ra);
};

struct Value{
    AttrType type;         /* type of value               */
    void     *data;        /* value                       */
			   /* print function              */
    friend std::ostream &operator<<(std::ostream &s, const Value &v);
};

struct Point{
	//AttrType type;
	double x;
	double y;
	//public:
	Point(double xval, double yval) {
		x = xval;
		y = yval;
	}
	friend std::ostream &operator<<(std::ostream &s, const Point &p);
};

struct Rect {
	double x1;
	double y1;
	double x2;
	double y2;
	Rect(double x1val, double y1val, double x2val, double y2val) {
		x1 = x1val;
		y1 = y1val;
		x2 = x2val;
		y2 = y2val;
	}
	friend std::ostream &operator<<(std::ostream &s, const Rect &rt);

};

struct Condition{
    RelAttr  lhsAttr;    /* left-hand side attribute            */
    CompOp   op;         /* comparison operator                 */
    int      bRhsIsAttr; /* TRUE if the rhs is an attribute,    */
                         /* in which case rhsAttr below is valid;*/
                         /* otherwise, rhsValue below is valid.  */
    RelAttr  rhsAttr;    /* right-hand side attribute            */
    Value    rhsValue;   /* right-hand side value                */
			 /* print function                               */
    friend std::ostream &operator<<(std::ostream &s, const Condition &c);

};

std::ostream &operator<<(std::ostream &s, const CompOp &op);
std::ostream &operator<<(std::ostream &s, const AttrType &at);

//
// Parse function
//
class QL_Manager;
class SM_Manager;

void RBparse(PF_Manager &pfm, SM_Manager &smm, QL_Manager &qlm);

//
// Error printing function; calls component-specific functions
//
void PrintError(RC rc);

// bQueryPlans is allocated by parse.y.  When bQueryPlans is 1 then the
// query plan chosen for the SFW query will be displayed.  When
// bQueryPlans is 0 then no query plan is shown.
extern int bQueryPlans;

#endif

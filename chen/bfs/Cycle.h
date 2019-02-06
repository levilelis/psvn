/*
 * Cycle.h
 *
 *  Created on: Nov 5, 2013
 *      Author: levilelis
 */

#ifndef CYCLE_H_
#define CYCLE_H_

#include "../Type.h"


class Cycle {

private:

	int d; // depth that the cycle occurs
	int l; // length of the cycle

	Type t; //type of the node in which the cycle is rooted at.

	// we store the type of the transpositions encountered for a given type.
	// when the cycle has an even length we store two types, but only one is necessary when
	// the cycle has an odd length.
	Type t1;
	Type t2;

public:

	Cycle(int d, int l)
	{
		this->d = d;
		this->l = l;
	}

	void setD(int d) { this->d = d; }
	void setL(int l) { this->l = l; }
	void setType1(Type t) { this->t1 = t; }
	void setType2(Type t) { this->t2 = t; }
	void setType(Type t) { this->t = t; }
	int getD() const { return d; }
	int getL() const { return l; }
	Type getType() const { return t; }
	Type getType1() const { return t1; }
	Type getType2() const { return t2; }
};

bool operator< (const Cycle& o1, const Cycle& o2)
{
	if(o1.getD() != o2.getD())
	{
		return o1.getD() < o2.getD();
	}

	if(o1.getL() != o2.getL())
	{
		return o1.getL() < o2.getL();
	}

	// equivalent for o1.getType1() != o2.getType1()
	if( !( o1.getType1() < o2.getType1() && o2.getType1() < o1.getType1() ) )
	{
		return o1.getType1() < o2.getType1();
	}

	return o1.getType2() < o2.getType2();
}

#endif /* CYCLE_H_ */

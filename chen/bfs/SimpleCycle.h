/*
 * SimpleCycle.h
 *
 *  Created on: Nov 28, 2013
 *      Author: levilelis
 */

#ifndef SIMPLECYCLE_H_
#define SIMPLECYCLE_H_

class SimpleCycle {

private:
	int depth; // depth of the cycle
	int l; // length of the cycle
	State * state; //state affected by the cycle, i.e., the state proved as transposition
	bool transposition; //if the node has to be pruned right away
	bool duplicate; //if we must divide the node's weight by the number of cycles encountered

public:
	SimpleCycle(int l, bool t, bool d)
	{
		this->transposition = t;
		this->duplicate = d;
		this->l = l;
		state = NULL;
	}

	bool isDuplicate() const
	{
		return duplicate;
	}

	bool isTransposition() const
	{
		return transposition;
	}

	void setIsDuplicate( bool d )
	{
		this->duplicate = d;
	}

	void setIsTransposition( bool t )
	{
		this->transposition = t;
	}

	void setL(int l)
	{
		this->l = l;
	}

	int getL() const
	{
		return l;
	}

	void setDepth(int l)
	{
		this->depth = l;
	}

	int getDepth() const
	{
		return depth;
	}

	State* getState() const
	{
		return state;
	}

	void setState( State* s )
	{
		state = s;
	}
};

bool operator< (const SimpleCycle& o1, const SimpleCycle& o2)
{
	if(o1.getL() != o2.getL())
	{
		return o1.getL() < o2.getL();
	}

	if(o1.getDepth() != o2.getDepth())
	{
		return o1.getDepth() < o2.getDepth();
	}

	return false;
}


#endif /* SIMPLECYCLE_H_ */

#ifndef Type_h__
#define Type_h__

#include <map>

using namespace std;

class Child {
private:
	int level;
	int parentHeuristic;
	int h;

public:
	Child(int level, int parentHeuristic, int h)
	{
		this->level = level;
		this->parentHeuristic = parentHeuristic;
		this->h = h;
	}

	int getLevel() const {return level;}
	int getH() const {return h;}

	friend bool operator!= (const Child& c1, const Child& c2)
	{
		if(c1.level != c2.level)
		{
			return true;
		}

		if(c1.h != c2.h)
		{
			return true;
		}

		return false;
	}

	void print() const
	{
		cout << "(" << level << ", " << parentHeuristic << ", " << h << ") ";
	}

	friend bool operator< (const Child& c1, const Child& c2)
	{
		if(c1.level != c2.level)
		{
			return c1.level < c2.level;
		}

		if(c1.h != c2.h)
		{
			return c1.h < c2.h;
		}

		return false;
	}
};


class TypeChildren {
private :
	map<Child, int> col;

public:
	TypeChildren();
	void addChild(Child h);
	void addChildValue(Child h, int number);
	map<Child, int> getCol() const;
	void print() const;

	friend bool operator< (const TypeChildren&, const TypeChildren&);
	friend class Object2;
};

TypeChildren::TypeChildren()
{

}

map<Child, int> TypeChildren::getCol() const
{
	return this->col;
}

void TypeChildren::addChild(Child c)
{
	map<Child, int>::iterator it = col.find(c);

	if(it == col.end())
	{
		col.insert(pair<Child, int>(c, 1));
	}
	else
	{
		++col[c];
	}
}

void TypeChildren::print() const
{
	map<Child, int>::const_iterator it = col.begin();
	for( ; it != col.end(); ++it )
	{
		it->first.print();
		cout << " = " << it->second << endl;
	}
}

void TypeChildren::addChildValue(Child h, int number)
{
	col.insert(pair<Child, int>(h, number));
}

bool operator< (const TypeChildren& o1, const TypeChildren& o2)
{
	if(o1.col.size() != o2.col.size())
	{
		return o1.col.size() < o2.col.size();
	}

	map<Child, int>::const_iterator it1 = o1.col.begin();
	map<Child, int>::const_iterator it2 = o2.col.begin();

	for(; it1 != o1.col.end(); ++it1, ++it2)
	{
		if(it1->first != it2->first)
		{
			return it1->first < it2->first;
		}

		if(it1->second != it2->second)
		{
			return it1->second < it2->second;
		}
	}

	return false;
}

class Type {

private:
	TypeChildren children;
	int p;
	long h;
	long level;

public:

	Type();
	Type(int parent, long heuristic);

	void addAddtionalInfo(int);
	TypeChildren& getChildren() {return children;}
	TypeChildren getConstChildren() const {return children;}
	void setChildren(TypeChildren c){this->children = c;}

	friend bool operator< (const Type&, const Type&);

	long getH() const {return h;}
	void setH(long i) {h = i;}
	int getP() const {return p;}
	void setP(int i) {p = i;}
	long getLevel() const {return level;}
	void setLevel(long i) {level = i;}
	void print() const;
	
	static int lookahead;
};

int Type::lookahead = 0;

Type::Type(int parent, long heuristic)
{
	this->p      = parent;
	this->h      = heuristic;
	this->level  = -1;
}

Type::Type()
{
	this->p      = -1;
	this->h      = -1;
	this->level  = -1;
}

void Type::print() const
{
	cout << "[" << level << ", " << p << ", " << h << "] " << endl;
	children.print();
}

bool operator< (const Type& o1, const Type& o2)
{
	if(o1.level != o2.level)
	{
		return o1.level < o2.level;
	}

	if(o1.h != o2.h)
	{
		return o1.h < o2.h;
	}

	if(o1.p != o2.p)
	{
		return o1.p < o2.p;
	}

	return o1.children < o2.children;
}

#endif


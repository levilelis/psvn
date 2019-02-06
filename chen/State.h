/*
 * State.h
 *
 *  Created on: Dec 29, 2011
 *      Author: levilelis
 */

#ifndef STATE_H_
#define STATE_H_

using namespace std;

class State {

private:
	state_t state;
	State* pred;
	double w;
	int history;
	int rule; // the operator used to generate this state
	int g;

public:

	State()	{}
	~State() {}
	state_t * getState() {return &state;}
	State*  getPred() {return pred;}
	double getW() {return w;}
	int getHistory() {return history;}
	int getG() { return g; }
	void setG( int g ) { this->g = g; }
	int getRule() { return this->rule; }
	void setRule( int r ) { this->rule = r; }

	void setState(const state_t s)
	{
		//copy_state(&state, s);
		state = s;
	}

	void setPred(State* s)
	{
		//copy_state(&pred, s);
		pred = s;
	}

	void setW(double w)
	{
		this->w = w;
	}

	void setHistory(int h)
	{
		this->history = h;
	}

	State(const State& s)
	{
		//copy_state(&state, &(s.state));
		//copy_state(&pred, &(s.pred));
		state = s.state;
		pred = s.pred;

		this->rule = s.rule;
		this->w = s.w;
		this->history = s.history;
	}

    const State& operator = (const State& s)
    {
           if (&s != this)
           {
               //copy_state(&state, &(s.state));
               //copy_state(&pred, &(s.pred));
        	   state = s.state;
               pred = s.pred;

               this->rule = s.rule;
               this->w = s.w;
               this->history = s.history;
           }

           return *this;
    }

};


#endif /* STATE_H_ */

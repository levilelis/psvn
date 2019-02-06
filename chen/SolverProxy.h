/*
 * SolverProxy.h
 *
 *  Created on: Dec 11, 2013
 *      Author: levilelis
 */

#ifndef SOLVERPROXY_H_
#define SOLVERPROXY_H_

#include <fstream>
#include <vector>
#include <map>

using namespace std;

struct NodesTime {
	long nodes;
	double time;
};

class SolverProxy {

public:
	SolverProxy(string filename);
	~SolverProxy();
	map<int, NodesTime>& solve();

private:
	map<int, NodesTime> nodesExpandedByLevel;
	ifstream in;

	void getTokens(char*, vector<char*>&);
};

SolverProxy::SolverProxy(string filename)
{
	this->in.open(filename.c_str());

	if(!in)
	{
		cout << "Could not open the proxy file!" << endl;
		exit(-1);
	}

	vector<char*> tokens;
	char line[200000];
	in.getline(line, 200000);
	getTokens(line, tokens);

	while(true)
	{
		//cout << line << endl;

		if(tokens.size() > 0 && strcmp(tokens[0], "problem") == 0)
		{
			break;
		}

		tokens.clear();
		in.getline(line, 200000);
		getTokens(line, tokens);
	}
}

SolverProxy::~SolverProxy()
{
	this->in.close();
}

map<int, NodesTime>& SolverProxy::solve()
{
	char line[200000];

	nodesExpandedByLevel.clear();

	vector<char*> tokens;
	while (!in.eof())
	{
		in.getline(line, 200000);
		//cout << line << endl;

		getTokens(line, tokens);

		//cout << "Tokens size: " << tokens.size() << endl;

		if(tokens.size() == 0)
		{
			tokens.clear();
			continue;
		}

		if(strcmp(tokens[0], "problem") == 0)
		{
			return nodesExpandedByLevel;
		}

		if(tokens.size() == 3)
		{
			NodesTime nodes_time;
			int depth = atoi(tokens[0]);
			nodes_time.nodes = atol(tokens[1]);
			nodes_time.time = atof(tokens[2]);
			nodesExpandedByLevel[depth] = nodes_time;

			//cout << "f-cost: " << depth << " nodes: " << nodes_time.nodes << " time: " << nodes_time.time << endl;
			tokens.clear();
		}

		tokens.clear();
	}

	return nodesExpandedByLevel;
}

void SolverProxy::getTokens(char * line, vector<char*> &tokens)
{
	char * token;
	token = strtok (line, " ");

	while (token != NULL)
	{
//		cout << token << " ";
		tokens.push_back(token);
		token = strtok (NULL, " ");
	}

//	cout << endl;
}


#endif /* SOLVERPROXY_H_ */

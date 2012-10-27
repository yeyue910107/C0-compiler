#include<fstream>
#include<iostream>
#include<string>
#include<cctype>
#include<map>
#include<vector>
#include<set>
#include<list>

using namespace std;

class DagNode {
public:
	int index, lnode, rnode, isLeaf, isInDeque;
	string op, id;
	vector<string> ids;
	vector<int> pnodes;
	DagNode() { }
	DagNode(string s, int n, int b) { op = s; index = n; isLeaf = b; isInDeque = lnode = rnode = 0; }
	~DagNode() { }
};

class Dag {
public:
	vector<DagNode> dagNodes;
	Dag() { }
	~Dag() { }
};

class ConflictGraphNode {
public:
	string name;
	int conflictNum, globalReg;
	set<string> conflictNodes;
	ConflictGraphNode() { }
	ConflictGraphNode(string s) { name = s; conflictNum = 0; globalReg = 0; }
	~ConflictGraphNode() { }
};

class ConflictGraph {
public:
	list<ConflictGraphNode> cgNodes;
	ConflictGraph() { }
	~ConflictGraph() { }
};

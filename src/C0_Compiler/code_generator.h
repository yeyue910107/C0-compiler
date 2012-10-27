#include<fstream>
#include<iostream>
#include<string>
#include<cctype>
#include<map>
#include<vector>
#include<set>
#include<list>
#include "syntax_parser.h"
#include "code_optimization.h"
//#include "lexical_analyzer.h"
//#include "error.h"
using namespace std;

class BasicBlock {
public:
	string proc;
	static int count;
	int index, quadNum;
	Dag dag;
	set<string> use, def, in, out;
	vector<Quadruple> quads;
	vector<int> predBlock;
	vector<int> subBlock;
	BasicBlock();
	BasicBlock(string s);
	~BasicBlock() { }
	int isInSet(set<string> v, string s);
	int isVariable(string s);
	int isTemp(string s);
	void getDAG();
	void dagToQuad();
	void dagToQuad(DagNode &node, vector<int> &nodes);
	void getDefAndUse();
	void getIn();
	void optimize();
	void output();
};

class Proc {
public:
	string name;
	Type type;
	int quadNum, tempNum, blockNum;
	Quadruple entryQuad;
	Table table;
	vector<BasicBlock> basicBlocks;
	vector<Quadruple> quads;
	set<string> vars, paramsSet, consts;
	vector<string> params;
	map<string, int> globalRegs;
	Proc() { }
	~Proc() { }
	void init(Parser p, string s);
	void getBasicBlocks();
	void dataflowAnalyze();
	void globalAlloc();
	void getConflictGraph(ConflictGraph &graph);
	void colorOnGraph(ConflictGraph &graph);
	void optimize();
	void output();
};

class Asm {
public:
	string op, dst, src, info;
	Asm() { }
	~Asm() { }
	Asm(string s) { info = s; }
	Asm(string s1, string s2, string s3) { op = s1; dst = s2; src = s3; }
	string toString();
};

class Generator {
public:
	int paramsNum, stackTop;
	Table globalTable;
	Proc curProc;
	BasicBlock curBlock;
	vector<Proc> procs;
	list<Asm> asms;
	map<string, string> globalVars, regDescriptor, addrDescriptor;
	Generator();
	~Generator() { }
	void init(Parser p);
	void init_n_optimize(Parser p);
	void gen();
	string getReg(Quadruple q, int index);
	string getAddr(string s);
	string getMem(string s);
	int getType(string s);
	int isImmediate(string s);
	string isConst(string s);
	void transform(string &s);
	int isUse(string s, int n);
	int isLive(string s);
	//int isInReg(string s);
	int isInMem(string s);
	string allocMem(string s);
	void updateRegDescriptor(string reg, string var);
	void updateAddrDescriptor(string addr, string var);
	void specifyReg(string reg, string op, string s);
	void releaseRegPool(string s, int index);
	void clearRegPool();
	void saveLiveVar();
	void saveRegVar(int index);
	void output();
};

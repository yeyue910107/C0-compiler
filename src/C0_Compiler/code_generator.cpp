#pragma warning(disable:4786)
#include<fstream>
#include<ostream>
#include<iostream>
#include<string>
#include<cctype>
#include<map>
#include<vector>
#include<set>
#include<list>
#include "code_generator.h"

using namespace std;

string intToString0(int n) {
	string s = "";
	int m = 1, temp = n, i = 0;
	while (temp > 0) {
		temp /= 10;
		m *= 10;
	}
	while (1) {
		m /= 10;
		if (m == 0)
			break;
		s += " ";
		s[i] = (char)(n / m + 48);
		n %= m;
		i++;
	}
	return s;
}

int BasicBlock::count = 0;

BasicBlock::BasicBlock() {
	index = ++count;
}

BasicBlock::BasicBlock(string s) {
	index = ++count;
	proc = s;
}

//
int BasicBlock::isInSet(set<string> v, string s) {
	set<string>::iterator iter;

	for (iter = v.begin(); iter != v.end(); ++iter) {
		if ((*iter) == s)
			return 1;
	}
	return 0;
}

//是否为变量
int BasicBlock::isVariable(string s) {
	if (s != "" && (isalpha(s[0]) || s[0] == '_'))
		return 1;
	return 0;
}

//是否为临时变量
int BasicBlock::isTemp(string s) {
	if (s != "" && s[0] == '@')
		return 1;
	return 0;
}

//创建DAG图并删除局部公共子表达式
void BasicBlock::getDAG() {
	int nodeIndex = 0, i = 0, j = 0, k = 0;
	map<string, int> nodes;
	vector<Quadruple>::iterator iter1;
	map<string, int>::iterator iter2;
	vector<DagNode>::iterator iter3;
	for (iter1 = quads.begin(); iter1 != quads.end(); ++iter1) {
		if ((*iter1).op == "+" || (*iter1).op == "-" || (*iter1).op == "*" 
			|| (*iter1).op == "/" || (*iter1).op == "=") {
				for (iter2 = nodes.begin(); iter2 != nodes.end(); ++iter2) {
					if ((*iter2).first == (*iter1).expr1) {
						i = (*iter2).second;
					}
					if ((*iter2).first == (*iter1).expr2) {
						j = (*iter2).second;
					}
					if (i != 0 && j != 0)
						break;
				}
				if (i == 0) {
					DagNode node1((*iter1).expr1, ++nodeIndex, 1);
					node1.id = node1.op;
					i = node1.index;
					nodes.insert(map<string, int>::value_type((*iter1).expr1, i));
					node1.ids.push_back(node1.id);
					dag.dagNodes.push_back(node1);
				}
				if ((*iter1).op != "=") {
					if (j == 0) {
						DagNode node2((*iter1).expr2, ++nodeIndex, 1);
						node2.id = node2.op;
						j = node2.index;
						nodes.insert(map<string, int>::value_type((*iter1).expr2, j));
						node2.ids.push_back(node2.id);
						dag.dagNodes.push_back(node2);
					}
					for (iter3 = dag.dagNodes.begin(); iter3 != dag.dagNodes.end(); ++iter3) {
						if ((*iter3).op == (*iter1).op && i == (*iter3).lnode && j == (*iter3).rnode) {
							k = (*iter3).index;
							break;
						}
					}
					if (k == 0) {
						DagNode node3((*iter1).op, ++nodeIndex, 0);
						node3.id = (*iter1).res;
						k = node3.index;
						node3.lnode = i;
						node3.rnode = j;
						//node3.ids.push_back(node3.id);
						dag.dagNodes.push_back(node3);
						dag.dagNodes[i - 1].pnodes.push_back(node3.index);
						dag.dagNodes[j - 1].pnodes.push_back(node3.index);
					}
				}
				else {
					k = i;
				}
				for (iter2 = nodes.begin(); iter2 != nodes.end(); ++iter2) {
					if ((*iter1).res == (*iter2).first) {
						(*iter2).second = k;
						dag.dagNodes[k - 1].ids.push_back((*iter1).res);
						break;
					}
				}
				if (iter2 == nodes.end()) {
					nodes.insert(map<string, int>::value_type((*iter1).res, k));
					dag.dagNodes[k - 1].ids.push_back((*iter1).res);
				}
		}
		i = j = k = 0;
	}
}

//从DAG图导出四元式
void BasicBlock::dagToQuad() {
	/*vector<DagNode> nodes;
	vector<DagNode>::iterator iter;
	quads.clear();
	for (iter = dag.dagNodes.begin(); iter != dag.dagNodes.end(); ++iter) {
		dagToQuad(*iter);
	}*/
	vector<int> nodes;
	vector<DagNode>::iterator iter0, iter1;
	vector<int>::reverse_iterator iter2;
	vector<string>::iterator iter3;
	quads.clear();
	DagNode node;
	int flag1 = 1, flag2;
	while (flag1) {
		for (iter1 = dag.dagNodes.begin(); iter1 != dag.dagNodes.end(); ++iter1) {
			dagToQuad(*iter1, nodes);
		}
		for (flag1 = 0, iter0 = dag.dagNodes.begin(); iter0 != dag.dagNodes.end(); ++iter0) {
			if (!((*iter0).isInDeque) && !((*iter0).isLeaf)) {
				flag1 = 1;
				break;
			}
		}
	}
	for (iter2 = nodes.rbegin(); iter2 != nodes.rend(); ++iter2) {
		flag2 = 0;
		node = dag.dagNodes[*iter2 - 1];
		for (iter3 = node.ids.begin(); iter3 != node.ids.end(); ++iter3) {
			if ((*iter3)[0] != '@') {
				if (!flag2) {
					node.id = *iter3;
					flag2 = 1;
				}
				Quadruple q1;
				q1.op = node.op;
				q1.res = *iter3;
				q1.expr1 = dag.dagNodes[node.lnode - 1].id;
				q1.expr2 = dag.dagNodes[node.rnode - 1].id;
				q1.quadstring = q1.res + " = " + q1.expr1 + " " + q1.op + " " + q1.expr2;
				quads.push_back(q1);
			}
		}
		if (!flag2) {
			Quadruple q2;
			iter3 = node.ids.begin();
			q2.op = node.op;
			q2.res = *iter3;
			q2.expr1 = dag.dagNodes[node.lnode - 1].id;
			q2.expr2 = dag.dagNodes[node.rnode - 1].id;
			q2.quadstring = q2.res + " = " + q2.expr1 + " " + q2.op + " " + q2.expr2;
			quads.push_back(q2);
		}
	}
}

void BasicBlock::dagToQuad(DagNode &node, vector<int> &nodes) {
	/*if (!node.isLeaf) {
		string expr1, expr2;
		if (node.lnode != 0) {
			expr1 = dag.dagNodes[node.lnode - 1].id;
			if (!dag.dagNodes[node.lnode - 1].isInDeque) {
				dagToQuad(dag.dagNodes[node.lnode - 1]);
			}
		}
		if (node.rnode != 0) {
			expr2 = dag.dagNodes[node.rnode - 1].id;
			if (!dag.dagNodes[node.rnode - 1].isInDeque) {
				dagToQuad(dag.dagNodes[node.lnode - 1]);
			}
		}
		Quadruple q;
		q.op = node.op; q.expr1 = expr1; q.expr2 = expr2; q.res = node.id;
		quads.push_back(q);
	}
	node.isInDeque = 1;*/
	vector<int>::iterator iter;
	if (!node.isLeaf && !node.isInDeque) {
		if (node.pnodes.size() > 0) {
			for (iter = node.pnodes.begin(); iter != node.pnodes.end(); ++iter) {
				if (!dag.dagNodes[*iter - 1].isInDeque)
					break;
			}
			if (iter != node.pnodes.end())
				return ;
			nodes.push_back(node.index);
			node.isInDeque = 1;
			dagToQuad(dag.dagNodes[node.lnode - 1], nodes);
		}
		else {
			nodes.push_back(node.index);
			node.isInDeque = 1;
			dagToQuad(dag.dagNodes[node.lnode - 1], nodes);
		}
	}
}

//计算基本块的def集和use集
void BasicBlock::getDefAndUse() {
	vector<Quadruple>::iterator iter;

	for (iter = quads.begin(); iter != quads.end(); ++iter) {
		if (isVariable((*iter).expr1) && !isInSet(def, (*iter).expr1)) {
			use.insert((*iter).expr1);
		}
		if (isVariable((*iter).expr2) && !isInSet(def, (*iter).expr2)) {
			use.insert((*iter).expr2);
		}
		if (isVariable((*iter).res) && !isInSet(use, (*iter).res)) {
			def.insert((*iter).res);
		}
	}
}

//计算基本块的in集
void BasicBlock::getIn() {
	//set<string> temp;
	set<string>::iterator iter1, iter2;
	in.clear();
	for (iter1 = out.begin(); iter1 != out.end(); ++iter1) {
		in.insert(*iter1);
	}
	//in.insert(out.begin(), out.end());
	for (iter1 = in.begin(); iter1 != in.end(); ++iter1) {
		for (iter2 = def.begin(); iter2 != def.end(); ++iter2) {
			if (*iter1 == *iter2) {
				//temp.insert(*iter1);
				in.erase(*iter1);
				iter1 = in.begin();
				break;
			}
		}
		if (in.size() == 0)
			break;
	}
	//in.erase(temp.begin(), temp.end());
	for (iter1 = use.begin(); iter1 != use.end(); ++iter1) {
		in.insert(*iter1);
	}
	//in.insert(use.begin(), use.end());
}

void BasicBlock::optimize() {
	getDAG();
	dagToQuad();
}

//过程初始化并划分基本块和流图
void Proc::init(Parser p, string s) {
	map<string, Id>::iterator iter1;
	vector<string>::iterator iter2;
	name = s;
	table = *(p.tables.get(name));
	quads = table.quad;
	tempNum = table.tempNum;
	//tempNum = table.temps.size();
	for (iter1 = table.table.begin(); iter1 != table.table.end(); ++iter1) {
		if ((*iter1).second.kind == CONSTANT)
			consts.insert((*iter1).first);
		else if ((*iter1).second.kind == VAR)
			vars.insert((*iter1).first);
		else {
			//params.push_back((*iter).first);
			paramsSet.insert((*iter1).first);
		}
	}
	for (iter2 = table.params.begin(); iter2 != table.params.end(); ++iter2)
		params.push_back(*iter2);
	entryQuad = quads[0];
	getBasicBlocks();
}

//建立基本块和流图
void Proc::getBasicBlocks() {
	int i = 0, j = 0, flag = 0, k = 0;
	//vector<Quadruple> entryQuads;
	BasicBlock::count = 0;
	BasicBlock block0(name);
	entryQuad.blockIndex = ++i;
	block0.quads.push_back(entryQuad);
	basicBlocks.push_back(block0);
	//entryQuads.push_back(entryQuad);
	vector<Quadruple>::iterator iter = quads.begin();
	++iter; ++k;
	while (iter != quads.end()) {
		if ((*iter).label != 0 || (*iter).op == ">=" || (*iter).op == ">" 
			|| (*iter).op == "<=" || (*iter).op == "<" || (*iter).op == "!="
			|| (*iter).op == "==" || (*iter).op == "goto") {
			if ((*iter).label != 0) {
				basicBlocks[j].quadNum = k; k = 0;
				BasicBlock block1(name);
				(*iter).blockIndex = ++i;
				block1.quads.push_back(*iter);
				basicBlocks.push_back(block1);
				++j;
				flag = 1;
				//entryQuads.push_back(*iter);
			}
			if ((*iter).op == ">=" || (*iter).op == ">" || (*iter).op == "<=" 
			|| (*iter).op == "<" || (*iter).op == "!=" || (*iter).op == "==" 
			|| (*iter).op == "goto") {
				if (!flag) {
					(*iter).blockIndex = i;
					basicBlocks[j].quads.push_back(*iter);
					flag = 0;
				}
				++iter; ++k;
				basicBlocks[j].quadNum = k; k = 0;
				BasicBlock block2(name);
				(*iter).blockIndex = ++i;
				block2.quads.push_back(*iter);
				basicBlocks.push_back(block2);
				++j;
				//entryQuads.push_back(*iter);
			}
			++iter; ++k;
			flag = 0;
			continue;
		}
		++k;
		(*iter).blockIndex = i;
		basicBlocks[j].quads.push_back(*iter);
		++iter;
		flag = 0;
	}
	basicBlocks[j].quadNum = k;
	blockNum = basicBlocks.size();
	vector<BasicBlock>::iterator iter1, iter2;
	int size, temp;
	for (iter1 = basicBlocks.begin(); iter1 != basicBlocks.end(); ++iter1) {
		size = (*iter1).quadNum;
		if ((*iter1).quads[size - 1].jumpTo != 0) {
			for (iter2 = basicBlocks.begin(); iter2 != basicBlocks.end(); ++iter2) {
				if ((*iter2).quads[0].label == (*iter1).quads[size - 1].jumpTo) {
					(*iter2).predBlock.push_back((*iter1).index);
					(*iter1).subBlock.push_back((*iter2).index);
				}
			}
			if ((*iter1).quads[size - 1].op != "goto") {
				temp = (*iter1).index + 1;
				if (temp <= blockNum) {
					(*iter1).subBlock.push_back(temp);
					basicBlocks[temp - 1].predBlock.push_back(temp - 1);
				}
			}
		}
		else {
			temp = (*iter1).index + 1;
			if (temp <= blockNum) {
				(*iter1).subBlock.push_back(temp);
				basicBlocks[temp - 1].predBlock.push_back(temp - 1);
			}
		}
	}
	
}

//数据流分析
void Proc::dataflowAnalyze() {
	vector<BasicBlock>::iterator iter1;
	vector<BasicBlock>::reverse_iterator iter2;
	vector<int>::iterator iter3;
	set<string>::iterator iter4;
	int flag, size;

	for (iter1 = basicBlocks.begin(); iter1 != basicBlocks.end(); ++iter1) {
		(*iter1).getDefAndUse();
	}
	do {
		flag = 0;
		for (iter2 = basicBlocks.rbegin(); iter2 != basicBlocks.rend(); ++iter2) {
			size = (*iter2).in.size();
			for (iter3 = (*iter2).subBlock.begin(); iter3 != (*iter2).subBlock.end(); ++iter3) {
				for (iter4 = basicBlocks[*iter3 - 1].in.begin(); iter4 != basicBlocks[*iter3 - 1].in.end(); ++iter4) {
					(*iter2).out.insert(*iter4);
				}
				//(*iter2).out.insert(basicBlocks[*iter3 - 1].in.begin(), basicBlocks[*iter3 - 1].in.end());
			}
			(*iter2).getIn();
			if (size != (*iter2).in.size())
				flag = 1;
		}
	}
	while (flag == 1);
}

//全局寄存器分配
void Proc::globalAlloc() {
	ConflictGraph graph;
	list<ConflictGraphNode>::iterator iter;
	getConflictGraph(graph);
	colorOnGraph(graph);
	for (iter = graph.cgNodes.begin(); iter != graph.cgNodes.end(); ++iter) {
		if ((*iter).globalReg > 0) {
			globalRegs.insert(map<string, int>::value_type((*iter).name, (*iter).globalReg));
		}
	}
}

//建立冲突图
void Proc::getConflictGraph(ConflictGraph &graph) {
	set<string> liveVars;
	set<string>::iterator iter1;
	vector<BasicBlock>::iterator iter2;
	set<string>::iterator iter3, iter5;
	list<ConflictGraphNode>::iterator iter4;
	
	for (iter1 = vars.begin(); iter1 != vars.end(); ++iter1) {
		ConflictGraphNode cgNode(*iter1);
		graph.cgNodes.push_back(cgNode);
	}

	for (iter2 = basicBlocks.begin(); iter2 != basicBlocks.end(); ++iter2) {
		liveVars.clear();
		for (iter5 = (*iter2).out.begin(); iter5 != (*iter2).out.end(); ++iter5) {
			liveVars.insert(*iter5);
		}
		for (iter5 = (*iter2).def.begin(); iter5 != (*iter2).def.end(); ++iter5) {
			liveVars.insert(*iter5);
		}
		for (iter5 = (*iter2).use.begin(); iter5 != (*iter2).use.end(); ++iter5) {
			liveVars.insert(*iter5);
		}
		/*liveVars.insert((*iter2).out.begin(), (*iter2).out.end());
		liveVars.insert((*iter2).def.begin(), (*iter2).def.end());
		liveVars.insert((*iter2).use.begin(), (*iter2).use.end());*/
		for (iter3 = liveVars.begin(); iter3 != liveVars.end(); ++iter3) {
			for (iter4 = graph.cgNodes.begin(); iter4 != graph.cgNodes.end(); ++iter4) {
				if ((*iter4).name == *iter3) {
					//(*iter4).conflictNodes.insert((*iter2).in.begin(), (*iter2).in.end());
					for (iter5 = liveVars.begin(); iter5 != liveVars.end(); ++iter5) {
						(*iter4).conflictNodes.insert(*iter5);
					}
					//(*iter4).conflictNodes.insert(liveVars.begin(), liveVars.end());
					(*iter4).conflictNodes.erase(*iter3);
					break;
				}
			}
		}
	}
	for (iter4 = graph.cgNodes.begin(); iter4 != graph.cgNodes.end(); ++iter4) {
		(*iter4).conflictNum = (*iter4).conflictNodes.size();
	}
}

//冲突图着色
void Proc::colorOnGraph(ConflictGraph &graph) {
	int i, flag = 0, nodeNum = 0, color[3] = {0}, reg = 0;
	string temp;
	ConflictGraph graph0;
	vector<ConflictGraphNode> nodeList;
	list<ConflictGraphNode>::iterator iter1, iter2;
	set<string>::iterator iter3;
	vector<ConflictGraphNode>::reverse_iterator iter4;
	for (iter1 = graph.cgNodes.begin(); iter1 != graph.cgNodes.end(); ++iter1) {
		graph0.cgNodes.push_back(*iter1);
		nodeNum++;
	}
	//分解冲突图，直到图中仅剩余一个节点
	while (nodeNum > 1) {
		//在冲突图中找到第一个连接边数小于3的节点，从图中移走并记录
		for (iter1 = graph.cgNodes.begin();nodeNum > 1 && iter1 != graph.cgNodes.end(); ++iter1) {
			if ((*iter1).conflictNum < 3) {
				(*iter1).globalReg = 0;
				nodeList.push_back(*iter1);
				temp = (*iter1).name;
				graph.cgNodes.erase(iter1);
				nodeNum--;
				for (iter2 = graph.cgNodes.begin();iter2 != graph.cgNodes.end(); ++iter2) {
					for (iter3 = (*iter2).conflictNodes.begin(); iter3 != (*iter2).conflictNodes.end(); ++iter3) {
						if ((*iter3) == temp) {
							//(*iter2).conflictNodes.erase(iter3);
							(*iter2).conflictNum--;
							break;
						}
					}
				}
				//
				iter1 = graph.cgNodes.begin();
			}
		}
		//选取一个适当的节点，标记为不分配全局寄存器的节点并从图中移走
		if (nodeNum > 1) {
			iter1 = graph.cgNodes.begin();
			(*iter1).globalReg = -1;
			nodeList.push_back(*iter1);
			temp = (*iter1).name;
			graph.cgNodes.erase(iter1);
			nodeNum--;
			for (iter2 = graph.cgNodes.begin();iter2 != graph.cgNodes.end(); ++iter2) {
				for (iter3 = (*iter2).conflictNodes.begin(); iter3 != (*iter2).conflictNodes.end(); ++iter3) {
					if ((*iter3) == temp) {
						//(*iter2).conflictNodes.erase(*iter3);
						(*iter2).conflictNum--;
						break;
					}
				}
			}
		}
	}
	//按照节点被移走的顺序，逆序添加，并给新加入的节点着色
	if (nodeNum > 0) {
		iter1 = graph.cgNodes.begin();
		(*iter1).globalReg = 1;
		for (iter4 = nodeList.rbegin(); iter4 != nodeList.rend(); ++iter4) {
			for (iter3 = (*iter4).conflictNodes.begin(); iter3 != (*iter4).conflictNodes.end(); ++iter3) {
				for (iter1 = graph.cgNodes.begin(); iter1 != graph.cgNodes.end(); ++iter1) {
					if ((*iter1).name == (*iter3) && (*iter1).globalReg > 0) {
						color[(*iter1).globalReg - 1] = 1;
					}
				}
			}
			for (i = 0; i < 3; i++) {
				if (!color[i]) {
					reg = i + 1;
					break;
				}	
			}
			(*iter4).globalReg = reg;
			graph.cgNodes.push_back(*iter4);
			/*for (iter1 = graph.cgNodes.begin(); iter1 != graph.cgNodes.end(); ++iter1) {
				if ((*iter1).name == (*iter4).name) {
					(*iter1).globalReg = reg;
					reg = 0;
					break;
				}
			}*/
			reg = 0;
			for (i = 0; i < 3; i++) {
				color[i] = 0;
			}
		}
	}
}

//过程内部优化：数据流分析-建立冲突图
void Proc::optimize() {
	/*vector<BasicBlock>::iterator iter;
	for (iter = basicBlocks.begin(); iter != basicBlocks.end(); ++iter) {
		(*iter).optimize();
	}*/
	dataflowAnalyze();
	globalAlloc();
}

void Proc::output() {
	vector<Quadruple>::iterator iter;

	cout << name << ":" << endl;
	for (iter = quads.begin(); iter != quads.end(); ++iter) {
		cout << (*iter).quadstring << endl;
	}
}

string Asm::toString() {
	if (info != "") {
		return info;
	}
	if (dst == "" && src == "") {
		return op;
	}
	if (src == "") {
		return (op + "\t" + dst);
	}
	return (op + "\t" + dst + ", " + src);
}

Generator::Generator() {
	BasicBlock::count--;
}

void Generator::init(Parser p) {
	map<string, Table>::iterator iter1;
	map<string, Id>::iterator iter2;

	globalTable = *(p.tables.get("global"));
	for (iter1 = p.tables.tables.begin(); iter1 != p.tables.tables.end(); ++iter1) {
		if ((*iter1).first != "global") {
			Proc proc;
			proc.init(p, (*iter1).first);
			proc.optimize();
			procs.push_back(proc);
			proc.output();
		}
	}
	for (iter2 = globalTable.table.begin(); iter2 != globalTable.table.end(); ++iter2) {
		if ((*iter2).second.kind == VAR)
			globalVars.insert(map<string, string>::value_type((*iter2).first, "0"));
		else {
			globalVars.insert(map<string, string>::value_type((*iter2).first, (*iter2).second.value));
		}
	}
}

void Generator::init_n_optimize(Parser p) {
	map<string, Table>::iterator iter1;
	map<string, Id>::iterator iter2;
	vector<BasicBlock>::iterator iter3;
	set<string>::iterator iter4;
	int i;

	globalTable = *(p.tables.get("global"));
	for (iter1 = p.tables.tables.begin(); iter1 != p.tables.tables.end(); ++iter1) {
		if ((*iter1).first != "global") {
			Proc proc;
			proc.init(p, (*iter1).first);
			for (iter3 = proc.basicBlocks.begin(); iter3 != proc.basicBlocks.end(); ++iter3) {
				(*iter3).out = proc.vars;
				(*iter3).def = proc.vars;
				(*iter3).use = proc.vars;
			}
			//proc.optimize();
			for (i = 0, iter4 = proc.vars.begin(); i < 3 && iter4 != proc.vars.end(); ++iter4, ++i) {
				proc.globalRegs.insert(map<string, int>::value_type(*iter4, i + 1));
			}
			procs.push_back(proc);
			proc.output();
		}
	}
	for (iter2 = globalTable.table.begin(); iter2 != globalTable.table.end(); ++iter2) {
		if ((*iter2).second.kind == VAR)
			globalVars.insert(map<string, string>::value_type((*iter2).first, "0"));
		else {
			globalVars.insert(map<string, string>::value_type((*iter2).first, (*iter2).second.value));
		}
	}
}

//生成X86汇编
void Generator::gen() {
	string dst, s1, s2, op, expr1, expr2, res, jumpTo, temp, place, perfix = "";
	int paramsNum = 0, i, tempSize, index, flag = 0, label, strNum = 0, type;

	asms.push_back(Asm(".386"));
	asms.push_back(Asm(".model flat"));
	asms.push_back(Asm("option casemap: none"));
	asms.push_back(Asm("includelib lib\\msvcrt.lib"));
	asms.push_back(Asm("printf proto c"));
	asms.push_back(Asm("scanf proto c"));
	asms.push_back(Asm(".data"));
	set<string>::iterator sIter;
	map<string, string>::iterator mapIter;
	list<Asm>::iterator asmIter;
	vector<string>::iterator vIter;
	/*for (sIter = globalVars.begin(); sIter != globalVars.end(); ++sIter) {
		asms.push_back(Asm("$" + (*sIter) + " dword 0"));
	}*/
	for (mapIter = globalVars.begin(); mapIter != globalVars.end(); ++mapIter) {
		asms.push_back(Asm("$" + (*mapIter).first + " dword " + (*mapIter).second));
	}
	//insertIndex = asms.size();
	asmIter = asms.end();
	asmIter--;
	asms.push_back(Asm("@figure dword ?"));
	asmIter++;
	asms.push_back(Asm("@figure_format_d byte \"%d\", 0"));
	asms.push_back(Asm("@figure_format_c byte \"%c\", 0"));
	asms.push_back(Asm("@figure_format_f byte \"%f\", 0"));
	asms.push_back(Asm("@space byte 20h, 0"));
	asms.push_back(Asm("@new_line byte 0dh, 0ah, 0"));
	asms.push_back(Asm(""));
	asms.push_back(Asm(".code"));
	vector<Proc>::iterator procIter;
	for (procIter = procs.begin(); procIter != procs.end(); ++procIter) {
		curProc = *procIter;
		stackTop = curProc.vars.size();
		addrDescriptor.clear();
		//paramsNum = curProc.params.size();
		for (i = 0, vIter = curProc.params.begin(); vIter != curProc.params.end(); i++, ++vIter) {
			updateAddrDescriptor("[ebp + " + intToString0((5 + i) * 4) + "]", (*vIter));
		}
		asms.push_back(Asm("_" + curProc.name + " proc"));
		asms.push_back(Asm("push", "ebx", ""));
		asms.push_back(Asm("push", "ebp", ""));
		asms.push_back(Asm("push", "esi", ""));
		asms.push_back(Asm("push", "edi", ""));
		asms.push_back(Asm("mov", "ebp", "esp"));
		tempSize = (curProc.tempNum + curProc.vars.size()) * 4;
		if (tempSize > 0) {
			asms.push_back(Asm("sub", "esp", intToString0(tempSize)));
		}

		vector<BasicBlock>::iterator blockIter;
		for (blockIter = curProc.basicBlocks.begin(); blockIter != curProc.basicBlocks.end(); ++blockIter) {
			flag = 0;
			curBlock = *blockIter;
			clearRegPool();
			vector<Quadruple>::iterator quadIter;
			for (index = 0, quadIter = curBlock.quads.begin(); quadIter != curBlock.quads.end(); ++quadIter, ++index) {
				op = (*quadIter).op;
				expr1 = (*quadIter).expr1;
				expr2 = (*quadIter).expr2;
				res = (*quadIter).res;
				jumpTo = "L" + intToString0((*quadIter).jumpTo);
				label = (*quadIter).label;
				if (label > 0) {
					asms.push_back(Asm("L" + intToString0(label) + ":"));
				}
				if (op == "+" || op == "-" || op == "*" || op == "/" || op == "minus") {
						dst = getReg(*quadIter, index);
						if (!isImmediate(expr1) && isConst(expr1) == "") {
							s1 = getAddr(expr1);
						}
						else if (!isImmediate(expr1)) {
							s1 = isConst(expr1);
						}
						else {
							s1 = expr1;
						}
						if (!isImmediate(expr2) && isConst(expr2) == "") {
							s2 = getAddr(expr2);
						}
						else if (!isImmediate(expr2)) {
							s2 = isConst(expr2);
						}
						else {
							s2 = expr2;
						}
						if (dst != s1) {
							asms.push_back(Asm("mov", dst, s1));
						}
						if (op == "+") {
							asms.push_back(Asm("add", dst, s2));
						}
						if (op == "-") {
							asms.push_back(Asm("sub", dst, s2));
						}
						if (op == "*") {
							asms.push_back(Asm("imul", dst, s2));
						}
						if (op == "minus") {
							asms.push_back(Asm("neg", dst, ""));
						}
						if (op == "/") {
							specifyReg("eax", res, dst);
							dst = "eax";// 将被除数放入eax寄存器中
							if (!isImmediate(expr2)) {// 重新得到除数的地址
								s2 = getAddr(expr2);
							}
							else {
								s2 = expr2;
							}
							specifyReg("ecx", expr2, s2);
							s2 = "ecx";// 将除数放入ecx寄存器中
							temp = (*(regDescriptor.find("edx"))).second;
							if (!isInMem(temp)) {
								place = allocMem(temp);
								if (place[0] != '[')
									asms.push_back(Asm("mov", place, "edx"));
								else
									asms.push_back(Asm("mov", "dword ptr " + place, "edx"));
								updateAddrDescriptor(place, temp);
							}
							updateRegDescriptor("edx", "");
							updateAddrDescriptor("edx", "");
							asms.push_back(Asm("cdq"));
							asms.push_back(Asm("idiv", s2, ""));
						}
						if (dst[0] == 'e') {
							updateRegDescriptor(dst, res);
						}
						updateAddrDescriptor(dst, res);
						if (!isImmediate(expr1)) {
							releaseRegPool(expr1, index);
						}
						if (!isImmediate(expr2)) {
							releaseRegPool(expr2, index);
						}
				}
				else if (op == "=") {
					//如果为立即数
					if (isImmediate(expr1)) {
						dst = getReg(*quadIter, index);
						
						if (dst[0] == '[') {
							asms.push_back(Asm("push", "eax", ""));
							asms.push_back(Asm("mov", "eax", expr1));
							asms.push_back(Asm("mov", "dword ptr " + dst, "eax"));
							asms.push_back(Asm("pop", "eax", ""));
						}
						else {
							asms.push_back(Asm("mov", dst, expr1));
							if (dst[0] == 'e') {
								updateRegDescriptor(dst, res);
							}
						}
						updateAddrDescriptor(dst, res);
					}
					//如果不是立即数
					else {
						s1 = getAddr(expr1);
						if ((s1 == "eax" || s1 == "ecx" || s1 == "edx") && 
							globalVars.find(s1) == globalVars.end() && 
							curProc.globalRegs.find(res) == curProc.globalRegs.end()) {
								if (isUse(expr1, index) && !isInMem(expr1)) {
									temp = allocMem(expr1);
									if (temp[0] != '[')
										asms.push_back(Asm("mov", temp, s1));
									else
										asms.push_back(Asm("mov", "dword ptr " + temp, s1));
									updateAddrDescriptor(temp, expr1);
									updateRegDescriptor(s1, "");
								}
								updateRegDescriptor(s1, res);
								updateAddrDescriptor(s1, res);
						}
						else {
							dst = getReg(*quadIter, index);
							asms.push_back(Asm("mov", dst, s1));
							if (dst[0] == 'e') {
								updateRegDescriptor(dst, res);
							}
							updateAddrDescriptor(dst, res);
						}
					}
				}
				else if (op == "goto") {
					asms.push_back(Asm("jmp", jumpTo, ""));
				}
				else if (op == ">" || op == ">=" || op == "<" || op == "<=" || 
					op == "==" || op == "!=") {
					int flag = 0;
					if (!isImmediate(expr1) && isConst(expr1) == "") {
						s1 = getAddr(expr1);
						if (s1[0] != 'e') {
							temp = getReg(*quadIter, index);
							asms.push_back(Asm("mov", temp, s1));
							updateRegDescriptor(temp, expr1);
							updateAddrDescriptor(temp, expr1);
							s1 = temp;
						}
					}
					else {
						if (!isImmediate(expr1))
							expr1 = isConst(expr1);
						s1 = getReg(*quadIter, index);
						asms.push_back(Asm("mov", s1, expr1));
						flag = 1;
					}
					if (!isImmediate(expr2) && isConst(expr2) == "") {
						s2 = getAddr(expr2);
					}
					else {
						if (!isImmediate(expr2))
							expr2 = isConst(expr2);
						s2 = expr2;
					}
					asms.push_back(Asm("cmp", s1, s2));
					if (flag) {
						updateRegDescriptor(s1, "");
					}
					if (op == ">=") {
						asms.push_back(Asm("jge", jumpTo, ""));
					}
					else if (op == ">") {
						asms.push_back(Asm("jg", jumpTo, ""));
					}
					else if (op == "<=") {
						asms.push_back(Asm("jle", jumpTo, ""));
					}
					else if (op == "<") {
						asms.push_back(Asm("jl", jumpTo, ""));
					}
					else if (op == "==") {
						asms.push_back(Asm("je", jumpTo, ""));
					}
					else if (op == "!=") {
						asms.push_back(Asm("jne", jumpTo, ""));
					}
				}
				else if (op == "param") {
					if (!isImmediate(expr1) && isConst(expr1) == "") {
						s1 = getAddr(expr1);
					}
					else {
						if (!isImmediate(expr1))
							expr1 = isConst(expr1);
						s1 = expr1;
					}
					asms.push_back(Asm("push", s1, ""));
					paramsNum++;
				}
				else if (op == "call") {
					saveRegVar(index);// 寄存器变量存入内存
					asms.push_back(Asm("call", "_" + expr1, ""));
					if (paramsNum != 0) {
						asms.push_back(Asm("add", "esp", intToString0(paramsNum * 4)));
					}
					if (res != "") {
						updateRegDescriptor("eax", res);
						updateAddrDescriptor("eax", res);
						updateRegDescriptor("ecx", "");
						updateAddrDescriptor("ecx", "");
						updateRegDescriptor("edx", "");
						updateAddrDescriptor("edx", "");
					}
					else {
						clearRegPool();// 函数调用完毕后寄存器清空
					}
					paramsNum = 0;
				}
				else if (op == "return") {
					saveRegVar(index);// 寄存器变量存入内存
					if (expr1 != "") {
						if (!isImmediate(expr1) && isConst(expr1) == "") {
							s1 = getAddr(expr1);
						}
						else {
							if (!isImmediate(expr1))
								expr1 = isConst(expr1);
							s1 = expr1;
						}
						asms.push_back(Asm("mov", "eax", s1));
					}
					asms.push_back(Asm("jmp", "@" + curProc.name + "_end", ""));
				}
				else if (op == "scanf") {
					saveRegVar(index);
					type = getType(expr1);
					if (type == CHAR) {
						asms.push_back(Asm("lea", "eax", "@figure"));
						asms.push_back(Asm("push", "eax", ""));
						asms.push_back(Asm("lea", "eax", "@figure_format_c"));
					}
					else if (type == INT) {
						asms.push_back(Asm("lea", "eax", "@figure"));
						asms.push_back(Asm("push", "eax", ""));
						asms.push_back(Asm("lea", "eax", "@figure_format_d"));
					}
					else if (type == FLOAT) {
					
					}
					asms.push_back(Asm("push", "eax", ""));
					asms.push_back(Asm("call", "scanf", ""));
					//asms.push_back(Asm("push", "eax", ""));
					asms.push_back(Asm("add", "esp", "8"));
					asms.push_back(Asm("mov", "eax", "@figure"));
					s1 = allocMem(expr1);
					if (s1[0] != '[')
						asms.push_back(Asm("mov", s1, "eax"));
					else
						asms.push_back(Asm("mov", "dword ptr " + s1, "eax"));
					updateAddrDescriptor(s1, expr1);
					clearRegPool();
				}
				else if (op == "printf") {
					saveRegVar(index);
					if (expr2 == "") {
						if (expr1[0] == '\"') {
							asms.insert(asmIter, Asm("@str" + intToString0(strNum) + " byte " + expr1 
								+ ", 0"));
							//asmIter++;
							//asms.push_back(Asm("@str" + intToString0(strNum) + " byte " + expr1 
							//	+ " ,0"));
							asms.push_back(Asm("lea", "eax", "@str" + intToString0(strNum++)));
							asms.push_back(Asm("push", "eax", ""));
							asms.push_back(Asm("call", "printf", ""));
							asms.push_back(Asm("add", "esp", "4"));
						}
						else {
							type = isImmediate(expr1);
							if (type) {
								s1 = expr1;
								//asms.push_back(Asm("push", s1, ""));
								if (type == CHAR) {
									asms.push_back(Asm("push", s1, ""));
									asms.push_back(Asm("lea", "eax", "@figure_format_c"));
									asms.push_back(Asm("push", "eax", ""));
									asms.push_back(Asm("call", "printf", ""));
									asms.push_back(Asm("add", "esp", "8"));
								}
								else if (type == INT) {
									asms.push_back(Asm("push", s1, ""));
									asms.push_back(Asm("lea", "eax", "@figure_format_d"));
									asms.push_back(Asm("push", "eax", ""));
									asms.push_back(Asm("call", "printf", ""));
									asms.push_back(Asm("add", "esp", "8"));
								}
								else {
									//FLOAT
									asms.insert(asmIter, Asm("@str" + intToString0(strNum) + " byte \"" + expr1 
									+ "\", 0"));
									//asmIter++;
									//asms.push_back(Asm("@str" + intToString0(strNum) + " byte " + expr1 
									//	+ " ,0"));
									asms.push_back(Asm("lea", "eax", "@str" + intToString0(strNum++)));
									asms.push_back(Asm("push", "eax", ""));
									asms.push_back(Asm("call", "printf", ""));
									asms.push_back(Asm("add", "esp", "4"));
									//asms.push_back(Asm())
									/*s1 = allocMem(expr1);
									if (s1[0] != '[')
										asms.push_back(Asm("mov", s1, expr1));
									else
										asms.push_back(Asm("mov", "dword ptr " + s1, expr1));
									//asms.push_back(Asm("push", s1, ""));
									asms.push_back(Asm("lea", "eax", "@figure_format_f"));*/
								}
							}
							else {
								s1 = getAddr(expr1);
								type = getType(expr1);
								asms.push_back(Asm("push", s1, ""));
								if (type == CHAR) {
									asms.push_back(Asm("lea", "eax", "@figure_format_c"));
									asms.push_back(Asm("push", "eax", ""));
									asms.push_back(Asm("call", "printf", ""));
									asms.push_back(Asm("add", "esp", "8"));
								}
								else if (type == INT) {
									asms.push_back(Asm("lea", "eax", "@figure_format_d"));
									asms.push_back(Asm("push", "eax", ""));
									asms.push_back(Asm("call", "printf", ""));
									asms.push_back(Asm("add", "esp", "8"));
								}
								else {
									//FLOAT
									//asms.push_back(Asm("lea", "eax", "@figure_format_f"));
									//asms.insert(++asmIter, Asm("@str" + intToString0(strNum) + " byte " + expr1 
									//+ ", 0"));
									asms.push_back(Asm("lea", "eax", s1));
									asms.push_back(Asm("push", "eax", ""));
									asms.push_back(Asm("call", "printf", ""));
									asms.push_back(Asm("add", "esp", "4"));
								}
							}
							//asms.push_back(Asm("push", s1, ""));
							//asms.push_back(Asm("lea", "eax", "@figure_format_d"));

						}
					}
					else {
						asms.insert(asmIter, Asm("@str" + intToString0(strNum) + " byte " + expr1 
							+ ", 0"));
						//asms.push_back(Asm("@str" + intToString0(strNum) + " byte " + expr1 
						//	+ " ,0"));
						asms.push_back(Asm("lea", "eax", "@str" + intToString0(strNum++)));
						asms.push_back(Asm("push", "eax", ""));
						asms.push_back(Asm("call", "printf", ""));
						asms.push_back(Asm("add", "esp", "4"));
						clearRegPool();
						/*asms.push_back(Asm("lea", "eax", "@space"));
						asms.push_back(Asm("push", "eax", ""));
						asms.push_back(Asm("call", "printf", ""));
						asms.push_back(Asm("add", "esp", "4"));*/
						type = isImmediate(expr2);
						if (type) {
							s2 = expr2;
							asms.push_back(Asm("push", s2, ""));
							if (type == CHAR) {
								asms.push_back(Asm("lea", "eax", "@figure_format_c"));
								asms.push_back(Asm("push", "eax", ""));
								asms.push_back(Asm("call", "printf", ""));
								asms.push_back(Asm("add", "esp", "8"));
							}
							else if (type == INT) {
								asms.push_back(Asm("lea", "eax", "@figure_format_d"));
								asms.push_back(Asm("push", "eax", ""));
								asms.push_back(Asm("call", "printf", ""));
								asms.push_back(Asm("add", "esp", "8"));
							}
							else {
								//FLOAT
								asms.insert(asmIter, Asm("@str" + intToString0(strNum) + " byte \"" + expr2 
									+ "\", 0"));
								asms.push_back(Asm("lea", "eax", "@str" + intToString0(strNum++)));
								asms.push_back(Asm("push", "eax", ""));
								asms.push_back(Asm("call", "printf", ""));
								asms.push_back(Asm("add", "esp", "4"));
							}
						}
						else {
							s2 = getAddr(expr2);
							type = getType(expr2);
							asms.push_back(Asm("push", s2, ""));
							if (type == CHAR) {
								asms.push_back(Asm("lea", "eax", "@figure_format_c"));
								asms.push_back(Asm("push", "eax", ""));
								asms.push_back(Asm("call", "printf", ""));
								asms.push_back(Asm("add", "esp", "8"));
							}
							else if (type == INT) {
								asms.push_back(Asm("lea", "eax", "@figure_format_d"));
								asms.push_back(Asm("push", "eax", ""));
								asms.push_back(Asm("call", "printf", ""));
								asms.push_back(Asm("add", "esp", "8"));
							}
							else {
								//FLOAT
								//asms.insert(++asmIter, Asm("@str" + intToString0(strNum) + " byte " + expr1 
								//	+ ", 0"));
								asms.push_back(Asm("lea", "eax", s2));
								asms.push_back(Asm("push", "eax", ""));
								asms.push_back(Asm("call", "printf", ""));
								asms.push_back(Asm("add", "esp", "4"));
							}
						}
					}
					asms.push_back(Asm("lea", "eax", "@new_line"));
					asms.push_back(Asm("push", "eax", ""));
					asms.push_back(Asm("call", "printf", ""));
					asms.push_back(Asm("add", "esp", "4"));
					//strNum++;
					clearRegPool();
				}
				else {
				}
				if (index == curBlock.quadNum - 2
					&& (curBlock.quads[index + 1].op == "goto" || curBlock.quads[index + 1].op == ">" 
					|| curBlock.quads[index + 1].op == ">=" || curBlock.quads[index + 1].op == "<" 
					|| curBlock.quads[index + 1].op == "<=" || curBlock.quads[index + 1].op == "==" 
					|| curBlock.quads[index + 1].op == "!=")) {
					saveLiveVar();
					flag = 1;
				}
			}
			if (flag == 0) {
				saveLiveVar();
			}
		}
		asms.push_back(Asm("@" + curProc.name + "_end:"));
		asms.push_back(Asm("mov", "esp", "ebp"));
		asms.push_back(Asm("pop", "edi", ""));
		asms.push_back(Asm("pop", "esi", ""));
		asms.push_back(Asm("pop", "ebp", ""));
		asms.push_back(Asm("pop", "ebx", ""));
		asms.push_back(Asm("ret", "", ""));
		asms.push_back(Asm("_" + curProc.name + " endp"));
	}
	asms.push_back(Asm("_start:"));
	asms.push_back(Asm("call", "_main", ""));
	asms.push_back(Asm("ret", "", ""));
	asms.push_back(Asm("end _start"));
}

//分配寄存器
string Generator::getReg(Quadruple q, int index) {
	string expr1, res, temp, addr;
	int regIndex;
	map<string, int>::iterator iter1;
	
	expr1 = q.expr1; res = q.res;
	//如果地址操作数在全局寄存器中，直接返回该全局寄存器
	iter1 = curProc.globalRegs.find(res);
	if (iter1 != curProc.globalRegs.end()) {
		regIndex = (*iter1).second;
		switch (regIndex) {
			case 1:
				return "ebx";
			case 2:
				return "esi";
			case 3:
				return "edi";
			default:
				break;
		}
	}
	//如果第一个操作数在寄存器中、在基本块当前位置后不被使用、不再活跃、不占用全局寄存器且不是全局变量，
	//返回该操作数所占用的寄存器
	else if (addrDescriptor.find(expr1) != addrDescriptor.end() && !isUse(expr1, index) && !isLive(expr1) && 
		curProc.globalRegs.find(expr1) == curProc.globalRegs.end() && 
		globalVars.find(expr1) == globalVars.end()) {
		return getAddr(expr1);
	}
	//如果临时寄存器池中有空闲寄存器，返回之
	else if ((*(regDescriptor.find("eax"))).second == "")
		return "eax";
	else if ((*(regDescriptor.find("ecx"))).second == "")
		return "ecx";
	else if ((*(regDescriptor.find("edx"))).second == "")
		return "edx";
	//如果临时寄存器池中没有空闲寄存器，则将在即将生成代码中不会被使用的寄存器写回相应的内存空间，
	//标识该寄存器被新的变量占用，返回该空闲寄存器
	else if (isUse(res, index) || q.op == "+" || q.op == "-" || q.op == "*" || 
		q.op == "/" || q.op == ">" || q.op == ">=" || q.op == "<" || q.op == "<=" || 
		q.op == "==" || q.op == "!=") {
			if (q.expr2 != "") {
				temp = (*(regDescriptor.find("eax"))).second;
				if (q.expr2 != temp) {
					if (!isInMem(temp)) {
						addr = allocMem(temp);
						if (addr[0] != '[')
							asms.push_back(Asm("mov", addr, "eax"));
						else
							asms.push_back(Asm("mov", "dword ptr " + addr, "eax"));
						updateAddrDescriptor(addr, temp);
					}
					return "eax";
				}
				temp = (*(regDescriptor.find("ecx"))).second;
				if (q.expr2 != temp) {
					if (!isInMem(temp)) {
						addr = allocMem(temp);
						if (addr[0] != '[')
							asms.push_back(Asm("mov", addr, "ecx"));
						else
							asms.push_back(Asm("mov", "dword ptr " + addr, "ecx"));
						updateAddrDescriptor(addr, temp);
					}
					return "ecx";
				}
				temp = (*(regDescriptor.find("edx"))).second;
				if (q.expr2 != temp) {
					if (!isInMem(temp)) {
						addr = allocMem(temp);
						if (addr[0] != '[')
							asms.push_back(Asm("mov", addr, "edx"));
						else
							asms.push_back(Asm("mov", "dword ptr " + addr, "edx"));
						updateAddrDescriptor(addr, temp);
					}
					return "edx";
				}
			}
	}
	//若不满足上述情况，则为该变量分配内存空间
	else
		return allocMem(res);
}

//获取变量的存储地址
string Generator::getAddr(string s) {
	string res;
	int regIndex;
	map<string, int>::iterator iter1;
	map<string, string>::iterator iter2;

	//如果变量在全局寄存器中
	iter1 = curProc.globalRegs.find(s);
	if (iter1 != curProc.globalRegs.end()) {
		regIndex = (*iter1).second;
		switch (regIndex) {
			case 1:
				return "ebx";
			case 2:
				return "esi";
			case 3:
				return "edi";
			default:
				break;
		}
	}
	//如果变量在临时寄存器中
	for (iter2 = addrDescriptor.begin(); iter2 != addrDescriptor.end(); ++iter2) {
		if ((*iter2).second != "" && (*iter2).second == s) {
			res = (*iter2).first;
			if (res[0] == 'e') {
				return res;
			}
		}
	}
	//如果变量为全局变量
	if (res == "" && globalVars.find(s) != globalVars.end()) {
		res = "$" + s;
	}
	if (res == "" || res[0] != '[')
		return res;
	else
		return "dword ptr " + res;
}

//获取变量在内存中的地址
string Generator::getMem(string s) {
	string res = "";
	map<string, string>::iterator iter;

	for (iter = addrDescriptor.begin(); iter != addrDescriptor.end(); ++iter) {
		if ((*iter).first[0] == '[' && (*iter).second != "" && (*iter).second == s) {
			res = (*iter).first;
			return res;
		}
	}
	return res;
}

//从符号表中获得标识符类型
int Generator::getType(string s) {
	if (s[0] == '@') {
		return (curProc.table.getTemp(s)).type.tag;
	}
	else {
		return (curProc.table.get(s)).type.tag;
	}
}

//判断是否为立即数并进行类型转换
int Generator::isImmediate(string s) {
	int i = 0, length = s.size(), temp = INT;
	if (s[0] == '\'')
		return CHAR;
	if (s[0] == '-' || s[0] == '+') {
		i++;
	}
	for (; i < length; i++) {
		if (s[i] == '.') {
			temp = FLOAT;
		}
		else if (!isdigit(s[i]))
			return 0;
	}
	return temp;
}

//类型转换
void Generator::transform(string &s) {
	int i = 0, length = s.size();

}

//是否为局部常量
string Generator::isConst(string s) {
	map<string, Id>::iterator iter;
	for (iter = curProc.table.table.begin(); iter != curProc.table.table.end(); ++iter) {
		if ((*iter).first == s) {
			return (*iter).second.value;
		}
	}
	return "";
}

//判断变量在当前基本块内当前位置后是否被使用
int Generator::isUse(string s, int n) {
	int i;
	for (i = n + 1; i < curBlock.quadNum; i++) {
		if (curBlock.quads[i].expr1 == s || curBlock.quads[i].expr2 == s)
			return 1;
	}
	return 0;
}

//判断变量是否活跃
int Generator::isLive(string s) {
	if (curBlock.out.find(s) != curBlock.out.end() 
		|| curBlock.def.find(s) != curBlock.def.end() 
		|| curBlock.use.find(s) != curBlock.use.end())
		return 1;
	return 0;
}

//
/*int Generator::isInReg(string s) {
	map<string, list<string>>::iterator iter1;
	list<string>::iterator iter2;
	iter1 = addrDescriptor.find(s);
	if (iter1 != addrDescriptor.end()) {
		for (iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); ++iter2) {
			if ((*iter2)[0] == 'e')
				return 1;
		}
	}
	return 0;
}*/

//
int Generator::isInMem(string s) {
	if (s == "")
		return 1;
	map<string, string>::iterator iter;
	for (iter = addrDescriptor.begin(); iter != addrDescriptor.end(); ++iter) {
		if ((*iter).first[0] == '[' && (*iter).second == s) {
			return 1;
		}
	}
	return 0;
}

//
string Generator::allocMem(string s) {
	int regIndex, i = 0;
	map<string, int>::iterator iter1;
	map<string, Id>::iterator iter2;
	set<string>::iterator iter3;
	vector<string>::iterator iter4;

	iter1 = curProc.globalRegs.find(s);
	if (iter1 != curProc.globalRegs.end()) {
		regIndex = (*iter1).second;
		switch (regIndex) {
			case 1:
				return "ebx";
			case 2:
				return "esi";
			case 3:
				return "edi";
			default:
				break;
		}
	}
	if (globalVars.find(s) != globalVars.end())
		return ("$" + s);
	for (i = 0, iter4 = curProc.params.begin(); iter4 != curProc.params.end(); ++iter4, ++i) {
		if (*iter4 == s)
			return "[ebp + " + intToString0((5 + i) * 4) + "]";
	}
	for (i = 0, iter3 = curProc.vars.begin(); iter3 != curProc.vars.end(); ++iter3, ++i) {
		if (*iter3 == s) {
			i++;
			break;
		}
	}
	if (iter3 == curProc.vars.end())
		i = ++stackTop;
	return "[ebp - " + intToString0(i * 4) + "]";
}

//更新寄存器描述符
void Generator::updateRegDescriptor(string reg, string var) {
	map<string, string>::iterator iter;
	int flag = 0;
	for (iter = regDescriptor.begin(); iter != regDescriptor.end(); ++iter) {
		if ((*iter).second == var) {
			(*iter).second = "";
		}
	}
	for (iter = regDescriptor.begin(); iter != regDescriptor.end(); ++iter) {
		if ((*iter).first == reg) {
			(*iter).second = var;
			break;
		}
	}
	if (iter == regDescriptor.end()) {
		regDescriptor.insert(map<string, string>::value_type(reg, var));
	}
}

//更新地址描述符
void Generator::updateAddrDescriptor(string addr, string var) {
	map<string, string>::iterator iter;
	/*for (iter = addrDescriptor.begin(); iter != addrDescriptor.end(); ++iter) {
		if ((*iter).second == var) {
			(*iter).second = "";
		}
	}*/
	for (iter = addrDescriptor.begin(); iter != addrDescriptor.end(); ++iter) {
		if ((*iter).first == addr) {
			(*iter).second = var;
			break;
		}
	}
	if (iter == addrDescriptor.end()) {
		addrDescriptor.insert(map<string, string>::value_type(addr, var));
	}
}

//为除法运算操作数分配特定寄存器
void Generator::specifyReg(string reg, string op, string s) {
	string temp, var, place;
	if (s != reg) {
		temp = (*(regDescriptor.find(reg))).second;
		if (s == "eax" || s == "ecx" || s == "edx") {
			asms.push_back(Asm("xchg", reg, s));
			updateRegDescriptor(s, temp);
			updateAddrDescriptor(s, temp);
			updateRegDescriptor(reg, op);
			updateAddrDescriptor(reg, op);
		}
		else {
			if (!isInMem(temp)) {
				var = temp;
				place = allocMem(var);
				if (place[0] != '[')
					asms.push_back(Asm("mov", place, reg));
				else
					asms.push_back(Asm("mov", "dword ptr " +place, reg));
				updateAddrDescriptor(place, var);
			}
			asms.push_back(Asm("mov", reg, s));
			updateRegDescriptor(reg, s);
			updateAddrDescriptor(reg, s);
		}
	}
}

//释放不使用的寄存器
void Generator::releaseRegPool(string s, int index) {
	map<string, string>::iterator iter;
	if (!isUse(s, index) && !isLive(s) && globalVars.find(s) == globalVars.end()) {
		for (iter = regDescriptor.begin(); iter != regDescriptor.end(); ++iter) {
			if ((*iter).second == s) {
				(*iter).second = "";
			}
		}
		for (iter = addrDescriptor.begin(); iter != addrDescriptor.end(); ++iter) {
			if ((*iter).second == s) {
				(*iter).second = "";
			}
		}
	}
}

//清空寄存器池
void Generator::clearRegPool() {
	map<string, string>::iterator iter;
	iter = regDescriptor.find("eax");
	if (iter == regDescriptor.end()) {
		regDescriptor.insert(map<string, string>::value_type("eax", ""));
	}
	else {
		(*iter).second = "";
	}
	iter = regDescriptor.find("ecx");
	if (iter == regDescriptor.end()) {
		regDescriptor.insert(map<string, string>::value_type("ecx", ""));
	}
	else {
		(*iter).second = "";
	}
	iter = regDescriptor.find("edx");
	if (iter == regDescriptor.end()) {
		regDescriptor.insert(map<string, string>::value_type("edx", ""));
	}
	else {
		(*iter).second = "";
	}
	iter = addrDescriptor.find("eax");
	if (iter == addrDescriptor.end()) {
		addrDescriptor.insert(map<string, string>::value_type("eax", ""));
	}
	else {
		(*iter).second = "";
	}
	iter = addrDescriptor.find("ecx");
	if (iter == addrDescriptor.end()) {
		addrDescriptor.insert(map<string, string>::value_type("ecx", ""));
	}
	else {
		(*iter).second = "";
	}
	iter = addrDescriptor.find("edx");
	if (iter == addrDescriptor.end()) {
		addrDescriptor.insert(map<string, string>::value_type("edx", ""));
	}
	else {
		(*iter).second = "";
	}
	/*regDescriptor.insert(map<string, string>::value_type("ecx", ""));
	regDescriptor.insert(map<string, string>::value_type("edx", ""));
	addrDescriptor.insert(map<string, string>::value_type("eax", ""));
	addrDescriptor.insert(map<string, string>::value_type("ecx", ""));
	addrDescriptor.insert(map<string, string>::value_type("edx", ""));*/
	//addrDescriptor.clear();
	//regDescriptor.clear();
}

//基本块结束时存储活跃变量
void Generator::saveLiveVar() {
	string eax, ecx, edx, temp, place;
	eax = (*(regDescriptor.find("eax"))).second;
	ecx = (*(regDescriptor.find("ecx"))).second;
	edx = (*(regDescriptor.find("edx"))).second;
	if (eax != "" && (curProc.paramsSet.find(eax) != curProc.paramsSet.end() 
		|| globalVars.find(eax) != globalVars.end() 
		|| isLive(eax))) {
			if (!isInMem(eax)) {
				temp = allocMem(eax);
				if (temp[0] != '[')
					asms.push_back(Asm("mov", temp, "eax"));
				else
					asms.push_back(Asm("mov", "dword ptr " + temp, "eax"));
				updateAddrDescriptor(temp, eax);
			}
			else {
				place = getMem(eax);
				if (place != "") {
					asms.push_back(Asm("mov", "dword ptr " + place, "eax"));
				}
			}
	}
	if (ecx != "" && (curProc.paramsSet.find(ecx) != curProc.paramsSet.end() 
		|| globalVars.find(ecx) != globalVars.end() 
		|| isLive(ecx))) {
			if (!isInMem(ecx)) {
				temp = allocMem(ecx);
				if (temp[0] != '[')
					asms.push_back(Asm("mov", temp, "ecx"));
				else
					asms.push_back(Asm("mov", "dword ptr " + temp, "ecx"));
				updateAddrDescriptor(temp, ecx);
			}
			else {
				place = getMem(ecx);
				if (place != "") {
					asms.push_back(Asm("mov", "dword ptr " + place, "ecx"));
				}
			}
	}
	if (edx != "" && (curProc.paramsSet.find(edx) != curProc.paramsSet.end() 
		|| globalVars.find(edx) != globalVars.end() 
		|| (isLive(edx)))) {
			if (!isInMem(edx)) {
				temp = allocMem(edx);
				if (temp[0] != '[')
					asms.push_back(Asm("mov", temp, "edx"));
				else
					asms.push_back(Asm("mov", "dword ptr " + temp, "edx"));
				updateAddrDescriptor(temp, edx);
			}
			else {
				place = getMem(edx);
				if (place != "") {
					asms.push_back(Asm("mov", "dword ptr " + place, "edx"));
				}
			}
	}
}

//调用函数前保存临时寄存器中的变量
void Generator::saveRegVar(int index) {
	string eax, ecx, edx, temp, place;
	eax = (*(regDescriptor.find("eax"))).second;
	ecx = (*(regDescriptor.find("ecx"))).second;
	edx = (*(regDescriptor.find("edx"))).second;
	if (eax != "" && (isUse(eax, index - 1) || isLive(eax) 
		|| globalVars.find(eax) != globalVars.end() 
		|| curProc.paramsSet.find(eax) != curProc.paramsSet.end())) {
		if (!isInMem(eax)) {
			temp = allocMem(eax);
			if (temp[0] != '[')
				asms.push_back(Asm("mov", temp, "eax"));
			else
				asms.push_back(Asm("mov", "dword ptr " + temp, "eax"));
			updateAddrDescriptor(temp, eax);
		}
		else {
			place = getMem(eax);
			if (place != "") {
				asms.push_back(Asm("mov", "dword ptr " + place, "eax"));
			}
		}
		/*temp = allocMem(eax);
		if (temp[0] != '[')
			asms.push_back(Asm("mov", temp, "eax"));
		else
			asms.push_back(Asm("mov", "dword ptr " + temp, "eax"));
		updateAddrDescriptor(temp, eax);*/
	}
	if (ecx != "" && (isUse(ecx, index - 1) || isLive(ecx) 
		|| globalVars.find(ecx) != globalVars.end() 
		|| curProc.paramsSet.find(ecx) != curProc.paramsSet.end())) {
		if (!isInMem(ecx)) {
			temp = allocMem(ecx);
			if (temp[0] != '[')
				asms.push_back(Asm("mov", temp, "ecx"));
			else
				asms.push_back(Asm("mov", "dword ptr " + temp, "ecx"));
			updateAddrDescriptor(temp, ecx);
		}
		else {
			place = getMem(ecx);
			if (place != "") {
				asms.push_back(Asm("mov", "dword ptr " + place, "ecx"));
			}
		}
		/*temp = allocMem(ecx);
		if (temp[0] != '[')
			asms.push_back(Asm("mov", temp, "ecx"));
		else
			asms.push_back(Asm("mov", "dword ptr " + temp, "ecx"));
		updateAddrDescriptor(temp, ecx);*/
	}
	if (edx != "" && (isUse(edx, index - 1) || isLive(edx) 
		|| globalVars.find(edx) != globalVars.end() 
		|| curProc.paramsSet.find(edx) != curProc.paramsSet.end())) {
		if (!isInMem(edx)) {
			temp = allocMem(edx);
			if (temp[0] != '[')
				asms.push_back(Asm("mov", temp, "edx"));
			else
				asms.push_back(Asm("mov", "dword ptr " + temp, "edx"));
			updateAddrDescriptor(temp, edx);
		}
		else {
			place = getMem(edx);
			if (place != "") {
				asms.push_back(Asm("mov", "dword ptr " + place, "edx"));
			}
		}
		/*temp = allocMem(edx);
		if (temp[0] != '[')
			asms.push_back(Asm("mov", temp, "edx"));
		else
			asms.push_back(Asm("mov", "dword ptr " + temp, "edx"));
		updateAddrDescriptor(temp, edx);*/
	}
}

void Generator::output() {
	list<Asm>::iterator iter;
	ofstream file("test.asm");

	for (iter = asms.begin(); iter != asms.end(); ++iter) {
		cout << (*iter).toString() << endl;
		file << (*iter).toString() << endl;
	}
}
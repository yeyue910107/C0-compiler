#include<fstream>
#include<iostream>
#include<string>
#include<cctype>
#include<map>
#include<vector>
#include "lexical_analyzer.h"
#include "error.h"
using namespace std;

//四元式
class Quadruple {
public:
	string op, expr1, expr2, res, quadstring;//操作符，操作数1， 操作数2，结果，四元式字符串
	int label, jumpTo;//标号，跳转到的标号
	int blockIndex;//在基本块中的位置
	Quadruple() { label = 0; jumpTo = 0; blockIndex = 0; }
	~Quadruple() { }
};

//语法节点
class Node {
public:
	int lexline;//所在行数
	string name;//节点名
	vector<Quadruple> code;//四元式序列
	static int labels;//标号
	Node() { lexline = Lexer::line; }
	~Node() { }
	virtual int newlabel() { return ++labels; }//生成新标号
};

//表达式
class Expr : public Node {
public:
	Token op;//操作符
	Type type;//类型
	Expr() : Node() { type = Type::_null; }
	Expr(Token tok, Type p) { op = tok; type = p; }
	~Expr() { }
	virtual void gen() { }//生成四元式
	virtual string toString() { return op.toString(); }
};

//标识符
class Id : public Expr {
public:
	int kind, offset, defLine;//种类，符号表中位置，定义行数
	string value;//值(如果为常量)
	Id() : Expr() { }
	Id(Word id, Type p, int b) : Expr(id, p) { offset = b; }
	Id(Word id, Type p, int b, int k, int d) : Expr(id, p) { kind = k; offset = b; defLine = d; }
	~Id() { }
};

//临时变量
class Temp : public Expr {
public:
	static int count;//数量
	int number;//序号
	Temp() : Expr() { }
	Temp(Type p) : Expr(TEMP, p) { number = ++count; op.lexeme = toString(); }
	~Temp() { }
	virtual void gen() { }//生成四元式
	virtual string toString();
};

//双目运算符
class Arith : public Expr {
public:
	Expr expr1, expr2;//左操作数，右操作数
	Temp temp;//生成的临时变量
	Arith() : Expr() { }
	Arith(Token tok, Expr x1, Expr x2);
	~Arith() { }
	virtual void gen();//生成四元式
	virtual string toString() { return temp.toString() + " = " + expr1.toString() + " " + op.toString() + " " + expr2.toString(); }
};


//单目运算符
class Unary : public Expr {
public:
	Expr expr;//操作数
	Temp temp;//生成的临时变量
	Unary() : Expr() { }
	Unary(Token tok, Expr x);
	~Unary() { }
	virtual void gen();//生成四元式
	virtual string toString() { return temp.toString() + " = " + op.toString() + " " + expr.toString(); }
};

//逻辑运算符
class Logical : public Expr {
public:
	Expr expr1, expr2;//左操作数，右操作数
	Logical() { }
	Logical(Token tok, Expr x1, Expr x2);
	~Logical() { }
	virtual void gen();//生成四元式
	virtual string toString() { return expr1.toString() + " " + op.toString() + " " + expr2.toString(); }
};

//关系运算符
class Rel : public Logical {
public:
	int truelabel, falselabel;//如果为真跳转到的标号，如果为假跳转到的标号
	Rel() { }
	Rel(Token tok, Expr x1, Expr x2) : Logical(tok, x1, x2) { }
	~Rel() { }
	virtual void gen();//生成四元式
};

//语句
class Stmt : public Node {
public:
	int next;//下条语句的标号
	Stmt() { next = 0; }
	~Stmt() { }
	virtual void gen(Stmt &s);//生成四元式
};

//If语句
class If : public Stmt {
public:
	Rel expr;//关系运算符
	Stmt stmt;//if语句
	If() { }
	If(Rel x, Stmt &s);
	~If() { }
	virtual void gen();//生成四元式
};

//Else语句
class Else : public Stmt {
public:
	Rel expr;//关系运算符
	Stmt stmt1, stmt2;//if语句，else语句
	Else() { }
	Else(Rel x, Stmt &s1, Stmt &s2) { expr = x; stmt1 = s1; stmt2 = s2; next = 0; }
	~Else() { }
	virtual void gen();//生成四元式
};

//While语句
class While : public Stmt {
public:
	Rel expr;//关系运算符
	Stmt stmt;//while语句
	While() { }
	~While() { }
	void init(Rel x, Stmt &s) { expr = x; stmt = s; next = 0; }
	virtual void gen();//生成四元式
};

//赋值语句
class Set : public Stmt {
public:
	Id id;//左侧标识符
	Expr expr;//右侧表达式
	Set() { }
	Set(Id i, Expr x);
	~Set() { }
	static Type check(Type p1, Type p2, int lexline);//类型检查和隐式转换
	virtual void gen();//生成四元式
};

//For语句
class For : public Stmt {
public:
	Rel expr;//关系运算符
	Stmt inits, paces, stmt;//初始化语句，步长语句，循环体
	For() { }
	~For() { }
	void init(Stmt s1, Rel x, Stmt s2, Stmt &s);
	virtual void gen();//生成四元式
};

//语句列
class Seq : public Stmt {
public:
	Stmt stmt1, stmt2;
	Seq(Stmt &s1, Stmt &s2) { stmt1 = s1; stmt2 = s2; }
	~Seq() { }
	virtual void gen();//生成四元式
};

//符号表
class Table {
public:
	map<string, Id> table, global;//函数符号表，全局符号表
	map<string, Temp> temps;//临时变量信息
	vector<Quadruple> quad;//四元式
	vector<int> paramsType;//参数类型
	vector<string> params;//参数名
	int paramsNum, tempNum;//参数个数，临时变量个数
	Type type;//函数返回值类型
	Lexer lex;//词法分析器
	Table();
	Table(Lexer l) { lex = l; }
	~Table() { }
	void put(string s, Id sym);//向符号表中插入变量、常量和参数
	void putTemp(string s, Temp temp);//向符号表中插入临时变量
	Id get(string s);//从符号表中取出变量、常量和参数
	Temp getTemp(string s);//从符号表中取出临时变量
};

//符号表字典
class Tables {
public:
	map<string, Table> tables;//所有函数符号表和全局符号表
	Lexer lex;//词法分析器
	Tables();
	~Tables() { }
	void put(string s, Table t);//向符号表字典中插入符号表
	Table* get(string s);//从符号表字典中取出符号表
};

//语法分析器
class Parser {
public:
	Tables tables;//符号表字典
	Lexer lex;//词法分析器
	Token look;//当前词法单元
	int used;
	string proc;//当前函数名

	Parser(Lexer l, ifstream &file);
	~Parser() { }
	void move(ifstream &file) { look = lex.scan(file); }//向前读取词法单元
	//void error(string s) { cout << s << endl; }
	void match(int t, ifstream &file);//读取词法单元并与指定词法单元匹配
	void match(int t, ifstream &file, string msg);//不匹配则输出错误信息
	void match(int t, ifstream &file, Error e);//不匹配则报错
	void program(ifstream &file);//程序
	void mainProc(ifstream &file, Stmt &s);//主函数
	void block(ifstream &file, Stmt &s);//复合语句
	void stmts(ifstream &file, Stmt &s);//语句列
	Stmt stmt(ifstream &file);//语句
	Stmt ifs(ifstream &file);//if语句
	Stmt whiles(ifstream &file);//while语句
	Stmt fors(ifstream &file);//for语句
	Stmt assign(ifstream &file);//赋值语句
	Stmt assign(ifstream &file, Token tok);
	Stmt scan(ifstream &file);//scanf语句
	Stmt print(ifstream &file);//printf语句
	Stmt returns(ifstream &file);//返回语句
	void procDef(ifstream &file, Token tok, Type type);//有返回值函数定义
	void voidProcDef(ifstream &file);//无返回值函数定义
	Stmt procCall(ifstream &file, Token tok);//有返回值函数调用
	Stmt procCall(ifstream &file);
	Expr procCallToExpr(ifstream &file, Token tok);
	Stmt voidProcCall(ifstream &file, Token tok);//有返回值函数调用
	Stmt voidProcCall(ifstream &file);
	Rel rel(ifstream &file);//条件
	Expr expr(ifstream &file);//表达式
	Expr term(ifstream &file);//项
	Expr factor(ifstream &file);//因子
	Stmt pace(ifstream &file, Token tok);//步长
	void varDecls(ifstream &file, Token tok1, Token tok2);//变量声明
	void varDecls(ifstream &file);
	void varDef(ifstream &file, Token tok1, Token tok2);//变量定义
	void varDef(ifstream &file);
	void constDecls(ifstream &file);//常量声明
	void constDef(ifstream &file, Token tok1, Token tok2);//常量定义
	void constDef(ifstream &file);
};

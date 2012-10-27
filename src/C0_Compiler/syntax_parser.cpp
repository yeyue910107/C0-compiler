#pragma warning(disable:4786)
#include<fstream>
#include<iostream>
#include<cctype>
#include<vector>
#include<map>
#include "syntax_parser.h"

using namespace std;

//functions
string intToString(int n) {
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

//语法节点
int Node::labels = 0;

//临时变量
int Temp::count = 0;

string Temp::toString() {
	string s = "@t";
	s += intToString(number);
	return s;
}

//双目运算符
Arith::Arith(Token tok, Expr x1, Expr x2) {
	op = tok;
	expr1 = x1;
	expr2 = x2;
	type = Type::max(x1.type, x2.type);
	temp = Temp(type);
	//TO-DO
}

void Arith::gen() {
	Quadruple q;
	q.expr1 = expr1.toString();
	q.expr2 = expr2.toString();
	q.op = op.lexeme;
	q.res = temp.toString();
	q.quadstring = q.res + " = " + q.expr1 + " " + q.op + " " + q.expr2;
	temp.code.insert(temp.code.end(), expr1.code.begin(), expr1.code.end());
	temp.code.insert(temp.code.end(), expr2.code.begin(), expr2.code.end());
	temp.code.push_back(q);
}

//单目运算符
Unary::Unary(Token tok, Expr x) {
	op = tok;
	expr = x;
	type = Type::max(Type::_int, x.type);
	temp = Temp(type);
}

void Unary::gen() {
	Quadruple q;
	q.expr1 = expr.toString();
	q.op = "minus";
	q.res = temp.toString();
	q.quadstring = q.res + " = " + q.op + " " + q.expr1;
	temp.code.insert(temp.code.end(), expr.code.begin(), expr.code.end());
	temp.code.push_back(q);
}

//逻辑运算符
Logical::Logical(Token tok, Expr x1, Expr x2) {
	op = tok;
	expr1 = x1;
	expr2 = x2;
};

void Logical::gen() {
	
}

//关系运算符
void Rel::gen() {
	Quadruple q1, q2;
	q1.expr1 = expr1.toString();
	q1.expr2 = expr2.toString();
	switch (op.tag) {
		case GT:
			q1.op = "<=";
			break;
		case GE:
			q1.op = "<";
			break;
		case LT:
			q1.op = ">=";
			break;
		case LE:
			q1.op = ">";
			break;
		case EQU:
			q1.op = "!=";
			break;
		case NE:
			q1.op = "==";
			break;
		default:
			break;
	}
	q1.jumpTo = truelabel;
	q1.quadstring = "if " + q1.expr1 + " " + q1.op + " " + q1.expr2 + " goto L" + intToString(q1.jumpTo);
	code.insert(code.end(), expr1.code.begin(), expr1.code.end());
	code.insert(code.end(), expr2.code.begin(), expr2.code.end());
	code.push_back(q1);
}

//语句
void Stmt::gen(Stmt &s) {
	if (code.size() > 0) {
		if (next != 0) {	
			if (s.code[0].label == 0)
			{
				s.code[0].label = newlabel();
				s.code[0].quadstring = "L" + intToString(s.code[0].label) + ": " + s.code[0].quadstring;
			}
			else {
				next = s.code[0].label;
			}
		}
	}
	next = s.next;
	vector<Quadruple>::iterator iter;
	int i;
	for (i = 0, iter = code.begin(); iter != code.end(); ++iter, ++i) {
		if ((*iter).jumpTo == -1) {
			code[i].jumpTo = s.code[0].label;
			code[i].quadstring += intToString(s.code[0].label);
		}
	}
	code.insert(code.end(), s.code.begin(), s.code.end());
}

//If语句
If::If(Rel x, Stmt &s) {
	expr = x;
	stmt = s;
	next = 0;
}

void If::gen() {
	next = -1;
	expr.falselabel = next;
	int temp = expr.code.size() - 1;
	expr.code[temp].jumpTo = expr.falselabel;
	expr.code[temp].quadstring = expr.code[temp].quadstring + intToString(expr.code[temp].jumpTo);
	code.insert(code.end(), expr.code.begin(), expr.code.end());
	code.insert(code.end(), stmt.code.begin(), stmt.code.end());
}

//Else语句
void Else::gen() {
	next = -1;
	if (stmt2.code[0].label == 0) {
		expr.falselabel = newlabel();
		stmt2.code[0].label = expr.falselabel;
		stmt2.code[0].quadstring = "L" + intToString(stmt2.code[0].label) + ": " + stmt2.code[0].quadstring;
	}
	else {
		expr.falselabel = stmt2.code[0].label;
	}
	int temp = expr.code.size() - 1;
	expr.code[temp].jumpTo = expr.falselabel;
	expr.code[temp].quadstring = expr.code[temp].quadstring + intToString(expr.code[temp].jumpTo);
	Quadruple q2;
	q2.op = "goto";
	q2.jumpTo = next;
	q2.quadstring = "goto L";
	code.insert(code.end(), expr.code.begin(), expr.code.end());
	code.insert(code.end(), stmt1.code.begin(), stmt1.code.end());
	code.push_back(q2);
	code.insert(code.end(), stmt2.code.begin(), stmt2.code.end());
}

//While语句
void While::gen() {
	next = -1;
	int begin = newlabel();
	expr.code[0].label = begin;
	expr.falselabel = next;
	int temp = expr.code.size() - 1;
	expr.code[temp].jumpTo = expr.falselabel;
	expr.code[temp].quadstring = expr.code[temp].quadstring + intToString(expr.code[temp].jumpTo);
	stmt.next = begin;
	Quadruple q2;
	q2.op = "goto";
	q2.jumpTo = begin;
	q2.quadstring = "goto L" + intToString(q2.jumpTo);
	expr.code[0].quadstring = "L" + intToString(expr.code[0].label) + ": " + expr.code[0].quadstring;
	code.insert(code.end(), expr.code.begin(), expr.code.end());
	code.insert(code.end(), stmt.code.begin(), stmt.code.end());
	code.push_back(q2);
}

//Set语句
Set::Set(Id i, Expr x) {
	id = i;
	expr = x;
	Type p = Set::check(id.type, expr.type, lexline);
}

Type Set::check(Type p1, Type p2, int lexline) {
	if (Type::numeric(p1) && Type::numeric(p2)) {
		if (p1.tag == CHAR && p2.tag == FLOAT) {
			Warning(1, lexline).print();
		}
		else if (p1.tag == INT && p2.tag == FLOAT) {
			Warning(0, lexline).print();
		}
		return p1;
	}
	else {
		Error(23, lexline).print();
		return Type::_null;
	}
}

void Set::gen() {
	Quadruple q;
	q.res = id.op.lexeme;
	q.expr1 = expr.toString();
	q.op = "=";
	q.quadstring = q.res + " = " + q.expr1;
	code.insert(code.end(), expr.code.begin(), expr.code.end());
	code.push_back(q);
}

//For语句
void For::init(Stmt s1, Rel x, Stmt s2, Stmt &s) {
	inits = s1;
	expr = x;
	paces = s2;
	stmt = s;
	next = 0;
}

void For::gen() {
	next = -1;
	int begin = newlabel();
	expr.code[0].label = begin;
	expr.falselabel = next;
	int temp = expr.code.size() - 1;
	expr.code[temp].jumpTo = expr.falselabel;
	expr.code[temp].quadstring = expr.code[temp].quadstring + intToString(expr.code[temp].jumpTo);
	stmt.next = begin;
	Quadruple q2;
	q2.op = "goto";
	q2.jumpTo = begin;
	q2.quadstring = "goto L" + intToString(q2.jumpTo);
	expr.code[0].quadstring = "L" + intToString(expr.code[0].label) + ": " + expr.code[0].quadstring;
	code.insert(code.end(), inits.code.begin(), inits.code.end());
	code.insert(code.end(), expr.code.begin(), expr.code.end());
	code.insert(code.end(), stmt.code.begin(), stmt.code.end());
	code.insert(code.end(), paces.code.begin(), paces.code.end());
	code.push_back(q2);
}

//语句列
void Seq::gen() {
	stmt1.next = newlabel();
	stmt2.next = next;
	stmt2.code[0].label = stmt1.next;
	stmt2.code[0].quadstring = "L" + intToString(stmt2.code[0].label) + ": " + stmt2.code[0].quadstring;
	code.insert(code.end(), stmt1.code.begin(), stmt1.code.end());
	code.insert(code.end(), stmt2.code.begin(), stmt2.code.end());
}

//符号表
Table::Table() {
	paramsNum = 0;
	tempNum = 0;
}

void Table::put(string s, Id sym) {
	map<string, Id>::iterator iter = table.find(s);
	if (iter == table.end())
		table.insert(map<string, Id>::value_type(s, sym));
	else {
		string msg = "identifier \'" + s + "\': redefinition.";
		Error(11, lex.line, msg).print();
	}
}

Id Table::get(string s) {
	map<string, Id>::iterator iter = table.find(s);
	if (iter != table.end())
		return (*iter).second;
	else {
		iter = global.find(s);
		if (iter != global.end()) {
			return (*iter).second;
		}
	}
	string msg = "\'" + s + "\': undeclared identifier.";
	Error(10, lex.line, msg).print();
	return Id();
}

void Table::putTemp(string s, Temp temp) {
	map<string, Temp>::iterator iter = temps.find(s);
	if (iter == temps.end())
		temps.insert(map<string, Temp>::value_type(s, temp));
}

Temp Table::getTemp(string s) {
	map<string, Temp>::iterator iter = temps.find(s);
	if (iter != temps.end())
		return (*iter).second;
	else {
		return Temp();
	}
}

//符号表字典
Tables::Tables() {
	tables.insert(map<string, Table>::value_type("global", Table(lex)));
}

void Tables::put(string s, Table t) {
	map<string, Table>::iterator iter = tables.find(s);
	if (iter == tables.end()) {
		tables.insert(map<string, Table>::value_type(s, t));
	}
	else {
		string msg = "function \'" + s + "\': redefinition.";
		Error(15, lex.line, msg).print();
	}
}

Table* Tables::get(string s) {
	map<string, Table>::iterator iter = tables.find(s);
	if (iter != tables.end())
		return &(iter->second);
	else {
		string msg = "function \'" + s + "\': undefined.";
		Error(14, lex.line, msg).print();
		return NULL;
	}
}

//语法分析器
Parser::Parser(Lexer l, ifstream &file) {
	lex = l;
	tables.lex = l;
	move(file);
	used = 0;
}

void Parser::match(int t, ifstream &file) {
	if (look.tag == t)
		move(file);
	//else
		//error("syntax error!");
}

void Parser::match(int t, ifstream &file, string msg) {
	if (look.tag == t)
		move(file);
	//else
		//error(msg);
}

void Parser::match(int t, ifstream &file, Error e) {
	if (look.tag == t)
		move(file);
	else
		e.print();
}

//<程序>::= [<常量说明部分>][<变量说明部分>]{<有返回值函数定义部分>|<无返回值函数定义部分>}<主函数>
void Parser::program(ifstream &file) {
	Stmt s;
	Token tok1, tok2;
	Type type;

	proc = "global";
	if (look.tag == CONST) {
		constDecls(file);
	}
	while (look.tag == INT || look.tag == FLOAT || look.tag == CHAR || look.tag == VOID) {
		tok1 = look.tag;
		if (look.tag == VOID) {
			move(file);
			if (look.tag == MAIN) {
				mainProc(file, s);
				break;
			}
			else if (look.tag == ID) {
				voidProcDef(file);
			}
			else
				Error(12, lex.line).print();
		}
		else {
			move(file);
			if (look.tag == ID) {
				tok2 = look;
				move(file);
				if (look.tag == LPAR) {
					switch (tok1.tag) {
						case INT:
							type = Type::_int;
							break;
						case FLOAT:
							type = Type::_float;
							break;
						case CHAR:
							type = Type::_char;
							break;
						default:
							break;
					}
					procDef(file, tok2, type);
				}
				else
					varDecls(file, tok1, tok2);
			}
		}
	}
	match(Null, file);
	if (Error::count > 0) {
		cout << Error::count << " error(s), " << Warning::count << " warning(s)" << endl;
		exit(0);
	}
}

//<主函数>::= void main '('')''{'<复合语句>'}'
void Parser::mainProc(ifstream &file, Stmt &s) {
	tables.tables.insert(map<string, Table>::value_type("main", Table(lex)));
	proc = "main";
	(*(tables.get("main"))).global = (*(tables.get("global"))).table;
	(*(tables.get("main"))).type = Type::_null;
	match(MAIN, file);
	match(LPAR, file, Error(3, lex.line));
	match(RPAR, file, Error(4, lex.line));
	Temp::count = 0;
	match(LBRACE, file, Error(7, lex.line));
	block(file, s);
	match(RBRACE, file, Error(8, lex.line));
	(*tables.get("main")).quad.insert((*tables.get("main")).quad.end(), s.code.begin(), s.code.end());
	(*tables.get("main")).tempNum = Temp::count;
	Temp::count = 0;
}

//<复合语句>::= [<常量说明部分>][<变量说明部分>]<语句列>
void Parser::block(ifstream &file, Stmt &s) {
	if (look.tag == CONST) {
		constDecls(file);
	}
	else if (look.tag == INT || look.tag == FLOAT || look.tag == CHAR)
		varDecls(file);
	stmts(file, s);
	int i = 0, label = s.newlabel();
	vector<Quadruple>::iterator iter;
	for (iter = s.code.begin(); iter != s.code.end(); ++iter, ++i) {	
		if (((*iter).op == ">=" || (*iter).op == ">" || (*iter).op == "<=" 
			|| (*iter).op == "<" || (*iter).op == "!=" || (*iter).op == "==" 
			|| (*iter).op == "goto") && (*iter).jumpTo == -1) {
			s.code[i].jumpTo = label;
			s.code[i].quadstring += intToString(label);
		}
	}
	Quadruple q;
	q.label = label;
	q.quadstring = "L" + intToString(q.label) + ":";
	s.code.push_back(q);
}

//<语句列>::= <语句>{<语句>}
void Parser::stmts(ifstream &file, Stmt &s) {
	while (look.tag != RBRACE) {
		if (look.tag == Null)
			return;
		s.gen(stmt(file));
	}
}

//<语句>::= <条件语句>|<循环语句>|'{'<语句列>'}'|<有返回值函数调用语句>; 
//			|<无返回值函数调用语句>;|<赋值语句>;|<读语句>;|<写语句>;|;|<返回语句>;

Stmt Parser::stmt(ifstream &file) {
	Rel x;
	Stmt s, s1, s2;
	Token tok;
	Quadruple q;;
	switch (look.tag) {
		case IF:
			return ifs(file);
		case WHILE:
			return whiles(file);
		case FOR:
			return fors(file);
		case LBRACE:
			match(LBRACE, file);
			stmts(file, s);
			match(RBRACE, file, Error(8, lex.line));
			return s;
		case ID:
			tok = look;
			move(file);
			if (look.tag == LPAR) {
				Type p = (*tables.get(tok.lexeme)).type;
				if (p.tag == Null)
					return voidProcCall(file, tok);
				else
					return procCall(file, tok);
			} 
			else if (look.tag == ASSIGN)
				return assign(file, tok);
			return s;
		case SCANF:
			return scan(file);
		case PRINTF:
			return print(file);
		case SEMI:
			match(SEMI, file);
			q.label = s.newlabel();
			q.quadstring = "L" + intToString(q.label) + ":";
			s.code.push_back(q);
			return s;
		case RETURN:
			return returns(file);
		default:
			move(file);
			return s;
	}
}

//If语句
Stmt Parser::ifs(ifstream &file) {
	Rel x;
	Stmt s1, s2;
	If ifnode;
	Else elsenode;

	match(IF, file);
	match(LPAR, file, Error(3, lex.line));
	x = rel(file);
	match(RPAR, file, Error(4, lex.line));
	s1 = stmt(file);
	ifnode = If(x, s1);
	if (look.tag != ELSE) {
		ifnode.gen();
		return ifnode;
	}
	match(ELSE, file);
	s2 = stmt(file);
	elsenode = Else(x, s1, s2);
	elsenode.gen();
	return elsenode;
}

//while语句
Stmt Parser::whiles(ifstream &file) {
	Rel x;
	Stmt s;
	While whilenode;

	match(WHILE, file);
	match(LPAR, file, Error(3, lex.line));
	x = rel(file);
	match(RPAR, file, Error(4, lex.line));
	s = stmt(file);
	whilenode.init(x, s);
	whilenode.gen();
	return whilenode;
}

//For语句
Stmt Parser::fors(ifstream &file) {
	Rel r;
	Stmt s, a, p;
	For fornode;
	Token tok;
	Id id;

	match(FOR, file);
	match(LPAR, file, Error(3, lex.line));
	a = assign(file);
	r = rel(file);
	match(SEMI, file, Error(6, lex.line));
	if (look.tag == ID) {
		tok = look;
		id = (*tables.get(proc)).get(look.lexeme);
		move(file);
	}
	else {
		Error(12, lex.line).print();
	}
	match(ASSIGN, file, Error(16, lex.line));
	p = pace(file, tok);
	match(RPAR, file, Error(4, lex.line));
	s = stmt(file);
	fornode.init(a, r, p, s);
	fornode.gen();
	return fornode;
}

//<赋值语句>::= <标识符>＝<表达式>
Stmt Parser::assign(ifstream &file) {
	Token tok = look;
	move(file);
	return assign(file, tok);
}

//<赋值语句>::= <标识符>＝<表达式>
Stmt Parser::assign(ifstream &file, Token tok) {
	Stmt stmt;
	Id id;
	if (tok.tag == ID) {
		id = (*(tables.get(proc))).get(tok.lexeme);
		if (id.kind == CONSTANT) {
			Error(24, lex.line, "\'" + id.op.lexeme + "\': cannot assign to a variable that is const.").print();
		}
	}
	else
		Error(12, lex.line).print();
	if (look.tag == ASSIGN) {
		move(file);
		Set stmt(id, expr(file));
		match(SEMI, file, Error(6, lex.line));
		stmt.gen();
		return stmt;
	}
	Error(16, lex.line).print();
	return stmt;
}

//<读语句>::= scanf'('<标识符>{,<标识符>}')'
Stmt Parser::scan(ifstream &file) {
	Stmt s;
	Id id;
	match(SCANF, file);
	match(LPAR, file, Error(3, lex.line));
	if (look.tag == ID) {
		while (1) {
			Quadruple q;
			if (look.tag == ID) {
				id = (*(tables.get(proc))).get(look.lexeme);
				move(file);
			}
			else
				Error(12, lex.line).print();
			q.expr1 = id.op.lexeme;
			q.op = "scanf";
			q.quadstring = q.op + " " + q.expr1;
			s.code.push_back(q);
			if (look.tag != COMMA) {
				if (look.tag == ID) {
					Error(5, lex.line).print();
					continue;
				}
				else
					break;
			}
			move(file);
		}
	}
	else {
		string msg = "\'scanf\': too few arguments for call.";
		Error(17, lex.line, msg).print();
	}
	match(RPAR, file, Error(4, lex.line));
	match(SEMI, file, Error(6, lex.line));
	return s;
}

//<写语句>::= printf'('<字符串>,<表达式>')'|printf'('<字符串>')'|printf'('<表达式>')'
Stmt Parser::print(ifstream &file) {
	Stmt s;
	Quadruple q;
	Id id;
	Expr x;
	match(PRINTF, file);
	match(LPAR, file, Error(3, lex.line));
	if (look.tag == STRING) {
		q.expr1 = look.lexeme;
		q.op = "printf";
		move(file);
		if (look.tag == COMMA) {
			move(file);
			x = expr(file);
			match(RPAR, file, Error(4, lex.line));
			match(SEMI, file, Error(6, lex.line));
			q.expr2 = x.op.lexeme;
			q.quadstring = q.op + " " + q.expr1 + " " + q.expr2;
			s.code.insert(s.code.begin(), x.code.begin(), x.code.end());
			s.code.push_back(q);
			return s;
		}
		match(RPAR, file, Error(4, lex.line));
		match(SEMI, file, Error(6, lex.line));
		q.quadstring = q.op + " " + q.expr1;
		s.code.push_back(q);
		return s;
	}
	else if (look.tag == PLUS || look.tag == MINUS || look.tag == ID || 
		look.tag == LPAR || look.tag == INTEGER || look.tag == REAL || 
		look.tag == CHARACTER) {
		x = expr(file);
		match(RPAR, file, Error(4, lex.line));
		match(SEMI, file, Error(6, lex.line));
		q.op = "printf";
		q.expr1 = x.op.lexeme;
		q.quadstring = q.op + " " + q.expr1;
		s.code.insert(s.code.begin(), x.code.begin(), x.code.end());
		s.code.push_back(q);
		return s;
	}
	else {
		string msg = "\'printf\': too few arguments for call.";
		Error(17, lex.line, msg).print();
	}
	match(RPAR, file, Error(4, lex.line));
	match(SEMI, file, Error(6, lex.line));
	return s;
}

//<返回语句>::= return['('<表达式>')']
Stmt Parser::returns(ifstream &file) {
	Stmt s;
	Quadruple q;
	Expr x;
	Type p = (*tables.get(proc)).type;

	match(RETURN, file);
	//if (p.tag != Type::_null.tag) {
		if (look.tag == LPAR) {
			move(file);
			if (look.tag == PLUS || look.tag == MINUS || look.tag == ID || 
			look.tag == LPAR || look.tag == INTEGER || look.tag == REAL || 
			look.tag == CHARACTER) {
				x = expr(file);
				match(RPAR, file);
				match(SEMI, file);
				if (Type::numeric(p) && Type::numeric(x.type)) {
					if (p.tag == CHAR && x.type.tag == FLOAT) {
						Warning(1, lex.line).print();
					}
					else if (p.tag == INT && x.type.tag == FLOAT) {
						Warning(0, lex.line).print();
					}
				}
				else if (p.tag == Type::_null.tag) {
					Error(21, lex.line, "\'" + proc + "\': \'void\' function returning a value.").print();
				}
				else {
					Error(22, lex.line).print();
				}
				q.op = "return";
				q.expr1 = x.op.lexeme;
				q.quadstring = q.op + " " + q.expr1;
				s.code.insert(s.code.begin(), x.code.begin(), x.code.end());
				s.code.push_back(q);
				return s;
			}
			else {
				string msg = "\'" + proc + "\': " + "must return a value.";
				Error(20, lex.line, msg).print();
			}
		}
		/*else if (look.tag == SEMI) {
			;
		}*/
		//else
			//Error(3, lex.line).print();
//		}
	//if ()
	match(SEMI, file, Error(6, lex.line));
	q.op = "return";
	q.quadstring = q.op;
	s.code.push_back(q);
	return s;
}

//<有返回值函数定义部分>::= <声明头部>'('<参数>')''{'<复合语句>'}'
void Parser::procDef(ifstream &file, Token tok, Type type) {
	int count = 0;
	Stmt s;
	Table table(lex);
	proc = tok.lexeme;

	match(LPAR, file, Error(3, lex.line));
	table.global = (*(tables.get("global"))).table;
	tables.put(proc, table);
	(*tables.get(proc)).type = type;
	Temp::count = 0;
	while (look.tag == INT || look.tag == FLOAT || look.tag == CHAR) {
		Type p;
		switch (look.tag) {
			case INT:
				p = Type::_int;
				break;
			case FLOAT:
				p = Type::_float;
				break;
			case CHAR:
				p = Type::_char;
				break;
			default:
				break;
		}
		move(file);
		Token t;
		t = look;
		match(ID, file, Error(12, lex.line));
		Id id = Id(Word(t.toString(), t.tag), p, used, PARA, lex.line);
		(*(tables.get(proc))).put(id.toString(), id);
		(*(tables.get(proc))).params.push_back(id.op.lexeme);
		(*(tables.get(proc))).paramsType.push_back(p.tag);
		count++;
		used += p.width;
		if (look.tag != COMMA)
			break;
		move(file);
	}
	(*(tables.get(proc))).paramsNum = count;
	match(RPAR, file, Error(4, lex.line));
	match(LBRACE, file, Error(7, lex.line));
	block(file, s);
	match(RBRACE, file, Error(8, lex.line));

	vector<Quadruple>::iterator iter;
	int hasReturn;
	for (hasReturn = 0, iter = s.code.begin(); iter != s.code.end(); ++iter) {
		if ((*iter).op == "return") {
			hasReturn = 1;
		}
	}
	if (!hasReturn) {
		Error(20, lex.line, "\'" + proc + "\': must return a value.").print();
	}
	(*tables.get(proc)).quad = s.code;
	(*tables.get(proc)).tempNum = Temp::count;
	Temp::count = 0;
}

//<无返回值函数定义部分>::= void<标识符>'('<参数>')''{'<复合语句>'}'
void Parser::voidProcDef(ifstream &file) {
	int count = 0;
	Stmt s;
	Table table(lex);
	proc = look.lexeme;
	move(file);
	match(LPAR, file, Error(3, lex.line));
	table.global = (*(tables.get("global"))).table;
	tables.put(proc, table);
	(*tables.get(proc)).type = Type::_null;
	Temp::count = 0;
	while (look.tag == INT || look.tag == FLOAT || look.tag == CHAR) {
		Type p;
		switch (look.tag) {
			case INT:
				p = Type::_int;
				break;
			case FLOAT:
				p = Type::_float;
				break;
			case CHAR:
				p = Type::_char;
				break;
			default:
				break;
		}
		move(file);
		Token t;
		t = look;
		match(ID, file, Error(12, lex.line));
		Id id = Id(Word(t.toString(), t.tag), p, used, PARA, lex.line);
		(*(tables.get(proc))).put(id.toString(), id);
		(*(tables.get(proc))).params.push_back(id.op.lexeme);
		(*(tables.get(proc))).paramsType.push_back(p.tag);
		count++;
		used += p.width;
		if (look.tag != COMMA)
			break;
		move(file);
	}
	(*tables.get(proc)).paramsNum = count;
	match(RPAR, file, Error(4, lex.line));
	match(LBRACE, file, Error(7, lex.line));
	block(file, s);
	match(RBRACE, file, Error(8, lex.line));

	/*vector<Quadruple>::iterator iter;
	for (iter = s.code.begin(); iter != s.code.end(); ++iter) {
		if ((*iter).op == "return" && (*iter).expr1 != "") {
			Error(21, lex.line, "\'" + proc + "\': \'void\' function returning a value.");
			break;
		}
	}*/
	(*tables.get(proc)).quad.insert((*tables.get(proc)).quad.end(), s.code.begin(), s.code.end());
	(*tables.get(proc)).tempNum = Temp::count;
	Temp::count = 0;
}

//<有返回值函数调用语句>::= <标识符>'('<值参数表>')'
Stmt Parser::procCall(ifstream &file, Token tok) {
	return voidProcCall(file, tok);
}

//<有返回值函数调用语句>::= <标识符>'('<值参数表>')'
Stmt Parser::procCall(ifstream &file) {
	Token tok = look;
	move(file);
	return procCall(file, tok);
}

//<有返回值函数调用语句>::= <标识符>'('<值参数表>')'
Expr Parser::procCallToExpr(ifstream &file, Token tok) {
	int paramsNum, count = 0;
	string proc;
	Table t;
	Type p;
	Temp temp;
	vector<int> paramsType;
	vector<Quadruple>::reverse_iterator iter;
	if (tok.tag == ID) {
		proc = tok.lexeme;
		if (tables.get(proc) != NULL) {
			t = *tables.get(proc);
			paramsType = (*(tables.get(proc))).paramsType;
			paramsNum = t.paramsNum;
			p = t.type;
			temp = Temp(p);
			(*(tables.get(this->proc))).putTemp(temp.op.lexeme, temp);
			vector<Expr> x;
			vector<Quadruple> q;
			Quadruple call;
			call.op = "call";
			call.expr1 = proc;
			call.res = temp.op.lexeme;
			call.quadstring = call.op + " " + call.expr1 + " " + temp.op.lexeme;
			while (look.tag == PLUS || look.tag == MINUS || look.tag == ID || 
				look.tag == LPAR || look.tag == INTEGER || look.tag == REAL || 
				look.tag == CHARACTER) {
				x.push_back(expr(file));
				
				if (Type::numeric(x[count].type)) {
					if (paramsType[count] == CHAR && x[count].type.tag == FLOAT) {
						Warning(1, lex.line).print();
					}
					else if (paramsType[count] == INT && x[count].type.tag == FLOAT) {
						Warning(0, lex.line).print();
					}
				}
				else {
					Error(21, lex.line, "\'" + proc + "\': type conversion error.").print();
				}
				temp.code.insert(temp.code.begin(), x[count].code.begin(), x[count].code.end());
				q.push_back(Quadruple());
				q[count].op = "param";
				q[count].expr1 = x[count].op.lexeme;
				q[count].quadstring = "param " + q[count].expr1;
				count++;
				if (look.tag != COMMA)
					break;
				move(file);
			}
			for (iter = q.rbegin(); iter != q.rend(); ++iter) {
				temp.code.push_back(*iter);
			}
			//temp.code.insert(temp.code.end(), q.rbegin(), q.rend());
			temp.code.push_back(call);
			if (count != paramsNum) {
				string msg;
				if (count > paramsNum) {
					msg = "\'" + proc + "\': too many actual parameters.";
					Error(18, lex.line, msg).print();
				}
				else {
					msg = "\'" + proc + "\': too few arguments for call.";
					Error(17, lex.line, msg).print();
				}
			}
		}
		match(RPAR, file, Error(4, lex.line));
	}
	return temp;
}

//<无返回值函数调用语句>::= <标识符>'('<值参数表>')'
Stmt Parser::voidProcCall(ifstream &file, Token tok) {
	int paramsNum, count = 0;
	string proc;
	Stmt s;
	vector<int> paramsType;
	vector<Quadruple>::reverse_iterator iter;
	if (tok.tag == ID) {
		proc = tok.lexeme;
		if (tables.get(proc) != NULL) {
			paramsType = (*tables.get(proc)).paramsType;
			paramsNum = (*tables.get(proc)).paramsNum;
			vector<Expr> x;
			vector<Quadruple> q;
			Quadruple call;
			match(LPAR, file, Error(3, lex.line));
			call.op = "call";
			call.expr1 = proc;
			call.quadstring = call.op + " " + call.expr1;
			while (look.tag == PLUS || look.tag == MINUS || look.tag == ID || 
				look.tag == LPAR || look.tag == INTEGER || look.tag == REAL || 
				look.tag == CHARACTER) {
				x.push_back(expr(file));
				
				if (Type::numeric(x[count].type)) {
					if (paramsType[count] == CHAR && x[count].type.tag == FLOAT) {
						Warning(1, lex.line).print();
					}
					else if (paramsType[count] == INT && x[count].type.tag == FLOAT) {
						Warning(0, lex.line).print();
					}
				}
				else {
					Error(21, lex.line, "\'" + proc + "\': type conversion error.").print();
				}
				/*for (iter1 = x[count].code.begin(); iter1 != x[count].code.end(); ++iter1) {
					s.code.push_back(*iter1);
				}*/
				s.code.insert(s.code.begin(), x[count].code.begin(), x[count].code.end());
				q.push_back(Quadruple());
				q[count].op = "param";
				q[count].expr1 = x[count].op.lexeme;
				q[count].quadstring = "param " + q[count].expr1;
				count++;
				if (look.tag != COMMA)
					break;
				move(file);
			}
			for (iter = q.rbegin(); iter != q.rend(); ++iter) {
				s.code.push_back(*iter);
			}
			//s.code.insert(s.code.end(), q.rbegin(), q.rend());
			s.code.push_back(call);
			if (count != paramsNum) {
				string msg;
				if (count > paramsNum) {
					msg = "\'" + proc + "\': too many actual parameters.";
					Error(18, lex.line, msg).print();
				}
				else {
					msg = "\'" + proc + "\': too few arguments for call.";
					Error(17, lex.line, msg).print();
				}
			}
		}
		match(RPAR, file, Error(4, lex.line));
		match(SEMI, file, Error(6, lex.line));
	}
	return s;
}


//<无返回值函数调用语句>::= <标识符>'('<值参数表>')'
Stmt Parser::voidProcCall(ifstream &file) {
	Token tok = look;
	move(file);
	return voidProcCall(file, tok);
}
//TO-DO
//<条件>::= <表达式><关系运算符><表达式>|<表达式>
Rel Parser::rel(ifstream &file) {
	Expr x = expr(file);
	Token tok;
	Rel r;
	switch (look.tag) {
		case LT: case LE: case GT: case GE: case NE: case EQU:
			tok = look;
			move(file);
			r = Rel(tok, x, expr(file));
			r.gen();
			return r;
		default:
			Expr x0;
			x0.op.tag = CONSTANT;
			x0.op.lexeme = "0";
			x0.type = Type::_int;
			r = Rel(Token(NE, "!="), x, x0);
			r.gen();
			return r;
	}
}

//<表达式>::= [+|-]<项>{<加法运算符><项>}
Expr Parser::expr(ifstream &file) {
	Expr x;
	if (look.tag == PLUS || look.tag == MINUS) {
		if (look.tag == MINUS) {
			move(file);
			/*if (look.tag == PLUS) {
				x = expr(file);
				return x;
			}*/
			Unary unary(Token(MINUS, "-"), term(file));
			unary.gen();
			x = unary.temp;
			(*(tables.get(proc))).putTemp(unary.temp.op.lexeme, unary.temp);
			//return x;
		}
	}
	//TO-DO <项>
	else
		x = term(file);
	while (look.tag == PLUS || look.tag == MINUS) {
		Token tok = look;
		move(file);
		Arith arith(tok, x, term(file));
		arith.gen();
		x = arith.temp;
		(*(tables.get(proc))).putTemp(arith.temp.op.lexeme, arith.temp);
	}
	return x;
}

//<项>::= <因子>{<乘法运算符><因子>}
Expr Parser::term(ifstream &file) {
	//TO-DO <因子>
	Expr x = factor(file);
	while (look.tag == STAR || look.tag == DIV) {
		//TO-DO <乘法运算符>
		Token tok = look;
		move(file);
		Arith arith(tok, x, factor(file));
		arith.gen();
		x = arith.temp;
		(*(tables.get(proc))).putTemp(arith.temp.op.lexeme, arith.temp);
	}
	return x;
}

//<因子>::= <标识符>|'('<表达式>')'|<整数>|<有返回值函数调用语句>|<实数>|<字符>
Expr Parser::factor(ifstream &file) {
	//TO-DO
	Expr x;
	Id id;
	Stmt s;
	Token tok;
	switch (look.tag) {
		case ID:
			tok = look;
			move(file);
			if (look.tag == LPAR) {
				move(file);
				return procCallToExpr(file, tok);
			}
			id = (*(tables.get(proc))).get(tok.lexeme);
			return id;
			break;
		case LPAR:
			//TO-DO <表达式>
			move(file);
			x = expr(file);
			match(RPAR, file, Error(4, lex.line));
			break;
		case INTEGER:
			x.op.lexeme = look.lexeme;
			x.op.tag = INTEGER;
			x.type = Type::_int;
			move(file);
			break;
		case REAL:
			x.op.lexeme = look.lexeme;
			x.op.tag = FLOAT;
			x.type = Type::_float;
			move(file);
			break;
		case CHARACTER:
			x.op.lexeme = look.lexeme;
			x.op.tag = CHARACTER;
			x.type = Type::_char;
			move(file);
			break;
		default:
			break;
	}
	return x;
}

//<标识符>=<标识符>(+|-)<步长>	<步长>::= <非零数字>{<数字>}
Stmt Parser::pace(ifstream &file, Token tok) {
	Token temp;
	Arith arith;
	Id id;
	Expr n;
	Set s;
	if (look.tag == ID && look.lexeme == tok.lexeme) {
		id = (*(tables.get(proc))).get(look.lexeme);
		move(file);
		if (look.tag == PLUS || look.tag == MINUS) {
			temp = look;
			move(file);
			if (look.tag == INTEGER) {
				n.op.lexeme = look.lexeme;
				n.op.tag = INTEGER;
				n.type = Type::_int;
				arith = Arith(temp, id, n);
				(*(tables.get(proc))).putTemp(arith.temp.op.lexeme, arith.temp);
				arith.gen();				
				s = Set(id, arith.temp);
				s.gen();
				move(file);
				return s;
			}
		}
	}
	Error(12, lex.line).print();
	return s;
}

//<变量说明部分>::= <变量定义>;{<变量定义>;}
void Parser::varDecls(ifstream &file, Token tok1, Token tok2) {
	varDef(file, tok1, tok2);
	match(SEMI, file, Error(6, lex.line));
}

//<变量说明部分>::= <变量定义>;{<变量定义>;}
void Parser::varDecls(ifstream &file) {
	while (look.tag == INT || look.tag == FLOAT || look.tag == CHAR) {
		varDef(file);
		match(SEMI, file, Error(6, lex.line));
	}
}

//<变量定义>::= <类型标识符><标识符>{,<标识符>}
void Parser::varDef(ifstream &file, Token tok1, Token tok2) {
	Id id0;
	Type p;
	switch (tok1.tag) {
		case INT:
			p = Type::_int;
			break;
		case FLOAT:
			p = Type::_float;
			break;
		case CHAR:
			p = Type::_char;
			break;
		default:
			break;
	}
	id0 = Id(Word(tok2.toString(), tok2.tag), p, used, VAR, lex.line);
	(*(tables.get(proc))).put(id0.toString(), id0);
	used += p.width;
	while (look.tag == COMMA) {
		move(file);
		if (look.tag == ID) {
			Id id = Id(Word(look.toString(), look.tag), p, used, VAR, lex.line);
			(*(tables.get(proc))).put(id.toString(), id);
			used += p.width;
			move(file);
		}
		else
			Error(12, lex.line).print();
	}
}

//<变量定义>::= <类型标识符><标识符>{,<标识符>}
void Parser::varDef(ifstream &file) {
	Token tok1, tok2;
	tok1 = look;
	move(file);
	if (look.tag == ID) {
		tok2 = look;
		move(file);
		varDef(file, tok1, tok2);
	}
	else {
		Error(12, lex.line).print();
	}
}

//<常量说明部分>::= const<常量定义>;{<常量定义>;}
void Parser::constDecls(ifstream &file) {
	Token tok1, tok2;
	match(CONST, file);
	if (look.tag == INT || look.tag == FLOAT || look.tag == CHAR) {
		constDef(file);
	}
	else {
		Error(12, lex.line).print();
	}
	while (look.tag == INT || look.tag == FLOAT || look.tag == CHAR) {
		tok1 = look;
		move(file);
		if (look.tag == ID) {
			tok2 = look;
			move(file);
			if (look.tag == ASSIGN)
				constDef(file, tok1, tok2);
			else if (look.tag == SEMI || look.tag == COMMA)
				varDecls(file, tok1, tok2);
			else if (look.tag == LPAR) {
				switch (tok1.tag) {
					case INT:
						procDef(file, tok2, Type::_int);
						break;
					case FLOAT:
						procDef(file, tok2, Type::_float);
						break;
					case CHAR:
						procDef(file, tok2, Type::_char);
						break;
					default:
						Error(13, lex.line).print();
				}
			}
			else
				Error(6, lex.line).print();
		}
		else
			Error(12, lex.line).print();
	}
}

//<常量定义>::= int<标识符>＝<整数>{,<标识符>=<整数>}
//				|float<标识符>=<实数>{,<标识符>=<实数>}
//				|char<标识符>=<字符>{,<标识符>=<字符>}
void Parser::constDef(ifstream &file, Token tok1, Token tok2) {
	match(ASSIGN, file);
	Id id0;
	Token tok0 = look;
	Type p;
	string temp1 = "";
	if (look.tag == PLUS || look.tag == MINUS) {
		if (look.tag == MINUS)
			temp1 = "-";
		move(file);
		tok0 = look;
	}
	switch (tok1.tag) {
		case INT:
			if (look.tag == INTEGER) {
				p = Type::_int;
				id0.value = temp1 + look.lexeme;
			}
			else
				Error(23, lex.line).print();
			break;
		case FLOAT:
			if (look.tag == REAL) {
				p = Type::_float;
				id0.value = temp1 + look.lexeme;
			}
			else
				Error(23, lex.line).print();
			break;
		case CHAR:
			if (look.tag == CHARACTER) {
				p = Type::_char;
				id0.value = temp1 + look.lexeme;
			}
			else
				Error(23, lex.line).print();
			break;
		default:
			break;
	}
	id0 = Id(Word(tok2.toString(), tok2.tag), p, used, CONSTANT, lex.line);
	id0.value = temp1 + look.lexeme;
	(*(tables.get(proc))).put(id0.toString(), id0);
	Quadruple q0;
	q0.res = id0.op.lexeme;
	q0.expr1 = id0.value;
	q0.op = "=";
	q0.quadstring = q0.res + " = " + q0.expr1;
	(*(tables.get(proc))).quad.push_back(q0);
	used += p.width;
	move(file);
	while (look.tag == COMMA) {
		move(file);
		tok2 = look;
		match(ID ,file, Error(12, lex.line));
		match(ASSIGN, file, Error(16, lex.line));
		Id id;
		string temp2 = "";
		if (look.tag == PLUS || look.tag == MINUS) {
			if (look.tag == MINUS)
				temp2 = "-";
			move(file);
		}
		if (look.tag == tok0.tag) {
			id = Id(Word(tok2.toString(), tok2.tag), p, used, CONSTANT, lex.line);
			id.value = temp2 + look.lexeme;
			(*(tables.get(proc))).put(id.toString(), id);
			used += p.width;
			Quadruple q;
			q.res = id.op.lexeme;
			q.expr1 = id.value;
			q.op = "=";
			q.quadstring = q.res + " = " + q.expr1;
			(*(tables.get(proc))).quad.push_back(q);
			move(file);
		}
		else
			Error(23, lex.line).print();
	}
	match(SEMI, file, Error(6, lex.line));
}

//<常量定义>::= int<标识符>＝<整数>{,<标识符>=<整数>}
//				|float<标识符>=<实数>{,<标识符>=<实数>}
//				|char<标识符>=<字符>{,<标识符>=<字符>}
void Parser::constDef(ifstream &file) {
	Token tok1, tok2;
	if (look.tag == INT || look.tag == FLOAT || look.tag == CHAR) {
		tok1 = look;
		move(file);
		if (look.tag == ID) {
			tok2 = look;
			move(file);
			constDef(file, tok1, tok2);
		}
		else
			Error(12, lex.line).print();
	}
	else
		Error(13, lex.line).print();
}

#include<fstream>
#include<iostream>
#include<string>
#include<cctype>
#include<map>
using namespace std;
//类别编码
enum Tag {
	Null = 0,
	CONST,
	INT,
	FLOAT,
	CHAR,
	VOID,
	IF,
	ELSE,
	WHILE,
	FOR,
	RETURN,
	MAIN,
	PRINTF,
	SCANF,
	ID,
	CHARACTER,
	STRING,
	INTEGER,
	REAL,
	PLUS,
	MINUS,
	STAR,
	DIV,
	LT,
	GT,
	ASSIGN,
	LPAR,
	RPAR,
	LBRACE,
	RBRACE,
	SQUOTE,
	DQUOTE,
	DOT,
	COMMA,
	SEMI,
	LE,
	GE,
	NE,
	EQU,
	CONSTANT,
	VAR,
	PARA,
	MAINPROC,
	VOIDPROC,
	PROC,
	NEG,
	TEMP
};

//词法单元
class Token {
public:
	int tag;//编码
	string lexeme;//词素
	Token() {}
	Token(int t) { tag = t; }
	Token(int t, string s) { tag = t; lexeme = s; }
	~Token() { }
	string toString() { return lexeme; }
	void print() { cout << tag << endl; }
};

//整数
class Num : public Token {
public:
	int value;//值
	Num(int v) : Token(INTEGER) { value = v; }
	~Num() { }
	void print() { cout << tag << "," << toString() << endl; }
};

//单词(包括关键字和标识符)
class Word : public Token {
public:
	static Word le, ge, ne, eq, neg, temp;
	Word() : Token() {}
	Word(string s, int tag) : Token(tag) { lexeme = s; }
	~Word() { }
	void print() { cout << tag << "," << toString() << endl; }
};

//实数
class Real : public Token {
public:
	float value;//值
	Real(float v) : Token(REAL) { value = v; }
	~Real() { }
	void print() { cout << tag << "," << toString() << endl; }
};

//字符
class Char : public Token {
public:
	char value;//值
	Char(char c) : Token(CHARACTER) {value = c; }
	~Char() { }
	int toInteger() { return ((int)value - 48); }
	void print() { cout << tag << "," << toString() << endl; }
};

//类型标识符
class Type : public Word {
public:
	int width;//所占内存大小
	static Type _int, _float, _char, _null;
	Type() : Word() { }
	Type(string s, int tag, int w) : Word(s, tag) { width = w; }
	~Type() { }
	static bool numeric(Type p);//是否为合法类型
	static Type max(Type p1, Type p2);//类型自动提升
};

//词法分析器
class Lexer {	
public:
	char peek;//当前字符
	map<string, Word> words;
	static int line;//当前行数
	Lexer();
	~Lexer() { }
	void reserve(Word w) { words.insert(map<string, Word>::value_type(w.lexeme, w)); }//保存关键字
	void readch(ifstream &file);//向前读一字符
	bool readch(char c, ifstream &file);//向前读一字符并判断是否为指定字符
	Token scan(ifstream &file);//读取词法单元并返回
};

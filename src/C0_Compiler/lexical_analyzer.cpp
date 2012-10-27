#pragma warning(disable:4786)
#include<fstream>
#include<cctype>
#include<map>
#include "lexical_analyzer.h"
#include "error.h"
using namespace std;

void tolowerString(string &s) {
	int i, length = s.size();

	for (i = 0; i < length; i++) {
		s[i] = tolower(s[i]);
	}
}

Word Word::le = Word::Word("<=", LE);
Word Word::ge = Word::Word(">=", GE);
Word Word::ne = Word::Word("!=", NE);
Word Word::eq = Word::Word("==", EQU);
Word Word::neg = Word::Word("neg", NEG);
Word Word::temp = Word::Word("t", TEMP);

Type Type::_int = Type("int", INT, 4);
Type Type::_float = Type("float", FLOAT, 8);
Type Type::_char = Type("char", CHAR, 1);
Type Type::_null = Type("null", Null, 0);

bool Type::numeric(Type p) {
	if (p.tag == INT || p.tag == FLOAT || p.tag == CHAR)
		return true;
	else
		return false;
}

Type Type::max(Type p1, Type p2) {
	if (!numeric(p1) || !numeric(p2))
		return Type::_null;
	else if (p1.tag == FLOAT || p2.tag == FLOAT)
		return Type::_float;
	else if (p1.tag == INT || p2.tag == INT)
		return Type::_int;
	else
		return Type::_char;
}

int Lexer::line = 1;

Lexer::Lexer() {
	peek = ' ';
	reserve(Word("const", CONST));
	reserve(Word("void", VOID));
	reserve(Word("if", IF));
	reserve(Word("else", ELSE));
	reserve(Word("while", WHILE));
	reserve(Word("for", FOR));
	reserve(Word("return", RETURN));
	reserve(Word("main", MAIN));
	reserve(Word("printf", PRINTF));
	reserve(Word("scanf", SCANF));
	reserve(Word::eq);
	reserve(Word::ge);
	reserve(Word::le);
	reserve(Word::ne);
	reserve(Word("int", INT));
	reserve(Word("float", FLOAT));
	reserve(Word("char", CHAR));
}

void Lexer::readch(ifstream &file) {
	//TO-DO
	if (file.eof())
		peek = EOF;
	file.get(peek);
}

bool Lexer::readch(char c, ifstream &file) {
	readch(file);
	if (peek != c)
		return false;
	peek = ' ';
	return true;
}

Token Lexer::scan(ifstream &file) {
	while (1) {
		if (peek == ' ' || peek == '\t') {
			readch(file);
			continue;
		}
		else if (peek == '\n') {
			line++;
			readch(file);
		}
		else
			break;
	}
	Token t;
	switch (peek) {
		case '<':
			if (readch('=', file)) {
				return Word::le;
			}
			else {
				t = Token(LT, "<");
				return t;
			}
		case '>':
			if (readch('=', file)) {
				return Word::ge;
			}
			else {
				t = Token(GT, ">");
				return t;
			}
		case '=':
			if (readch('=', file)) {
				return Word::eq;
			}
			else {
				t = Token(ASSIGN, "=");
				return t;
			}
		case '!':
			if (readch('=', file)) {
				return Word::ne;
			}
			else {
				Error(16, line).print();
			}
		case '+':
			t = Token(PLUS, "+");
			readch(file);
			return t;
		case '-':
			t = Token(MINUS, "-");
			readch(file);
			return t;
		case '*':
			t = Token(STAR, "*");
			readch(file);
			return t;
		case '/':
			t = Token(DIV, "/");
			readch(file);
			return t;
		case '(':
			t = Token(LPAR);
			readch(file);
			return t;
		case ')':
			t = Token(RPAR);
			readch(file);
			return t;
		case '{':
			t = Token(LBRACE);
			readch(file);
			return t;
		case '}':
			t = Token(RBRACE);
			readch(file);
			return t;
		case ',':
			t = Token(COMMA);
			readch(file);
			return t;
		case ';':
			t = Token(SEMI);
			readch(file);
			return t;
		case EOF:
			t = Token(Null);
			return t;
		default:
			break;
	}
	if (isdigit(peek)) {
		int v = 0;
		string s = "";

		do {
			s += peek;
			v = 10 * v + (int)peek - 48;
			readch(file);
		}
		while (isdigit(peek));
		if (peek != '.') {
			Num n = Num(v);
			n.lexeme = s;
			return (reinterpret_cast<Token &>(n));
		}
		s += ".";
		float x = v;
		float d = 10;
		while (1) {
			readch(file);
			if (!isdigit(peek))
				break;
			s += peek;
			x += ((int)peek - 48) / d;
			d *= 10;
		}
		Real r = Real(x);
		r.lexeme = s;
		return (reinterpret_cast<Token &>(r));
	}
	if (isalpha(peek) || peek == '_') {
		string s;
		do {
			s += peek;
			readch(file);
		}
		while (isalnum(peek) || peek == '_');
		map<string, Word>::iterator iter = words.find(s);
		Word w;
		if (iter != words.end()) {
			w = (*iter).second;
			return (reinterpret_cast<Token &>(w));
		}
		tolowerString(s);
		w.lexeme = s;
		w.tag = ID;
		words.insert(map<string, Word>::value_type(s, w));
		return (reinterpret_cast<Token &>(w));
	}
	if (peek == '\'') {
		readch(file);
		Char c(peek);
		c.lexeme = "\' \'";
		c.lexeme[1] = peek;
		readch(file);
		if (peek != '\'') {
			Error(1, line).print();
		}
		else
			readch(file);
		return (reinterpret_cast<Token &>(c));
	}
	if (peek == '\"') {
		string s;
		s += "\"";
		while (1) {
			readch(file);
			if (peek == '\"') {
				readch(file);
				break;
			}
			s += peek;
		}
		s += "\"";
		Word w(s, STRING);
		return w;
	}
	string msg = "illegal character.";
	Error(0, line, msg).print();
	readch(file);
	return scan(file);
}

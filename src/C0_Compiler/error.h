#include<iostream>
#include<string>

using namespace std;

class Error {
public:
	static int count;
	int errorType, errorLine;
	string errorMessage;
	Error() { }
	Error(int type, int line);
	Error(int type, int line, string msg);
	~Error() { }
	void print();
};

class Warning {
public:
	static int count;
	int warnType, warnLine;
	string warnMessage;
	Warning() { }
	Warning(int type, int line);
	Warning(int type, int line, string msg);
	~Warning() { }
	void print();
};

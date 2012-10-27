#pragma warning(disable:4786)
#include<iostream>
#include<string>
#include "error.h"
using namespace std;

//functions
string intToString1(int n) {
	string s = "";

	if (n == 0) {
		s = "0";	
		return s;
	}
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

int Error::count = 0;

Error::Error(int type, int line) {
	//count++;
	errorType = type;
	errorLine = line;
	switch (type) {
		//非法字符
		case 0:
			errorMessage = "illegal character.";
			break;
		//缺少'\''
		case 1:
			errorMessage = "missing '\''.";
			break;
		//缺少'\"'
		case 2:
			errorMessage = "missing '\"'.";
			break;
		//缺少'('
		case 3:
			errorMessage = "missing '('.";
			break;
		//缺少')'
		case 4:
			errorMessage = "missing ')'.";
			break;
		//缺少','
		case 5:
			errorMessage = "missing ','.";
			break;
		//缺少';'
		case 6:
			errorMessage = "missing ';'.";
			break;
		//缺少'{'
		case 7:
			errorMessage = "missing '{'.";
			break;
		//缺少'}'
		case 8:
			errorMessage = "missing '}'.";
			break;
		//非法符号
		case 9:
			errorMessage = "illegal token.";
			break;
		//标识符未定义
		case 10:
			errorMessage = "identifier has not been definited.";
			break;
		//标识符重复定义
		case 11:
			errorMessage = "identifier has already been definited.";
			break;
		//应是标识符
		case 12:
			errorMessage = "missing identifier.";
			break;
		//应是类型标识符
		case 13:
			errorMessage = "missing type.";
			break;
		//函数未定义
		case 14:
			errorMessage = "function has not been definited.";
			break;
		//函数重复定义
		case 15:
			errorMessage = "function has already been definited.";
			break;
		//缺少'='
		case 16:
			errorMessage = "missing '='.";
			break;
		//函数调用时参数过少
		case 17:
			errorMessage = "too few arguments for call.";
			break;
		//函数调用时参数过多
		case 18:
			errorMessage = "too many actual parameters.";
			break;
		//函数调用时参数类型不匹配
		case 19:
			errorMessage = "proc: type conversion error.";
			break;
		//函数没有返回值
		case 20:
			errorMessage = "must return a value.";
			break;
		//void函数有返回值
		case 21:
			errorMessage = "\'void\' function returning a value.";
			break;
		//返回值类型不匹配
		case 22:
			errorMessage = "return: type error.";
			break;
		//类型不匹配
		case 23:
			errorMessage = "type conversion error.";
			break;
		//常量被修改
		case 24:
			errorMessage = "cannot assign to a variable that is const.";
			break;
		default:
			break;
	}
}

Error::Error(int type, int line, string msg) {
	//count++;
	errorType = type;
	errorLine = line;
	errorMessage = msg;
}

void Error::print() {
	count++;
	cout << "error " + intToString1(errorType) + ": " + errorMessage + "\tline " + intToString1(errorLine) << endl;
}

int Warning::count = 0;

Warning::Warning(int type, int line) {
	
	warnType = type;
	warnLine = line;
	switch (type) {
		//float类型转换至int类型
		case 0:
			warnMessage = "conversion from \'float\' to \'int\', possible loss of data.";
			break;
		//float类型转换至char类型
		case 1:
			warnMessage = "conversion from \'float\' to \'char\', possible loss of data.";
			break;
		default:
			break;
	}
}

Warning::Warning(int type, int line, string msg) {
	warnType = type;
	warnLine = line;
	warnMessage = msg;
}

void Warning::print() {
	count++;
	cout << "warning " + intToString1(warnType) + ": " + warnMessage + "\tline " + intToString1(warnLine) << endl;
}
#pragma warning(disable:4786)
#include<fstream>
#include<iostream>
#include<string>
#include "code_generator.h"
#define MAX_LENGTH 128
using namespace std;

int main()
{
	ifstream file;
	Lexer lexer;
	Generator generator;
	char path[MAX_LENGTH] = "test.txt", is_default, is_optimize;
	
	cout << "是否使用默认源程序文件(Y/N):" << endl;
	cin >> is_default;
	if (is_default != 'Y' && is_default != 'y') {
		cout << "请输入源程序文件名:" << endl;
		cin >> path;
	}
	cout << "是否开启优化(Y/N):" << endl;
	cin >> is_optimize;
	file.open(path);
	if (file) {
		Parser parse(lexer, file);

		parse.program(file);
		cout << "中间代码:" << endl;
		if (is_optimize == 'Y' || is_optimize == 'y') {
			generator.init(parse);
		}
		else {
			generator.init_n_optimize(parse);
		}
		cout << endl << "X86汇编指令:" << endl;
		generator.gen();
		generator.output();
		file.close();
	}
	else {
		cout << "file does not exist." << endl;
	}
	return 0;
}

#pragma once
#include <iostream>
#include <string>
using std::string;
using std::cout;
using std::endl;

class Logger
{
private:
	string name;
	void head() {
		cout << "[" << name << "] ";
	}
public:
	Logger(){
		name = "Wheat";
	}
	Logger(string name_) {
		name = name_;
	}

	void info(string msg) {
		head();
		cout << (msg) << endl;
	}
	void info(int msg) {
		head();
		cout << msg << endl;
	}
	void error(string msg) {
		head();
		cout << msg << endl;
	}
	void error(int msg) {
		head();
		cout << msg << endl;
	}
};

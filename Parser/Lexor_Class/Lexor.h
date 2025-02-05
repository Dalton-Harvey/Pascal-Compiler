#pragma once
#include "../includes.h"


struct CToken
{
	std::string tokenType = "";
	std::string tokenValue= "";
};

class CLex
{

public:

	CLex(std::string LexTarg);

	CLex();

	void InitLex(std::string LexTarg);

	bool GetToken(CToken& Token);

private:
	std::ifstream Fdfa;
	std::ifstream Code;
	std::ifstream Frwords;
	char CurrChar;
	int dfa[128][15];
	int Pstate;
	int Cstate;
	bool Reserved;
	std::map<int, std::string> types{ {1, "word"}, { 2, "integer" }, {3,"integer"}, { 4, "Real" }, {9, "Real"}, {10, "Special"}, {99, "ERROR"}, {11, "Special"}, {12, "Special"},{13, "Special"}, {14,"Special"}};
	std::vector<std::string> rwords;

};
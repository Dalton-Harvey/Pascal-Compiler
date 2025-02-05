#include "Parser_Class/Parser.h"



int main()
{
	std::string FileName = "code.txt";

	TParser Parser(FileName);

	if (Parser.parse())
		std::cout << "Valid Code" << std::endl;
	else
		std::cout << "Invalid Code" << std::endl;

	return 0;
}











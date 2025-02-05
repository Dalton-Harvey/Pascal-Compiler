#include "Lexor.h"



CLex::CLex(std::string LexTarg)
{
	//Open dfa file
	Code.open(LexTarg);
	Fdfa.open("Lexor_Class/dfa.txt");
	Frwords.open("Lexor_Class/rwords.txt");
	//Read dfa into 2d array
	for (int y = 0; y <= 127; y++)
	{
		for (int x = 0; x <= 13; x++)
		{
			Fdfa >> dfa[y][x];
		}
	}
	std::string Temp;
	while (true)
	{
		Frwords >> Temp;
		rwords.push_back(Temp); //breaks the production up into 2 parts the first 2 characters are reduction everything else is what is being reduced
		if (Frwords.eof())
			break;
	}
}

CLex::CLex()
{

	
}

void CLex::InitLex(std::string LexTarg)
{
	//Open dfa file
	Fdfa.open("Lexor_Class/dfa.txt");
	Code.open(LexTarg);
	Frwords.open("Lexor_Class/rwords.txt");

	if (!Code.is_open())
	{
		std::cout << "Code File Not Found" << std::endl;
	}

	//Read dfa into 2d array
	for (int y = 0; y <= 127; y++)
	{
		for (int x = 0; x <= 14; x++)
		{
			Fdfa >> dfa[y][x];
		}
	}
	std::string Temp;
	while (true)
	{
		Frwords >> Temp;
		rwords.push_back(Temp); //breaks the production up into 2 parts the first 2 characters are reduction everything else is what is being reduced
		if (Frwords.eof())
			break;
	}
}

bool CLex::GetToken(CToken& Token)
{
	Reserved = false;
	Cstate = 0;
	Pstate = 0;
	Token.tokenType = "";
	Token.tokenValue = "";
	while (Code.peek() != -1)
	{
		CurrChar = Code.get();
		Pstate = Cstate;
		Cstate = dfa[CurrChar][Cstate];

		if (Cstate != 0 && Cstate != 55 && Cstate != 99)//if character does not finish a valid data type or make a current one invalid
		{
			Token.tokenValue += CurrChar;
		}
		else if (Cstate == 55 && (CurrChar != 32 && CurrChar != 10 && CurrChar != 9 && CurrChar != 13))
		{
			Code.unget();
			if (Pstate == 3)
			{
				Code.unget();
				Token.tokenValue.pop_back();
			}
		}
		if (Cstate == 55)//if character is delimiter and the token is valid
		{
			if (Pstate == 1)//if token is a word
			{
				for (int x = 0; x < rwords.size(); x++)//loop through rwords array
				{
					if (Token.tokenValue == rwords[x])//if word is a reserved word set type to rword
					{
						Reserved = true;
						break;
					}
				}
			}
			if (Reserved == true)
				Token.tokenType = "rword";
			else
				Token.tokenType = types[Pstate];
			return true;
		}
		else if (Cstate == 99)//if character makes the token invalid
		{
			Token.tokenType = types[99];
			Token.tokenValue += CurrChar;
			return false;
		}
	}
	if (Cstate == 55 || Cstate == 1 || Cstate == 10 || Cstate == 2 || Cstate == 9 || Cstate == 4 || Cstate == 12 || Cstate == 13 || Cstate == 11 || Cstate==14)//if you reach the end of file and the current state is a final state
	{
		if (Cstate == 1)//if token is a word
		{
			for (int x = 0; x < rwords.size(); x++)//loop through rwords array
			{
				if (Token.tokenValue == rwords[x])//if word is a reserved word set type to rword
				{
					Reserved = true;
				}
			}
		}
		if(Reserved == true)
			Token.tokenType = "rword";
		else
			Token.tokenType = types[Cstate];
		return true;
	}
	else
	{
		if(Cstate == 99)
		{
			std::cout << "Lex Error" << std::endl;
			return false;
		}
		else
		{
			Token.tokenType = "Special";
			Token.tokenValue = "$";
			return true;
		}
	}
}



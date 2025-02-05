#include "Parser.h"

TParser::TParser(std::string FileName)
{
	Lexer.InitLex(FileName);
	AssemblyFile.open("../AssemblyCode/assembler.cpp");
	


	RegisterUsed.push("edx");
	RegisterUsed.push("ecx");
	RegisterUsed.push("ebx");
	RegisterUsed.push("eax");

	Opcodes["<"] = "jl";
	Opcodes[">"] = "jg";
	Opcodes["="] = "je";
	Opcodes["+"] = "add";
	Opcodes["-"] = "sub";
	Opcodes["*"] = "imul";
	Opcodes["/"] = "idiv";
	Opcodes["or"] = "or";
	Opcodes["and"] = "and";

	CompReverseOpcodes["<"] = "jge";
	CompReverseOpcodes[">"] = "jle";
	CompReverseOpcodes["="] = "jne";

	ScopeRegisters["global"] = "edi";
	ScopeRegisters["local"] = "ebp";
	ScopeRegisters["param"] = "ebp";

	ScopeDirection["global"] = "+";
	ScopeDirection["local"] = "-";
	ScopeDirection["param"] = "+";


}

bool TParser::parse()
{
	if (!AssemblyFile.is_open())
	{
		std::cout << "Failed to open Assembly File" << std::endl;
		return false;
	}

	if (!GetNextDToken())
		return false;
	
	//dump program header into assmbler.cpp
	ProgramHeader();

	//can be reduced to return (VPF() && block() && DKey == "." && GetNextDToken() && DKey.compare("$") != 0)
	if (!VPF())
		return false;
	
	//Creating label for main function for the assembly code
	AssemblyFile << "		Kmain:" << std::endl << std::endl;

	if (!block())
		return false;

	if (DKey != ".")
		return false;

	if (!GetNextDToken())
		return false;

	//dump program footer into assembler.cpp
	ProgramFooter();

	AssemblyFile.close();

	if (DKey.compare("$") != 0)//if there are any uncosumed tokens bad code
		return false;


	return true;
}

bool TParser::block()
{
	if (DKey == "begin")
	{
		if (!GetNextDToken())
			return false;

		if (S() && Sp() && DKey == "end")
		{
			if (!GetNextDToken())
				return false;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool TParser::S() 
{
	if (DKey == "word")
	{
		std::string TempWord = DToken.tokenValue;

		if (!FindandPush())//check if variable exists then push datatype to stack
			return false;

		if (!GetNextDToken())
			return false;

		//if word is an array
		if (DKey == "[")
		{
			if (!GetNextDToken())
				return false;

			ArrayData = &PStack.top();

			if (E())//check for valid index
			{
				if (PStack.top().DataType != "integer")
				{
					std::cout << "Invalid Index type" << std::endl;
					return false;
				}

				if (DKey == "]")
				{
					if (!GetNextDToken())
						return false;

					Array1DAssembly(true);
				}
				else if (DKey == ",")
				{
					if (!GetNextDToken())
						return false;

					if (E())
					{
						if(PStack.top().DataType != "integer")
						{
							std::cout << "Invlaid Index type" << std::endl;
							return false;
						}

						if (DKey == "]")
						{
							if (!GetNextDToken())
								return false;

							Array2DAssembly(true);
						}

					}
				}
				else
					return false;


			}
			else
			{
				std::cout << "Invalid Index" << std::endl;
				return false;
			}
			
		}

		if (DKey == ":=")
		{
			Assignment = true;
			if (FuncProc && !Table.ValidReturnVaraible(TempWord))//if the variable being called is the name of a function compare the name of the variable to the name of the scope
			{
				std::cout << "incorrect usage of return variable::" << TempWord << std::endl;
				return false;
			}

			if (!GetNextDToken())
				return false;

			return E() && PopandCheck() && AssignmentAssembly();//pop the top two datatypes off and check if they are the same
		}
		else if (DKey == "(" && FuncProc)//if the word is a function or procedure
		{
			if (!GetNextDToken())
				return false;

			StackData Temp = PStack.top();

			if (Temp.DataType == "void")
				PStack.pop();

			return CheckValidParam(TempWord);
		}

	}
	else if (DKey == "begin")
	{
		if (!GetNextDToken())
			return false;

		if (S() && Sp())
		{
			if (DKey == "end")
			{
				if (!GetNextDToken())
					return false;
				return true;
			}
		}
	}
	else if (DKey == "while")
	{
		if (!GetNextDToken())
			return false;
		LogiData Temp;
		Temp.Depth = IfWhileCount;
		Temp.LogicType = "While";
		*asmOut << "	TopWhile" + std::to_string(IfWhileCount) + ":" << std::endl;
		IfWhileCount++;
		
		IfWhile.push(Temp);
		if (E())
		{
			if (DKey == "do")
			{
				if (!GetNextDToken())
					return false;
				WhileAssembly();
				return S() && EndWhileAssembly();
			}
		}
	}
	else if (DKey == "if")
	{
		if (!GetNextDToken())
			return false;

		LogiData Temp;
		Temp.Depth = IfWhileCount;
		Temp.LogicType = "If";
		IfWhile.push(Temp);
		IfWhileCount++;

		if (E())
		{
			StackData Temp = PStack.top();
			if (Temp.DataType == "boolean")
			{
				PStack.pop();
				if (DKey == "then")
				{
					if (!GetNextDToken())
						return false;

					IfAssembly();
					IfWhileLabel = "If";
					return (S() && IFp() && EndIfAssembly());
				}
			}
			else
			{
				std::cout << "If statements only take booleans" << std::endl;
				return false;
			}
		}
	}
	else
		return true;
}

bool TParser::Sp()
{
	if (DKey == ";")
	{
		if (!GetNextDToken())
			return false;

		return (S() && Sp());
	}
	else
		return true;
}

bool TParser::IFp()
{
	if (DKey == "else")
	{
		if (!GetNextDToken())
			return false;

		ElseAssembly();

		return S();
	}
	else
		return true;
}

bool TParser::E()
{
	return SE();
}

bool TParser::SE()
{
	return (SER() && SEp());
}

bool TParser::SEp()
{
	if (DKey == "<" || DKey == ">" || DKey == "=")
	{

		std::string CurrentOperation = DKey;
		LastOperation = DKey;
		if (!GetNextDToken())
			return false;

		if (SER() && PopandCheck())//check if expression is valid and check if operation between datatypes is valid
		{
			StackData Temp;
			Temp.DataType = "boolean";
			if (RightData.VarScope == "literal" && LeftData.VarScope == "literal" && Assignment)//if both are literal just do the operation 
			{
				if(CurrentOperation == "<")
					Temp.location = std::to_string(std::stoi(LeftData.location) < std::stoi(RightData.location));
				else if(CurrentOperation == ">")
					Temp.location = std::to_string(std::stoi(LeftData.location) > std::stoi(RightData.location));
				else if(CurrentOperation == "=")
					Temp.location = std::to_string(std::stoi(LeftData.location) == std::stoi(RightData.location));

				Temp.VarScope = "literal";
			}
			else
			{

				Temp.location = ComparisonAssembly(CurrentOperation);//dump the assembly code for the operation if assigning bool value
				

				Temp.InReg = true;
				Temp.VarScope = "";
			}

			PStack.push(Temp);

			return SEp();
		}
		else
			return false;
	}
	else
		return true;

}

bool TParser::SER()
{
	return T();
}

bool TParser::T()
{
	return (TR() && Tp());
}

bool TParser::Tp()
{
	if (DKey == "+" || DKey == "-")
	{
		std::string CurrentOperation = DKey;

		if (!GetNextDToken())
			return false;

		if (TR() && PopandCheck())//check is term is valid then check if operation can be done on datatypes
		{
			StackData Temp;
			Temp.DataType = "integer";
			
			if (LeftData.VarScope == "literal" && RightData.VarScope == "literal")
			{
				if (CurrentOperation == "+")
					Temp.location = std::to_string(std::stoi(LeftData.location) + std::stoi(RightData.location));
				else
					Temp.location = std::to_string(std::stoi(LeftData.location) - std::stoi(RightData.location));
				Temp.VarScope = "literal";
			}
			else
			{
				Temp.location = AddSubAssembly(CurrentOperation);
				Temp.InReg = true;
				Temp.VarScope = "";
			}

			PStack.push(Temp);

			return Tp();
		}
		else
			return false;

	}
	else if (DKey == "or")
	{
		std::string CurrentOperation = DKey;

		OrAssembly();

		if (!GetNextDToken())
			return false;

		if (TR() && PopandCheck())//check is term is valid then check if operation can be done on datatypes
		{
			if (LeftData.DataType != "boolean")
			{
				std::cout << "boolean required to do the OR operation" << std::endl;
				return false;
			}
			StackData Temp;
			Temp.DataType = "boolean";
			if(Assignment)
			{

				if (LeftData.VarScope == "literal" && RightData.VarScope == "literal")
				{
					Temp.location = std::to_string(std::stoi(LeftData.location) | std::stoi(RightData.location));
					Temp.VarScope = "literal";
				}
				else
				{
					Temp.location = AddSubAssembly(CurrentOperation);
					Temp.InReg = true;
					Temp.VarScope = "";
				}
			}

			PStack.push(Temp);

			return Tp();
		}
		else
			return false;

	}
	else
		return true;

}

bool TParser::TR()
{
	return F();
}

bool TParser::F()
{
	return FR() && Fp();
}

bool TParser::Fp()
{

	if (DKey == "*" || DKey== "/")
	{
		std::string CurrentOperation = DKey;

		if (!GetNextDToken())
			return false;

		if (FR() && PopandCheck())
		{
			StackData Temp;
			Temp.DataType = "integer";

			if (LeftData.VarScope == "literal" && RightData.VarScope == "literal")
			{
				if (CurrentOperation == "*")
					Temp.location = std::to_string(std::stoi(LeftData.location) * std::stoi(RightData.location));
				else
					Temp.location = std::to_string(std::stoi(LeftData.location) / std::stoi(RightData.location));
				Temp.VarScope = "literal";
			}
			else
			{
				if (CurrentOperation == "*")
					Temp.location = MultiAssembly();
				else
					Temp.location = DivAssembly();
				Temp.InReg = true;
				Temp.VarScope = "";
			}

			PStack.push(Temp);

			return Fp();
		}
		else
			return false;
	}
	else if (DKey == "and")
	{
		std::string LastOperation = DKey;

		AndAssembly();

		if (!GetNextDToken())
			return false;

		if (FR() && PopandCheck())
		{
			if (LeftData.DataType != "boolean")
			{
				std::cout << "boolean required to do the OR operation" << std::endl;
				return false;
			}

			StackData Temp;
			Temp.DataType = "boolean";

			if(Assignment)
			{
				if (LeftData.VarScope == "literal" && RightData.VarScope == "literal")
				{
					Temp.location = std::to_string(std::stoi(LeftData.location) & std::stoi(RightData.location));
					Temp.VarScope = "literal";
				}
				else
				{
					Temp.location = AddSubAssembly(LastOperation);
					Temp.InReg = true;
					Temp.VarScope = "";
				}
			}

			PStack.push(Temp);

			return Fp();
		}
		else
			return false;
	}
	else
		return true;

}

bool TParser::FR()
{
	StackData Temp;
	if (DKey == "Real" || DKey == "integer")
	{
		Temp.DataType = DKey;
		Temp.VarScope = "literal";
		Temp.location = DToken.tokenValue;

		if (Negative)
		{
			Negative = false;
			Temp.Negative = true;
		}

		PStack.push(Temp);



		if (!GetNextDToken())
			return false;

		return true;
	}
	else if (DKey == "true" || DKey == "false")
	{
		Temp.DataType = "boolean";
		Temp.VarScope = "literal";
		Temp.location = DKey == "true" ? "1" : "0";
		PStack.push(Temp);



		return GetNextDToken();
	}
	else if (DKey == "word")
	{
		std::string TempWord = DToken.tokenValue;
		if (FindandPush() && GetNextDToken())
		{
			if (FuncProc && DKey == "(")//if word is function or procedure
			{
				if (!GetNextDToken())
					return false;
				
				return CheckValidParam(TempWord);
			}
			if (DKey == "[")//if word is an array
			{
				if (!GetNextDToken())
					return false;

				ArrayData = &PStack.top();
				
				if (E())//check for valid index
				{
					if (PStack.top().DataType != "integer")
					{
						std::cout << "Invalid Index type" << std::endl;
						return false;
					}

					if (DKey == "]")
					{
						if (!GetNextDToken())
							return false;

						StackData NewTop;

						NewTop.location = Array1DAssembly(false);
						NewTop.DataType = PStack.top().DataType;
						NewTop.InReg = true;
						PStack.pop();
						PStack.push(NewTop);
					}
					else if (DKey == ",")
					{
						if (!GetNextDToken())
							return false;

						if (E())
						{
							if (PStack.top().DataType != "integer")
							{
								std::cout << "Invlaid Index type" << std::endl;
								return false;
							}

							if (DKey == "]")
							{
								if (!GetNextDToken())
									return false;


								StackData NewTop;

								NewTop.location = Array2DAssembly(false);
								NewTop.DataType = PStack.top().DataType;
								NewTop.InReg = true;
								PStack.pop();
								PStack.push(NewTop);

								
							}

						}
					}
					else
						return false;


				}
				else
				{
					std::cout << "Invalid Index" << std::endl;
					return false;
				}

			}
			
			return true;
		}
		else
			return false;
	}
	else if (DKey.compare("+") == 0 || DKey.compare("-") == 0)
	{
		if (DKey == "-")
			Negative = true;

		if (!GetNextDToken())
			return false;

		return TR();
	}
	else if (DKey.compare("(") == 0)
	{
		if (!GetNextDToken())
			return false;

		if (E())
		{
			if (DKey.compare(")") == 0)
			{
				if (!GetNextDToken())
					return false;

				return true;
			}
		}
	}
	

	return false;
}

bool TParser::VPF()
{
	if (DKey == "var")
	{
		if (!GetNextDToken())
			return false;

		if (DKey == "word")
		{
			WordList.push_back(DToken.tokenValue);

			if (!GetNextDToken())
				return false;

			if (VarList() && DKey == ":")
			{
				if (!GetNextDToken())
					return false;

				if (DataType(false) && DKey == ";")
				{
					if (!GetNextDToken())
						return false;

					return mvar() && VPF();
				}
				else
				{
					return false;
				}

			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else if (DKey == "procedure")
	{
		
		if (!GetNextDToken())
			return false;

		if (DKey == "word")
		{
			FuncName = DToken.tokenValue;
			Table.AddFuncProd(DToken.tokenValue, "void", "procedure");

			if (!GetNextDToken())
				return false;

			if (DKey == "(")
			{
				if (!GetNextDToken())
					return false;

				if (Param() && DKey == ")")
				{
					if (!GetNextDToken())
						return false;

					if (DKey == ";")
					{
						if (!GetNextDToken())
							return false;

						if (VPF() && ProcedureHeader() && block() && DKey == ";")
						{
							ProcedureFooter();
							Table.BackOut();

							if (!GetNextDToken())
								return false;

							return VPF();
						}
						else
							return false;
					}
					else
						return false;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	else if (DKey == "function")
	{
		if (!GetNextDToken())
			return false;

		if (DKey == "word")
		{
			FuncName = DToken.tokenValue;
			Table.AddFuncProd(DToken.tokenValue, "", "function");
			if (!GetNextDToken())
				return false;

			if (DKey == "(")
			{
				if (!GetNextDToken())
					return false;

				if (Param() && DKey == ")")
				{

					if (!GetNextDToken())
						return false;

					if (DKey == ":")
					{
						if (!GetNextDToken())
							return false;

						if (DataType() && DKey == ";")
						{
							if (!GetNextDToken())
								return false;

							if (VPF() && block() && DKey == ";")
							{

								Table.BackOut();

								if (!GetNextDToken())
									return false;

								return VPF();

							}
							else
								return false;
						}
						else false;
					}
					else
						return false;
				}
				else
					return false;
			}
			else
				return false;
		}
		else
			return false;
	}
	return true;
}

bool TParser::VarList()
{
	if (DKey == ",")
	{
		if (!GetNextDToken())
			return false;

		if (DKey == "word")
		{
			WordList.push_back(DToken.tokenValue);

			if (!GetNextDToken())
				return false;

			return VarList();
		}
		else
			return false;
	}
	return true;
}

bool TParser::DataType(bool Param)//overload for checking datatype of parameter or variable
{
	if (DKey == "boolean" || DKey == "integer" || DKey == "char" || DKey == "Real")
	{

		return AddVartoTable(Param) && GetNextDToken();

	}
	else if (DKey == "array")
	{
		int rowlowerbound;
		int rowupperbound;
		int collowerbound = 1;
		int colupperbound = 1;
		if(!GetNextDToken())
			return false;

		if (DKey == "[" && GetNextDToken())
		{
			if(DKey == "integer")
			{
				rowlowerbound = std::stoi(DToken.tokenValue);
				if (GetNextDToken() && DKey == ".." && GetNextDToken())
				{
					rowupperbound = std::stoi(DToken.tokenValue);
					if (!GetNextDToken())
						return false;

					if (DKey == "]" && GetNextDToken())//if declaring 1D array
					{
						if (DKey == "of")
						{
							if (!GetNextDToken())
								return false;

							if (DKey == "boolean" || DKey == "integer" || DKey == "char" || DKey == "Real")
							{
								return AddArraytoTable(DKey, rowlowerbound, rowupperbound, collowerbound, colupperbound) && GetNextDToken();
							}
							else
							{
								std::cout << "Invalid DataType" << std::endl;
								return false;
							}
						}
						else
							return false;
					}
					else if (DKey == "," && GetNextDToken())//if Declaring 2D array
					{
						if (DKey == "integer")
						{
							collowerbound = std::stoi(DToken.tokenValue);
							if (GetNextDToken() && DKey == ".." && GetNextDToken())
							{
								if (DKey == "integer")
								{
									colupperbound = std::stoi(DToken.tokenValue);
									if (!GetNextDToken())
										return false;
									if (DKey == "]" && GetNextDToken())
									{
										if (DKey == "of")
										{
											if (!GetNextDToken())
												return false;

											if (DKey == "boolean" || DKey == "integer" || DKey == "char" || DKey == "Real")
											{
												return AddArraytoTable(DKey, rowlowerbound, rowupperbound, collowerbound,colupperbound) && GetNextDToken();
											}
											else
											{
												std::cout << "Invalid DataType" << std::endl;
												return false;
											}
										}
										else
											return false;
									}
									else
										return false;

								}
							}
						}
					}
				}
				else
					return false;
			}

		}
		else
			return false;

	}
	else
		return false;
}

bool TParser::DataType()//overload for checking return type for function
{
	if (DKey == "boolean" || DKey == "integer" || DKey == "char" || DKey == "Real")
	{

		return (Table.UpdateReturnType(FuncName,DKey) && GetNextDToken());

	}
	else
		return false;
}

bool TParser::mvar()
{
	if (DKey == "word")
	{
		WordList.push_back(DToken.tokenValue);

		if (!GetNextDToken())
			return false;

		if (VarList() && DKey == ":")
		{
			if (!GetNextDToken())
				return false;

			if (DataType(false) && DKey == ";")
			{
				if (!GetNextDToken())
					return false;

				return mvar() && VPF();
			}
			else
				return false;
		}
		else
			return false;
	}
	else
		return true;
}

bool TParser::Param()
{
	if (DKey == "var")
	{
		PassbyRef = true;

		if (!GetNextDToken())
			return false;

		if (DKey == "word")
		{
			WordList.push_back(DToken.tokenValue);

			if (!GetNextDToken())
				return false;

			if (VarList() && DKey == ":")
			{
				if (!GetNextDToken())
					return false;

				return (DataType(true) && MParam());

			}
			else
				return false;
		}
		else
			return false;
	}
	else if (DKey == "word")
	{
		WordList.push_back(DToken.tokenValue);

		if (!GetNextDToken())
			return false;

		if (VarList() && DKey == ":")
		{
			if (!GetNextDToken())
				return false;

			return  (DataType(true) && MParam());
		}
		else
			return false;
	}
	return true;
}

bool TParser::MParam()
{
	if (DKey == ";")
	{
		if (!GetNextDToken())
			return false;

		return GParam();
	}
	return true;
}

bool TParser::GParam()
{
	if (DKey == "var")
	{

		PassbyRef = true;

		if (!GetNextDToken())
			return false;

		if (DKey == "word")
		{
			WordList.push_back(DToken.tokenValue);//add name to parser vector

			if (!GetNextDToken())
				return false;

			if (VarList() && DKey == ":")
			{
				if (!GetNextDToken())
					return false;

				if (DataType(true))
				{
					return MParam();
				}
			}
			else
				return false;
		}
	}
	else if (DKey == "word")
	{
		WordList.push_back(DToken.tokenValue);

		if (!GetNextDToken())
			return false;

		if (VarList() && DKey == ":")
		{
			if (!GetNextDToken())
				return false;

			return (DataType(true) && MParam());
		}
		else
			return false;
	}
	return false;
}

//Functions for Symbol Table and Token Manipulation
bool TParser::GetNextDToken()
{
	if (!Lexer.GetToken(DToken))
		return false;

	if (DToken.tokenType.compare("Special") == 0 || DToken.tokenType == "rword")//if token type is special the key is the value of the token
		DKey = DToken.tokenValue;
	else// If token type is a number ie real, int etc the key is the type of the token
		DKey = DToken.tokenType;

	return true;
}

bool TParser::PopandCheck()
{
	//pop the top two datatypes off and make sure they are the same
	RightData = PStack.top();
	PStack.pop();
	LeftData = PStack.top();
	PStack.pop();

	if (RightData.DataType != LeftData.DataType)
		std::cout << "Type Mismatch: " << LeftData.DataType << ":" << RightData.DataType << std::endl;

	return (RightData.DataType == LeftData.DataType);//make sure the two expressions have the same DataType
}

bool TParser::AddVartoTable(bool Param)
{
	//Add list to symbol table
	for (int x = 0; x < WordList.size(); x++)
	{
		int size = 0;
		if (DKey == "integer" || DKey == "Real")
			size = 4;
		else if (DKey == "boolean")
			size = 2;
		else if (DKey == "char")
			size = 1;
		
		if (!Table.AddVar(WordList[x], DKey, size, PassbyRef, Param))
		{
			std::cout << "Duplicate Identifier: " << WordList[x]<< std::endl;
			return false;
		}
	}

	if (PassbyRef)
		PassbyRef = false;

	WordList.clear();

	return true;
}

bool TParser::AddArraytoTable(std::string Datatype, int rowlowerbound, int rowupperbound, int collowerbound, int colupperbound)
{
	for (int x = 0; x < WordList.size(); x++)
	{
		int size = 0;
		if (DKey == "integer" || DKey == "Real")
			size = 4;
		else if (DKey == "boolean")
			size = 2;
		else if (DKey == "char")
			size = 1;

		int rowsize = (rowupperbound - rowlowerbound) + 1;
		int colsize = (colupperbound - collowerbound) + 1;

		size = size * (rowsize * colsize);

		if (!Table.AddArray(WordList[x], DKey, size, rowlowerbound, rowupperbound, collowerbound, colupperbound))
		{
			std::cout << "Duplicate Identifier: " << WordList[x] << std::endl;
			return false;
		}
	}

	WordList.clear(); 

	return true;
}

bool TParser::AddFunctoTable(std::string ReturnType)
{
	if (!Table.AddFuncProd(FuncName, ReturnType, "Function"))
	{
		std::cout << "Duplicate Identifier" << std::endl;
		
		return false;
	}

	return true;
}

bool TParser::FindandPush()
{
	StackData Temp;
	VarData OutData;
	if (Table.FindData(DToken.tokenValue, OutData, FuncProc))//find if word exists in the SymTable
	{
		Temp = OutData;
		PStack.push(Temp);

		return true;
	}
	else
	{
		std::cout << "Undeclared Identifier: " << DToken.tokenValue << std::endl;
		return false;
	}
}

bool TParser::CheckValidParam(std::string Name)
{
	AssemblyFile << "		//" << Name << " Parameter Assembly" << std::endl;
	AssemblyFile << "		//Begin" << std::endl << std::endl;

	asmOut = &ParamAssembly;
	std::string CurrRegister;
	StackData CurrData;
	StackData ParamData;
	VarData OutData;
	int paramCount = Table.ParamCount(Name);
	int CurrParam = 0;
	
	while (CurrParam < paramCount)
	{
		Table.FindParam(Name, CurrParam, CurrData.DataType, CurrData.ref, CurrData.Offset);
		ParamAssembly << "		//Param #" << CurrParam + 1 << std::endl;
		if (CurrData.ref)
		{
			if (DKey == "word")
			{
				Table.FindData(DToken.tokenValue, OutData, ParamData.InReg);
				ParamData = OutData;
				if (CurrData.DataType != ParamData.DataType)
				{
					std::cout << Name << "::" << "No Parameter with: " << ParamData.DataType << std::endl;
					return false;
				}

				//move Scope Register into first available register then add offset to that register then push onto Assembly Stack
				CurrRegister = FindEmptyRegister();
				ParamAssembly << "		mov " << CurrRegister << ", " <<  std::to_string(ParamData.Offset) << std::endl;
				ParamAssembly << "		add " << CurrRegister << ", " << ScopeRegisters[ParamData.VarScope] << std::endl;
				ParamAssembly << "		push " << CurrRegister << std::endl << std::endl;
				ParamStack.push(ParamAssembly.str());
				RegisterUsed.push(CurrRegister);
				ParamAssembly = std::ostringstream();


				if (!GetNextDToken())
					return false;
			}
			else
			{
				std::cout << Name << "::" << "Parameter needs to be variable" << std::endl;
				return false;
			}
		}
		else
		{
			if(E())
			{
				ParamData = PStack.top();
				PStack.pop();
				if (CurrData.DataType != ParamData.DataType)
				{
					std::cout << Name << "::" << "Parameter Type Mismatch: " << CurrData.DataType << ":" << ParamData.DataType << std::endl;
					return false;
				}

				//Add Assembly code here
				LeftData = ParamData;

				
				ParamAssembly << "		//put parameter into register if needed then push to stack" << std::endl;
				if (LeftData.InReg)
					ParamAssembly << "		push " << LeftData.Register << std::endl << std::endl;	
				else
				{
					bool LeftLiteral = false;
					CurrRegister = FindEmptyRegister();
					std::string LeftLocation = MoveintoReg(LeftData,CurrRegister);
					ParamAssembly << "		push " << LeftLocation << std::endl << std::endl;
				}

				RegisterUsed.push(CurrRegister);
				ParamStack.push(ParamAssembly.str());
				ParamAssembly = std::ostringstream();
			}
			else
				return false;
			
		}




		CurrParam++; 
		if (CurrParam < paramCount)
		{
			if (DKey == ",")
			{
				if (!GetNextDToken())
					return false;
			}
			else
			{
				std::cout << Name << "::" << "Expected \',\' instead of: " << DKey << std::endl;
				return false;
			}
		}
	}


	while (!ParamStack.empty())
	{
		AssemblyFile << ParamStack.top();
		ParamStack.pop();
	}

	asmOut = &AssemblyFile;


	AssemblyFile << "		call " + Name << std::endl;


	AssemblyFile << "		//End" << std::endl << std::endl;

	if(DKey == ")")
	{
		if (!GetNextDToken())
			return false;
		return true;
	}
	else
	{
		std::cout << Name << "::" << "Missing closing \")\"" << std::endl;
		return false;
	}
}

//Functions that write assembly code to assembler.cpp
std::string TParser::FindEmptyRegister()
{
	std::string Register = RegisterUsed.top();
	RegisterUsed.pop();
	return Register;

}

std::string TParser::MoveintoReg(StackData ToMove, std::string CurrRegister)
{
	if (ToMove.ref)//if RightData is passed by reference moved the pointer into esi
	{
		*asmOut << "		mov esi, " + ToMove.location << std::endl;
		*asmOut << "		mov " << CurrRegister << ", " << "[esi]" << "//Move the value stored at the variables address into a register" << std::endl;
	}
	else
		*asmOut << "		mov " << CurrRegister << ", " << ToMove.location << "//move left side into a register" << std::endl;

	if (ToMove.Negative)
		*asmOut << "		neg " + CurrRegister << std::endl;

	ToMove.location = CurrRegister;

	return ToMove.location;
}

std::string TParser::Move1DArrayintoReg(StackData ToMove, std::string CurrRegister)
{
	////get the index information from the stack
	//StackData IndexInfo = PStack.top();
	//PStack.pop();

	//std::string size;
	//if (ToMove.DataType == "integer" || ToMove.DataType == "Real")
	//	size = "4";
	//else if (ToMove.DataType == "boolean")
	//	size = "2";
	//else if (ToMove.DataType == "char")
	//	size = "1";


	////move it into a register if needed
	//if (!IndexInfo.InReg)
	//	IndexInfo.location = MoveintoReg(IndexInfo, CurrRegister);
	//else
	//	RegisterUsed.push(CurrRegister);

	////zero base index if the lowerbound is greater than 0
	//if (ToMove.rowlowerbound > 0)
	//	*asmOut << "		sub " + IndexInfo.location + ", " + std::to_string(ToMove.rowlowerbound) << std::endl;

	////multiply the index by the size of the data the array contains
	//*asmOut << "		imul " + IndexInfo.location + ", " + size << std::endl;

	////Add the offset to the beginning of the array to the index
	//if (ToMove.Offset > 0)
	//	*asmOut << "		add " + IndexInfo.location + ", " + std::to_string(ToMove.Offset) << std::endl;

	//ToMove.location = "[" + ScopeRegisters[ToMove.VarScope] + ScopeDirection[ToMove.VarScope] + IndexInfo.location + "]";

	//
	//*asmOut << "		mov " + IndexInfo.location + ", " + ToMove.location << std::endl;
	*asmOut << "		mov " + CurrRegister + ", " + ToMove.location << std::endl;

	return CurrRegister;
}

std::string TParser::Move2DArrayintoReg(StackData ToMove, std::string CurrRegister)
{
	return std::string();
}

std::string TParser::Moveintoesi(std::string Location)
{
	*asmOut << "		mov esi, " + Location << std::endl;
	return "[esi]";
}

void TParser::ProgramHeader()
{
	AssemblyFile << "#include <iostream>" << std::endl;
	AssemblyFile << "char DataSegment[65536];" << std::endl;
	AssemblyFile << "int* look;" << std::endl;
	AssemblyFile << "int main()" << std::endl;
	AssemblyFile << "{" << std::endl;
	AssemblyFile << "	look = (int*)DataSegment;" << std::endl;
	AssemblyFile << "	_asm{" << std::endl;
	AssemblyFile << "		push eax" << std::endl;
	AssemblyFile << "		push ebx" << std::endl;
	AssemblyFile << "		push ecx" << std::endl;
	AssemblyFile << "		push edx" << std::endl;
	AssemblyFile << "		push ebp" << std::endl;
	AssemblyFile << "		push edi" << std::endl;
	AssemblyFile << "		push esi" << std::endl;
	AssemblyFile << "		push esp" << std::endl;
	AssemblyFile << "		lea edi, DataSegment" << std::endl;
	AssemblyFile << "		jmp Kmain" << std::endl << std::endl;
}

void TParser::ProgramFooter()
{
	AssemblyFile << "		pop esp" << std::endl;
	AssemblyFile << "		pop esi" << std::endl;
	AssemblyFile << "		pop edi" << std::endl;
	AssemblyFile << "		pop ebp" << std::endl;
	AssemblyFile << "		pop edx" << std::endl;
	AssemblyFile << "		pop ecx" << std::endl;
	AssemblyFile << "		pop ebx" << std::endl;
	AssemblyFile << "		pop eax" << std::endl;
	AssemblyFile << "	}" << std::endl;
	AssemblyFile << "return 0;" << std::endl;
	AssemblyFile << "}" << std::endl;
}

bool TParser::ProcedureHeader()
{
	AssemblyFile << "		" << FuncName << ":" << std::endl;
	AssemblyFile << "		push ebp" << std::endl;
	AssemblyFile << "		mov ebp, esp" << std::endl;
	AssemblyFile << "		sub esp, " << abs(Table.GetLocalVarSize()) << std::endl;
	AssemblyFile << "		push eax" << std::endl;
	AssemblyFile << "		push ebx" << std::endl;
	AssemblyFile << "		push ecx" << std::endl;
	AssemblyFile << "		push edx" << std::endl;
	AssemblyFile << "		push ebp" << std::endl;
	AssemblyFile << "		push edi" << std::endl;
	AssemblyFile << "		push esi" << std::endl << std::endl;
	return true;
}

bool TParser::ProcedureFooter()
{
	AssemblyFile << "		pop esi" << std::endl;
	AssemblyFile << "		pop edi" << std::endl;
	AssemblyFile << "		pop ebp" << std::endl;
	AssemblyFile << "		pop edx" << std::endl;
	AssemblyFile << "		pop ecx" << std::endl;
	AssemblyFile << "		pop ebx" << std::endl;
	AssemblyFile << "		pop eax" << std::endl;
	AssemblyFile << "		add esp, " << abs(Table.GetLocalVarSize())<< std::endl;
	AssemblyFile << "		pop ebp" << std::endl;
	AssemblyFile << "		ret " << Table.GetParamVarSize() << std::endl << std::endl;


	return true;
}

std::string TParser::Array1DAssembly(bool assign)
{
	//get the index information from the stack
	StackData IndexInfo = PStack.top();
	PStack.pop();

	std::string size;
	if (ArrayData->DataType == "integer" || ArrayData->DataType == "Real")
		size = "4";
	else if (ArrayData->DataType == "boolean")
		size = "2";
	else if (ArrayData->DataType == "char")
		size = "1";


	//move it into a register if needed
	if (!IndexInfo.InReg)
		IndexInfo.location = MoveintoReg(IndexInfo, FindEmptyRegister());

	//zero base index if the lowerbound is greater than 0
	if(ArrayData->rowlowerbound > 0)
		*asmOut << "		sub " + IndexInfo.location + ", " + std::to_string(ArrayData->rowlowerbound) << std::endl;

	//multiply the index by the size of the data the array contains
	*asmOut << "		imul " + IndexInfo.location + ", " + size << std::endl;

	//Add the offset to the beginning of the array to the index
	if(ArrayData->Offset > 0)
		*asmOut << "		add " + IndexInfo.location + ", " + std::to_string(ArrayData->Offset) << std::endl;

	ArrayData->location = "[" + ScopeRegisters[ArrayData->VarScope] + ScopeDirection[ArrayData->VarScope] + IndexInfo.location + "]";

	if (assign)
		IndexRegisters.push(IndexInfo.location);
	else
	{
		Move1DArrayintoReg(*ArrayData, IndexInfo.location);
	}

	return IndexInfo.location;
}

std::string TParser::Array2DAssembly(bool assign)
{
	StackData ColInfo = PStack.top();
	PStack.pop();
	StackData RowInfo = PStack.top();
	PStack.pop();

	std::string size;
	if (ArrayData->DataType == "integer" || ArrayData->DataType == "Real")
		size = "4";
	else if (ArrayData->DataType == "boolean")
		size = "2";
	else if (ArrayData->DataType == "char")
		size = "1";

	if (!RowInfo.InReg)
		RowInfo.location = MoveintoReg(RowInfo, FindEmptyRegister());

	if(!ColInfo.InReg)
		ColInfo.location = MoveintoReg(ColInfo, FindEmptyRegister());

	//do calculations for row index

	//zero the row index
	if (ArrayData->rowlowerbound > 0)
		*asmOut << "		sub " + RowInfo.location + ", " + std::to_string(ArrayData->rowlowerbound)<< "//zero base index" << std::endl;

	std::string Rowsize = std::to_string(std::stoi(size) * ((ArrayData->colupperbound - ArrayData->collowerbound) + 1));//size of a row

	*asmOut << "		imul " + RowInfo.location + ", " + Rowsize << "//row size times row index" << std::endl;

	//do calculation for column index

	//zero the col index 
	if (ArrayData->collowerbound > 0)
		*asmOut << "		sub " + ColInfo.location + ", " + std::to_string(ArrayData->collowerbound) << "//zero base index" << std::endl;

	std::string Colsize = std::to_string(std::stoi(size) * ((ArrayData->rowupperbound - ArrayData->rowlowerbound) + 1));

	*asmOut << "		imul " + ColInfo.location + ", " + size << "//column size times column index" << std::endl;

	*asmOut << "		add " + RowInfo.location + ", " + ColInfo.location << std::endl;


	if (ArrayData->Offset > 0)
		*asmOut << "		add " + RowInfo.location + ", " + std::to_string(ArrayData->Offset) << std::endl;

	ArrayData->location = "[" + ScopeRegisters[ArrayData->VarScope] + ScopeDirection[ArrayData->VarScope] + RowInfo.location + "]";

	//return column register
	RegisterUsed.push(ColInfo.location);

	if (assign)
		IndexRegisters.push(RowInfo.location);
	else
	{
		Move1DArrayintoReg(*ArrayData, RowInfo.location);
	}

	return RowInfo.location;

}

std::string TParser::ComparisonAssembly(std::string Operator)
{
	*asmOut << "		//" << LeftData.DataType << "(" << LeftData.VarScope << ", " << LeftData.ref << ")" << " " << Operator << " " << RightData.DataType << "(" << RightData.VarScope << ", " << RightData.ref << ")" << std::endl;


	StackData Left= LeftData;
	StackData Right = RightData;
	std::string TargetLocation;

	if (!Left.InReg)
		Left.location = MoveintoReg(Left, FindEmptyRegister());
	
	if(Assignment)
	{
		*asmOut << "		cmp " << Left.location << ", " << Right.location << "//Compare two values" << std::endl;
		*asmOut << "		" << Opcodes[Operator] << " " << "true1" << "//If the comparison returns true jump to the true1 label at set the register to 1" << std::endl << std::endl;
		*asmOut << "		mov " << Left.location << ", 0" << "//If false" << std::endl;
		*asmOut << "		jmp false1" << std::endl << std::endl;
		*asmOut << "		true1:" << "//If true" << std::endl;
		*asmOut << "		mov " << Left.location << ", 1" << std::endl;
		*asmOut << "		false1:" << std::endl << std::endl;
	}
	else
	{
		*asmOut << "		cmp " + Left.location + ", " + Right.location << std::endl;
	}

	//If Right side is in a register return it to an unused state
	if (Right.InReg)
		RegisterUsed.push(Right.location);

	if (!Assignment)
		RegisterUsed.push(Left.location);

	return Left.location;//return the register the answer is in

}

std::string TParser::AddSubAssembly(std::string Operator)
{
	*asmOut << "		//" << LeftData.DataType << "(" << LeftData.VarScope << ", " << LeftData.ref << ")" << " " << Operator << " " << RightData.DataType << "(" << RightData.VarScope << ", " << RightData.ref << ")" << std::endl;

	StackData Left = LeftData;
	StackData Right = RightData;

	if ((!Left.InReg && !Right.InReg) || (!Left.InReg && Operator == "-"))
		Left.location = MoveintoReg(Left, FindEmptyRegister());
	else if (Right.InReg && !Left.InReg && Operator != "-")
	{
		StackData Temp = Left;
		Left = Right;
		Right = Temp;
	}

	if (Right.ref)
		Right.location = Moveintoesi(Right.location);

	if (Right.Negative)
	{
		Right.location = MoveintoReg(Right, FindEmptyRegister());
		Right.InReg = true;
	}

	*asmOut << "		" << Opcodes[Operator] << " " << Left.location << ", " << Right.location << std::endl << std::endl;


	//If Right side is in a register return it to an unused state
	if (Right.InReg)
		RegisterUsed.push(Right.location);

	return Left.location;
}

std::string TParser::MultiAssembly()
{
	*asmOut << "		//" << LeftData.DataType << "(" << LeftData.VarScope << ", " << LeftData.ref << ")" << " * " << RightData.DataType << "(" << RightData.VarScope << ", " << RightData.ref << ")" << std::endl;

	StackData Left = LeftData;
	StackData Right = RightData;
	std::string Target = Left.location;
	std::string Instruction = "";

	if ((!Left.InReg && !Right.InReg && !Right.Negative && !Left.Negative))
	{
		Target = FindEmptyRegister();
		Instruction = "		imul " + Target + ", " + Left.location + ", " + Right.location;
	}
	else if (Right.InReg && !Left.InReg)
	{
		StackData Temp = Left;
		Left = Right;
		Right = Temp;
		Target = Left.location;
	}

	if (Right.ref)
		Right.location = Moveintoesi(Right.location);

	if (Right.Negative)
	{
		Right.location = MoveintoReg(Right, FindEmptyRegister());
		Right.InReg = true;
	}

	if (Left.Negative)
	{
		Left.location = MoveintoReg(Left, FindEmptyRegister());
	}

	if (Instruction == "")
		Instruction = "		imul " + Left.location + ", " + Right.location;

	*asmOut << Instruction << std::endl << std::endl;


	//If Right side is in a register return it to an unused state
	if (Right.InReg)
		RegisterUsed.push(Right.location);

	return Target;
}

std::string TParser::DivAssembly()
{
	*asmOut << "		//" << LeftData.DataType << "(" << LeftData.VarScope << ", " << LeftData.ref << ")" << " " << "/" << " " << RightData.DataType << "(" << RightData.VarScope << ", " << RightData.ref << ")" << std::endl;



	StackData Left = LeftData;
	StackData Right = RightData;
	StackData Target;//Target Location will act as the output location which will change depending on if eax is avaliable to use

	if (RegisterUsed.top() == "eax" || LeftData.location == "eax")
	{
		Left.location = Left.location != "eax" ? MoveintoReg(Left, FindEmptyRegister()) : Left.location;
		if(!Right.InReg)
 			Right.location = MoveintoReg(Right, FindEmptyRegister());

		Target = Left;

		*asmOut << "		cdq" << std::endl;
		*asmOut << "		idiv " << Right.location << std::endl << std::endl;

		RegisterUsed.push(Right.location);
	}
	else
	{
		*asmOut << "		push eax" << std::endl;
		*asmOut << "		push edx" << std::endl;

		Left.location = MoveintoReg(LeftData, "eax");
		if (!RightData.InReg)
			Right.location = MoveintoReg(Right, FindEmptyRegister());

		Target = Right;

		*asmOut << "		cdq" << std::endl;
		*asmOut << "		idiv " << Right.location << std::endl;
		*asmOut << "		mov " << Right.location << ", " << "eax" << std::endl;

		*asmOut << "		pop edx" << std::endl;
		*asmOut << "		pop eax" << std::endl << std::endl;

		if (Left.InReg)
			RegisterUsed.push(Left.location);

	}


	return Target.location;
}

std::string TParser::OrAssembly()
{
	*asmOut << "		" + Opcodes[LastOperation] + " " + "Inside" + IfWhile.top().LogicType << std::to_string(IfWhile.top().Depth) << std::endl;
	if(OrUsed)
	{
		*asmOut << "	Or" + std::to_string(OrCount) + ":" << std::endl;
		OrUsed = false;
		OrCount++; 
	}


	return std::string();
}

std::string TParser::AndAssembly()
{
	*asmOut << "		" + CompReverseOpcodes[LastOperation] + " " + "Or" + std::to_string(OrCount) << std::endl;
	OrUsed = true;

	return std::string();
}

bool TParser::IfAssembly()
{
	*asmOut << "		" + Opcodes[LastOperation] + " InsideIf" + std::to_string(IfWhile.top().Depth) << std::endl;
	if(OrUsed)
	{
		*asmOut << "	Or" + std::to_string(OrCount) + ":" << std::endl;
		OrUsed = false;
	}
	*asmOut << "		jmp elseif" + std::to_string(IfWhile.top().Depth) << std::endl;
	*asmOut << "	InsideIf" + std::to_string(IfWhile.top().Depth) + ":" << std::endl;



	return true;
}

bool TParser::WhileAssembly()
{
	*asmOut << "		" + Opcodes[LastOperation] + " InsideWhile" + std::to_string(IfWhile.top().Depth) << std::endl;
	if (OrUsed)
	{
		*asmOut << "	Or" + std::to_string(OrCount) + ":" << std::endl;
		OrUsed = false;
	}

	*asmOut << "		jmp EndWhile" + std::to_string(IfWhile.top().Depth) << std::endl;
	*asmOut << "	InsideWhile" + std::to_string(IfWhile.top().Depth) + ":" << std::endl;
	return true;
}

bool TParser::ElseAssembly()
{
	*asmOut << "		jmp endif" + std::to_string(IfWhile.top().Depth) << std::endl;
	*asmOut << "	elseif" + std::to_string(IfWhile.top().Depth) + ":" << std::endl;
	ElseUsed = true;
	return true;
}

bool TParser::EndIfAssembly()
{
	*asmOut << "	endif" + std::to_string(IfWhile.top().Depth) + ":" << std::endl;
	if (!ElseUsed)
		*asmOut << "	elseif" + std::to_string(IfWhile.top().Depth) + ":" << std::endl;
	IfWhile.pop();
	/*PStack.pop();*/


	return true;
}

bool TParser::EndWhileAssembly()
{

	*asmOut << "		jmp TopWhile" + std::to_string(IfWhile.top().Depth) << std::endl;
	*asmOut << "	EndWhile" + std::to_string(IfWhile.top().Depth) + ":" << std::endl;


	IfWhile.pop();
	PStack.pop();

	return true;
}

bool TParser::AssignmentAssembly()
{
	*asmOut << "		//" << LeftData.DataType << "(" << LeftData.VarScope << ", " << LeftData.ref << ")" << " = " << RightData.DataType << "(" << RightData.VarScope << ", " << RightData.ref << ")" << std::endl;



	bool LeftLiteral = LeftData.VarScope == "literal" ? true : false;
	bool RightLiteral = RightData.VarScope == "literal" ? true : false;
	StackData Left = LeftData;
	StackData Right= RightData;

	if((!Right.InReg && !RightLiteral) || (Right.Negative))
		Right.location = MoveintoReg(Right, FindEmptyRegister());

	if (Left.ref)
		Left.location = Moveintoesi(Left.location);

	*asmOut << "		mov " << Left.location << ", " << Right.location << std::endl << std::endl;


	//when done doing operator code return register to an unused state
	if(!RightLiteral)
		RegisterUsed.push(Right.location);
	
	//return any registers used for indexing an array
	while(!IndexRegisters.empty())
	{
		RegisterUsed.push(IndexRegisters.top());
		IndexRegisters.pop();
	}

	Assignment = false;

	return true;
}







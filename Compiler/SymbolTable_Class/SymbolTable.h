#pragma once
#include "../includes.h"

struct Scope;

struct VarData;

struct Scope
{
	std::string ScopeName = "";
	std::map<std::string, VarData> DataTable;//Holds the data for functions, local variables, and Parameters for the scope
	std::vector<std::string> ParamOrder;//Keeps track of the order the Parameters were in when placed on the stack

	Scope* PS = nullptr;//Pointer pointing to parent scope

	std::stack<std::string> VarStack;//Stack that holds the datatypes for the local variables and Parameters in the scope when doing operations
	int VariableOffset = 4;//The Next Offset to be used when adding a variable to the stack
	int ParamOffset = 8;//The Next Offset to be used when adding a parameter to the stack
};

struct VarData
{
	std::string DataType;
	int rowlowerbound;
	int rowupperbound;
	int collowerbound;
	int colupperbound;
	int Offset;//holds the position on the stack of the  var 
	int size;//holds the size of the variable
	bool isarray = false;
	bool ref;//used for parameters and is to determine passed by copy or reference
	Scope* NS;//holds a pointer to the next scope of the function or procedure
	std::string VarScope;
	std::string location;
};


class SymbolTable
{
public:
	SymbolTable();

	//Functions to alter CurrentScope
	bool AddVar(std::string Name, std::string Datatype,int size, bool reference, bool Param);
	bool AddArray(std::string Name, std::string Datatype, int size, int rowlowerbound, int rowupperbound, int collowerbound, int colupperbound);
	bool AddFuncProd(std::string Name, std::string ReturnType, std::string FuncProd);
	bool UpdateReturnType(std::string Name, std::string ReturnType);

	//Functions to Find Data in the Current Scope
	bool FindData(std::string Name, VarData& Output, bool& FuncProc);
	bool FindParam(std::string ProcName, int CurrParam, std::string& DataType, bool& ref, int& Offset);
	std::vector<std::string> FindParamTypes(std::string Name);
	int ParamCount(std::string Target);
	std::string GetReturnType(std::string Target);
	bool ValidReturnVaraible(std::string Name);
	int GetLocalVarSize();
	int GetParamVarSize();

	//Functions to Traverse the Scopes
	void BackOut();
	Scope* FindFuncProc(std::string Name);



private:
	Scope* CurrentScope;

	std::map<std::string, std::string> ScopeRegisters;
	std::map<std::string, std::string> ScopeDirection;

};

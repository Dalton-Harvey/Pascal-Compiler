#pragma once
#include "../Lexor_Class/Lexor.h"
#include "../SymbolTable_Class/SymbolTable.h"

struct LogiData
{
	std::string LogicType;
	int Depth = 0;
};
struct StackData
{
	std::string DataType = "";
	std::string VarScope = "";//if the variable is literal local or global
	int Offset;//The offset of the variable on the stack
	bool InReg = false;//if the variable is already in a register
	std::string Register = "";//what register the variable is in
	std::string location = "";//[edi + 4] or [ebp + 8] or [ebp -4] or 10 or 5
	bool ref = false;
	int rowlowerbound;
	int rowupperbound;
	int collowerbound;
	int colupperbound;
	bool isarray = false;
	bool Negative = false;


	void operator=(VarData Right)
	{
		this->DataType = Right.DataType;
		this->VarScope = Right.VarScope;
		this->Offset = Right.Offset;
		this->ref = Right.ref;
		this->location = Right.location;
		this->rowlowerbound = Right.rowlowerbound;
		this->rowupperbound = Right.rowupperbound;
		this->collowerbound = Right.collowerbound;
		this->colupperbound = Right.colupperbound;
		this->isarray = Right.isarray;
	}

};

class TParser
{
public:

	TParser(std::string FileName);

	bool parse();

	bool block();
	bool S();
	bool Sp();
	bool IFp();
	bool E();
	bool SE();
	bool SEp();
	bool SER();
	bool T();
	bool Tp();
	bool TR();
	bool F();
	bool Fp();
	bool FR();
	bool VPF();
	bool VarList();
	bool DataType(bool Param);
	bool DataType();
	bool mvar();
	bool Param();
	bool MParam();
	bool GParam();

	//functions used to manipulate the SymbolTable and Driving Token
	bool GetNextDToken();
	bool PopandCheck();
	bool AddVartoTable(bool Param);
	bool AddArraytoTable(std::string Datatype, int rowlowerbound, int rowupperbound, int collowerbound, int colupperbound);
	bool AddFunctoTable(std::string ReturnType);
	bool FindandPush();
	bool CheckValidParam(std::string Name);
	std::string FindEmptyRegister();

	//functions used when creating assembly code
	//std::string MoveLeftintoReg(std::string RightLocation, std::string CurrRegister);
	std::string MoveintoReg(StackData ToMove, std::string CurrRegister);
	std::string Move1DArrayintoReg(StackData ToMove, std::string CurrRegister);
	std::string Move2DArrayintoReg(StackData ToMove, std::string CurrRegister);
	std::string Moveintoesi(std::string Location);
	void ProgramHeader();
	void ProgramFooter();
	bool ProcedureHeader();
	bool ProcedureFooter();
	std::string Array1DAssembly(bool assign);
	std::string Array2DAssembly(bool assign);
	std::string ComparisonAssembly(std::string Operator);
	std::string AddSubAssembly(std::string Operator);
	std::string MultiAssembly();
	std::string DivAssembly();
	std::string OrAssembly();
	std::string AndAssembly();
	bool IfAssembly();
	bool WhileAssembly();
	bool ElseAssembly();
	bool EndIfAssembly();
	bool EndWhileAssembly();
	bool AssignmentAssembly();
	

private:

	CLex Lexer;
	CToken DToken;
	std::string DKey;
	SymbolTable Table;
	std::stack<StackData> PStack;

	std::vector<std::string> WordList;
	std::stack<std::string> IndexRegisters;
	std::string FuncName;

	bool PassbyRef = false;
	bool FuncProc = false;
	bool Assignment = false;
	bool Else = false;
	bool Negative = false;

	std::string LastWord;
	StackData RightData;
	StackData LeftData;
	StackData* ArrayData;

	std::ofstream AssemblyFile;
	std::ostream* asmOut = &AssemblyFile;
	std::stack<std::string> RegisterUsed;
	std::map<std::string, std::string> Opcodes;
	std::map<std::string, std::string> ScopeRegisters;
	std::map<std::string, std::string> ScopeDirection;
	std::map<std::string, std::string> CompReverseOpcodes;

	std::ostringstream ParamAssembly;
	std::string ParamString;
	std::stack<std::string> ParamStack;
	
	int OrCount = 1;
	bool OrUsed = false;
	bool ElseUsed = false;
	std::string LastOperation;
	int IfWhileCount = 1;
	std::string IfWhileLabel;
	std::stack<LogiData> IfWhile;


};
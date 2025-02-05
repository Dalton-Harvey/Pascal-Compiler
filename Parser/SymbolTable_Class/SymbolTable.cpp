#include "SymbolTable.h"

SymbolTable::SymbolTable()
{
    CurrentScope = new Scope;
    CurrentScope->ParamOffset = 0;
    CurrentScope->VariableOffset = 0;

    ScopeRegisters["global"] = "edi";
    ScopeRegisters["local"] = "ebp";
    ScopeRegisters["param"] = "ebp";

    ScopeDirection["global"] = "+";
    ScopeDirection["local"] = "-";
    ScopeDirection["param"] = "+";
}

bool SymbolTable::AddVar(std::string Name, std::string Datatype,int size, bool reference, bool Param)
{
    if (CurrentScope->DataTable.find(Name) != CurrentScope->DataTable.end())//The variable exists in the DataTable already
    {
        return false;
    }

    CurrentScope->DataTable[Name].DataType = Datatype;
    CurrentScope->DataTable[Name].ref = reference;
    CurrentScope->DataTable[Name].NS = nullptr;
    CurrentScope->DataTable[Name].size = size;

    if(Param)
    {
        CurrentScope->DataTable[Name].Offset = CurrentScope->ParamOffset;
        CurrentScope->ParamOffset += size;
        CurrentScope->ParamOrder.push_back(Name);
        CurrentScope->DataTable[Name].VarScope = "param";
    }
    else
    {
        if (CurrentScope->ScopeName == "")//if the Scope is Global
            CurrentScope->DataTable[Name].VarScope = "global";
        else
            CurrentScope->DataTable[Name].VarScope = "local";
    
        CurrentScope->DataTable[Name].Offset = CurrentScope->VariableOffset;
        CurrentScope->VariableOffset += size;
    }

    CurrentScope->DataTable[Name].location = "[" + ScopeRegisters[CurrentScope->DataTable[Name].VarScope] + ScopeDirection[CurrentScope->DataTable[Name].VarScope] + std::to_string(CurrentScope->DataTable[Name].Offset) +"]";

    return true;
}

bool SymbolTable::AddArray(std::string Name, std::string Datatype, int size, int rowlowerbound, int rowupperbound, int collowerbound, int colupperbound)
{
    if (CurrentScope->DataTable.find(Name) != CurrentScope->DataTable.end())//The variable exists in the DataTable already
    {
        return false;
    }

    CurrentScope->DataTable[Name].DataType = Datatype;
    CurrentScope->DataTable[Name].size = size;
    CurrentScope->DataTable[Name].rowlowerbound = rowlowerbound;
    CurrentScope->DataTable[Name].rowupperbound = rowupperbound;
    CurrentScope->DataTable[Name].collowerbound = collowerbound;
    CurrentScope->DataTable[Name].colupperbound = colupperbound;
    CurrentScope->DataTable[Name].Offset = CurrentScope->VariableOffset;
    CurrentScope->VariableOffset += size;
  
    if (CurrentScope->ScopeName == "")//if the Scope is Global
        CurrentScope->DataTable[Name].VarScope = "global";
    else
        CurrentScope->DataTable[Name].VarScope = "local";

    CurrentScope->DataTable[Name].location = "[" + ScopeRegisters[CurrentScope->DataTable[Name].VarScope] + ScopeDirection[CurrentScope->DataTable[Name].VarScope] + std::to_string(CurrentScope->DataTable[Name].Offset) + "]";
    CurrentScope->DataTable[Name].isarray = true;


    return true;
}

bool SymbolTable::AddFuncProd(std::string Name, std::string ReturnType, std::string FuncProd)
{
    if (CurrentScope->DataTable.find(Name) != CurrentScope->DataTable.end())//The variable exists in the DataTable already
    {
        return false;
    }
    CurrentScope->DataTable[Name].DataType = ReturnType;
    CurrentScope->DataTable[Name].size = 0;
    CurrentScope->DataTable[Name].Offset = 0;
    CurrentScope->DataTable[Name].ref = false;
    CurrentScope->DataTable[Name].NS = new Scope;

    Scope* Temp = CurrentScope;

    //CurrentScope->DataTable[Name].NS = new Scope;
    CurrentScope = CurrentScope->DataTable[Name].NS;
    CurrentScope->PS = Temp;
    CurrentScope->ScopeName = Name;

    return true;
}

bool SymbolTable::UpdateReturnType(std::string Name, std::string ReturnType)
{
    VarData Temp;
    bool funcproc;
    if (FindData(Name, Temp, funcproc))
        CurrentScope->PS->DataTable[Name].DataType = ReturnType;
    else
        return false;
    return true;
}

bool SymbolTable::FindData(std::string Name, VarData& Output, bool& FuncProc)
{
    Scope* Trav = CurrentScope;
    while (Trav != nullptr)
    {
        if (Trav->DataTable.find(Name) == Trav->DataTable.end())//Did not find the Search Target
        {
            Trav = Trav->PS;//Go Up one scope
        }
        else//Found the Search Target
        {
            Output.Offset = Trav->DataTable[Name].Offset;
            Output.DataType = Trav->DataTable[Name].DataType;
            FuncProc = Trav->DataTable[Name].NS;
            Output.VarScope = Trav->DataTable[Name].VarScope;
            Output.ref = Trav->DataTable[Name].ref;
            Output.location = Trav->DataTable[Name].location;
            Output.rowlowerbound = Trav->DataTable[Name].rowlowerbound;
            Output.rowupperbound = Trav->DataTable[Name].rowupperbound;
            Output.collowerbound = Trav->DataTable[Name].collowerbound;
            Output.colupperbound = Trav->DataTable[Name].colupperbound;
            return true;
        }
    }
    return false;
}

bool SymbolTable::FindParam(std::string ProcName, int CurrParam, std::string &DataType, bool& ref, int& Offset)
{
    VarData Temp = CurrentScope->DataTable[ProcName].NS->DataTable[CurrentScope->DataTable[ProcName].NS->ParamOrder[CurrParam]];

    DataType = Temp.DataType;
    ref = Temp.ref;
    Offset = Temp.Offset;

    return true;
}

std::vector<std::string> SymbolTable::FindParamTypes(std::string Name)
{
    std::vector<std::string> ParamTypes;

    Scope* Trav = CurrentScope;
    //find the Function or Procedure being called and enter it's scope
    while (Trav != nullptr)
    {
        if (Trav->DataTable.find(Name) == Trav->DataTable.end())
        {
            Trav = Trav->PS;//Go Up one scope
        }
        else//Found the Search Target
        {
            Trav = Trav->DataTable[Name].NS;
        }
    }

    for (int x = 0; x < Trav->ParamOrder.size(); x++)
    {   
        ParamTypes.push_back(Trav->DataTable[Trav->ParamOrder[x]].DataType);
    }
    return ParamTypes;

}

int SymbolTable::ParamCount(std::string Target)
{
    return CurrentScope->DataTable[Target].NS->ParamOrder.size();
}

std::string SymbolTable::GetReturnType(std::string Target)
{
    VarData Temp;
    bool FuncProc;
    std::string scope;

    if (FindData(Target, Temp, FuncProc))
    {
        return Temp.DataType;
    }

    return "";
}

bool SymbolTable::ValidReturnVaraible(std::string Name)
{
    //if the Scope name and var name are equal and it is not a procedure then it is a valid return variable
    return (CurrentScope->ScopeName == Name && CurrentScope->PS->DataTable[Name].DataType != "void");
       
}

int SymbolTable::GetLocalVarSize()
{
    return CurrentScope->VariableOffset - 4;
}

int SymbolTable::GetParamVarSize()
{
    return CurrentScope->ParamOffset - 8;
}

void SymbolTable::BackOut()
{
    if (CurrentScope->PS)
        CurrentScope = CurrentScope->PS;
}

Scope* SymbolTable::FindFuncProc(std::string Name)
{
    Scope* Trav = CurrentScope;
    //find the Function or Procedure being called and enter it's scope
    while (Trav != nullptr)
    {
        if (Trav->DataTable.find(Name) == Trav->DataTable.end())
        {
            Trav = Trav->PS;//Go Up one scope
        }
        else//Found the Search Target
        {
            Trav = Trav->DataTable[Name].NS;
        }
    }
    return Trav;
}

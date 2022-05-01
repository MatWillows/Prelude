#ifndef SCRIPTFUNCS_H
#define SCRIPTFUNCS_H

#include "script.h"
#include "creatures.h"
#include "items.h"

void LoadFuncs();

ScriptArg *CallFunc(int FuncNum, ScriptArg* ArgList, ScriptArg *Destination);

int GetFuncID(char *FuncName);

void GetFuncName(int ID, char *Dest);

void ClearStack();

void GetSub(ScriptArg *ToFill, char *SubString);

void Push(int);
void Push(char *String);
void Push(Creature *pCreature);
void Push(Item *pItem);

void CallScript(const char *ScriptName, const char *FileName);

ScriptArg *Pop();


#endif
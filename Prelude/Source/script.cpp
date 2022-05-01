#include "script.h"
#include "scriptfuncs.h"
#include "zsutilities.h"
#include "zstalk.h"
#include "things.h"
#include "creatures.h"
#include "items.h"
#include <assert.h>
#include "party.h"

#define IDC_TALK_WIN		666

int G_NumBlocks = 0;
int G_NumArgs = 0;

ScriptBlock *ScriptContextBlock;
ZSWindow *ScriptContextWindow;
Flags PreludeFlags;


ScriptArg::ScriptArg()
{
	Value = NULL;
	Type = ARG_NONE;
	G_NumArgs++;
}

ScriptArg::~ScriptArg()
{
	if(Value)
	{
		switch(Type)
		{
			case ARG_LABEL:
			case ARG_CHARACTER_LABEL:
			case ARG_STRING:
				delete[] (char *)Value;
				break;
			case ARG_BLOCK:
				delete (ScriptBlock *)Value;
				break;
		}
	}
	Value = NULL;
	Type = ARG_NONE;
	G_NumArgs--;
}

void ScriptArg::ClearValue()
{
	if(Value)
	{
		switch(Type)
		{
			case ARG_LABEL:
			case ARG_CHARACTER_LABEL:
			case ARG_STRING:
				delete[] (char *)Value;
				break;
			case ARG_BLOCK:
				delete (ScriptBlock *)Value;
				break;
		}
	}
	Value = NULL;
	Type = ARG_NONE;

}


ScriptArg::ScriptArg(ScriptArg *pFrom)
{
	Type = pFrom->GetType();

	switch(pFrom->Type)
	{
		case ARG_LABEL:
		case ARG_CHARACTER_LABEL:
		case ARG_STRING:
			Value = new char[strlen((char *)pFrom->GetValue()) + 1];
			assert(Value);
			strcpy((char *)Value,(char *)pFrom->GetValue());
			break;
		case ARG_BLOCK:
			SafeExit("error: attempted to copy Script block\n");
			break;
		default:
			Value = pFrom->GetValue();
			break;
	}
	G_NumArgs++;
}

void ScriptArg::Save(FILE *fp)
{
//write out the typeof arg
	fwrite(&Type,sizeof(Type),1,fp);
	int ID;
	unsigned short Length;
	
	switch(Type)
	{
	case ARG_TERMINATOR:
	case ARG_NONE:
		break;
	case	ARG_FUNC_ID:
	case	ARG_NUMBER:
		fwrite(&Value,sizeof(Value),1,fp);
		break;
	case ARG_LABEL:
	case ARG_CHARACTER_LABEL:
	case ARG_STRING:
		Length = strlen((char *)Value) + 1;
		fwrite(&Length,sizeof(Length),1,fp);
		fwrite(Value,Length-1,1,fp);
		break;
	case ARG_BLOCK:
		((ScriptBlock *)Value)->Save(fp);
		break;
	case ARG_ITEM:
	case ARG_CREATURE:
		ID = ((Thing *)Value)->GetData(INDEX_ID).Value;
		fwrite(&ID,sizeof(ID),1,fp);
		break;
	case ARG_FLAG:
		Length = strlen(((Flag *)Value)->Name) + 1;
		fwrite(&Length,sizeof(Length),1,fp);
		fwrite(((Flag *)Value)->Name,Length-1,1,fp);
		break;
	default:
		break;
	}
}

void ScriptArg::Load(FILE *fp)
{
//write out the typeof arg
	fread(&Type,sizeof(Type),1,fp);
	int ID;
	char TempString[128];
	unsigned short Length;
	Value = NULL;
	
	switch(Type)
	{
	case ARG_TERMINATOR:
	case ARG_NONE:
		break;
	case ARG_FUNC_ID:
	case ARG_NUMBER:
		fread(&Value,sizeof(Value),1,fp);
		break;
	case ARG_LABEL:
	case ARG_CHARACTER_LABEL:
	case ARG_STRING:
		fread(&Length,sizeof(Length),1,fp);
		Value = new char[Length];
		assert(Value);
		fgets((char *)Value,Length,fp);
		break;
	case ARG_BLOCK:
		Value = new ScriptBlock;
		assert(Value);
		((ScriptBlock *)Value)->Load(fp);
		break;
	case ARG_ITEM:
		fread(&ID,sizeof(ID),1,fp);
		Value = (void *)Thing::Find(Item::GetFirst(),ID);
		if(!Value)
		{
			sprintf(TempString,"Item: %i not found in load\n",ID);
			SafeExit(TempString);
		}
		break;
	case ARG_CREATURE:
		fread(&ID,sizeof(ID),1,fp);
		Value = (void *)Thing::Find(Creature::GetFirst(),ID, 0);
		if(!Value)
		{
			sprintf(TempString,"Creature: %i not found in load\n",ID);
			SafeExit(TempString);
		}
		break;
	case ARG_FLAG:
		fread(&Length,sizeof(Length),1,fp);
		fgets(TempString,Length,fp);
		assert(Length < 128);
		Value = (void *)PreludeFlags.Get(TempString);
		break;
	default:
		break;
	}
}

bool ScriptArg::operator == (ScriptArg& CompArg)
{
	if(this->Type == CompArg.Type)
	{
		return (this->Value == CompArg.Value);
	}
	else
	{
		if(this->Type >= ARG_CREATURE && CompArg.Type >= ARG_CREATURE)
		{
			return (this->GetCreature() == CompArg.GetCreature());	
		}
	}
	return FALSE;
}

ScriptArg::operator = (ScriptArg& OtherArg)
{
	Type = OtherArg.GetType();

	switch(Type)
	{
		case ARG_LABEL:
		case ARG_CHARACTER_LABEL:
		case ARG_STRING:
			Value = new char[strlen((char *)OtherArg.GetValue()) + 1];
			assert(Value);
			strcpy((char *)Value,(char *)OtherArg.GetValue());
			break;
		case ARG_BLOCK:
			SafeExit("error: attempted to copy Script block\n");
			break;
		default:
			Value = OtherArg.GetValue();
			break;
	}
	
	return FALSE;
}


Creature *ScriptArg::GetCreature()
{
	switch(this->Type)
	{
		case ARG_CREATURE:
		case ARG_NUMBER:
			return (Creature *)Value;
		case ARG_LEADER:
			return PreludeParty.GetLeader();
		case ARG_PARTYONE:
			return PreludeParty.GetMember(0);
		case ARG_PARTYTWO:
			return PreludeParty.GetMember(1);
		case ARG_PARTYTHREE:
			return PreludeParty.GetMember(2);
		case ARG_PARTYFOUR:
			return PreludeParty.GetMember(3);
		case ARG_PARTYFIVE:
			return PreludeParty.GetMember(4);
		case ARG_PARTYSIX:
			return PreludeParty.GetMember(5);
		default:
			break;
	}
	return NULL;
}


void ScriptBlock::Save(FILE *fp)
{
	fwrite(&NumArgs,sizeof(NumArgs),1,fp);

	int n;

	for(n = 0; n < NumArgs; n++)
	{
		ArgList[n].Save(fp);
	}
}


void ScriptBlock::Load(FILE *fp)
{
	fread(&NumArgs,sizeof(NumArgs),1,fp);

	ArgList = new ScriptArg[NumArgs];
	assert(ArgList);
	
	int n;

	for(n = 0; n < NumArgs; n++)
	{
		ArgList[n].Load(fp);
	}

}
void ScriptBlock::Import(const char *FileName)
{
	FILE *fp;
	fp = SafeFileOpen(FileName, "rt");

	//seek to the first paren
	SeekTo(fp, "(");

	Import(fp);

	fclose(fp);
}

void ScriptBlock::Import(FILE *fp)
{
	ScriptArg TempArgs[MAX_ARGS];
	
	NumArgs = 0;

	//look at our next char
	char c = '\0';

	char *TempString;
	Thing *pThing;

	int n;

	while(1)
	{
		c = GetChar(fp);
		if(c == '(')
		{
			TempArgs[NumArgs].SetType(ARG_BLOCK);
			TempArgs[NumArgs].SetValue(new ScriptBlock);
			if(!TempArgs[NumArgs].GetValue())
			{
				SafeExit("COULD not allocate block!!!!!");
			}
			else
			{
				((ScriptBlock *)TempArgs[NumArgs].GetValue())->Import(fp);
			}
		}	
		else
		if(isalpha(c))
		{
			TempArgs[NumArgs].SetType(ARG_FUNC_ID);
			char *FuncName;
			fseek(fp,-1,1);
			FuncName = GetPureString(fp);
			TempArgs[NumArgs].SetValue((void *)GetFuncID(FuncName));
			delete[] FuncName;
		}
		else
		if(c == '[')
		{
			TempArgs[NumArgs].SetType(ARG_STRING);
			TempArgs[NumArgs].SetValue(GetString(fp, ']'));
		}
		else
		if(c == '!')
		{
			TempArgs[NumArgs].SetType(ARG_LABEL);
			TempArgs[NumArgs].SetValue(GetString(fp, '!'));
		}
		else
		if(c == '#')
		{
			TempArgs[NumArgs].SetType(ARG_CHARACTER_LABEL);
			TempArgs[NumArgs].SetValue(GetString(fp, '#'));
		}
		else
		if(c == '^')
		{
			TempArgs[NumArgs].SetType(ARG_NUMBER);
			TempString = GetString(fp,'^');
			//check special substitutions
			GetSub(&TempArgs[NumArgs],TempString);
			if(TempArgs[NumArgs].GetType() == ARG_NONE)
			{
				//creatures
				pThing = Thing::Find(Creature::GetFirst(), TempString);
				if(pThing)
				{
					TempArgs[NumArgs].SetValue(pThing);
					TempArgs[NumArgs].SetType(ARG_CREATURE);
				}
				else
				{
					//then items
					pThing = Thing::Find(Item::GetFirst(), TempString);
					if(pThing)
					{
						TempArgs[NumArgs].SetValue(pThing);
						TempArgs[NumArgs].SetType(ARG_ITEM);
					}
				}
				if(!pThing)
				{
					//didn't find in things or creatures
					ConvertToCapitals(TempString);
					
					n = Creature::GetFirst()->GetIndex(TempString);
					if(n != - 1)
					{
						TempArgs[NumArgs].SetValue((void *)n);
						TempArgs[NumArgs].SetType(ARG_NUMBER);
					}
					else
					{
						n = Item::GetFirst()->GetIndex(TempString);
						if(n != -1)
						{
							TempArgs[NumArgs].SetValue((void *)n);
						}
						else
						{
							TempArgs[NumArgs].SetValue((void *)PreludeFlags.Get(TempString));
							TempArgs[NumArgs].SetType(ARG_FLAG);
						}
					}
				}
			}
			if(TempString)
				delete[] TempString;
		}
		else
		if(isdigit(c) || c == '-')
		{
			fseek(fp,-1,1);
			TempArgs[NumArgs].SetType(ARG_NUMBER);
			TempArgs[NumArgs].SetValue((void *)GetInt(fp));
		}	
		if(c == ')' || feof(fp)) 
		{
			if(feof(fp))
			{
				DEBUG_INFO("Hit eof early when importing script");
			}
			break;
		}
		else
		{
			NumArgs++;
			if(NumArgs >= MAX_ARGS)
			{
				SafeExit("Too many args!\n");
			}
		}
	}

	ArgList = new ScriptArg[NumArgs + 1];
	assert(ArgList);

	for(n = 0; n < NumArgs; n++)
	{
		memcpy(&ArgList[n],&TempArgs[n],sizeof(ScriptArg));
		TempArgs[n].SetValue(NULL);
		TempArgs[n].SetType(ARG_NONE);
	}
	ArgList[NumArgs].SetValue(NULL);
	ArgList[NumArgs].SetType(ARG_TERMINATOR);
	NumArgs++;
	return;
}


ScriptBlock::ScriptBlock()
{
	NumArgs = 0;
	ArgList = NULL;
	G_NumBlocks++;
}

ScriptBlock::~ScriptBlock()
{
	if(ArgList)
	{
		Value.ClearValue();

		delete[] ArgList;
		ArgList = NULL;
	}
	G_NumBlocks--;
}

void ScriptBlock::Export(FILE *fp)
{
	fprintf(fp,"(");
	int n;
	for(n = 0; n < NumArgs; n++)
	{
		ArgList[n].Print(fp);
	}
	fprintf(fp,") \n");
}

ScriptArg *ScriptBlock::Process()
{
	int n;

	Value.ClearValue();

	ScriptArg *pArgZero;
	pArgZero = ArgList[0].Evaluate();

	if(pArgZero->GetType() == ARG_FUNC_ID)
	{
		CallFunc((int)ArgList[0].GetValue(), &ArgList[1], &Value);
		return &Value;
	}
	if(pArgZero->GetType() == ARG_CHARACTER_LABEL)
	{
		ZSWindow *pWin;
		pWin = new ZSTalkWin(IDC_TALK_WIN,125, 125, 550,350, this);

		pWin->Show();
	
		ZSWindow::GetMain()->AddTopChild(pWin);
	
		pWin->SetFocus(pWin);

		pWin->GoModal();

		pWin->ReleaseFocus();
		pWin->Hide();
		
		ZSWindow::GetMain()->RemoveChild(pWin);
	}
	else
	{
		Value = *pArgZero;
		if(Value.GetType() != ARG_TERMINATOR)
		{
			n = 1;
			while(TRUE)
			{

				if(ArgList[n].Evaluate()->GetType() == ARG_TERMINATOR)
				{
					if(ArgList[n].GetType() != ARG_TERMINATOR)
					{
						Value.SetType(ARG_TERMINATOR);
					}
					break;
				}
				n++;
			}
		}
		else
		{
			if(ArgList[0].GetType() == ARG_TERMINATOR)
			{
				Value.SetType(ARG_NONE);
			}
		}
	}
	return &Value;
}

ScriptArg *ScriptArg::Evaluate()
{
	if(Type == ARG_BLOCK)
	{
		return ((ScriptBlock *)Value)->Process();
	}
	else
	{
		return this;
	}
}



ScriptBlock *ScriptBlock::FindLabel(char *Label)
{
	int n;
	for(n = 0; n < NumArgs; n++)
	{
		if(ArgList[n].GetType() == ARG_BLOCK)
		{
			if(((ScriptBlock *)ArgList[n].GetValue())->GetArg(0)->GetType() == ARG_LABEL)
			{
				if(!strcmp((char *)((ScriptBlock *)ArgList[n].GetValue())->GetArg(0)->GetValue(),Label))
				{
					return (ScriptBlock *)ArgList[n].GetValue();
				}
			}
		}
	}
	return NULL;
}

void ScriptArg::Print(FILE *fp)
{

	char FuncName[32];
	switch(Type)
	{
		case ARG_FUNC_ID:
			GetFuncName((int)Value,FuncName);
			fprintf(fp,"%s",FuncName);
			break;
		case ARG_LABEL:
			fprintf(fp,"!%s!\n",(char *)Value);
			break;
		case ARG_CHARACTER_LABEL:
			fprintf(fp,"#%s#\n",(char *)Value);
			break;
		case ARG_STRING:
			fprintf(fp," [%s]",(char *)Value);
			break;
		case ARG_NUMBER:
			fprintf(fp," %i",(int)Value);
			break;
		case ARG_BLOCK:
			((ScriptBlock *)Value)->Export(fp);
			break;
		case ARG_FLAG:
			fprintf(fp," ^%s^", ((Flag *)Value)->Name);
			break;
		case ARG_ITEM:
		case ARG_CREATURE:
			fprintf(fp," ^%s^", ((Thing *)Value)->GetData(INDEX_NAME).String);
			break;
		case ARG_TERMINATOR:
			break;
		case ARG_PARTY:
			fprintf(fp, "^PARTY^");
			break;
		case ARG_LEADER:
		case ARG_PARTYONE:
		case ARG_PARTYTWO:
		case ARG_PARTYTHREE:
		case ARG_PARTYFOUR:
		case ARG_PARTYFIVE:
		case ARG_PARTYSIX:
			if(GetCreature())
				fprintf(fp," ^%s^", GetCreature()->GetData(INDEX_NAME).String);
			else
				fprintf(fp,"NULL");
			break;
		default:
			fprintf(fp," NULL");
			break;
	}
}

void ScriptArg::UnsetCreature()
{
		int ID;
		char *flagname;
	switch(this->Type)
	{
		case ARG_CREATURE:
			ID = ((Creature *)Value)->GetData(INDEX_ID).Value;
			Value = (void *)ID;
			break;
		case ARG_BLOCK:
			((ScriptBlock *)Value)->UnsetCreatures();
			break;
		case ARG_FLAG:
			flagname = new char[32];
			memcpy(flagname, ((Flag *)Value)->Name, sizeof(char) * 32);
			Value = (void *)flagname;
			break;
		default:
			break;
	}
	return;
}

void ScriptArg::SetCreature()
{
	int ID;
	Flag *pFlag;
	char *flagname;
	switch(this->Type)
	{
		case ARG_CREATURE:
			ID = (int)Value;
			Value = (void *)Thing::Find(Creature::GetFirst(),ID, 0);
			break;
		case ARG_BLOCK:
			((ScriptBlock *)Value)->SetCreatures();
			break;
		case ARG_FLAG:
			flagname = (char *)Value;
			pFlag = PreludeFlags.Get(flagname);
			delete[] flagname;
			Value = (void *)pFlag;
			break;
		default:
			break;
	}
}

void ScriptBlock::UnsetCreatures()
{
	for(int n = 0; n < NumArgs; n++)
	{
		ArgList[n].UnsetCreature();
	}
}

void ScriptBlock::SetCreatures()
{
	for(int n = 0; n < NumArgs; n++)
	{
		ArgList[n].SetCreature();
	}
}

ScriptBlock *ScriptManager::GetBlock(int Size)
{

	return NULL;
};

void ScriptManager::AddBlock(int Size)
{

};

void ScriptManager::FreeBlock(ScriptBlock *)
{

}

	
ScriptManager::ScriptManager()
{

}

ScriptManager::~ScriptManager()
{

}


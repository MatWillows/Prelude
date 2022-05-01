#include "scriptfuncs.h"
#include <stdio.h>
#include "defs.h"
#include <assert.h>
#include "zswindow.h"
#include "zstalk.h"
//#include "zssay.h"
#include "zsdescribe.h"
#include "zsaskwin.h"
#include "things.h"
#include "zsutilities.h"
#include "zssaychar.h"
#include "world.h"
#include "party.h"
#include "mainwindow.h"
#include "barter.h"
#include "pickpocket.h"
#include "events.h"
#include "gameitem.h"
#include "zsgetnumber.h"
#include "zsgettarget.h"
#include "combatmanager.h"
#include "deathwin.h"
#include "zsParticle.h"
#include "zscutscene.h"
#include "explosion.h"
#include "pattern.h"
#include "modifiers.h"
#include "spells.h"
#include "entrance.h"
#include "zsHelpWin.h"

#define IDC_ASK			989898
#define IDC_SAY_CHAR		6543
#define IDC_TALK_START  78901


static int NextTalkWin = IDC_TALK_START;

/* talk:  takes a character¹s name, i.e. talk(³Matt²), and 
	searches through people.txt until it finds the character, 
	then starts parsing after the !begin! keyword to start 
	conversation */

ScriptArg *ScriptStack[128];
int StackTop = 0;
ScriptArg *(*ScriptFunctions[256])(ScriptArg *ArgList, ScriptArg *pDestination);

ScriptArg *talk(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SAWho, *SAWhere;
	
	char FileName[32];
	
	SAWho = ArgList[0].Evaluate();

	if(!SAWho->GetCreature())
	{
		SafeExit("Talk arg not person\nCouldn't talk to something????\n");
	}

	SAWhere = ArgList[1].Evaluate();
	if(SAWhere->GetType() != ARG_STRING)
	{
		strcpy(FileName,"people.txt");
	}
	else
	{
		strcpy(FileName,(char *)SAWhere->GetValue());
	}

	Talk(SAWho->GetCreature());
/*
	sprintf(CharacterID,"#%s#",((Thing *)SAWho->GetValue())->GetData(INDEX_NAME).String);

	DEBUG_INFO(CharacterID);
	DEBUG_INFO("\n");

	FILE *fp;
	fp = fopen(FileName,"rt");
	
	if(fp && SeekTo(fp,CharacterID))
	{
		fseek(fp, -(strlen(CharacterID) + 1), SEEK_CUR);
		ScriptBlock SB;
	
		SB.Import(fp);	
		fclose(fp);		
	
		ZSWindow *pWin;
		pWin = new ZSTalkWin(NextTalkWin++,125, 125, 550,350, &SB);

		((ZSTalkWin *)pWin)->SetPortrait(((Thing *)SAWho->GetValue())->GetData(INDEX_PORTRAIT).String);

		pWin->Show();
	
		ZSWindow::GetMain()->AddChild(pWin);
	
		pWin->SetFocus(pWin);

		pWin->GoModal();

		pWin->ReleaseFocus();
		pWin->Hide();
		
		ZSWindow::GetMain()->RemoveChild(pWin);
		NextTalkWin--;
	}
	else
	{
		DEBUG_INFO("Failed to find character: ");
		DEBUG_INFO(CharacterID);
		DEBUG_INFO(" in file: ");
		DEBUG_INFO(FileName);
		DEBUG_INFO("\n");
		Describe("Failed to find character: ");
		Describe(CharacterID);
		Describe(" in file: ");
		Describe(FileName);
		Describe("\n");
		if(fp)
			fclose(fp);
	}
*/	
	return pDestination; 
}

/* addword:  takes a word and adds it to the current menu 
	choices */
ScriptArg *addword(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_STRING)
	{
		SafeExit("can't add non-string\n");
	}

	if(ScriptContextBlock->FindLabel((char *)SA->GetValue()))
	{
		((ZSTalkWin *)ScriptContextWindow)->AddWord((char *)SA->GetValue());
	}
	else
	{
		Describe("Bad Add Word.");
		Describe((char *)SA->GetValue());
	}

	
	
	pDestination->SetValue((void *)1);
	pDestination->SetType(ARG_NUMBER);
	return pDestination;
}

/* goword:  finds the word in people.txt within the current 
	character and begins parsing after it */
ScriptArg *goword(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	ScriptBlock *pSB;
	SA = ArgList[0].Evaluate();
	pSB = ScriptContextBlock->FindLabel((char *)SA->GetValue());
	
	DEBUG_INFO("Goword:  ");
	DEBUG_INFO((char *)SA->GetValue());
	DEBUG_INFO("\n");

	if(pSB)
	{
		pSB->Process();
		
		pDestination->SetType(ARG_TERMINATOR);
	}
	else
	{
		DEBUG_INFO("failed to goto: ");
		DEBUG_INFO((char *)SA->GetValue());
		DEBUG_INFO("\n");
		Describe("failed goto: ");
		Describe((char *)SA->GetValue());
		
		pDestination->SetType(ARG_TERMINATOR);
	}
	return pDestination; 
}

/* has: checks to see if the character has a particular item */
ScriptArg *has(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	

	Thing *pWho, *pWhat;
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_ITEM &&
		SA->GetType() != ARG_THING &&
		SA->GetType() != ARG_PARTY &&
		!SA->GetCreature())
	{
		DEBUG_INFO("Bad First Argument to has.\n");
		Describe("Bad First Argument to has.");
		return pDestination;
	}
	else
	{
		pWho = (Thing *)SA->GetValue();
	}

	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_ITEM &&
		SB->GetType() != ARG_THING &&
		!SB->GetCreature())
	{
		DEBUG_INFO("Bad Second Argument to has.");
		Describe("Bad Second Argument to has.");
		return pDestination;
	}
	else
	{
		pWhat = (Thing *)SB->GetValue();
	}
		
	pDestination->SetType(ARG_NUMBER);

	int DoesHave;

	if(SA->GetType() != ARG_PARTY)
	{
		DoesHave = pWho->Has(pWhat);
	}
	else
	{
		DoesHave = PreludeParty.Has(pWhat);
	}

	pDestination->SetValue((void *)DoesHave);
	
	return pDestination;
}

/* has: checks to see if the character has a particular item */
ScriptArg *hastype(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	

	Thing *pWho;
	int TypeNum;
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_ITEM &&
		SA->GetType() != ARG_THING &&
		SA->GetType() != ARG_PARTY &&
		!SA->GetCreature())
	{
		DEBUG_INFO("Nobody First Argument to hastype.\n");
		//Describe("Bad First Argument to has.");
		return pDestination;
	}
	else
	{
		pWho = (Thing *)SA->GetValue();
	}

	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Second Argument to hastype.");
		Describe("Bad Second Argument to hastype.");
		return pDestination;
	}
	else
	{
		TypeNum = (int)SB->GetValue();
	}
		
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	Object *pOb;
	GameItem *pGI;

	int n;
	if(SA->GetType() != ARG_PARTY)
	{
		pOb = pWho->GetContents();

		while(pOb)
		{
			pGI = (GameItem *)pOb;
			if(pGI->GetData(INDEX_TYPE).Value == TypeNum)
			{
				pDestination->SetValue((void *)1);
			}
			pOb = pOb->GetNext();
		}

		for(n = 0; n < MAX_EQUIPMENT; n ++)
		{
			pOb = ((Creature *)pWho)->GetEquipment(n);
			if(pOb)
			{
				pGI = (GameItem *)pOb;
				if(pGI->GetData(INDEX_TYPE).Value == TypeNum)
				{
					pDestination->SetValue((void *)1);
				}
			}
		}
	}
	else
	{
		for(n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			pWho = PreludeParty.GetMember(n);
			if(pWho)
			{
				pOb = pWho->GetContents();

				while(pOb)
				{
					pGI = (GameItem *)pOb;
					if(pGI->GetData(INDEX_TYPE).Value == TypeNum)
					{
						pDestination->SetValue((void *)1);
					}
					pOb = pOb->GetNext();
				}

				for(int n = 0; n < MAX_EQUIPMENT; n ++)
				{
					pOb = ((Creature *)pWho)->GetEquipment(n);
					if(pOb)
					{
						pGI = (GameItem *)pOb;
						if(pGI->GetData(INDEX_TYPE).Value == TypeNum)
						{
							pDestination->SetValue((void *)1);
						}
					}
				}
			}
		}
	}

	return pDestination;
}



/* take: takes a particular item from a character */
ScriptArg *take(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	

	Thing *pWho, *pWhat;
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_ITEM &&
		SA->GetType() != ARG_THING &&
		SA->GetType() != ARG_PARTY 
		&& !SA->GetCreature())
	{
		DEBUG_INFO("Bad First Argument to Take.\n");
		Describe("Bad First Argument to Take.");
		return pDestination;
	}
	else
	{
		pWho = (Thing *)SA->GetValue();
	}

	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_ITEM &&
		SB->GetType() != ARG_THING &&
		!SB->GetCreature())
	{
		DEBUG_INFO("Bad Second Argument to Take.");
		Describe("Bad Second Argument to Take.");
		return pDestination;
	}
	else
	{
		pWhat = (Thing *)SB->GetValue();
	}
		
	pDestination->SetType(ARG_NUMBER);

	int DoesHave;
	int Quantity = 1;
	ScriptArg *SQ;
	SQ = ArgList[2].Evaluate();
	if(SQ->GetType() != ARG_TERMINATOR)
	{
		Quantity = (int)SQ->GetValue();
	}


	if(SA->GetType() != ARG_PARTY)
	{
		DoesHave = pWho->Take(pWhat,Quantity);
	}
	else
	{
		DoesHave = PreludeParty.Take(pWhat,Quantity);
	}

	pDestination->SetValue((void *)DoesHave);
	
	return pDestination;
}

ScriptArg *equipped(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SB;
	
	Creature *pWho;
	Item *pWhat;
	

	SA = ArgList[0].Evaluate();
	if(!SA->GetCreature())
	{
		DEBUG_INFO("Bad Creature to check equip!\n");
			return pDestination;
	}
	
	pWho = (Creature *)SA->GetValue();

	SB = ArgList[1].Evaluate();

	if(SB->GetType() != ARG_ITEM)
	{
		DEBUG_INFO("Bad item to equip.\n");
		Describe("Bad Item to Equip");
		pDestination->SetValue((void *)NULL);
		pDestination->SetType(ARG_NUMBER);
		return pDestination;
	}
	

	pWhat = (Item *)SB->GetValue();

	GameItem *pGI;
	pGI = pWho->GetEquipment(pWhat->GetData("EQUIPLOCATION").String);


	if(!pGI || pGI->GetItem() != pWhat)
	{
		pDestination->SetValue((void *)NULL);
		pDestination->SetType(ARG_NUMBER);
	}
	else
	{
		pDestination->SetValue((void *)1);
		pDestination->SetType(ARG_NUMBER);
	}

	return pDestination;
	
}

/* has: checks to see if the character has a particular item */
ScriptArg *equippedtype(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	

	Thing *pWho;
	int TypeNum;
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_ITEM &&
		SA->GetType() != ARG_THING &&
		SA->GetType() != ARG_PARTY &&
		!SA->GetCreature())
	{
		DEBUG_INFO("Nobody First Argument to equippedtype.\n");
		//Describe("Bad First Argument to has.");
		return pDestination;
	}
	else
	{
		pWho = (Thing *)SA->GetValue();
	}

	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Second Argument to equippedtype.");
		Describe("Bad Second Argument to equippedtype.");
		return pDestination;
	}
	else
	{
		TypeNum = (int)SB->GetValue();
	}
		
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	Object *pOb;
	GameItem *pGI;

	int n;
	if(SA->GetType() != ARG_PARTY)
	{
		for(n = 0; n < MAX_EQUIPMENT; n ++)
		{
			pOb = ((Creature *)pWho)->GetEquipment(n);
			if(pOb)
			{
				pGI = (GameItem *)pOb;
				if(pGI->GetData(INDEX_TYPE).Value == TypeNum)
				{
					pDestination->SetValue((void *)1);
				}
			}
		}
	}
	else
	{
		for(n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			pWho = PreludeParty.GetMember(n);
			if(pWho)
			{
				for(int n = 0; n < MAX_EQUIPMENT; n ++)
				{
					pOb = ((Creature *)pWho)->GetEquipment(n);
					if(pOb)
					{
						pGI = (GameItem *)pOb;
						if(pGI->GetData(INDEX_TYPE).Value == TypeNum)
						{
							pDestination->SetValue((void *)1);
						}
					}
				}
			}
		}
	}

	return pDestination;
}

ScriptArg *equip(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SB, *SAmount;

	Creature *pWho;
	Thing *pWhat;

	SA = ArgList[0].Evaluate();
	if(!SA->GetCreature())
	{
		SafeExit("Bad Creature to equip!\n");
	}
	
	pWho = (Creature *)SA->GetValue();

	SB = ArgList[1].Evaluate();

	if(SB->GetType() == ARG_TERMINATOR)
	{
		pWho->ReEquip();
		pWho->CreateTexture();
		pWho->SetEquipObjects();
		
		pDestination->SetValue((void *)1);
		pDestination->SetType(ARG_NUMBER);
		return pDestination;
	}
	if(SB->GetType() != ARG_ITEM)
	{
		DEBUG_INFO("Bad item to equip.\n");
		Describe("Bad Item to Equip");
		
		pDestination->SetValue((void *)NULL);
		pDestination->SetType(ARG_NUMBER);
		return pDestination;
	}
		
	pWhat = (Thing *)SB->GetValue();
	
	SAmount = ArgList[2].Evaluate();

	int Amount;
	Amount = 1;
	if(SAmount->GetType() != ARG_TERMINATOR)
	{
		Amount = (int)SAmount->GetValue();
	}

	int Success;

	GameItem *pGI;
	pGI = new GameItem();
	pGI->SetItem((Item *)pWhat, Amount);

	Success = pWho->Equip(pGI, Amount);
	
	if(!Success)
	{
		delete pGI;
	}


	
	pDestination->SetValue((void *)Success);
	pDestination->SetType(ARG_NUMBER);
	return pDestination;
}

ScriptArg *toggleinventory(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	ZSMainWindow *pMainWin;

	pMainWin = (ZSMainWindow *)ZSWindow::GetMain();

	if(pMainWin->InventoryShown())
	{
		pMainWin->HideInventory();
	}
	else
	{
		pMainWin->ShowInventory();
	}

	pDestination->SetValue((void *)1);
	pDestination->SetType(ARG_NUMBER);
	return pDestination;
}

ScriptArg *equipall(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SB;

	Creature *pWho;

	SA = ArgList[0].Evaluate();
	if(!SA->GetCreature())
	{
		SafeExit("Bad Creature to equip!\n");
	}
	
	pWho = (Creature *)SA->GetValue();

	SB = ArgList[1].Evaluate();
	GameItem *pGI = NULL;
	GameItem *pToEquip = NULL;
	BOOL WasEquipped = FALSE;
	
	int ec = 0; //equip counter

	//cycle through and attempt to equip everything in inventory
	pGI = (GameItem *)pWho->GetContents();
	while(pGI)
	{
		pToEquip = pGI;
		pGI = (GameItem *)pGI->GetNext();

		WasEquipped = pWho->Equip(pToEquip);

		if(WasEquipped)
		{
			WasEquipped = FALSE;
			pWho->RemoveItem(pToEquip);
			pToEquip = NULL;
		}
	}

	pWho->ReEquip();
	pWho->CreateTexture();
	pWho->SetEquipObjects();
		
	pDestination->SetValue((void *)1);
	pDestination->SetType(ARG_NUMBER);
	return pDestination;
}

/* getnumber:  Get a number from the player in range 0...n with starting value x, and text t */
ScriptArg *getnumber(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSText, *pSMin, *pSMax, *pSStart;
	pSText = ArgList[0].Evaluate();
	pSMin  = ArgList[1].Evaluate();
	pSMax = ArgList[2].Evaluate();
	pSStart = ArgList[3].Evaluate();

	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)GetNumber((int)pSMin->GetValue(),(int)pSMax->GetValue(),(int)pSStart->GetValue(),(char *)pSText->GetValue()));

	return pDestination;
	



}


/* give: puts a particular item in a characters inventory */
ScriptArg *give(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	

	Thing *pWho, *pWhat;
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_ITEM &&
		SA->GetType() != ARG_THING &&
		SA->GetType() != ARG_PARTY &&
		!SA->GetCreature())
	{
		DEBUG_INFO("Bad First Argument to Give.\n");
		Describe("Bad First Argument to Give.");
		return pDestination;
	}
	else
	{
		pWho = (Thing *)SA->GetValue();
	}

	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_ITEM &&
		SB->GetType() != ARG_THING &&
		SB->GetType() != ARG_NUMBER &&
		!SB->GetCreature())
	{
		DEBUG_INFO("Bad Second Argument to Give.");
		Describe("Bad Second Argument to Give.");
		return pDestination;
	}
	else
	{
		if(SB->GetType() == ARG_NUMBER)
		{
			pWhat = Thing::Find((Thing *)Item::GetFirst(),(int)SB->GetValue() % 1000);
			if(!pWhat)
			{
				return pDestination;
			}
		}
		else
		{
			pWhat = (Thing *)SB->GetValue();
		}
	}
		
	pDestination->SetType(ARG_NUMBER);

	int DoesHave;
	int Quantity = 1;
	ScriptArg *SQ;
	SQ = ArgList[2].Evaluate();
	if(SQ->GetType() != ARG_TERMINATOR)
	{
		Quantity = (int)SQ->GetValue();
	}
	else
	if(SB->GetType() == ARG_NUMBER)
	{
		Quantity = (int)SB->GetValue() /1000;
	}
	if(SA->GetType() != ARG_PARTY)
	{
		DoesHave = pWho->Give(pWhat,Quantity);
	}
	else
	{
		DoesHave = PreludeParty.Give(pWhat,Quantity);
	}

	pDestination->SetValue((void *)DoesHave);
	
	return pDestination;
}

/* ask:  displays a submenu of choices and ask the player to 
	choose one, then acts on that response */
ScriptArg *ask(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ZSWindow *pWin;

	RECT AskRect;

	AskRect.left = 100;
	AskRect.right = 440;
	AskRect.top = 240;
	AskRect.bottom = 380;

	if(ScriptContextWindow)
	{
		ScriptContextWindow->GetChild(IDC_REPLY)->GetBounds(&AskRect);
	}

	pWin = new ZSAskWin(IDC_ASK, XYWH(AskRect), ArgList);

	if(ScriptContextWindow && ScriptContextWindow->IsVisible())
	{
		ScriptContextWindow->AddTopChild(pWin);
	}
	else
	{
		ZSWindow::GetMain()->AddTopChild(pWin);
	}

	pWin->Show();

	int Result;

	pWin->SetFocus(pWin);

	Result = pWin->GoModal();

	pWin->ReleaseFocus();

	pWin->GetParent()->RemoveChild(pWin);

	

	

	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)Result);

	if(ScriptContextWindow)
	{
		((ZSTalkWin *)ScriptContextWindow)->SayClear();
	}
	return pDestination; 
}

/* sflag:  (s)et flag sets a flag to a number*/
ScriptArg *sflag(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	Flag *pFlag;
	
	
	
	

	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_FLAG)
	{
		DEBUG_INFO("Bad argument to Set Flag\n");
		Describe("Bad argument to Set Flag\n");
		return pDestination;
	}
	else
	{
		pFlag = (Flag *)SA->GetValue();
		pFlag->Value = ArgList[1].Evaluate()->GetValue();
	}
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)pFlag->Value);

#ifndef NDEBUG
	DEBUG_INFO("Setting Flag:");
	DEBUG_INFO(pFlag->Name);
	DEBUG_INFO("\n");
#endif

	return pDestination;
}

/* flagm: flag (m)inus subtracts a number from a flag */
ScriptArg *flagm(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	Flag *pFlag;
	ScriptArg *SA;
	
	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_FLAG)
	{
		DEBUG_INFO("Bad argument to Flag Minus\n");
		Describe("Bad argument to Flag Minus\n");
		return pDestination;
	}
	else
	{
		pFlag = (Flag *)SA->GetValue();
		pFlag->Value = (void *)((int)pFlag->Value - (int)ArgList[1].Evaluate()->GetValue());
	}

	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)pFlag->Value);

	return pDestination;
}

/* killflag:  kill a given flag */
/* true if flag existed, false if not */
ScriptArg *killflag(ScriptArg *ArgList, ScriptArg *pDestination)
{
	Flag *pFlag;
	ScriptArg *SA;
	int BResult;

	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_FLAG)
	{
		DEBUG_INFO("Bad argument to KillFlag\n");
		Describe("Bad argument to KillFlag.");
		return pDestination;
	}
	else
	{
		pFlag = (Flag *)SA->GetValue();
		BResult = PreludeFlags.Kill(pFlag->Name);
	}
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)BResult);

	return pDestination;


}

/* flagp: flag (p)lus adds a number to a flag */
ScriptArg *flagp(ScriptArg *ArgList, ScriptArg *pDestination)
{
	Flag *pFlag;
	ScriptArg *SA;
	
	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_FLAG)
	{
		DEBUG_INFO("Bad argument to Flag Plus\n");
		Describe("Bad argument to Flag Plus.");
		return pDestination;
	}
	else
	{
		pFlag = (Flag *)SA->GetValue();
		pFlag->Value = (void *) ((int)pFlag->Value + (int)ArgList[1].Evaluate()->GetValue());
	}
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)pFlag->Value);

	return pDestination;
}

/* flag and skill:  returns the value of flag# or skill# */
ScriptArg *flag(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	Flag *pFlag;

	ScriptArg *SA;
	
	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_FLAG)
	{
		DEBUG_INFO("Bad argument to Get Flag\n");
		Describe("Bad argument to Get Flag\n");
		return pDestination;
	}
	else
	{
		pFlag = (Flag *)SA->GetValue();
	}
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)pFlag->Value);

	return pDestination; 
}

/* iff:  if checks the next argument which must resolve to a 
	0,1,2 then acts on the appropriate response, acting on 0 if 
	anything other that a 0,1,2 is given */
ScriptArg *iff(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pCondition;

	pCondition = ArgList[0].Evaluate();
	
	if(pCondition->GetType() == ARG_PARTYONE ||
		pCondition->GetType() == ARG_PARTYTWO ||
		pCondition->GetType() == ARG_PARTYTHREE ||
		pCondition->GetType() == ARG_PARTYFOUR ||
		pCondition->GetType() == ARG_PARTYFIVE ||
		pCondition->GetType() == ARG_PARTYSIX)
	{
		if(pCondition->GetCreature())
		{
			*pDestination = *(ArgList[1].Evaluate());
		}
		else
		{
			*pDestination = *(ArgList[2].Evaluate());
		}
	
		return pDestination;

	}
	if((int)pCondition->GetValue())
	{
		*pDestination = *(ArgList[1].Evaluate());
	}
	else
	{
		*pDestination = *(ArgList[2].Evaluate());
	}

	return pDestination; 
}

ScriptArg *cond(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pCondition;

	pCondition = ArgList[0].Evaluate();
	
	int ResultNum = (int)pCondition->GetValue() + 1;

	for(int n = 0; n < ResultNum; n++)
	{
		if(ArgList[n].GetType() == ARG_TERMINATOR)
		{
			DEBUG_INFO("Bad Number of result in Cond\n");	
			Describe("Bad Number of result in Cond\n");	
			
			return pDestination;
		}
	}

	*pDestination = *(ArgList[ResultNum].Evaluate());
	
	return pDestination; 
}

/* basic math functions */
ScriptArg *add(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	
	int n;
	int Total;
	Total = 0;

	n = 0;
	while(1)
	{
		SA = ArgList[n].Evaluate();
		if(SA->GetType() == ARG_TERMINATOR)
		{
			break;
		}
		if(SA->GetType() != ARG_NUMBER)
		{
			DEBUG_INFO("Bad Argument to add, not a number\n");
			Describe("Bad Argument to add, not a number\n");
		}
		else
		{
			Total += (int)SA->GetValue();
		}
		n++;
	}

	pDestination->SetValue((void *)Total);
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
}

ScriptArg *sub(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	

	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad first Argument to subtract\n");
		Describe("Bad first Argument to subtract\n");
		return pDestination;
	}

	ScriptArg *SB;
	
	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Second Argument to subtract\n");
		Describe("Bad Second Argument to subtract\n");
		return pDestination;
	}
	
	pDestination->SetValue((void *)((int)SA->GetValue() - (int)SB->GetValue()));
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
	
}

ScriptArg *mul(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	

	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad first Argument to multiply\n");
		Describe("Bad first Argument to multiply\n");
		return pDestination;
	}

	ScriptArg *SB;
	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Second Argument to Multiply\n");
		Describe("Bad Second Argument to Multiply\n");
		return pDestination;
	}


	pDestination->SetValue((void *)((int)SA->GetValue() * (int)SB->GetValue()));
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
}

ScriptArg *div(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	

	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad first Argument to divide\n");
		Describe("Bad first Argument to divide\n");
		return pDestination;
	}

	ScriptArg *SB;
	SB = ArgList[1].Evaluate();
	if(SB->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Second Argument to Divide\n");
		Describe("Bad Second Argument to divide\n");
		return pDestination;
	}


	pDestination->SetValue((void *)((int)SA->GetValue() / (int)SB->GetValue()));
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
}

/* randnum: returns a random number between two values */
ScriptArg *randnum(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	

	

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Argument to Random\n");
		Describe("Bad Argument to Random\n");
		return pDestination;
	}


	pDestination->SetValue((void *)(rand() % (int)SA->GetValue()));
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
}

ScriptArg *prand(ScriptArg *ArgList, ScriptArg *pDestination)
{
	int Modifier;
	ScriptArg *pSA, *pSB;
	

	

	pSA = ArgList[0].Evaluate();
	if(pSA->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Argument to Random\n");
		Describe("Bad Argument to Random\n");
		return pDestination;
	}

	pSB = ArgList[0].Evaluate();
	if(pSB->GetType() != ARG_NUMBER)
	{
		DEBUG_INFO("Bad Argument to Random\n");
		Describe("Bad Argument to Random\n");
		return pDestination;
	}

	Modifier = (int)pSB->GetValue();

	int Base;
	Base = rand();

	pDestination->SetValue((void *)(Base % (int)pSA->GetValue()));
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
}


/* comp: compares two numbers  returns: 1 if a<b, 2 if a=b, 3 if a>b */
ScriptArg *comp(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA, *SB;
	
	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_NUMBER)
	{
		SafeExit("Comp 1st arg not number not good\n");
	}
	
	SB = ArgList[1].Evaluate();
	
	if(SB->GetType() != ARG_NUMBER)
	{
		SafeExit("Comp 2nd arg not number not good\n");
	}
	
	
	pDestination->SetType(ARG_NUMBER);

	if((int)SA->GetValue() < (int)SB->GetValue())
	{
		pDestination->SetValue((void *)0);
	}
	else
	if((int)SA->GetValue() == (int)SB->GetValue())
	{
		pDestination->SetValue((void *)1);
	}
	else
	if((int)SA->GetValue() > (int)SB->GetValue())
	{
		pDestination->SetValue((void *)2);
	}
	return pDestination; 
}

/* removeword: removes a word from the menu list */
ScriptArg *removeword(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_STRING);

	((ZSTalkWin *)ScriptContextWindow)->RemoveWord((char *)SA->GetValue());

	
	

	return pDestination;
}

ScriptArg *schedule(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA, *SB;
	Creature *pCreature;
	

	SA = ArgList[0].Evaluate();
	SB = ArgList[1].Evaluate();

	pCreature = SA->GetCreature();

	
	if(SB->GetType() == ARG_TERMINATOR)
	{
		while(pCreature->GetNumLocators())
		{
			pCreature->RemoveLocator(0);
		}
	}
	else
	{
		ScriptArg *pSLeft, *pSTop, *pSRight, *pSBottom, *pSStart, *pSEnd, *pSAngle, *pSState, *pSArea;
		pSLeft = SB;
		if(pSLeft->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad left arg to script schedule");
		}
		pSTop = ArgList[2].Evaluate();
		if(pSTop->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad top arg to script schedule");
		}
		pSRight = ArgList[3].Evaluate();
		if(pSRight->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad right arg to script schedule");
		}
		pSBottom = ArgList[4].Evaluate();
		if(pSBottom->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad bottom arg to script schedule");
		}
		pSStart = ArgList[5].Evaluate();
		if(pSStart->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad start arg to script schedule");
		}
		pSEnd = ArgList[6].Evaluate();
		if(pSEnd->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad end arg to script schedule");
		}
		pSAngle = ArgList[7].Evaluate();
		if(pSAngle->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad facing arg to script schedule");
		}
		pSState = ArgList[8].Evaluate();
		if(pSState->GetType() != ARG_NUMBER)
		{
			SafeExit("Bad state arg to script schedule");
		}
		pSArea = ArgList[9].Evaluate();
		
		
		RECT rBounds;
		rBounds.left = (int)pSLeft->GetValue();
		rBounds.right = (int)pSRight->GetValue();
		rBounds.top = (int)pSTop->GetValue();
		rBounds.bottom = (int)pSBottom->GetValue();

		Locator *pLoc;
		pCreature->AddLocator();
		pLoc = pCreature->GetLocator(pCreature->GetNumLocators() - 1);

		pLoc->SetBounds(&rBounds);
		pLoc->SetAngle((BYTE)(int)pSAngle->GetValue());
		pLoc->SetState((BYTE)(int)pSState->GetValue());
		pLoc->SetStart((BYTE)(int)pSStart->GetValue());
		pLoc->SetEnd((BYTE)(int)pSEnd->GetValue());
		if(pSArea->GetType() == ARG_STRING)
		{
			pLoc->SetArea(PreludeWorld->GetAreaNum((char *)pSArea->GetValue()));
		}
		else
		{
			if(pCreature->GetAreaIn() != -1)
				pLoc->SetArea(pCreature->GetAreaIn());
			else
				pLoc->SetArea(PreludeWorld->GetCurAreaNum());
		}
	}
	
	

	return pDestination;
}

ScriptArg *returnschedule(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	Creature *pCreature;
	

	SA = ArgList[0].Evaluate();

	pCreature = SA->GetCreature();

	pCreature->SetLastPlacedTime(0);

	

	return pDestination;
}

BOOL Improving = FALSE;


/* for Items and characters */
ScriptArg *getChar(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA, *pSACreature;;
	Creature *pCreature;
	int Index;

	if(ArgList[0].GetType() == ARG_NUMBER)
	{
		pCreature = (Creature *)ArgList[0].GetValue();
	}
	else
	{
		pSACreature = ArgList[0].Evaluate();
		pCreature = pSACreature->GetCreature();
	}
	

	SA = ArgList[1].Evaluate();

	if(SA->GetType() != ARG_NUMBER)
	{
		SafeExit("bad index to get char not a number\n");
	}

	Index = (int)SA->GetValue();

	

	

	if(!pCreature)
	{
		Describe("Bad creature to getchar");
		return pDestination;
	}

	switch(pCreature->GetType(Index))
	{
	case DATA_STRING:
		char *pString;
		pString = new char[strlen(pCreature->GetData(Index).String) + 1];
		strcpy(pString,pCreature->GetData(Index).String);
		pDestination->SetValue(pString);
		pDestination->SetType(ARG_STRING);
		break;
	case DATA_INT:
		pDestination->SetValue((void *)pCreature->GetData(Index).Value);
		pDestination->SetType(ARG_NUMBER);
		break;
	}

//	DEBUG_INFO("Checking Stat");
//	DEBUG_INFO(pCreature->GetName(Index));
//	DEBUG_INFO("\n");

	return pDestination; 
}

ScriptArg *setChar(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	Thing *pThing;
	int Index;
	
	if(ArgList[0].GetType() == ARG_NUMBER)
	{
		DEBUG_INFO("\n***************\nnumber to setchar\n\n");
		pThing = (Thing *)ArgList[0].GetValue();
	}
	else
	{
		pThing = (Thing *)ArgList[0].Evaluate()->GetValue();
	}

	Index = (int)ArgList[1].Evaluate()->GetValue();

	

	

	ScriptArg *Value;
	Value = ArgList[2].Evaluate();

	if(!pThing)
	{
		Describe("Bad creature to setchar");
	}
	else
	switch(Value->GetType())
	{
		case ARG_STRING:
			char *tempstring;
			tempstring = new char[strlen((char *)Value->GetValue()) + 1];
			strcpy(tempstring,(char *)Value->GetValue());
			pThing->SetData(Index,tempstring);
			break;
		default:
			pThing->SetData(Index,(int)Value->GetValue());
			break;
	}

	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)1);

	return pDestination; 
}

/* endgame:  displays endgame window and quits all */
ScriptArg *endgame(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSA = NULL;
	pSA = ArgList[0].Evaluate();
	
	//create death screen
	DeathWin *pDeathWin;
	pDeathWin = new DeathWin;
	ZSWindow::GetMain()->AddTopChild(pDeathWin);
	if(pSA->GetType() != ARG_TERMINATOR)
	{
		pDeathWin->GetChild(IDC_DEATH_TEXT)->SetText((char *)pSA->GetValue());
	}
	
	pDeathWin->Show();
	pDeathWin->SetFocus(pDeathWin);
	
	pDeathWin->GoModal();
	
	pDeathWin->ReleaseFocus();

	pDeathWin->Hide();

	ZSWindow::GetMain()->RemoveChild(pDeathWin);

	return pDestination;
}

/* wingame:  displays winning animation window and quits all */
ScriptArg *wingame(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSA = NULL;
	//endgameintro cut scene;
	char IntroDir[256];
	//get the maindirectory
	strcpy(IntroDir, Engine->GetRootDirectory());
	strcat(IntroDir, "\\Intro");

	DEBUG_INFO("switching to End directory\n");

	//switch to the Texture directory
	SetCurrentDirectory(IntroDir);

	ZSCutScene *pCutScene;
	FILE *fp;
	fp = fopen("endscene.txt","rt");
	if(fp)
	{
		pCutScene = new ZSCutScene;
		ZSWindow::GetMain()->AddChild(pCutScene);
		pCutScene->Load(fp);
		fclose(fp);

		pCutScene->Show();
		pCutScene->SetFocus(pCutScene);
		pCutScene->GoModal();
		
		pCutScene->ReleaseFocus();

		ZSWindow::GetMain()->RemoveChild(pCutScene);
		
//		ZSWindow::GetMain()->SetState(WINDOW_STATE_DONE);
	}
	else
	{
		Describe ("endgame sequence not found");
	}

	//switch back to root directory
	SetCurrentDirectory(Engine->GetRootDirectory());
	DEBUG_INFO("switching back to root directory\n");

	ZSWindow::GetMain()->SetState(WINDOW_STATE_DONE);


	return pDestination;
}


/* from here down unimplemented */

/* for adding text to the say window instead of overwriting it */
ScriptArg *sayadd_win(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_STRING);

	((ZSTalkWin *)ScriptContextWindow)->SayAdd((char *)SA->GetValue());

	
	

	return pDestination;
}

/* for drawing the screen */
ScriptArg *ScriptDrawScreen(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	static DWORD LastFrame = 0;
	DWORD NextFrame;
	
	NextFrame = LastFrame + ((ZSMainWindow *)ZSWindow::GetMain())->GetFrameRate();
	
	while(timeGetTime() < NextFrame) 
	{
		//do AI updates and what-not here
	}
	
	((ZSMainWindow *)ZSWindow::GetMain())->SetTarget(NULL);

	ZSWindow::GetMain()->Draw();
	Engine->Graphics()->Flip();
	LastFrame = timeGetTime();
	
	return pDestination; 
}



/* for Loops */
ScriptArg *DoLoop(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	do
	{
		pDestination->ClearValue();
		ArgList[0].Evaluate();
	} while((int)ArgList[1].Evaluate()->GetValue());
	
	return pDestination; 
}

ScriptArg *WhileLoop(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	while((int)ArgList[0].Evaluate()->GetValue())
	{
		ArgList[1].Evaluate();
	}
	return pDestination; 
}

ScriptArg *CountLoop(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	int n;
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_NUMBER);

	for(n = 0; n < (int)SA->GetValue(); n++)
	{
		ArgList[1].Evaluate();
	}

	return pDestination; 
}

/* for Updating People */
ScriptArg *ScriptUpdate(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	if(SA->GetType() == ARG_PARTY)
	{
		for(int n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			PreludeParty.GetMember(n)->AdvanceFrame();
		}

		PreludeParty.Occupy();
	}
	else
	{
		Creature *pCreature;
		pCreature = SA->GetCreature();

		BOOL Result;
		Result = pCreature->AdvanceFrame();

		if(!Result)
		{
			Valley->RemoveFromUpdate((Object *)pCreature);
			if(PreludeWorld->InCombat())
			{
				PreludeWorld->GetCombat()->RemoveFromCombat((Object *)pCreature);
			}
		}

		if(PreludeParty.IsMember(pCreature))
		{
			PreludeParty.Occupy();
		}
	}

	return pDestination; 
}

ScriptArg *ScriptUpdateAll(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	PreludeWorld->Update();
	return pDestination; 
}

ScriptArg *ScriptUpdateScreen(ScriptArg *ArgList, ScriptArg *pDestination)
{ 

	return pDestination; 
}

ScriptArg *ClearActions(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	Creature *pCreature;
	pCreature = SA->GetCreature();

	pCreature->ClearActions();

	if(pCreature->GetAction()->GetType() != ACTION_USER && 
		pCreature->GetAction()->GetType() != ACTION_THINK)
	{
		pCreature->RemoveCurrentAction();
	}

	return pDestination; 
}


/* another math function*/
ScriptArg *Mod(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_NUMBER);

	SB = ArgList[1].Evaluate();
	assert(SB->GetType() == ARG_NUMBER);

	

	pDestination->SetValue((void *)((int)SA%(int)SB));
	pDestination->SetType(ARG_NUMBER);

	return pDestination; 
}

ScriptArg *Equals(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();

	SB = ArgList[1].Evaluate();

	

	pDestination->SetValue((void *)(*SA == *SB));
	pDestination->SetType(ARG_NUMBER);

	return pDestination; 
}

/* for defining variables */
ScriptArg *DefVar(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *FreeVar(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//cfunctions for above
void AddVariable(char *name) { return; }
void FreeVariable(char *name) { return; }
ScriptArg *FindVariable(char *name) { return NULL; }

/* for pictures */
ScriptArg *DisplayPicture (ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *RemovePicture (ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

/* for a character saying something outside of a conversation */
ScriptArg *CharSays(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

/* for getting map positions */
ScriptArg *GetMapX(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetMapY(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetScreenX(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetScreenY(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetScreenCenterX(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetScreenCenterY(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetRegionName(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }


/* for changing party display mode */
ScriptArg *HideParty(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *ShowParty(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

/* for affecting people */
ScriptArg *SetCharState(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

ScriptArg *FindPath(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSAX1, *pSAY1, *pSAX2, *pSAY2;
	int x1,y1,x2,y2;

	pSAX1 = ArgList[0].Evaluate();
	pSAY1 = ArgList[1].Evaluate();
	pSAX2 = ArgList[2].Evaluate();
	pSAY2 = ArgList[3].Evaluate();

	x1 = (int)pSAX1->GetValue();
	y1 = (int)pSAY1->GetValue();
	x2 = (int)pSAX2->GetValue();
	y2 = (int)pSAY2->GetValue();
	
	pDestination->SetType(ARG_NUMBER);

	int RetValue = 1;

	if(Valley->GetBlocking(x1,y1) || Valley->GetBlocking(x2,y2))
	{
		RetValue = 0;

		pDestination->SetValue((void *)RetValue);
		return pDestination;
	}

	Path MyPath;
	if(!MyPath.FindPath(x1,y1,x2,y2) || MyPath.GetLength() == 666)
	{
		RetValue = FALSE;
	}
	else
	{
		RetValue = MyPath.GetLength();
	}
	

	pDestination->SetValue((void *)RetValue);

	return pDestination;
}



ScriptArg *PlacePortal(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSADestination, *pSADestX, *pSADestY, *pSAEvent,
		*pSAMesh, *pSATexture, *pSAX, *pSAY, *pSAZ, *pSAScale, *pSARotation;

	pSADestination = ArgList[0].Evaluate();
	pSADestX = ArgList[1].Evaluate();
	pSADestY = ArgList[2].Evaluate();
	pSAEvent = ArgList[3].Evaluate();
	pSAMesh = ArgList[4].Evaluate();
	pSATexture = ArgList[5].Evaluate();
	pSAX = ArgList[6].Evaluate();
	pSAY = ArgList[7].Evaluate();
	pSAZ = ArgList[8].Evaluate();
	pSAScale = ArgList[9].Evaluate();
	pSARotation = ArgList[10].Evaluate();

	D3DVECTOR vLocation;
	vLocation.x = (float)(int)pSAX->GetValue() + 0.5f;
	vLocation.y = (float)(int)pSAY->GetValue() + 0.5f;
	vLocation.z = ((float)(int)pSAZ->GetValue() / 10.0f) +
					Valley->GetTileHeight((int)pSAX->GetValue(),(int)pSAY->GetValue());
	float fScale;
	fScale = (float)(int)pSAScale->GetValue() / 100.0f;
		
	float fRotation;
	fRotation = DegToRad((float)(int)pSARotation->GetValue());

	Entrance *pPortal;
	pPortal = new Entrance;
	pPortal->SetAngle(fRotation);
	pPortal->SetScale(fScale);
	pPortal->SetMesh(Engine->GetMesh((char *)pSAMesh->GetValue()));
	pPortal->SetTexture(Engine->GetTexture((char *)pSATexture->GetValue()));
	pPortal->SetEvent((int)pSAEvent->GetValue());
	pPortal->SetDestX((int)pSADestX->GetValue());
	pPortal->SetDestY((int)pSADestY->GetValue());
	sprintf(pPortal->GetDestinationName(),"%s",(char *)pSADestination->GetValue());
	pPortal->SetPosition(&vLocation);
	
	Valley->AddToUpdate(pPortal);

	return pDestination; 
}



ScriptArg *WhatIsAt(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

/* for creating strings (equivlaent of printf) */
ScriptArg *CreateString(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	char TempString[512];
	char Temp[16];
	char *SubString;

	SA = ArgList[0].Evaluate();
	if(SA->GetType() != ARG_STRING)
	{
		DEBUG_INFO("Bad argument to create string\n");
		Describe("bad argument to create string\n");
		return pDestination;
	}

	SubString = (char *)SA->GetValue();

	int n = 0;
	int sn = 0;
	int Argn = 1;

	while(SubString[n] != '\0')
	{
		if(SubString[n] != '%')
		{
			TempString[sn] = SubString[n];
			n++;
			sn++;
		}
		else
		{
			TempString[sn] = '\0';
			n++;
			switch(SubString[n])
			{
			case 'i':
				SA = ArgList[Argn++].Evaluate();
				if(SA->GetType() != ARG_NUMBER)
				{
					SafeExit("Bad Arg to String, not a number\n");
					return NULL;
				}
				sprintf(Temp,"%i",(int)SA->GetValue());
				strcat(TempString,Temp);
				sn += strlen(Temp);
				break;
			case 's':
				SA = ArgList[Argn++].Evaluate();
				if(SA->GetType() != ARG_STRING)
				{
					SafeExit("Bad Arg to String, not a string\n");
					return NULL;
				}
				strcat(TempString,(char *)SA->GetValue());
				sn += strlen((char *)SA->GetValue());
				break;
			}
			n++;
		}
	}

	TempString[sn] = '\0';

	

	char *RetString;
	RetString = new char[strlen(TempString) + 1];
	strcpy(RetString,TempString);

	pDestination->SetType(ARG_STRING);
	pDestination->SetValue(RetString);

	return pDestination;; 
}

/* for managing the time of day */
ScriptArg *GetHour(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)PreludeWorld->GetHour());
	return pDestination;
}

ScriptArg *GetDay(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *GetMin(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *GetTotalTime(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)PreludeWorld->GetTotalTime());

	return pDestination; 
}

//advance time by a given quantity of minutes.
ScriptArg *AdvanceTime(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	PreludeWorld->AdvanceTime((int)SA->GetValue());
	
	return pDestination; 
}

/* for event management */
ScriptArg *DoOnEndCombat(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *DoOnStartCombat(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

/* for removing items from the game */
ScriptArg *RemoveChar(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	Creature *pCreature;
	pCreature = SA->GetCreature();
	if(pCreature)
	{	
		DEBUG_INFO("removeing character and killing locators\n");
		Area *pArea = NULL;
	
		if(PreludeWorld->InCombat())
		{
			PreludeWorld->GetCombat()->RemoveFromCombat((Object *)pCreature);
		}

		pArea = PreludeWorld->GetArea(pCreature->GetAreaIn());
		if(pArea) 
			pArea->RemoveFromUpdate(pCreature);
		while(pCreature->GetNumLocators())
		{
			pCreature->RemoveLocator(0);
		}
	}
	else
	{
		DEBUG_INFO("Tried to remove someone who wasn't there.");
	}
	return pDestination; 
}


ScriptArg *Place(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA, *SX, *SY, *SArea, *pSAmount;

	SA = ArgList[0].Evaluate();

	SX = ArgList[1].Evaluate();
	SY = ArgList[2].Evaluate();
	SArea = ArgList[3].Evaluate();

	D3DVECTOR vNewPos;

	int AreaNum;
	
	vNewPos.x = (float)((int)SX->GetValue()) + 0.5;
	vNewPos.y = (float)((int)SY->GetValue()) + 0.5;
	vNewPos.z = Valley->GetTileHeight((int)vNewPos.x,(int)vNewPos.y);

	if(SArea->GetType() != ARG_STRING)
	{
		AreaNum = PreludeWorld->GetCurAreaNum();	
		pSAmount = SArea;
	}
	else
	{
		AreaNum = PreludeWorld->GetAreaNum((char *)SArea->GetValue());
		pSAmount = ArgList[4].Evaluate();
	}

	int Amount;
	Amount = 1;
	if(SArea->GetType() == ARG_NUMBER)
	{
		Amount = (int)SArea->GetValue();
	}

	D3DVECTOR *pPosition;
	
	

	switch(SA->GetType())
	{
	case ARG_CREATURE:
		Creature *pCreature, *pNewCreature;
		pCreature = SA->GetCreature();
		//is it unique
		if(pCreature->GetData(INDEX_BATTLEID).Value)
		{
			//if so add a unique copy
			pNewCreature = new Creature(pCreature);
			pCreature = pNewCreature;
			Locator *pLoc;
			pCreature->AddLocator();
			pLoc = pCreature->GetLocator(0);
			pLoc->SetStart(0);
			pLoc->SetEnd(23);
			RECT rBounds;
			rBounds.left = (int)vNewPos.x;
			rBounds.right = (int)vNewPos.x + 1;
			rBounds.top = (int)vNewPos.y;
			rBounds.bottom = (int)vNewPos.y + 1;
			pLoc->SetBounds(&rBounds);
			pLoc->SetArea(AreaNum);
		}
		else
		{
			if(PreludeWorld->GetArea(pCreature->GetAreaIn()))
			{
				PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate(pCreature);
			}
		}
		pPosition = pCreature->GetPosition();
		*pPosition = vNewPos;
		pCreature->SetPosition(&vNewPos);
		pCreature->SetAreaIn(AreaNum);
		pCreature->AddToWorld();
		pCreature->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pCreature->SetLastPlacedTime(PreludeWorld->GetTotalTime());
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pCreature);
		break;
	case ARG_THING:
		Thing *pThing;
		pThing = (Thing *)SA->GetValue();
		break;
	case ARG_ITEM:
		Item *pItem;
		pItem = (Item *)SA->GetValue();
		GameItem *pGI;
		pGI = new GameItem();
		pGI->SetItem(pItem);
		pGI->SetQuantity(Amount);
		vNewPos.z += 0.1f;
		pGI->SetPosition(&vNewPos);
		PreludeWorld->GetArea(AreaNum)->AddToUpdate((Object *)pGI);
		pGI->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pGI->SetLocation(LOCATION_WORLD,NULL);
		pGI->SetOwner(PreludeParty.GetLeader());
		break;
	default:
		pCreature = SA->GetCreature();
		if(!pCreature)
		{
			SafeExit("Bad thing being placed!\n");
		}
		pPosition = pCreature->GetPosition();
		*pPosition = vNewPos;
		pCreature->SetPosition(&vNewPos);
		pCreature->SetAreaIn(AreaNum);
		pCreature->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pCreature->AddToWorld();
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pCreature);
		break;
	}
	
	return pDestination; 
}

/* logic functions */
ScriptArg *And(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;

	BOOL Result = TRUE;
	int n = 0;

	while(Result)
	{
		SA = ArgList[n].Evaluate();
		if(SA->GetType() == ARG_TERMINATOR)
		{
			break;	
		}
//		assert(SA->GetType() == ARG_NUMBER);
		Result = (BOOL)SA->GetValue();
		n++;
	}
	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)Result);

	return pDestination; 
}

ScriptArg *Or(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	BOOL Result = FALSE;
	int n = 0;

	while(!Result)
	{
		SA = ArgList[n].Evaluate();
		if(SA->GetType() == ARG_TERMINATOR)
		{
			break;
		}
		assert(SA->GetType() == ARG_NUMBER);
		Result = (BOOL)SA->GetValue();
		n++;
	}
	
	
	pDestination->SetValue((void *)Result);
	pDestination->SetType(ARG_NUMBER);
	return pDestination;
}

ScriptArg *Not(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	
	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_NUMBER);

	
	pDestination->SetType(ARG_NUMBER);

	if((int)SA->GetValue())
	{
		pDestination->SetValue((void *)0);
	}
	else
	{
		pDestination->SetValue((void *)1);
	}
	return pDestination;
}

/* checking functions */
ScriptArg *IsEmpty(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination; }
ScriptArg *IsMoving(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	Creature *pCreature;
	ScriptArg *pSA;
	pSA = ArgList[0].Evaluate();
	pCreature = pSA->GetCreature();

	pDestination->SetValue((void *)pCreature->IsMoving());
	pDestination->SetType(ARG_NUMBER);
	return pDestination; 
}

/*stack functions */
ScriptArg *Push(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptStack[StackTop] = new ScriptArg(ArgList[0].Evaluate());	
	
	*pDestination = *ScriptStack[StackTop];

	StackTop++;
	return pDestination; 
}
ScriptArg *Pop(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	StackTop--;
	
	*pDestination = *ScriptStack[StackTop];
	
	delete ScriptStack[StackTop];

	ScriptStack[StackTop] = NULL;

	return pDestination; 
}

ScriptArg *Switch(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *Temp;
	Temp = ScriptStack[StackTop - 1];
	ScriptStack[StackTop - 1] = ScriptStack[StackTop - 2];
	ScriptStack[StackTop - 2] = Temp;
	return pDestination;; 
}

ScriptArg *Dupe(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptStack[StackTop] = new ScriptArg(ScriptStack[StackTop-1]);	

	
	*pDestination = *ScriptStack[StackTop];

	StackTop++;
	return pDestination; 
}

ScriptArg *LookStack(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	
	if(ArgList[0].GetType() != ARG_TERMINATOR)
	{
		int offset;
		ScriptArg *SA;
		SA = ArgList[0].Evaluate();
		offset = (int)ArgList[0].GetValue();
		*pDestination = *ScriptStack[StackTop - (1 + offset)];
	}
	else
	{
		*pDestination = *ScriptStack[StackTop - 1];
	}
	return pDestination; 
}

/* for calling extern script files */
ScriptArg *CallScript(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptBlock SB;

	ScriptArg *SA, *SID;

	SID = ArgList[0].Evaluate();
	if(SID->GetType() != ARG_STRING)
	{
		DEBUG_INFO("bad function in call script");
		return pDestination;
	}


	SA = ArgList[1].Evaluate();

	char FileName[32];	
	int n = 0;

	if(SA->GetType() == ARG_STRING)
	{
		strcpy(FileName,(char *)SA->GetValue());
		n = 2;
	}
	else
	{
		strcpy(FileName,"Functions.txt");
		n = 1;
	}
		
	FILE *fp;

	fp = SafeFileOpen(FileName,"rt");

	int NumArgs = 0;
	ScriptArg *pSA[32];

	while(ArgList[n].GetType() != ARG_TERMINATOR)
	{
		pSA[n] = ArgList[n].Evaluate();
		n++;
		NumArgs++;
	}

	for(int carg = 0; carg < NumArgs; carg++)
	{
		n--;
		Push(pSA[n], pDestination);
	}
	
	char CallScriptID[64];
	BOOL Found = FALSE;

	if(fp)
	{
		sprintf(CallScriptID,"#%s#",(char *)SID->GetValue());

		if(SeekTo(fp,CallScriptID))
		{
			Found = TRUE;
			SB.Import(fp);	
			fclose(fp);		
		}
	}
	else
	{
		fp = fopen(FileName,"rb");
		if(!fp)
		{
			DEBUG_INFO("Couldn't open script file: ")
			DEBUG_INFO((char *)SA->GetValue());
			DEBUG_INFO("\n");
			Describe("Callscript failed: ");
			Describe((char *)SA->GetValue());
			return NULL;
		}
		
		SB.Import(fp);
		fclose(fp);
		Found = TRUE;
	}

	ScriptBlock *OldContext;
	OldContext = ScriptContextBlock;
	
	if(Found)
	{
		ScriptContextBlock = &SB;

		SB.Process();
	
	}
	else
	{
		Describe("Couldn't find function");
		Describe(CallScriptID);

	}

	
	ScriptContextBlock = OldContext;

	return pDestination; 
}

//Improve a skill
ScriptArg *ImproveSkill(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA, *pSACreature;;
	Creature *pCreature;
	int Index;

	if(ArgList[0].GetType() == ARG_NUMBER)
	{
		pCreature = (Creature *)ArgList[0].GetValue();
	}
	else
	{
		pSACreature = ArgList[0].Evaluate();
		pCreature = pSACreature->GetCreature();
	}
	

	SA = ArgList[1].Evaluate();

	if(SA->GetType() != ARG_NUMBER)
	{
		SafeExit("bad index to get char not a number\n");
	}

	Index = (int)SA->GetValue();

	

	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)pCreature->ImproveSkill(Index));

	if(!pCreature)
	{
		Describe("Bad creature to improveskill");
		return pDestination;
	}


	return pDestination; 
}

//wait for a user event or until time n has passed
ScriptArg *WaitFor(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//runs the attack animation for a character
ScriptArg *AttackScript(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//gets the data from a person
ScriptArg *GetCharacterDataNum(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//sets the global "target"
ScriptArg *SetTarget(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//places the party in a map at an x/y
ScriptArg *PlaceParty(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//creates a c-like switch statement
ScriptArg *ScriptSwitch(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//removes all words on the current menu
ScriptArg *removeall(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)((ZSTalkWin *)ScriptContextWindow)->RemoveAll());
	
	return pDestination; 
}

ScriptArg *removeitem(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSX, *pSY, *pSWhat;

	pSX = ArgList[0].Evaluate();
	pSY = ArgList[1].Evaluate();
	pSWhat = ArgList[2].Evaluate();

	float left, right, top, bottom;
	Object *pOb;
	GameItem *pGI;

	left = (int)pSX->GetValue();
	right = left + 1.0f;
	top = (int)pSY->GetValue();
	bottom = top + 1.0f;

	if(pSWhat->GetType() != ARG_TERMINATOR)
	{
		pOb = Valley->GetUpdateSegment(((int)left + 1) / UPDATE_SEGMENT_WIDTH, ((int)top + 1) / UPDATE_SEGMENT_HEIGHT);

		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_ITEM)
			{
				pGI = (GameItem *)pOb;
				pOb = pOb->GetNextUpdate();		
				if(pGI->GetItem() == (Item *)pSWhat->GetValue() &&
					pGI->GetPosition()->x >= left &&
					pGI->GetPosition()->y >= top &&
					pGI->GetPosition()->x <= right &&
					pGI->GetPosition()->y <= bottom)
				{
					Valley->RemoveFromUpdate(pGI);
					delete pGI;
				}
			}
			else
			{
				pOb = pOb->GetNextUpdate();
			}
		}
	}
	else
	{
		pOb = Valley->GetUpdateSegment(((int)left + 1) / UPDATE_SEGMENT_WIDTH, ((int)top + 1) / UPDATE_SEGMENT_HEIGHT);

		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_ITEM)
			{
				pGI = (GameItem *)pOb;
				pOb = pOb->GetNextUpdate();		
				if(	pGI->GetPosition()->x >= left &&
					pGI->GetPosition()->y >= top &&
					pGI->GetPosition()->x <= right &&
					pGI->GetPosition()->y <= bottom)
				{
					Valley->RemoveFromUpdate(pGI);
					delete pGI;
				}
			}
			else
			{
				pOb = pOb->GetNextUpdate();
			}
		}
	}

	return pDestination;
}

ScriptArg *moveitem(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSXFrom, *pSYFrom, *pSXTo, *pSYTo, *pSWhat, *pSRate;

	pSWhat = ArgList[0].Evaluate();
	pSXFrom = ArgList[1].Evaluate();
	pSYFrom = ArgList[2].Evaluate();
	pSXTo = ArgList[3].Evaluate();
	pSYTo = ArgList[4].Evaluate();
	pSRate = ArgList[5].Evaluate();
	
	float left, right, top, bottom, fxto, fyto, frate, fdist;
	
	Object *pOb;
	GameItem *pGI, *pToMove;

	left = (int)pSXFrom->GetValue() - 1;
	right = left + 2.0f;
	top = (int)pSYFrom->GetValue() - 1;
	bottom = top + 2.0f;

	pOb = Valley->GetUpdateSegment(((int)left + 1) / UPDATE_SEGMENT_WIDTH, ((int)top + 1) / UPDATE_SEGMENT_HEIGHT);

	pToMove = FALSE;
	while(pOb && !pToMove)
	{
		if(pOb->GetObjectType() == OBJECT_ITEM)
		{
			pGI = (GameItem *)pOb;
			pOb = pOb->GetNextUpdate();		
			if(pGI->GetItem() == (Item *)pSWhat->GetValue() &&
				pGI->GetPosition()->x >= left &&
				pGI->GetPosition()->y >= top &&
				pGI->GetPosition()->x <= right &&
				pGI->GetPosition()->y <= bottom)
			{
				pToMove = pGI;
			}
		}
		else
		{
			pOb = pOb->GetNextUpdate();
		}
	}

	if(!pToMove)
	{
		Describe("failed to find object to move");
		return pDestination;
	}

	D3DVECTOR vFrom;
	D3DVECTOR vTo;
	D3DVECTOR vMove;
	
	vTo.x = fxto = (int)pSXTo->GetValue() + 0.5f;
	vTo.y = fyto = (int)pSYTo->GetValue() + 0.5f;
	vTo.z = Valley->GetTileHeight((int)pSXTo->GetValue(),(int)pSYTo->GetValue());
	
	vFrom = *pToMove->GetPosition();
	
	frate = 30.0f / (int)pSRate->GetValue();

	int NumFrames;
	fdist = GetDistance(&vFrom, &vTo);

	NumFrames = fdist * frate;
		
	vMove.x = (vTo.x - vFrom.x) / (float)NumFrames;
	vMove.y = (vTo.y - vFrom.y) / (float)NumFrames;
	vMove.z = (vTo.z - vFrom.z) / (float)NumFrames;

	for(int n = 0; n < NumFrames; n++)
	{
		pToMove->Move(&vMove);
		((ZSMainWindow *)ZSWindow::GetMain())->DrawAndUpdate();
	}

	Valley->RemoveFromUpdate(pToMove,vFrom.x / UPDATE_SEGMENT_WIDTH, vFrom.y / UPDATE_SEGMENT_HEIGHT);
	Valley->AddToUpdate(pToMove);

	return pDestination;
}

//clear animation
ScriptArg *removeanimation(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//animate -- 
ScriptArg *animate(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SWho, *SRange;

	SWho = ArgList[0].Evaluate();

	SRange = ArgList[1].Evaluate();

	Creature *pCreature;

	pCreature = SWho->GetCreature();

	if(!pCreature)
	{
		SafeExit("Attempting to animate non-creature");
	}

	pCreature->InsertAction(ACTION_ANIMATE,NULL,SRange->GetValue());

	return pDestination; 
}

//damage -- deal damage to a person
ScriptArg *damage(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSWho, *pSAmount, *pSSource;

	pSWho = ArgList[0].Evaluate();

	pSAmount = ArgList[1].Evaluate();

	pSSource = ArgList[2].Evaluate();

	Creature *pCreature;
	pCreature = pSWho->GetCreature();

	pCreature->TakeDamage(pSSource->GetCreature(), (int)pSAmount->GetValue(), DAMAGE_NORMAL);
	
	return pDestination; 
}

//saychar -- a particular character says something
ScriptArg *saychar(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SX, *SY, *SText, *SWho;

	SWho = ArgList[0].Evaluate();
	
	SX = ArgList[1].Evaluate();
	assert(SX->GetType() == ARG_NUMBER);

	SY = ArgList[2].Evaluate();
	assert(SY->GetType() == ARG_NUMBER);
	
	SText = ArgList[3].Evaluate();
	assert(SText->GetType() == ARG_STRING);

	ZSWindow *pWin;

	pWin = new ZSSayChar(IDC_SAY_CHAR,(int)SX->GetValue(),(int)SY->GetValue(),0,0,(char *)SText->GetValue(), (Thing *)SWho->GetCreature());

	ZSWindow::GetMain()->AddTopChild(pWin);
	pWin->Show();

	pWin->SetFocus(pWin);

	pWin->GoModal();
	
	pWin->ReleaseFocus();

	ZSWindow::GetMain()->RemoveChild(pWin);

	

	

	return pDestination; 
}

//saychardesc -- a particular character describes something
ScriptArg *saychardesc(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SX, *SY, *SText, *SWho;

	SWho = ArgList[0].Evaluate();
	
	SX = ArgList[1].Evaluate();
	assert(SX->GetType() == ARG_NUMBER);

	SY = ArgList[2].Evaluate();
	assert(SY->GetType() == ARG_NUMBER);
	
	SText = ArgList[3].Evaluate();
	assert(SText->GetType() == ARG_STRING);

	ZSWindow *pWin, *pSubWin;

	pWin = new ZSSayChar(IDC_SAY_CHAR,(int)SX->GetValue(),(int)SY->GetValue(),0,0,(char *)SText->GetValue(), (Thing *)SWho->GetValue());
	pSubWin = pWin->GetChild(10);
	pSubWin->SetTextColor(TEXT_LIGHT_GREY_PARCHMENT);

	ZSWindow::GetMain()->AddTopChild(pWin);
	pWin->Show();

	pWin->SetFocus(pWin);

	pWin->GoModal();
	
	pWin->ReleaseFocus();

	ZSWindow::GetMain()->RemoveChild(pWin);

	

	

	return pDestination; 
}

//playsound/music
ScriptArg *playsound(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSA;

	pSA = ArgList[0].Evaluate();

	if(pSA->GetType() == ARG_STRING)
	{
		Engine->Sound()->PlayMusic((char *)pSA->GetValue());
	}
	else
	{
		Engine->Sound()->PlayEffect((int)pSA->GetValue());

	}
	return pDestination; 
}

ScriptArg *playmusic(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *incombat(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)PreludeWorld->InCombat());
	return pDestination; 
}

//get the player with the best of a given statistic
ScriptArg *getbest(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	

	pDestination->SetType(ARG_CREATURE);
	pDestination->SetValue((void *)PreludeParty.GetLeader());

	if(SA->GetType() != ARG_NUMBER)
	{
		Describe("Bad argument to get best, using leader");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.GetBest((int)SA->GetValue()));
	}
	return pDestination; 
}

//get the player with the best of a given statistic
ScriptArg *getaverage(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	

	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(SA->GetType() != ARG_NUMBER)
	{
		Describe("Bad argument to get best, using leader");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.GetAverage((int)SA->GetValue()));
	}

	return pDestination; 
}

ScriptArg *getbestskill(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	

	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue(0);

	int BestValue = 0;
	int CurValue = 0;
	int skillindex = 0;
	switch((int)SA->GetValue())
	{
	default:
	case BEST_WEAPON_SKILL:
		for(skillindex = INDEX_SWORD; skillindex < INDEX_ARMOR; skillindex ++)
		{
			CurValue = PreludeParty.GetBest(skillindex)->GetData(skillindex).Value;
			if(CurValue > BestValue)
			{
				BestValue = CurValue;
			}
		}
		break;
	case BEST_MAGIC_SKILL:
		for(skillindex = INDEX_POWER_OF_FLAME; skillindex < INDEX_PICTURE; skillindex ++)
		{
			CurValue = PreludeParty.GetBest(skillindex)->GetData(skillindex).Value;
			if(CurValue > BestValue)
			{
				BestValue = CurValue;
			}
		}
		break;
	case BEST_NONWEAPON_SKILL:
		for(skillindex = INDEX_STEALTH; skillindex < INDEX_POWER_OF_FLAME; skillindex ++)
		{
			CurValue = PreludeParty.GetBest(skillindex)->GetData(skillindex).Value;
			if(CurValue > BestValue)
			{
				BestValue = CurValue;
			}
		}
		break;
	case BEST_DAMAGE:
		for (int n = 0; n < PreludeParty.GetNumMembers(); n ++)
		{
			CurValue = PreludeParty.GetMember(n)->GetDamage(FALSE, TRUE);
			if(CurValue > BestValue)
			{
				BestValue = CurValue;	
			}
		}
		break;
	}

	pDestination->SetValue((void *)BestValue);

	return pDestination; 
}

ScriptArg *getworst(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();

	

	pDestination->SetType(ARG_CREATURE);
	pDestination->SetValue((void *)PreludeParty.GetLeader());

	if(SA->GetType() != ARG_NUMBER)
	{
		Describe("Bad argument to get worst, using leader");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.GetWorst((int)SA->GetValue()));
	}
	return pDestination; 
}

ScriptArg *placeclearlarge(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SX, *SY, *SArea, *pSAmount;

	SA = ArgList[0].Evaluate();

	SX = ArgList[1].Evaluate();
	SY = ArgList[2].Evaluate();
	SArea = ArgList[3].Evaluate();

	Creature *pCreature, *pNewCreature;

	D3DVECTOR vNewPos;

	int AreaNum;

	int CurX;
	int CurY;
	int StartX;
	int StartY;

	CurX = StartX = (int)SX->GetValue();
	CurY = StartY = (int)SY->GetValue();

	
	//demands a clear path to leader
	D3DVECTOR vLeader, vPlace;
	Path TempPath;

	vLeader = *PreludeParty.GetLeader()->GetPosition();
	vPlace.z = 0.0f;
	vLeader.z = 0.0f;
	
	int LeadX;
	LeadX = vLeader.x;
	int LeadY;
	LeadY = vLeader.y;

	vPlace.x = (float)CurX + 0.5f;
	vPlace.y = (float)CurY + 0.5f;

	
	BOOL FoundClearSpot = FALSE;

	TempPath.SetDepth(1000);
	int OffsetX = 0;
	int OffsetY = 0;
	
	if(GetDistance(&vPlace,&vLeader) > 36.0f)
	{
		FoundClearSpot = TRUE;
	}
	else
	if(!(Valley->IsClear(CurX, CurY) && Valley->IsClear(CurX+1, CurY) && Valley->IsClear(CurX, CurY+1) && Valley->IsClear(CurX+1, CurY+1)) || !TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetLeader()))
	{
		int Start;
		int End;
		int n;


		while(!FoundClearSpot && OffsetX < 24)
		{
			OffsetX++;
			OffsetY++;
			Start = StartX - OffsetX;
			End = StartX + OffsetX;
			CurY = StartY - OffsetY;

			for(n = Start; n <= End; n++)
			{
				CurX = n;
				if((Valley->IsClear(CurX, CurY) && Valley->IsClear(CurX+1, CurY) && Valley->IsClear(CurX, CurY+1) && Valley->IsClear(CurX+1, CurY+1)) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}
			if(FoundClearSpot) break;


			CurY = StartY + OffsetY;
			for(n = Start; n <= End; n++)
			{
				CurX = n;
				if((Valley->IsClear(CurX, CurY) && Valley->IsClear(CurX+1, CurY) && Valley->IsClear(CurX, CurY+1) && Valley->IsClear(CurX+1, CurY+1)) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}

			if(FoundClearSpot) break;
			Start = StartY - (OffsetY - 1);
			End = StartY + (OffsetY + 1);
			CurX = StartX + OffsetX;

			for(n = Start; n <= End; n++)
			{
				CurY = n;
				if((Valley->IsClear(CurX, CurY) && Valley->IsClear(CurX+1, CurY) && Valley->IsClear(CurX, CurY+1) && Valley->IsClear(CurX+1, CurY+1)) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}

			if(FoundClearSpot) break;
			CurX = StartX - OffsetX;

			for(n = Start; n <= End; n++)
			{
				CurY = n;
				if((Valley->IsClear(CurX, CurY) && Valley->IsClear(CurX+1, CurY) && Valley->IsClear(CurX, CurY+1) && Valley->IsClear(CurX+1, CurY+1)) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}
			if(FoundClearSpot) break;
		}
	}
	if(OffsetX >= 24)
	{
		CurX = StartX;
		CurY = StartY;
	}


	vNewPos.x = (float)CurX + 0.5f;
	vNewPos.y = (float)CurY + 0.5f;
	vNewPos.z = Valley->GetTileHeight((int)vNewPos.x, (int)vNewPos.y);

	if(SArea->GetType() != ARG_STRING)
	{
		AreaNum = PreludeWorld->GetCurAreaNum();	
		pSAmount = SArea;
	}
	else
	{
		AreaNum = PreludeWorld->GetAreaNum((char *)SArea->GetValue());
		pSAmount = ArgList[4].Evaluate();
	}

	int Amount;
	Amount = 1;
	if(SArea->GetType() == ARG_NUMBER)
	{
		Amount = (int)SArea->GetValue();
	}

	D3DVECTOR *pPosition;
	
	

	switch(SA->GetType())
	{
	case ARG_CREATURE:
		pCreature = SA->GetCreature();
		//is it unique
		if(pCreature->GetData(INDEX_BATTLEID).Value)
		{
			//if so add a unique copy
			pNewCreature = new Creature(pCreature);
			pCreature = pNewCreature;
			Locator *pLoc;
			pCreature->AddLocator();
			pLoc = pCreature->GetLocator(0);
			pLoc->SetStart(0);
			pLoc->SetEnd(23);
			RECT rBounds;
			rBounds.left = (int)vNewPos.x;
			rBounds.right = (int)vNewPos.x + 1;
			rBounds.top = (int)vNewPos.y;
			rBounds.bottom = (int)vNewPos.y + 1;
			pLoc->SetBounds(&rBounds);
			pLoc->SetArea(AreaNum);
		}
		else
		{
			if(PreludeWorld->GetArea(pCreature->GetAreaIn()))
			{
				PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate(pCreature);
			}
		}
		pPosition = pCreature->GetPosition();
		*pPosition = vNewPos;
		pCreature->SetPosition(&vNewPos);
		pCreature->SetAreaIn(AreaNum);
		pCreature->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pCreature->AddToWorld();
		pCreature->SetAngle(((float)rand() / (float)RAND_MAX) * PI_MUL_2);
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pCreature);
		pCreature->SetLastPlacedTime(PreludeWorld->GetTotalTime());
		break;
	case ARG_THING:
		Thing *pThing;
		pThing = (Thing *)SA->GetValue();
		break;
	case ARG_ITEM:
		Item *pItem;
		pItem = (Item *)SA->GetValue();
		GameItem *pGI;
		pGI = new GameItem();
		pGI->SetItem(pItem);
		pGI->SetQuantity(Amount);
		vNewPos.z += 0.1f;
		pGI->SetPosition(&vNewPos);
		PreludeWorld->GetArea(AreaNum)->AddToUpdate((Object *)pGI);
		pGI->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pGI->SetLocation(LOCATION_WORLD,NULL);
		pGI->SetOwner(PreludeParty.GetLeader());
		break;
	default:
		pCreature = SA->GetCreature();
		if(!pCreature)
		{
			SafeExit("Bad thing being placed!\n");
		}
		pPosition = pCreature->GetPosition();
		*pPosition = vNewPos;
		pCreature->SetPosition(&vNewPos);
		pCreature->SetAreaIn(AreaNum);
		pCreature->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pCreature->AddToWorld();
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pCreature);
		break;

	}
	
	return pDestination; 
}

ScriptArg *placeclear(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SX, *SY, *SArea, *pSAmount;

	SA = ArgList[0].Evaluate();

	SX = ArgList[1].Evaluate();
	SY = ArgList[2].Evaluate();
	SArea = ArgList[3].Evaluate();

	Creature *pCreature, *pNewCreature;
	pCreature = SA->GetCreature();
	if(pCreature && pCreature->IsLarge())
	{
		return placeclearlarge(ArgList, pDestination);
	}
	

	D3DVECTOR vNewPos;

	int AreaNum;

	int CurX;
	int CurY;
	int StartX;
	int StartY;

	CurX = StartX = (int)SX->GetValue();
	CurY = StartY = (int)SY->GetValue();

	
	//demands a clear path to leader
	D3DVECTOR vLeader, vPlace;
	Path TempPath;

	vLeader = *PreludeParty.GetLeader()->GetPosition();
	vPlace.z = 0.0f;
	vLeader.z = 0.0f;
	
	int LeadX;
	LeadX = vLeader.x;
	int LeadY;
	LeadY = vLeader.y;

	vPlace.x = (float)CurX + 0.5f;
	vPlace.y = (float)CurY + 0.5f;
	int OffsetX = 0;
	int OffsetY = 0;
	
	
	BOOL FoundClearSpot = FALSE;

	if(GetDistance(&vPlace,&vLeader) > 36.0f)
	{
		FoundClearSpot = TRUE;
	}
	else
	if(!Valley->IsClear(CurX, CurY) || !TempPath.FindPath(LeadX,LeadY,CurX,CurY, 0.0f, PreludeParty.GetLeader()))
	{
		int Start;
		int End;
		int n;


		while(!FoundClearSpot && OffsetX < 24)
		{
			OffsetX++;
			OffsetY++;
			Start = StartX - OffsetX;
			End = StartX + OffsetX;
			CurY = StartY - OffsetY;

			for(n = Start; n <= End; n++)
			{
				CurX = n;
				if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}
			if(FoundClearSpot) break;


			CurY = StartY + OffsetY;
			for(n = Start; n <= End; n++)
			{
				CurX = n;
				if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}

			if(FoundClearSpot) break;
			Start = StartY - (OffsetY - 1);
			End = StartY + (OffsetY + 1);
			CurX = StartX + OffsetX;

			for(n = Start; n <= End; n++)
			{
				CurY = n;
				if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}

			if(FoundClearSpot) break;
			CurX = StartX - OffsetX;

			for(n = Start; n <= End; n++)
			{
				CurY = n;
				if(Valley->IsClear(CurX, CurY) && TempPath.FindPath(LeadX,LeadY,CurX,CurY,  0.0f, PreludeParty.GetLeader()))
				{
					FoundClearSpot = TRUE;
					break;
				}
			}
			if(FoundClearSpot) break;
		}
	}
	
	if(OffsetX >= 24)
	{
		CurX = StartX;
		CurY = StartY;
	}

	vNewPos.x = (float)CurX + 0.5f;
	vNewPos.y = (float)CurY + 0.5f;
	vNewPos.z = Valley->GetTileHeight((int)vNewPos.x, (int)vNewPos.y);

	if(SArea->GetType() != ARG_STRING)
	{
		AreaNum = PreludeWorld->GetCurAreaNum();	
		pSAmount = SArea;
	}
	else
	{
		AreaNum = PreludeWorld->GetAreaNum((char *)SArea->GetValue());
		pSAmount = ArgList[4].Evaluate();
	}

	int Amount;
	Amount = 1;
	if(SArea->GetType() == ARG_NUMBER)
	{
		Amount = (int)SArea->GetValue();
	}

	D3DVECTOR *pPosition;
	
	

	switch(SA->GetType())
	{
	case ARG_CREATURE:
		pCreature = SA->GetCreature();
		//is it unique
		if(pCreature->GetData(INDEX_BATTLEID).Value)
		{
			//if so add a unique copy
			pNewCreature = new Creature(pCreature);
			pCreature = pNewCreature;
			Locator *pLoc;
			pCreature->AddLocator();
			pLoc = pCreature->GetLocator(0);
			pLoc->SetStart(0);
			pLoc->SetEnd(23);
			RECT rBounds;
			rBounds.left = (int)vNewPos.x;
			rBounds.right = (int)vNewPos.x + 1;
			rBounds.top = (int)vNewPos.y;
			rBounds.bottom = (int)vNewPos.y + 1;
			pLoc->SetBounds(&rBounds);
			pLoc->SetArea(AreaNum);
		}
		else
		{
			if(PreludeWorld->GetArea(pCreature->GetAreaIn()))
			{
				PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate(pCreature);
			}
		}
		pPosition = pCreature->GetPosition();
		*pPosition = vNewPos;
		pCreature->SetPosition(&vNewPos);
		pCreature->SetAreaIn(AreaNum);
		pCreature->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pCreature->SetLastPlacedTime(PreludeWorld->GetTotalTime());
		pCreature->AddToWorld();
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pCreature);
		break;
	case ARG_THING:
		Thing *pThing;
		pThing = (Thing *)SA->GetValue();
		break;
	case ARG_ITEM:
		Item *pItem;
		pItem = (Item *)SA->GetValue();
		GameItem *pGI;
		pGI = new GameItem();
		pGI->SetItem(pItem);
		pGI->SetQuantity(Amount);
		vNewPos.z += 0.1f;
		pGI->SetPosition(&vNewPos);
		PreludeWorld->GetArea(AreaNum)->AddToUpdate((Object *)pGI);
		pGI->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pGI->SetLocation(LOCATION_WORLD,NULL);
		pGI->SetOwner(PreludeParty.GetLeader());
		break;
	default:
		pCreature = SA->GetCreature();
		if(!pCreature)
		{
			SafeExit("Bad thing being placed!\n");
		}
		pPosition = pCreature->GetPosition();
		*pPosition = vNewPos;
		pCreature->SetPosition(&vNewPos);
		pCreature->SetAreaIn(AreaNum);
		pCreature->SetRegionIn(PreludeWorld->GetArea(AreaNum)->GetRegion(&vNewPos));
		pCreature->AddToWorld();
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pCreature);
		break;

	}
	
	return pDestination; 
}




ScriptArg *startcombat(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	PreludeWorld->GetCombat()->Start();
	return pDestination; 
}

ScriptArg *endcombat(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	PreludeWorld->GetCombat()->End();
	return pDestination; 
}

//fade palette up/down
ScriptArg *fade(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	D3DMATERIAL7 matBlack;
	matBlack.specular.r = 0.0f;
	matBlack.specular.g = 0.0f;
	matBlack.specular.b = 0.0f;
	matBlack.specular.a = 1.0f;

	matBlack.emissive.r = 0.0f;
	matBlack.emissive.g = 0.0f;
	matBlack.emissive.b = 0.0f;
	matBlack.emissive.a = 0.0f;

	matBlack.power = 1.0f;
	matBlack.ambient.r = matBlack.diffuse.r = 0.0f;
	matBlack.ambient.g = matBlack.diffuse.g = 0.0f;
	matBlack.ambient.b = matBlack.diffuse.b = 0.0f;
	matBlack.ambient.a = matBlack.diffuse.a = 0.5f;
	
	float CurAlpha;
	float AlphaMod;
	DWORD LastFrame;
	DWORD NextFrame;
	
	ScriptArg *SA, *SB;
	SA = ArgList[0].Evaluate();
	SB = ArgList[1].Evaluate();
	if(SA->GetValue())
	{
		//fadeup
		CurAlpha = 1.0f;
		AlphaMod = -0.05f;
	}
	else
	{
		CurAlpha = 0.0f;
		AlphaMod = 0.05f;
	}
	int FadeLength = 0;

	if(SB->GetType() != ARG_TERMINATOR)
	{
		FadeLength = 1;
	}
	else
	{
		FadeLength = 20;
	}


	D3DTLVERTEX Verts[4];
	Verts[0].sx = 0;
	Verts[0].sy = 0;
	Verts[0].sz = 0;
	
	Verts[1].sx = Engine->Graphics()->GetWidth();
	Verts[1].sy = 0;
	Verts[1].sz = 0;

	Verts[2].sx = 0;
	Verts[2].sy = (Engine->Graphics()->GetHeight() - 100);
	Verts[2].sz = 0;

	Verts[3].sx = Engine->Graphics()->GetWidth();
	Verts[3].sy = (Engine->Graphics()->GetHeight() - 100);
	Verts[3].sz = 0;
	Verts[0].rhw = Verts[1].rhw = Verts[2].rhw = Verts[3].rhw = 1.0f;
	
	for(int n = 0; n < FadeLength; n++)
	{
		matBlack.ambient.a = matBlack.diffuse.a = matBlack.emissive.a = CurAlpha; 
		Verts[0].color = Verts[1].color = Verts[2].color = Verts[3].color = D3DRGBA(0.0f,0.0f,0.0f,CurAlpha);
		LastFrame = timeGetTime();
		NextFrame = LastFrame + ((ZSMainWindow *)ZSWindow::GetMain())->GetFrameRate();
	//	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
		ZSWindow::GetMain()->Draw();
	//	Engine->Graphics()->GetD3D()->SetMaterial(&matBlack);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING,FALSE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
		
		Engine->Graphics()->GetD3D()->BeginScene();
		Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, Verts, 4, 0);
		Engine->Graphics()->GetD3D()->EndScene();
		Engine->Graphics()->Flip();
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING,TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
		CurAlpha += AlphaMod;

		while(timeGetTime() < NextFrame) 
		{
			//do AI updates and what-not here
		}
	}

	return pDestination; 
}

//goscript
ScriptArg *goscript(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination; }

//gettarget function
ScriptArg *gettarget(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSAType, *pSAx, *pSAy, *pSARange, *pSAMessage;

	pSAType = ArgList[0].Evaluate();
	pSAx = ArgList[1].Evaluate();
	pSAy = ArgList[2].Evaluate();
	pSARange = ArgList[3].Evaluate();
	pSAMessage = ArgList[4].Evaluate();

	BOOL TargetFound;
	void *pTarget;
	TARGET_T TType;

	TType = (TARGET_T)(int)pSAType->GetValue();

	D3DVECTOR vSource;

	vSource.x = (float)(int)pSAx->GetValue();
	vSource.y = (float)(int)pSAy->GetValue();
	vSource.z = Valley->GetTileHeight(vSource.x, vSource.y);

	float fRange;
	fRange = (float)(int)pSARange->GetValue();

	TargetFound = GetTarget(&pTarget, TType, vSource, fRange, (char *)pSAMessage->GetValue());

	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)TargetFound);

	if(TargetFound)
	{
		switch(TType)
		{
		case TARGET_LOCATION:
			Push((int) ((D3DVECTOR *)pTarget)->x);
			Push((int) ((D3DVECTOR *)pTarget)->y);
			break;
	
		case TARGET_CREATURE:
			Push((Creature *)pTarget);
			break;

		case TARGET_ITEM:
			Push((Item *)pTarget);
			break;

		case TARGET_DIRECTION:
			Push((int)pTarget);
			break;
		}
	}

	return pDestination; 
}

ScriptArg *tx(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *ty(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *tz(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *tID(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }

//functions for spells
ScriptArg *addmod(ScriptArg *ArgList, ScriptArg *pDestination)
{ 
	Modifier *pMod;
	pMod = new Modifier;
	ScriptArg *pTarget, *pStat, *pAmount, *pDuration;
	pTarget = ArgList[0].Evaluate();
	pStat = ArgList[1].Evaluate();
	pAmount = ArgList[2].Evaluate();
	pDuration = ArgList[3].Evaluate();

	pMod->SetTarget(pTarget->GetCreature());
	pMod->SetStat((int)pStat->GetValue());
	pMod->SetAmount((int)pAmount->GetValue());
	pMod->SetDuration((int)pDuration->GetValue());
	pMod->SetStart(PreludeWorld->GetTotalTime());
	pMod->Apply();

	PreludeWorld->AddMainObject((Object *)pMod);
	
	return pDestination; 
} //displays a characters spellbook

ScriptArg *addspell(ScriptArg *ArgList, ScriptArg *pDestination) 
{	
	ScriptArg *pSAWho, *pSAWhat;

	pSAWho = ArgList[0].Evaluate();
	pSAWhat = ArgList[1].Evaluate();
	
	if(pSAWho->GetType() == ARG_PARTY)
	{
		for(int n = 0; n < PreludeParty.GetNumMembers(); n ++)
		{
			PreludeParty.GetMember(n)->AddSpell((int)pSAWhat->GetValue());
		}
	}
	else
	{
		Creature *pCreature;
		pCreature = pSAWho->GetCreature();
		assert(pCreature);
		
		pCreature->AddSpell((int)pSAWhat->GetValue());
	}

	return pDestination; 
} //adds a spell to a characters spellbook


ScriptArg *cast(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSASpellNum;
	ScriptArg *pSASpellLevel;
	ScriptArg *pSATarget;
	ScriptArg *pSACaster;

	pSASpellNum = ArgList[0].Evaluate();
	pSASpellLevel = ArgList[1].Evaluate();
	pSACaster = ArgList[2].Evaluate();
	pSATarget = ArgList[3].Evaluate();
	
	PreludeSpells.Cast((int)pSASpellNum->GetValue(),(int)pSASpellLevel->GetValue(),
						(Object *)pSACaster->GetCreature(), (Object *)pSATarget->GetCreature());
	
	return pDestination; 
}		//casts a spell

//functions for adding events to the various event queues
ScriptArg *addeventtimed(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA, *psStart, *psEnd, *psBegin;
	SA = ArgList[0].Evaluate();

	psStart = ArgList[1].Evaluate();
	psBegin = ArgList[2].Evaluate();
	psEnd = ArgList[3].Evaluate();
	
	//Event *pEvent;
	//pEvent = new Event();

	//pEvent->SetNum((int)SA->GetValue());
	//pEvent->SetStart((unsigned long)psStart->GetValue());
	//pEvent->SetEnd((unsigned long)psStart->GetValue());
		
	PreludeEvents.AddTimed((int)SA->GetValue(), (unsigned long)psBegin->GetValue(), (unsigned long)psEnd->GetValue(), (unsigned long)psStart->GetValue(), 10 );
	
	return pDestination; 
}

ScriptArg *addeventstartcombat(ScriptArg *ArgList, ScriptArg *pDestination)
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	PreludeEvents.AddStartCombat((int)SA->GetValue());
	
	return pDestination; 
}

ScriptArg *addeventendcombat(ScriptArg *ArgList, ScriptArg *pDestination)
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	PreludeEvents.AddEndCombat((int)SA->GetValue());
	
	return pDestination; 
}

ScriptArg *addeventrest(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	PreludeEvents.AddRest((int)SA->GetValue());
	
	return pDestination; 
}

ScriptArg *addevent(ScriptArg *ArgList, ScriptArg *pDestination)
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
//	PreludeEvents.AddTimed((int)SA->GetValue());
	
	return pDestination; 
}

ScriptArg *addeventrandom(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	PreludeEvents.AddRest((int)SA->GetValue());
	
	return pDestination; 
}

ScriptArg *event(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	int EventNum;
	EventNum = (int)SA->GetValue();

	PreludeEvents.RunEvent(EventNum);

	return pDestination;
}

ScriptArg *scriptend(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	if(ScriptContextWindow)
		((ZSTalkWin *)ScriptContextWindow)->SetState(WINDOW_STATE_DONE);
	
	

	
	pDestination->SetType(ARG_TERMINATOR);

	return pDestination; 
}

ScriptArg *say_win(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_STRING);

	if(ScriptContextWindow)
	{
		((ZSTalkWin *)ScriptContextWindow)->Say((char *)SA->GetValue());
	}
	else
	{
		Describe((char *)SA->GetValue());
	}
	
	

	return pDestination;
}

ScriptArg *saydesc(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	assert(SA->GetType() == ARG_STRING);

	if(ScriptContextWindow)
	{
		((ZSTalkWin *)ScriptContextWindow)->SayDesc((char *)SA->GetValue());
	}
	else
	{
		Describe((char *)SA->GetValue());
	}
	
	

	return pDestination;
}

ScriptArg *sayclear(ScriptArg *ArgList, ScriptArg *pDestination)
{
	if(ScriptContextWindow)
	{
		((ZSTalkWin *)ScriptContextWindow)->SayClear();
	}
	
	

	return pDestination;
}


ScriptArg *scriptdescribe(ScriptArg *ArgList, ScriptArg *pDestination) 
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();
	Describe((char *)SA->GetValue());
	return pDestination; 
}

ScriptArg *setpic(ScriptArg *ArgList, ScriptArg *pDestination)
{ 
	return pDestination;; 
}

ScriptArg *addnpc(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(SA->GetType() != ARG_CREATURE)
	{
		Describe("Bad Argument to addnpc");
	}
	else
	{
		Creature *pCreature = (Creature *)SA->GetCreature();
		while(pCreature->GetNumLocators())
			pCreature->RemoveLocator(0);

		pDestination->SetValue((void *)PreludeParty.AddMember(pCreature));
		if(PreludeWorld->GetArea(pCreature->GetAreaIn()))
			PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate(pCreature);
		pCreature->SetAreaIn(PreludeWorld->GetCurAreaNum());
		pCreature->AddToWorld();

		//check the NPC for drachs
		Object *pOb;
		GameItem *pGI;
		pOb = pCreature->GetContents();

		while(pOb)
		{
			pGI = (GameItem *)pOb;
			if(pGI->GetData(INDEX_ID).Value == 100)
			{
				Flag *pFlag;
				pFlag = PreludeFlags.Get("PARTYDRACHS");
				pFlag->Value = (void *) ((int) pFlag->Value + pGI->GetQuantity());
				char blarg[128];
				sprintf(blarg,"%s gives the party %i drachs.",pCreature->GetData(INDEX_NAME).String, pGI->GetQuantity());
				Describe(blarg);
				pCreature->RemoveItem(pGI);
				pOb = pCreature->GetContents();
			}
			else
			{
				pOb = pOb->GetNext();
			}
		}
	}
	return pDestination; 
}

ScriptArg *removenpc(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(SA->GetType() != ARG_CREATURE)
	{
		Describe("Bad Argument to removenpc");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.RemoveMember((Creature *)SA->GetValue()));
		if(((Creature *)SA->GetValue())->GetPortrait())
			ZSWindow::GetMain()->RemoveChild((ZSWindow *)((Creature *)SA->GetValue())->GetPortrait());
	}
	return pDestination; 
}

ScriptArg *inparty(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(!SA->GetCreature())
	{
		Describe("Bad Argument to inparty");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.IsMember(SA->GetCreature()));
	}
	return pDestination; 
}

ScriptArg *partysize(ScriptArg *ArgList, ScriptArg *pDestination)
{
	

	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)PreludeParty.GetNumMembers());

	return pDestination;
}

ScriptArg *isleader(ScriptArg *ArgList, ScriptArg *pDestination)
{
ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(SA->GetType() != ARG_CREATURE)
	{
		Describe("Bad Argument to isleader");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.IsLeader((Creature *)SA->GetValue()));
	}
	return pDestination; 
}

ScriptArg *setleader(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg  *SA;
	SA = ArgList[0].Evaluate();
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(!SA->GetCreature())
	{
		Describe("Bad Argument to setleader");
	}
	else
	{
		pDestination->SetValue((void *)PreludeParty.SetLeader((Creature *)SA->GetValue()));
	}
	return pDestination; 
}

ScriptArg *isnpc(ScriptArg *ArgList, ScriptArg *pDestination)
{
	//blarg
	ScriptArg  *SA;
	SA = ArgList[0].Evaluate();
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)0);

	if(!SA->GetCreature())
	{
		Describe("Bad Argument to isnpc");
	}
	else
	{
		Creature *pCreature;
		pCreature = SA->GetCreature();
	
		pDestination->SetValue((void *)pCreature->WasCreated());
	}
	return pDestination; 
}


ScriptArg *damageblock(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *GetBlockInfo(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *DisplayAllBlockInfo(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *NotNum(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *generalcombat(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *cheatcombat(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	int n;
	n = 0;
	for(n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		PreludeParty.GetMember(n)->SetData(INDEX_HITPOINTS, PreludeParty.GetMember(n)->GetData(INDEX_MAXHITPOINTS).Value);
	}

	Object *pOb;
	Creature *pCreature;
	
	pOb = PreludeWorld->GetCombat()->GetCombatants();

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_CREATURE)
		{
			pCreature = (Creature *)pOb;
			if(!PreludeParty.IsMember(pCreature))
			{
				if(pCreature->GetData(INDEX_BATTLESIDE).Value)
				{
					pCreature->SetData(INDEX_HITPOINTS,1);
					pCreature->SetData(INDEX_ACTIONPOINTS,1);
					pCreature->SetData(INDEX_MAXACTIONPOINTS,1);
				}
			}
		}
		pOb = pOb->GetNextUpdate();
	}

	return pDestination; 
}



ScriptArg *addicon(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *removeicon(ScriptArg *ArgList, ScriptArg *pDestination) { return pDestination;; }
ScriptArg *los(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 

	return pDestination; 
}


ScriptArg *missile(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}


ScriptArg *countenemies(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	RECT rBounds;
	RECT rUpdateBounds;

	rBounds.left = (int)ArgList[0].Evaluate()->GetValue();
	rBounds.top = (int)ArgList[1].Evaluate()->GetValue();
	rBounds.right = (int)ArgList[2].Evaluate()->GetValue();
	rBounds.bottom = (int)ArgList[3].Evaluate()->GetValue();

	Area *pArea;
	if(ArgList[4].GetType() == ARG_TERMINATOR)
	{
		pArea = Valley;
	}
	else
	{
		char *pAreaName;
		pAreaName = (char *)ArgList[4].Evaluate()->GetValue();
		pArea = PreludeWorld->GetArea(PreludeWorld->GetAreaNum(pAreaName));
	}
	rUpdateBounds.left = rBounds.left / UPDATE_SEGMENT_WIDTH;
	rUpdateBounds.right = rBounds.right / UPDATE_SEGMENT_WIDTH;
	rUpdateBounds.top = rBounds.top / UPDATE_SEGMENT_HEIGHT;
	rUpdateBounds.bottom = rBounds.bottom / UPDATE_SEGMENT_HEIGHT;

	Object *pOb;
	
	int xn;
	int yn;
	
	int NumEnemies = 0;
	
	for(yn = rUpdateBounds.top; yn <= rUpdateBounds.bottom; yn ++)
	for(xn = rUpdateBounds.left; xn <= rUpdateBounds.right; xn ++)
	{
		pOb = pArea->GetUpdateSegment(xn,yn);
		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_CREATURE)
			{
				if(pOb->GetPosition()->x >= rBounds.left &&
				  pOb->GetPosition()->x <= rBounds.right &&
				  pOb->GetPosition()->y >= rBounds.top &&
				  pOb->GetPosition()->y <= rBounds.bottom &&
				  !PreludeParty.IsMember((Creature *)pOb) &&
				  ((Creature *)pOb)->GetData(INDEX_BATTLESIDE).Value)
				{
					NumEnemies++;
				}
			}
			pOb = pOb->GetNextUpdate();
		}
	}

	if(pArea == Valley)
	{
		pOb = PreludeWorld->GetCombat()->GetCombatants();
		while(pOb)
		{

			if(pOb->GetObjectType() == OBJECT_CREATURE &&
				  pOb->GetPosition()->x >= rBounds.left &&
				  pOb->GetPosition()->x <= rBounds.right &&
				  pOb->GetPosition()->y >= rBounds.top &&
				  pOb->GetPosition()->y <= rBounds.bottom &&
				  !PreludeParty.IsMember((Creature *)pOb) &&
				  ((Creature *)pOb)->GetData(INDEX_BATTLESIDE).Value)
			{
				NumEnemies++;
			}
			pOb = pOb->GetNextUpdate();
		}
	}
	
	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)NumEnemies);

	return pDestination; 
}

ScriptArg *removecreatures(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	RECT rBounds;
	RECT rUpdateBounds;

	ScriptArg *pSA;
	Creature *pToRemove;


	pSA = ArgList[0].Evaluate();

	pToRemove = pSA->GetCreature();

	int CreatureID = 0;
	CreatureID = (int)pToRemove->GetData(INDEX_ID).Value;

	rBounds.left = (int)ArgList[1].Evaluate()->GetValue();
	rBounds.top = (int)ArgList[2].Evaluate()->GetValue();
	rBounds.right = (int)ArgList[3].Evaluate()->GetValue();
	rBounds.bottom = (int)ArgList[4].Evaluate()->GetValue();

	Area *pArea;
	if(ArgList[5].GetType() == ARG_TERMINATOR)
	{
		pArea = Valley;
	}
	else
	{
		char *pAreaName;
		pAreaName = (char *)ArgList[5].Evaluate()->GetValue();
		pArea = PreludeWorld->GetArea(PreludeWorld->GetAreaNum(pAreaName));
	}
	rUpdateBounds.left = rBounds.left / UPDATE_SEGMENT_WIDTH;
	rUpdateBounds.right = rBounds.right / UPDATE_SEGMENT_WIDTH;
	rUpdateBounds.top = rBounds.top / UPDATE_SEGMENT_HEIGHT;
	rUpdateBounds.bottom = rBounds.bottom / UPDATE_SEGMENT_HEIGHT;

	Object *pOb;
	Object *pRemove;
	
	int xn;
	int yn;
	
	int NumEnemies = 0;
	
	for(yn = rUpdateBounds.top; yn <= rUpdateBounds.bottom; yn ++)
	for(xn = rUpdateBounds.left; xn <= rUpdateBounds.right; xn ++)
	{
		pOb = pArea->GetUpdateSegment(xn,yn);
		while(pOb)
		{
			pRemove = pOb;
			pOb = pOb->GetNextUpdate();
			if(pRemove->GetObjectType() == OBJECT_CREATURE)
			{
				if(pRemove->GetPosition()->x >= rBounds.left &&
				  pRemove->GetPosition()->x <= rBounds.right &&
				  pRemove->GetPosition()->y >= rBounds.top &&
				  pRemove->GetPosition()->y <= rBounds.bottom &&
				  !PreludeParty.IsMember((Creature *)pRemove) &&
				  ((Creature *)pRemove)->GetData(INDEX_ID).Value == CreatureID)
				{
					DEBUG_INFO("removeing character and killing locators\n");
					Area *pArea = NULL;
				
					pToRemove = (Creature *)pRemove;
					if(PreludeWorld->InCombat())
					{
						PreludeWorld->GetCombat()->RemoveFromCombat((Object *)pToRemove);
					}

					pArea = PreludeWorld->GetArea(pToRemove->GetAreaIn());
					if(pArea) 
						pArea->RemoveFromUpdate(pToRemove);
					while(pToRemove->GetNumLocators())
					{
						pToRemove->RemoveLocator(0);
					}

				}
			}
			
		}

	}

	pOb = PreludeWorld->GetCombat()->GetCombatants();
	while(pOb)
	{
		pRemove = pOb;
		pOb = pOb->GetNextUpdate();

		if(pRemove->GetObjectType() == OBJECT_CREATURE)
		{
			if(pRemove->GetPosition()->x >= rBounds.left &&
			  pRemove->GetPosition()->x <= rBounds.right &&
			  pRemove->GetPosition()->y >= rBounds.top &&
			  pRemove->GetPosition()->y <= rBounds.bottom &&
			  !PreludeParty.IsMember((Creature *)pRemove) &&
			  ((Creature *)pRemove)->GetData(INDEX_ID).Value == CreatureID)
			{
				DEBUG_INFO("removeing character and killing locators\n");
				Area *pArea = NULL;
			
				pToRemove = (Creature *)pRemove;
				if(PreludeWorld->InCombat())
				{
					PreludeWorld->GetCombat()->RemoveFromCombat((Object *)pToRemove);
				}

				pArea = PreludeWorld->GetArea(pToRemove->GetAreaIn());
				if(pArea) 
					pArea->RemoveFromUpdate(pToRemove);
				while(pToRemove->GetNumLocators())
				{
					pToRemove->RemoveLocator(0);
				}

			}
		}
	}

	
	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)NumEnemies);

	return pDestination; 
}

ScriptArg *getat(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSAx, *pSAy;
	
	pSAx = ArgList[0].Evaluate();
	pSAy = ArgList[1].Evaluate();

	Object *pObject;
	pObject = Valley->FindObject((int)pSAx->GetValue(),(int)pSAy->GetValue(),OBJECT_CREATURE);

	if(pObject)
	{
		pDestination->SetType(ARG_CREATURE);
		pDestination->SetValue((void *)pObject);
	}
	else
	{
		pDestination->SetType(ARG_NUMBER);
		pDestination->SetValue(NULL);
	}

	return pDestination;
}

ScriptArg *getnonunique(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSABaseCreature, *pSANonID;

	pSABaseCreature = ArgList[0].Evaluate();

	pSANonID = ArgList[1].Evaluate();

	

	Creature *pCreature, *pBaseCreature;
	
	pBaseCreature = (Creature *)pSABaseCreature->GetValue();

	int ID;

	ID = pBaseCreature->GetData(INDEX_ID).Value;

	int NonID;

	NonID = (int)pSANonID->GetValue();

	pCreature = (Creature *)Thing::Find(Creature::GetFirst(),ID,NonID);

	pDestination->SetType(ARG_CREATURE);
	pDestination->SetValue((void *)pCreature);

	return pDestination;
}

ScriptArg *sdistance(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSX1, *pSX2, *pSY1, *pSY2;

	pSX1 = ArgList[0].Evaluate();
	pSY1 = ArgList[1].Evaluate();
	pSX2 = ArgList[2].Evaluate();
	pSY2 = ArgList[3].Evaluate();

	

	

	int Distance;

	D3DVECTOR vA, vB;
	vA.x = (int)pSX1->GetValue();
	vA.y = (int)pSY1->GetValue();

	vB.x = (int)pSX2->GetValue();
	vB.y = (int)pSY2->GetValue();

	Distance = (int)GetDistance(&vA,&vB);

	pDestination->SetValue((void *)Distance);
	pDestination->SetType(ARG_NUMBER);
	
	return pDestination; 
}

ScriptArg *mandistance(ScriptArg *ArgList, ScriptArg *pDestination)
{
	return pDestination; 
}

ScriptArg *journal(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	int Num;
	Num = (int)SA->GetValue();

	if(PreludeParty.GetJournal()->AddEntry(Num))
	{
		Describe("Journal Updated");
	}
	else
	{

	}

	return pDestination;; 
}

ScriptArg *removejournal(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	int Num;
	Num = (int)SA->GetValue();

	PreludeParty.GetJournal()->RemoveEntry(Num);
	return pDestination;; 
}

ScriptArg *barter(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA, *SB;

	SA = ArgList[0].Evaluate();
	SB = ArgList[1].Evaluate();

	Creature *pMerchant, *pTarget;

	pMerchant = SA->GetCreature();
	pTarget = SB->GetCreature();

	if(!pMerchant || !pTarget)
	{
		SafeExit("Bad Argument to Barter\n");
	}

	BarterWin *pBarter;
	pBarter = new BarterWin(321123,100,100,600,400,pMerchant, pTarget);
	pBarter->Show();
	ZSWindow::GetMain()->AddTopChild(pBarter);
	if(ScriptContextWindow)
	{
		ScriptContextWindow->Hide();
	}

	if(!PreludeWorld->GetBarterHelp())
	{
		PreludeWorld->SetBarterHelp(TRUE);
		ShowHelp("Barter Screen");
	}

	pBarter->SetFocus(pBarter);
	pBarter->GoModal();
	pBarter->ReleaseFocus();

	ZSWindow::GetMain()->RemoveChild(pBarter);
	if(ScriptContextWindow)
	{
		ScriptContextWindow->Show();
	}

	return pDestination; 
}
ScriptArg *use(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *pSA;
	pSA = ArgList[0].Evaluate();
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)(pSA->GetCreature()->GetUniqueID()));
		
	return pDestination;
}



ScriptArg *hidetalk(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	if(ScriptContextWindow)
	{
		ScriptContextWindow->Hide();
	}
	return pDestination; 
}

ScriptArg *showtalk(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	if(ScriptContextWindow)
	{
		ScriptContextWindow->Show();
		ScriptContextWindow->SetState(WINDOW_STATE_NORMAL);
	}
	return pDestination; 
}



ScriptArg *opentalk(ScriptArg *ArgList, ScriptArg *pDestination)
{
	
	
	
	ZSTalkWin *pWin;
	pWin = new ZSTalkWin(NextTalkWin++, 125, 125, 550,350, ScriptContextBlock);

	pWin->Show();
	
	ZSWindow::GetMain()->AddChild(pWin);
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)1);
	
	return pDestination;
}

ScriptArg *setportrait(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;

	
	pDestination->SetValue((void *)1);

	SA = ArgList[0].Evaluate();

	if(SA->GetType() != ARG_STRING)
	{
		((ZSTalkWin *)ScriptContextWindow)->SetPortrait(NULL);
	}
	else
	{
		((ZSTalkWin *)ScriptContextWindow)->SetPortrait((char *)SA->GetValue());
	}
	return pDestination;
}

ScriptArg *closetalk(ScriptArg *ArgList, ScriptArg *pDestination)
{
	if(ScriptContextWindow)
	{
		ScriptContextWindow->Hide();
		ScriptContextWindow->SetState(WINDOW_STATE_DONE);
	}
		
	

	
	pDestination->SetValue((void *)1);

	return pDestination;
}

ScriptArg *haskey(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg  *SA;

	SA = ArgList[0].Evaluate();

	int Num;
	Num = (int)SA->GetValue();

	
	
	pDestination->SetValue((void *)PreludeParty.HasKey(Num));

	return pDestination;
}

ScriptArg *getx(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();

	Creature *pCreature;
	pCreature = SA->GetCreature();

	D3DVECTOR *pPosition;
	pPosition = pCreature->GetPosition();

	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)((int)pPosition->x));

	return pDestination;
}

ScriptArg *gety(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();

	Creature *pCreature;
	pCreature = SA->GetCreature();

	D3DVECTOR *pPosition;
	pPosition = pCreature->GetPosition();

	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)((int)pPosition->y));

	return pDestination;
}

ScriptArg *getz(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();

	Creature *pCreature;
	pCreature = SA->GetCreature();

	D3DVECTOR *pPosition;
	pPosition = pCreature->GetPosition();

	
	
	pDestination->SetType(ARG_NUMBER);
	pDestination->SetValue((void *)((int)pPosition->z));

	return pDestination;
}

ScriptArg *move(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SX, *SY, *SZ;

	SA = ArgList[0].Evaluate();

	SX = ArgList[1].Evaluate();
	SY = ArgList[2].Evaluate();
	SZ = ArgList[3].Evaluate();

	D3DVECTOR vNewPos;

	vNewPos.x = (float)((int)SX->GetValue()) + 0.5;
	vNewPos.y = (float)((int)SY->GetValue()) + 0.5;
	if(SZ->GetType() == ARG_TERMINATOR)
	{
		vNewPos.z = Valley->GetTileHeight(vNewPos.x,vNewPos.y);
	}
	else
	{
		vNewPos.z = (float)((int)SZ->GetValue()) + 0.5;
	}
	Creature *pCreature;
	pCreature = SA->GetCreature();
	if(pCreature)
	{
		pCreature->InsertAction(ACTION_MOVETO, SX->GetValue(), SY->GetValue());
	}
	else
	{
		SafeExit("Bad thing being moved!\n");
	}
	
	

	return pDestination; 
}

ScriptArg *face(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SDir;

	SA = ArgList[0].Evaluate();
	SDir = ArgList[1].Evaluate();

	Creature *pCreature;
	pCreature = SA->GetCreature();

	if(pCreature)
	{
		pCreature->InsertAction(ACTION_ROTATE, NULL, SDir->GetValue());
	}
	else
	{
		DEBUG_INFO("Attempting to change facing of non-creature\n");
	}
	return pDestination;
}


ScriptArg *moveto(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA, *SX, *SY, *SZ;

	SA = ArgList[0].Evaluate();

	SX = ArgList[1].Evaluate();
	SY = ArgList[2].Evaluate();
	SZ = ArgList[3].Evaluate();

	Creature *pCreature;
	D3DVECTOR vNewPosition;
	if(SA->GetType() == ARG_PARTY)
	{
		int NumMembers;
		NumMembers = PreludeParty.GetNumMembers();
		for(int n = 0; n < NumMembers; n++)
		{
			pCreature = PreludeParty.GetMember(n);
			vNewPosition.x = (float)((int)SX->GetValue()) + 0.5f + (float)n;
			vNewPosition.y = (float)((int)SY->GetValue()) + 0.5f + (float)n;

			if(!Valley->GetChunk((int)SX->GetValue()/CHUNK_TILE_WIDTH,(int)SY->GetValue()/CHUNK_TILE_HEIGHT))
			{
				Valley->LoadChunk((int)SX->GetValue()/CHUNK_TILE_WIDTH,(int)SY->GetValue()/CHUNK_TILE_HEIGHT);
				Engine->Graphics()->GetD3D()->BeginScene();
				Valley->GetChunk((int)SX->GetValue()/CHUNK_TILE_WIDTH,(int)SY->GetValue()/CHUNK_TILE_HEIGHT)->CreateTexture(Valley->GetBaseTexture());
				Engine->Graphics()->GetD3D()->EndScene();
			}
			vNewPosition.z = Valley->GetTileHeight(vNewPosition.x,vNewPosition.y);

			
			Valley->RemoveFromUpdate(pCreature);
			pCreature->SetPosition(&vNewPosition);
			Valley->AddToUpdate(pCreature);
			pCreature->SetRegionIn(Valley->GetRegion(&vNewPosition));
			pCreature->ClearActions();
			PreludeParty.Occupy();
		}
		PreludeWorld->LookAt(PreludeParty.GetLeader());
	}
	else
	{
		pCreature = SA->GetCreature();
		vNewPosition.x = (float)((int)SX->GetValue()) + 0.5f;
		vNewPosition.y = (float)((int)SY->GetValue()) + 0.5f;
		vNewPosition.z = Valley->GetTileHeight(vNewPosition.x,vNewPosition.y);

		PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate(pCreature);
		pCreature->SetPosition(&vNewPosition);
		pCreature->SetRegionIn(Valley->GetRegion(&vNewPosition));
		Valley->AddToUpdate(pCreature);
		
		pCreature->ClearActions();
		if(PreludeParty.IsMember(pCreature))
		{
			PreludeParty.Occupy();
		}
		if(PreludeParty.IsLeader(pCreature))
		{
			PreludeWorld->LookAt(pCreature);
		}
	}


	return pDestination;
}

ScriptArg *approach(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SWho, *STarget;

	SWho = ArgList[0].Evaluate();
	STarget = ArgList[1].Evaluate();

	SWho->GetCreature()->InsertAction(ACTION_APPROACH,STarget->GetValue(),NULL);

	return pDestination; 
}

ScriptArg *enemiesaround(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *addlocation(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSAX, *pSAY, *pSATag, *pSATele, *pSAMapNum, *pSAMapX, *pSAMapY;

	pSAMapX = NULL;
	pSAMapY = NULL;

	pSATag = ArgList[0].Evaluate();
	pSAX = ArgList[1].Evaluate();
	pSAY = ArgList[2].Evaluate();
	pSATele = ArgList[3].Evaluate();

	pSAMapX = pSAMapY = NULL;
	if(pSATele->GetType() != ARG_TERMINATOR)
	{
		pSAMapNum = ArgList[4].Evaluate();
		if(pSAMapNum->GetType() != ARG_TERMINATOR)
		{
			pSAMapX = ArgList[5].Evaluate();
			pSAMapY = ArgList[6].Evaluate();
		}

	}

	BOOL WasAdded = FALSE;

	if(pSAMapX)
	{
		WasAdded = PreludeParty.AddLocation((char *)pSATag->GetValue(), (int)pSAX->GetValue(), (int)pSAY->GetValue(), (BOOL)pSATele->GetValue(), (int)pSAMapNum->GetValue(),(int)pSAMapX->GetValue(),(int)pSAMapY->GetValue());
	}
	else
	{
		WasAdded = PreludeParty.AddLocation((char *)pSATag->GetValue(), (int)pSAX->GetValue(), (int)pSAY->GetValue(), (BOOL)pSATele->GetValue(),0,0,0);
	}
	
	if(WasAdded) 
		Describe("Location added to map.");

	return pDestination;
}

ScriptArg *getpower(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSA;
	
	pSA = ArgList[0].Evaluate();

	Creature *pCreature;
	pCreature = (Creature *)pSA->GetCreature();

	assert(pCreature);

	pDestination->SetValue((void *)(int)(pCreature->GetPowerLevel()));
	pDestination->SetType(ARG_NUMBER);

	return pDestination;
	
}


ScriptArg *camerazoom(ScriptArg *ArgList, ScriptArg *pDestination)
{
	Engine->Graphics()->Zoom(-50.0f);
	
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();

	int ZoomLevel;
	ZoomLevel = (int)SA->GetValue();

	Engine->Graphics()->Zoom((float)ZoomLevel);

	return pDestination;
}

ScriptArg *cameraheight(ScriptArg *ArgList, ScriptArg *pDestination)
{
	Engine->Graphics()->Zoom(-50.0f);
	
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();

	int ZoomLevel;
	ZoomLevel = (int)SA->GetValue();

	Engine->Graphics()->Zoom((float)ZoomLevel);

	return pDestination;
}

ScriptArg *cameraangle(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *SA;
	SA = ArgList[0].Evaluate();

	float degreeangle;
	degreeangle = (float)(int)SA->GetValue();
	degreeangle += 180.0f;

	float radangle;
	radangle = DegToRad(degreeangle);
	
	PreludeWorld->SetCameraAngle(radangle);

	return pDestination;
}


//cameracontrol functions
ScriptArg *cameramove(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *cameralookat(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	ScriptArg *SA;

	SA = ArgList[0].Evaluate();
	PreludeWorld->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));

	if(ArgList[1].GetType() != ARG_TERMINATOR)
	{
		int CameraAngle;
		CameraAngle = (int)ArgList[1].Evaluate()->GetValue();
		float Angle;
		Angle = (float)CameraAngle / 360.0f;
		Angle = DegToRad(Angle);
		Angle = Angle - PreludeWorld->GetCameraAngle();
		PreludeWorld->RotateCamera(Angle);
	}

	PreludeWorld->LookAt(SA->GetCreature());

	return pDestination; 
}


ScriptArg *cameramode(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *activate(ScriptArg *ArgList, ScriptArg *pDestination) 
{ 
	return pDestination; 
}

ScriptArg *showcursor(ScriptArg *ArgList, ScriptArg *pDestination)
{
	return pDestination; 
}

//open/close the door at x,y
ScriptArg *open(ScriptArg *ArgList, ScriptArg *pDestination)
{
	return pDestination; 
}

//lock/unlock the door at x,y
ScriptArg *lock(ScriptArg *ArgList, ScriptArg *pDestination)
{
	return pDestination; 
}

ScriptArg *goarea(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSAName, *pSAX, *pSAY;

	pSAName = ArgList[0].Evaluate();
	pSAX = ArgList[1].Evaluate();
	pSAY = ArgList[2].Evaluate();

	int AreaNum;

	AreaNum = PreludeWorld->GetAreaNum((char *)pSAName->GetValue());

	if(AreaNum < 0)
	{
		PreludeWorld->AddArea((char *)pSAName->GetValue());
		AreaNum = PreludeWorld->GetAreaNum((char *)pSAName->GetValue());
	}

	PreludeWorld->GotoArea(AreaNum,(int)pSAX->GetValue(),(int)pSAY->GetValue());

	PreludeParty.Occupy();

	return pDestination;
}

ScriptArg *explosion(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSX, *pSY, *pSZ, *pSRadius, *pSR, *pSDamMin, *pSDamMax, *pSG, *pSB; 
	D3DVECTOR vTemp;
	D3DVECTOR vStart, vEnd;
	pSX = ArgList[0].Evaluate();
	pSY = ArgList[1].Evaluate();
	pSZ = ArgList[2].Evaluate();
	pSRadius = ArgList[3].Evaluate();
	pSDamMin = ArgList[4].Evaluate();
	pSDamMax = ArgList[5].Evaluate();
	pSR = ArgList[6].Evaluate();
	pSG = ArgList[7].Evaluate();
	pSB = ArgList[8].Evaluate();
	vTemp.x = (float)(int)pSX->GetValue() + 0.5f;
	vTemp.y = (float)(int)pSY->GetValue() + 0.5f;
	vTemp.z = ((float)((int)pSZ->GetValue())) / 100.0f + Valley->GetTileHeight((int)pSX->GetValue(),(int)pSY->GetValue());

	float Radius = (float)(int)pSRadius->GetValue() / 10.0f;

	Explosion *pExplosion;
	pExplosion = new Explosion(vTemp, Radius, (int)pSDamMin->GetValue(), (int)pSDamMax->GetValue(), NULL);
	
	PreludeWorld->AddMainObject(pExplosion);

	return pDestination;
}

ScriptArg *lightning(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSX1, *pSY1, *pSZ1, *pSX2, *pSY2, *pSZ2, *pSSize, *pSColor, *pSLife; 
	D3DVECTOR vStart, vEnd;
	Bolt *pBolt;

	pSX1 = ArgList[0].Evaluate();
	pSY1 = ArgList[1].Evaluate();
	pSZ1 = ArgList[2].Evaluate();
	vStart.x =  (float)((int)pSX1->GetValue()) + 0.5f; 
	vStart.y =  (float)((int)pSY1->GetValue()) + 0.5f;
	vStart.z =  ((float)((int)pSZ1->GetValue())) / 100.0f + Valley->GetTileHeight((int)pSX1->GetValue(),(int)pSY1->GetValue());

	pSX2 = ArgList[3].Evaluate();
	pSY2 = ArgList[4].Evaluate();
	pSZ2 = ArgList[5].Evaluate();
	vEnd.x =  (float)((int)pSX2->GetValue()) + 0.5f; 
	vEnd.y =  (float)((int)pSY2->GetValue()) + 0.5f;
	vEnd.z =  ((float)((int)pSZ2->GetValue())) / 100.0f + Valley->GetTileHeight((int)pSX2->GetValue(),(int)pSY2->GetValue());

	pSSize = ArgList[6].Evaluate();
	pSColor = ArgList[7].Evaluate();
	pSLife = ArgList[8].Evaluate();

	pBolt = new Bolt;
	
	float fVar;
	fVar = (float)(int)pSSize->GetValue();
	fVar /= 100.0f;

	pBolt->Create(&vStart, &vEnd, (COLOR_T)(int)pSColor->GetValue(), fVar);

	PreludeWorld->AddMainObject(pBolt);

	int Life;
	Life = (int)pSLife->GetValue();
	
	pBolt->SetDuration(Life);

	return pDestination;
}

ScriptArg *scriptfx(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSX, *pSY, *pSZ, *pSR, *pSG, *pSB; 
	D3DVECTOR vTemp;
	D3DVECTOR vStart, vEnd;
	ParticleSystem *pSys;
	pSys = new ParticleSystem;


	//origin
	pSX = ArgList[0].Evaluate();
	pSY = ArgList[1].Evaluate();
	pSZ = ArgList[2].Evaluate();
	vStart.x = vTemp.x = (float)((int)pSX->GetValue()) + 0.5f; 
	vStart.y = vTemp.y = (float)((int)pSY->GetValue()) + 0.5f;
	vStart.z = vTemp.z = ((float)((int)pSZ->GetValue())) / 100.0f + Valley->GetTileHeight((int)pSX->GetValue(),(int)pSY->GetValue());

	pSys->SetOrigin(vTemp);
	D3DMATERIAL7 mat;
	mat = *Engine->Graphics()->GetMaterial(COLOR_WHITE);

	pSR = ArgList[3].Evaluate();
	pSG = ArgList[4].Evaluate();
	pSB = ArgList[5].Evaluate();
	
	mat.ambient.r = mat.diffuse.r = (float)((int)pSR->GetValue()) / 100.0f;
	mat.ambient.g = mat.diffuse.g = (float)((int)pSG->GetValue()) / 100.0f;
	mat.ambient.b = mat.diffuse.b = (float)((int)pSB->GetValue()) / 100.0f;
	pSys->SetMaterial(&mat);

	pSX = ArgList[6].Evaluate();
	pSY = ArgList[7].Evaluate();
	pSZ = ArgList[8].Evaluate();
	vTemp.x = (float)((int)pSX->GetValue()) /3000.0f; 
	vTemp.y = (float)((int)pSY->GetValue()) /3000.0f;
	vTemp.z = (float)((int)pSZ->GetValue()) /3000.0f;

	pSys->SetInitialVector(vTemp);

	pSX = ArgList[9].Evaluate();
	pSY = ArgList[10].Evaluate();
	pSZ = ArgList[11].Evaluate();
	vTemp.x = (float)((int)pSX->GetValue()) /3000.0f; 
	vTemp.y = (float)((int)pSY->GetValue()) /3000.0f;
	vTemp.z = (float)((int)pSZ->GetValue()) /3000.0f;

	pSys->SetInitialVariance(vTemp);

	pSX = ArgList[12].Evaluate();
	pSY = ArgList[13].Evaluate();
	pSZ = ArgList[14].Evaluate();
	vTemp.x = (float)((int)pSX->GetValue()) /30000.0f; 
	vTemp.y = (float)((int)pSY->GetValue()) /30000.0f;
	vTemp.z = (float)((int)pSZ->GetValue()) /30000.0f;
	pSys->SetGravity(vTemp);
	
	pSX = ArgList[15].Evaluate();
	pSY = ArgList[16].Evaluate();
	pSZ = ArgList[17].Evaluate();
	vEnd.x = vTemp.x = (float)((int)pSX->GetValue()) + 0.5f; 
	vEnd.y = vTemp.y = (float)((int)pSY->GetValue()) + 0.5f;
	vEnd.z = vTemp.z = ((float)((int)pSZ->GetValue())) / 100.0f + Valley->GetTileHeight((int)pSX->GetValue(),(int)pSY->GetValue());
//	pSys->SetDestination(vTemp);
	
	pSB = ArgList[18].Evaluate();
//	pSys->SetPulling((BOOL)pSB->GetValue());


	int NumMoveFrames;
	NumMoveFrames = (int)pSB->GetValue();

	if(NumMoveFrames)
	{
		vTemp.x = (vEnd.x - vStart.x) / (float)NumMoveFrames;	
		vTemp.y = (vEnd.y - vStart.y) / (float)NumMoveFrames;
		vTemp.z = (vEnd.z - vStart.z) / (float)NumMoveFrames;
	
		pSys->SetMotion(vTemp);
	}
	
	pSX = ArgList[19].Evaluate();
	pSY = ArgList[20].Evaluate();
	pSZ = ArgList[21].Evaluate();
	vTemp.x = (float)((int)pSX->GetValue()) /3000.0f; 
	vTemp.y = (float)((int)pSY->GetValue()) /3000.0f;
	vTemp.z = (float)((int)pSZ->GetValue()) /3000.0f;
	pSys->SetRotation(vTemp);
	
	pSX = ArgList[22].Evaluate();
	pSY = ArgList[23].Evaluate();
	pSZ = ArgList[24].Evaluate();
	vTemp.x = (float)((int)pSX->GetValue()) + 0.5f; 
	vTemp.y = (float)((int)pSY->GetValue()) + 0.5f;
	vTemp.z = ((float)((int)pSZ->GetValue())) / 100.0f + Valley->GetTileHeight((int)pSX->GetValue(),(int)pSY->GetValue());
	pSys->SetOrbit(vTemp);
	
	pSB = ArgList[25].Evaluate();
	pSys->SetOrbitting((BOOL)pSB->GetValue());

	pSB = ArgList[26].Evaluate();
	pSys->SetEnd((int)pSB->GetValue());

	pSB = ArgList[27].Evaluate();
	pSys->SetLife((int)pSB->GetValue());

	pSB = ArgList[28].Evaluate();
	pSys->SetLifeVariance((int)pSB->GetValue());
	
	pSB = ArgList[29].Evaluate();
	pSys->SetRadius((float)((int)pSB->GetValue()) / 100.0f);

	pSB = ArgList[30].Evaluate();
	pSys->SetEmissionRate((int)pSB->GetValue());

	pSB = ArgList[31].Evaluate();
	pSys->SetEmissionQuantity((int)pSB->GetValue());

	pSX = ArgList[32].Evaluate();
	pSY = ArgList[33].Evaluate();
	pSZ = ArgList[34].Evaluate();
	vTemp.x = (float)((int)pSX->GetValue()) /100.0f; 
	vTemp.y = (float)((int)pSY->GetValue()) /100.0f;
	vTemp.z = (float)((int)pSZ->GetValue()) /100.0f;

	pSys->SetAddVector(vTemp);

	PreludeWorld->AddMainObject(pSys);
	pSys->AdjustCamera();
	pSys->AddParticle();

	return pDestination; 
}

ScriptArg *getmod(ScriptArg *ArgList, ScriptArg *pDestination)
{
	return pDestination; 
}

ScriptArg *killmod(ScriptArg *ArgList, ScriptArg *pDestination)
{
	return pDestination; 
}

ScriptArg *illegal(ScriptArg *ArgList, ScriptArg *pDestination)
{
	ScriptArg *pSA;
	pSA = ArgList[0].Evaluate();

	Valley->IllegalActivity(pSA->GetCreature());

	return pDestination; 
}




ScriptArg *CallFunc(int FuncNum, ScriptArg *ArgList, ScriptArg *pDestination)
{
	if(!ScriptFunctions[FuncNum])
	{
		SafeExit("Attempted to call non-existent function\n");
	}

	ScriptArg *SA;

#ifdef SHOW_SCRIPT_DEBUG
	FILE *fp;
	char FuncName[32];
	fp = SafeFileOpen("scriptlog.txt","a+t");
	GetFuncName(FuncNum,FuncName);
	fprintf(fp,"Calling: %s with: ",FuncName);
	int n = 0;
	while(ArgList[n].GetType() != ARG_TERMINATOR)
	{
		ArgList[n].Print(fp);
		n++;
	}
	fprintf(fp,"\n");
	fclose(fp);
#endif
	
	SA = ScriptFunctions[FuncNum](ArgList,pDestination);

#ifdef SHOW_SCRIPT_DEBUG
	fp = SafeFileOpen("Scriptlog.txt","a+t");
	fprintf(fp,"Result of %s:",FuncName);
	if(SA)
		SA->Print(fp);
	else
		fprintf(fp, "NULL");
	fprintf(fp,"\n");
	fclose(fp);
#endif

	return pDestination;
}

void LoadFuncs()
{
	int n = 1;
	ScriptFunctions[n++] = say_win;
	ScriptFunctions[n++] = addword;
	ScriptFunctions[n++] = removeword;
	ScriptFunctions[n++] = Place;
	ScriptFunctions[n++] = goword;
	ScriptFunctions[n++] = haskey;
	ScriptFunctions[n++] = sflag;
	ScriptFunctions[n++] = flagm;
	ScriptFunctions[n++] = flag;
	ScriptFunctions[n++] = flagp;
	ScriptFunctions[n++] = scriptend;
	ScriptFunctions[n++] = iff;
	ScriptFunctions[n++] = cond;
	ScriptFunctions[n++] = give;
	ScriptFunctions[n++] = take;
	ScriptFunctions[n++] = has;
	ScriptFunctions[n++] = moveto;
	ScriptFunctions[n++] = add;
	ScriptFunctions[n++] = sub;
	ScriptFunctions[n++] = mul;
	ScriptFunctions[n++] = div;
	ScriptFunctions[n++] = ask;
	ScriptFunctions[n++] = randnum;
	ScriptFunctions[n++] = getChar; 
	ScriptFunctions[n++] = setChar; 
	ScriptFunctions[n++] = saydesc; 
	ScriptFunctions[n++] = sayclear; 
	ScriptFunctions[n++] = comp;  
	ScriptFunctions[n++] = addeventtimed;
	ScriptFunctions[n++] = addeventrest;
	ScriptFunctions[n++] = addeventstartcombat;
	ScriptFunctions[n++] = addeventendcombat;
	ScriptFunctions[n++] = talk; 
	ScriptFunctions[n++] = saychar;
	ScriptFunctions[n++] = scriptdescribe; 
	ScriptFunctions[n++] = endgame; 
	ScriptFunctions[n++] = sayadd_win; 
	ScriptFunctions[n++] = ScriptDrawScreen; 
	ScriptFunctions[n++] = DoLoop; 
	ScriptFunctions[n++] = WhileLoop; 
	ScriptFunctions[n++] = ScriptUpdate; 
	ScriptFunctions[n++] = ScriptUpdateAll; 
	ScriptFunctions[n++] = ScriptUpdateScreen;
	ScriptFunctions[n++] = Mod; 
	ScriptFunctions[n++] = Equals; 
	ScriptFunctions[n++] = addeventrandom; 
	ScriptFunctions[n++] = scriptfx; 
	ScriptFunctions[n++] = getaverage; 
	ScriptFunctions[n++] = addnpc; 
	ScriptFunctions[n++] = getx; 
	ScriptFunctions[n++] = gety; 
	ScriptFunctions[n++] = getz; 
	ScriptFunctions[n++] = prand; 
	ScriptFunctions[n++] = GetScreenCenterX; 
	ScriptFunctions[n++] = GetScreenCenterY; 
	ScriptFunctions[n++] = equippedtype; 
	ScriptFunctions[n++] = removenpc; 
	ScriptFunctions[n++] = sdistance; 
	ScriptFunctions[n++] = getnonunique; 
	ScriptFunctions[n++] = lightning; 
	ScriptFunctions[n++] = getat; 
	ScriptFunctions[n++] = cameraangle; 
	ScriptFunctions[n++] = move; 
	ScriptFunctions[n++] = face; 
	ScriptFunctions[n++] = PlacePortal; 
	ScriptFunctions[n++] = WhatIsAt; 
	ScriptFunctions[n++] = cameralookat; 
	ScriptFunctions[n++] = addlocation;
	ScriptFunctions[n++] = CallScript;
	ScriptFunctions[n++] = And;
	ScriptFunctions[n++] = Not;
	ScriptFunctions[n++] = Or;
	ScriptFunctions[n++] = CreateString;
	ScriptFunctions[n++] = CountLoop;
	ScriptFunctions[n++] = Push;
	ScriptFunctions[n++] = Pop;
	ScriptFunctions[n++] = IsEmpty;
	ScriptFunctions[n++] = IsMoving;
	ScriptFunctions[n++] = Dupe;
	ScriptFunctions[n++] = Switch;
	ScriptFunctions[n++] = LookStack;
	ScriptFunctions[n++] = ImproveSkill;
	ScriptFunctions[n++] = getbestskill;
	ScriptFunctions[n++] = AttackScript;
	ScriptFunctions[n++] = SetTarget;
	ScriptFunctions[n++] = damage;
	ScriptFunctions[n++] = damageblock;
	ScriptFunctions[n++] = saychardesc;
	ScriptFunctions[n++] = event;
	ScriptFunctions[n++] = camerazoom; 
	ScriptFunctions[n++] = FindPath; 
	ScriptFunctions[n++] = removejournal;//PlaceParty;
	ScriptFunctions[n++] = explosion;
	ScriptFunctions[n++] = removeall;
	ScriptFunctions[n++] = removeitem;
	ScriptFunctions[n++] = moveitem;
	ScriptFunctions[n++] = wingame;//removeanimation;
	ScriptFunctions[n++] = animate;
	ScriptFunctions[n++] = playsound;
	ScriptFunctions[n++] = playmusic;
	ScriptFunctions[n++] = incombat;
	ScriptFunctions[n++] = placeclear;
	ScriptFunctions[n++] = startcombat;
	ScriptFunctions[n++] = endcombat;
	ScriptFunctions[n++] = fade;
	ScriptFunctions[n++] = ClearActions;
	ScriptFunctions[n++] = gettarget;
	ScriptFunctions[n++] = getnumber;
	ScriptFunctions[n++] = getpower;
	ScriptFunctions[n++] = goarea;
	ScriptFunctions[n++] = equipped;
	ScriptFunctions[n++] = addspell;
	ScriptFunctions[n++] = addmod;
	ScriptFunctions[n++] = cast;
	ScriptFunctions[n++] = AdvanceTime;
	ScriptFunctions[n++] = GetHour;
	ScriptFunctions[n++] = GetMin;
	ScriptFunctions[n++] = GetDay;
	ScriptFunctions[n++] = GetTotalTime;
	ScriptFunctions[n++] = getbest;
	ScriptFunctions[n++] = hastype;
	ScriptFunctions[n++] = getworst;
	ScriptFunctions[n++] = cheatcombat;
	ScriptFunctions[n++] = illegal;
	ScriptFunctions[n++] = cameraheight;
	ScriptFunctions[n++] = los;
	ScriptFunctions[n++] = missile;
	ScriptFunctions[n++] = countenemies;
	ScriptFunctions[n++] = schedule;
	ScriptFunctions[n++] = journal;
	ScriptFunctions[n++] = barter;
	ScriptFunctions[n++] = open;
	ScriptFunctions[n++] = returnschedule;
	ScriptFunctions[n++] = lock;
	ScriptFunctions[n++] = hidetalk;
	ScriptFunctions[n++] = equip;
	ScriptFunctions[n++] = RemoveChar;
	ScriptFunctions[n++] = approach;
	ScriptFunctions[n++] = enemiesaround;
	ScriptFunctions[n++] = isnpc;
	ScriptFunctions[n++] = isleader;
	ScriptFunctions[n++] = setleader;
	ScriptFunctions[n++] = inparty;
	ScriptFunctions[n++] = partysize;
	ScriptFunctions[n++] = killflag;
	ScriptFunctions[n++] = showtalk;
	ScriptFunctions[n++] = opentalk;
	ScriptFunctions[n++] = setportrait;
	ScriptFunctions[n++] = closetalk;
	ScriptFunctions[n++] = equipall;
	ScriptFunctions[n++] = toggleinventory;
	ScriptFunctions[n++] = removecreatures;

	DEBUG_INFO("\nFunction Pointers assigned\n\n");

}

int GetFuncID(char *FuncName)
{
	char funcname[32];

	FILE *gfp;
	
	gfp = SafeFileOpen("funcs.txt","rt");
	
	char c = '7';
	int n = 0;
	int offset = 0;
	while (!feof(gfp) && c != EOF)
	{
		offset = 0;
		c = fgetc(gfp);
		if(c == '"')
		{
			c = fgetc(gfp);
			while(c != '"' && c != EOF && !feof(gfp))
			{
				funcname[offset] = c;
				offset++;
				c = fgetc(gfp);
			}
			funcname[offset] = '\0';
			if(!strcmp(FuncName, funcname))
			{
				fclose(gfp);
				return n;
			}
			n++;
		}
		
	}
	fclose(gfp);

	char blarg[64];
	sprintf(blarg,"unknown func: %s",FuncName);
	SafeExit(blarg);
	return FALSE;

}

void GetFuncName(int ID, char *Dest)
{
	FILE *gfp;
	
	gfp = SafeFileOpen("funcs.txt","rt");
	
	char c = '7';
	int n = 0;
	int offset = 0;
	while ((n < ID) && !feof(gfp) && c != EOF)
	{
		c = fgetc(gfp);
		if(c == '"')
		{
			c = fgetc(gfp);
			while(c != '"' && c != EOF && !feof(gfp))
			{
				c = fgetc(gfp);
			}
			n++;
		}
		
	}

	c = fgetc(gfp);
	while(c != '"') 
		c = fgetc(gfp);

	offset = 0;
	c = fgetc(gfp);
	while(c != '"' && c != EOF && !feof(gfp))
	{
		Dest[offset] = c;
		offset++;
		c = fgetc(gfp);
	}
	Dest[offset] = '\0';
	
	fclose(gfp);
	
	return;
	
	char blarg[64];
	sprintf(blarg,"unknown number: %i", ID);
	SafeExit(blarg);
	return;
}

void ClearStack()
{
	for(int n = 0; n < StackTop; n++)
	{
		if(ScriptStack[n])
		{
			delete ScriptStack[n];
			ScriptStack[n] = NULL;
		}
	}
	StackTop = 0;
}

void GetSub(ScriptArg *ToFill, char *SubString)
{
	ToFill->SetValue((void *)0);
	ToFill->SetType(ARG_NONE);

	if(!strcmp(SubString,"LEAD"))
	{
		ToFill->SetType(ARG_LEADER);
	}
	else
	if(!strcmp(SubString, "PARTY"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTY);
	}
	else
	if(!strcmp(SubString, "PARTY1"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTYONE);
	}
	else
	if(!strcmp(SubString, "PARTY2"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTYTWO);
	}
	else
	if(!strcmp(SubString, "PARTY3"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTYTHREE);
	}
	else
	if(!strcmp(SubString, "PARTY4"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTYFOUR);
	}
	else
	if(!strcmp(SubString, "PARTY5"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTYFIVE);
	}
	else
	if(!strcmp(SubString, "PARTY6"))
	{
		ToFill->SetValue(NULL);
		ToFill->SetType(ARG_PARTYSIX);
	}
	else
	if(!strcmp(SubString, "N"))
	{
		ToFill->SetValue((void *)NORTH);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "S"))
	{
		ToFill->SetValue((void *)SOUTH);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "E"))
	{
		ToFill->SetValue((void *)EAST);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "W"))
	{
		ToFill->SetValue((void *)WEST);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "NE"))
	{
		ToFill->SetValue((void *)NORTHEAST);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "NW"))
	{
		ToFill->SetValue((void *)NORTHWEST);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "SE"))
	{
		ToFill->SetValue((void *)SOUTHEAST);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "SW"))
	{
		ToFill->SetValue((void *)SOUTHWEST);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "WEAPON_SKILL"))
	{
		ToFill->SetValue((void *)BEST_WEAPON_SKILL);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "MAGIC_SKILL"))
	{
		ToFill->SetValue((void *)BEST_MAGIC_SKILL);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "NONWEAPON_SKILL"))
	{
		ToFill->SetValue((void *)BEST_NONWEAPON_SKILL);
		ToFill->SetType(ARG_NUMBER);
	}
	else
	if(!strcmp(SubString, "DAMAGE"))
	{
		ToFill->SetValue((void *)BEST_DAMAGE);
		ToFill->SetType(ARG_NUMBER);
	}
	
}

void Push(int n)
{
	ScriptArg *ToPush;

	ToPush = new ScriptArg;
	ToPush->SetValue((void *)n);
	ToPush->SetType(ARG_NUMBER);

	ScriptStack[StackTop] = ToPush;
	StackTop++;
}

void Push(char *String)
{
	ScriptArg *ToPush;

	ToPush = new ScriptArg;
	ToPush->SetValue((void *)String);
	ToPush->SetType(ARG_STRING);

	ScriptStack[StackTop] = ToPush;
	StackTop++;
	
}

void Push(Creature *pCreature)
{
	ScriptArg *ToPush;

	ToPush = new ScriptArg;
	ToPush->SetValue((void *)pCreature);
	ToPush->SetType(ARG_CREATURE);

	ScriptStack[StackTop] = ToPush;
	StackTop++;


}

void Push(Item *pItem)
{
	ScriptArg *ToPush;

	ToPush = new ScriptArg;
	ToPush->SetValue((void *)pItem);
	ToPush->SetType(ARG_ITEM);

	ScriptStack[StackTop] = ToPush;
	StackTop++;
}

ScriptArg *Pop()
{
	StackTop--;
	return ScriptStack[StackTop];
}

void CallScript(const char *ScriptName, const char *FileName)
{
	ScriptBlock SB;

	FILE *fp;

	fp = fopen(FileName,"rt");

	char CallScriptID[64];

	if(fp)
	{
		sprintf(CallScriptID,"#%s#",ScriptName);

		if(SeekTo(fp,CallScriptID))
		{
			SB.Import(fp);	
			fclose(fp);		
		}
	}
	else
	{
		fp = fopen(FileName,"rb");
		if(!fp)
		{
			SafeExit("Couldn't open script file.");
			return;
		}
		
		SB.Import(fp);
		fclose(fp);
	}

	ScriptBlock *OldContext;
	OldContext = ScriptContextBlock;
	ScriptContextBlock = &SB;

	SB.Process();

	ScriptContextBlock = OldContext;

	return; 
}





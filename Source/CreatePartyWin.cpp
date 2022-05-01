#include "createpartywin.h"
#include "zsengine.h"
#include "zsbutton.h"
#include "zstext.h"
#include "zslistbox.h"
#include "zsutilities.h"
#include "scriptfuncs.h"
#include "zsdescribe.h"
#include <assert.h>
#include "zseditwindow.h"
#include "party.h"
#include "world.h"
#include "zsconfirm.h"
#include "zshelpwin.h"
#include "zsOldListBox.h"


typedef enum
{
	IDC_STAT1,
	IDC_STAT2,
	IDC_STAT3,
	IDC_STAT4,
	IDC_STAT5,
	IDC_STAT6,
	IDC_STAT7,
	
	IDC_PLUS1,
	IDC_PLUS2,
	IDC_PLUS3,
	IDC_PLUS4,
	IDC_PLUS5,
	IDC_PLUS6,
	IDC_PLUS7,
	
	IDC_MINUS1,
	IDC_MINUS2,
	IDC_MINUS3,
	IDC_MINUS4,
	IDC_MINUS5,
	IDC_MINUS6,
	IDC_MINUS7,

	IDC_SNAME1,
	IDC_SNAME2,
	IDC_SNAME3,
	IDC_SNAME4,
	IDC_SNAME5,
	IDC_SNAME6,
	IDC_SNAME7,
	
	IDC_QUIT,
	IDC_BEGIN,

	IDC_BACKGROUNDS,

	IDC_RACE,
	IDC_SEX,

	IDC_NAME,

	IDC_PORTRAIT,

	IDC_DEFAULT,

	IDC_PORTRAIT_RIGHT,
	IDC_PORTRAIT_LEFT,

	IDC_MEMBER_ONE,
	IDC_MEMBER_TWO,
	IDC_MEMBER_THREE,
	IDC_MEMBER_FOUR,
	IDC_MEMBER_FIVE,
	IDC_MEMBER_SIX,

	IDC_MEMBER_ONE_NAME,
	IDC_MEMBER_TWO_NAME,
	IDC_MEMBER_THREE_NAME,
	IDC_MEMBER_FOUR_NAME,
	IDC_MEMBER_FIVE_NAME,
	IDC_MEMBER_SIX_NAME,

	IDC_MEMBER_ONE_DELETE,
	IDC_MEMBER_TWO_DELETE,
	IDC_MEMBER_THREE_DELETE,
	IDC_MEMBER_FOUR_DELETE,
	IDC_MEMBER_FIVE_DELETE,
	IDC_MEMBER_SIX_DELETE,

	IDC_COMBAT,
	IDC_COMBAT_POINTS,
	IDC_NONCOMBAT,
	IDC_NONCOMBAT_POINTS,
	IDC_SKILL,
	IDC_SKILL_POINTS,

	IDC_STATPOINTS,
	
	IDC_DESCRIPTIONS,

	IDC_STATPOINT_ID,
	IDC_RACE_ID,
	IDC_SEX_ID,
	IDC_HAIR_ID,
	IDC_WEIGHT,
	IDC_HEIGHT,
	IDC_HAIR,

	IDC_SELECTBACKGROUND,

	IDC_SKILL1,

	IDC_SKILLNAME1 = IDC_SKILL1 + 50,

	IDC_SKILLPLUS1 = IDC_SKILLNAME1 + 50,

	IDC_SKILLMINUS1 = IDC_SKILLPLUS1 + 50,

} CREATION_WINDOWS;


int CreatePartyWin::Command(int IDFrom, int Command, int Param)
{
	ZSWindow *pWin;
	int n;
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_DEFAULT)
		{
			CallScript("GetDefault","createparty.txt");
			ScriptArg *SA;

			for(n = 0; n < NUM_CREATE_MEMBERS; n++)
			{
				SA = Pop();
				if(pMembers[n])
				{
					delete pMembers[n];
				}
				pMembers[n] = new Creature((Creature *)SA->GetValue());
				pWin = GetChild(IDC_MEMBER_ONE_NAME + n);
				pWin->Show();
				pWin->SetText(pMembers[n]->GetData(INDEX_NAME).String);
			}
			pWin = GetChild(IDC_BEGIN);
			pWin->Show();
		}
		else
		if(IDFrom == IDC_BEGIN)
		{
			ZeroMemory(&PreludeParty, sizeof(Party));
			for(n = 0; n < NUM_CREATE_MEMBERS; n++)
			{
				PreludeParty.AddMember(pMembers[n]);
			}
			PreludeParty.SetLeader(PreludeParty.GetMember(0));
			State = WINDOW_STATE_DONE;
			ReturnCode = TRUE;
		}
		else
		if(IDFrom == IDC_QUIT)
		{
			State = WINDOW_STATE_DONE;
			ReturnCode = FALSE;
		}
		else
		{
			int CharacterNum;
			CharacterNum = IDFrom - IDC_MEMBER_ONE;
			char TempString[128];
			sprintf(TempString,"Replace %s?",pMembers[CharacterNum]->GetData(INDEX_NAME).String);
			if(Confirm(this,TempString,"Yes","No"))
			{
				pWin = GetMain()->GetChild(CREATE_CHARACTER_ID);
				if(pWin)
				{
					CreateCharacterWin *pCreate;
					pCreate = (CreateCharacterWin *)pWin;
					Creature *pCreature;
					pCreature = new Creature((Creature *)Thing::Find(Creature::GetFirst(), "RiverBase"));
					pCreate->SetTarget(pCreature);
					Hide();
					pCreate->Show();
					int Created;

					SetFocus(pCreate);
					Created = pCreate->GoModal();
					ReleaseFocus();
					pCreate->Hide();
					Show();
					if(!Created)
					{
						delete pCreature;
					}
					else
					{
						delete pMembers[CharacterNum];

						pMembers[CharacterNum] = pCreature;

						pWin = GetChild(IDC_MEMBER_ONE_NAME + CharacterNum);
						pWin->Show();
						pWin->SetText(pMembers[CharacterNum]->GetData(INDEX_NAME).String);
						
						pWin = GetChild(IDC_MEMBER_ONE + CharacterNum);
						pWin->CreatePortrait(pMembers[CharacterNum]->GetData(INDEX_PORTRAIT).String);
						pWin->AddBorder(8,1);
					}
				}
			}
		}
	
	}
	return TRUE;
}

void CreateCharacterWin::SetTarget(Creature *NewTarget)
{
	pTarget = NewTarget;
	*pTarget = *pRiverBase;

	GetValidProfessions();
	StoreTarget();
	ResetTarget();
	SetPortrait();
}

	
CreatePartyWin::CreatePartyWin()
{
	ID = CREATE_PARTY_ID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	Bounds.left = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.top = 0;
	Bounds.bottom = Engine->Graphics()->GetHeight();

	FILE *fp;
	fp = SafeFileOpen ("startscreen.bmp","rb");

	if(fp)
	{
		fclose(fp);
		
		BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("StartScreen.bmp",800,600,NULL,NULL);
	}

	RECT rArea;
	rArea.top = 100;
	rArea.bottom = 450;
	rArea.left = 230;
	rArea.right = 550;

	RECT rOne;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"CREATEPARTYWINDOW");

	SeekTo(fp,"MEMBERONE");
	LoadRect(&rOne, fp);

	fclose(fp);

	RECT rBase;
	rBase.left = 0;
	rBase.top = 0;
	rBase.bottom = 600;
	rBase.right = 800;

	ScaleRect(&rOne,	 &rBase, &Bounds);

	ScaleRect(&rArea, &rBase, &Bounds);

	Engine->Graphics()->FillSurface(BackGroundSurface, 0, &rArea);

	Cursor = CURSOR_POINT;
	
	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	Border = 0;

	ZSButton *pButton;
	ZSEdit *pName;
		
	int ButtonWidth;
	ButtonWidth = rOne.right - rOne.left;

	for(int n = 0; n < NUM_CREATE_MEMBERS; n++)
	{
		pButton = new ZSButton(BUTTON_NONE, IDC_MEMBER_ONE + n, XYWH(rOne));
		pButton->Show();
		
		AddChild(pButton);

		pName = new ZSEdit(WINDOW_EDIT, IDC_MEMBER_ONE_NAME +n, rOne.left, rOne.bottom,rOne.right-rOne.left,32);
		pName->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
		pName->CreateParchmentBorderedBackground(4);
		AddChild(pName);

		rOne.left += ButtonWidth + 2;
		rOne.right += ButtonWidth + 2;
		pMembers[n] = NULL;
	}

	int Width;
	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Return");

	pButton = new ZSButton("returnbutton", IDC_QUIT, (Bounds.right / 2) - 74, 424, 74, 43, 74, 43, 1);
	pButton->Show();
	//pButton->SetText("Return");
	AddChild(pButton);

	ZSText *pText;
	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Click on a portrait to create a party member");	

	pText = new ZSText(-1, 400 - Width /2, 100, "Click on a portrait to create a party member");
	
	pText->Show();
	AddChild(pText);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Begin Game");

	pButton = new ZSButton("beginButton", IDC_BEGIN, (Bounds.right / 2), 424, 74, 43, 74, 43, 1);
	//pButton->SetText("Begin Game");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Default Party");

/*	pButton = new ZSButton(BUTTON_NORMAL, IDC_DEFAULT, (Bounds.right / 2) - Width/2, Bounds.bottom - 192, Width, 24);
	pButton->SetText("Default Party");
	pButton->Show();
	AddChild(pButton);
*/
	CreateCharacterWin *pCreate;
	pCreate = new CreateCharacterWin(NULL);
	pCreate->Hide();
	GetMain()->AddTopChild(pCreate);

	CallScript("GetDefault","createparty.txt");
	ScriptArg *SA;
	ZSWindow *pWin;

	for(n = 0; n < NUM_CREATE_MEMBERS; n++)
	{
		SA = Pop();
		if(pMembers[n]) 
			delete pMembers[n];
		
		pMembers[n] = new Creature((Creature *)SA->GetValue());
		pWin = GetChild(IDC_MEMBER_ONE_NAME + n);
		pWin->Show();
		pWin->SetText(pMembers[n]->GetData(INDEX_NAME).String);
	
		pWin = GetChild(IDC_MEMBER_ONE + n);
		pWin->SetBorderWidth(8);
		pWin->CreatePortrait(pMembers[n]->GetData(INDEX_PORTRAIT).String);
		pWin->AddBorder(8,1);
	}

	//TBD:  Delete buttons
}

void CreateCharacterWin::RaiseStat(int Num)
{
	//check to see where we're raising to
	int StatStartIndex;
	StatStartIndex = INDEX_STRENGTH;
	int RacialAverage = 0;
	ZSWindow *pWin;
	//get racial stat

	//what is our race
	if(pTarget->GetData(INDEX_FLAMEBLOOD).Value && pTarget->GetData(INDEX_RIVERBLOOD).Value)
	{
		RacialAverage = pMixedBase->GetData(StatStartIndex + Num).Value;
	}
	else
	if(pTarget->GetData(INDEX_FLAMEBLOOD).Value)
	{
		RacialAverage = pFlameBase->GetData(StatStartIndex + Num).Value;
	}
	else
	{
		RacialAverage = pRiverBase->GetData(StatStartIndex + Num).Value;
	}

	int CurNum;

	CurNum = pTarget->GetData(StatStartIndex + Num).Value;

	if(CurNum - RacialAverage >= 5)
	{
		if(CreationPoints >= 2 && CurNum < 23)
		{
			CurNum++;
			CreationPoints -= 2;
			pTarget->SetData(StatStartIndex + Num, CurNum);
		}
		else
		{

		}
	}
	else
	{
		if(CreationPoints > 0)
		{
			CurNum++;
			CreationPoints--;
			pTarget->SetData(StatStartIndex + Num, CurNum);
			
		}
	}


	pWin = GetChild(IDC_STAT1 + Num);

	pWin->SetText(CurNum);
	if(CurNum - RacialAverage >= 5)
	{
		pWin->SetTextColor(TEXT_RED_PARCHMENT);
	}
	else
	{
		pWin->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	}

	pWin = GetChild(IDC_STATPOINTS);

	pWin->SetText(CreationPoints);
	
	if(!CreationPoints)
	{
		pWin = GetChild(IDC_SELECTBACKGROUND);
		pWin->Show();
	}
}

void CreateCharacterWin::LowerStat(int Num)
{
	//check to see where we're raising to
	int StatStartIndex;
	StatStartIndex = pFlameBase->GetIndex("STRENGTH");
	int RacialAverage = 0;
	ZSWindow *pWin;
	//get racial stat

	//what is our race
	if(pTarget->GetData(INDEX_FLAMEBLOOD).Value && pTarget->GetData(INDEX_RIVERBLOOD).Value)
	{
		RacialAverage = pMixedBase->GetData(StatStartIndex + Num).Value;
	}
	else
	if(pTarget->GetData(INDEX_FLAMEBLOOD).Value)
	{
		RacialAverage = pFlameBase->GetData(StatStartIndex + Num).Value;
	}
	else
	{
		RacialAverage = pRiverBase->GetData(StatStartIndex + Num).Value;
	}

	int CurNum;

	CurNum = pTarget->GetData(StatStartIndex + Num).Value;

	if(CurNum - RacialAverage > 5)
	{
		CurNum--;
		CreationPoints += 2;
		pTarget->SetData(StatStartIndex + Num, CurNum);
	}
	else
	{
		if(CurNum > 5)
		{
			CurNum--;
			CreationPoints++;
			pTarget->SetData(StatStartIndex + Num, CurNum);
		}
	}

	pWin = GetChild(IDC_STAT1 + Num);

	pWin->SetText(CurNum);
	if(CurNum - RacialAverage >= 5)
	{
		pWin->SetTextColor(TEXT_RED_PARCHMENT);
	}
	else
	{
		pWin->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	}

	pWin = GetChild(IDC_STATPOINTS);

	pWin->SetText(CreationPoints);

	if(CreationPoints)
	{
		pWin = GetChild(IDC_SELECTBACKGROUND);
		pWin->Hide();
	}
}

void CreateCharacterWin::ChangeRace()
{
	ZSWindow *pChild;
	char *ChildText;
	Creature *pChangeTo;

	pChild = GetChild(IDC_RACE);
	ChildText = pChild->GetText();

	if(!strcmp(ChildText,"River"))
	{
		pChild->SetText("Flame");
		pChangeTo = pFlameBase;
	}
	else
	if(!strcmp(ChildText,"Flame"))
	{
		pChild->SetText("Mixed");
		pChangeTo = pMixedBase;
	}
	else
	{
		pChild->SetText("River");
		pChangeTo = pRiverBase;
	}

	int StatStart;
	StatStart = pChangeTo->GetIndex("STRENGTH");

	for(int n = 0; n < 7; n++)
	{
		pTarget->SetData(StatStart + n, pChangeTo->GetData(StatStart + n).Value);
		pChild = GetChild(IDC_STAT1 + n);
		pChild->SetText(pTarget->GetData(StatStart + n).Value);
		pChild->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	}

	pTarget->SetData(INDEX_SKIN, pChangeTo->GetData(INDEX_SKIN).Value);
	
	pTarget->SetData(INDEX_RIVERBLOOD, pChangeTo->GetData(INDEX_RIVERBLOOD).Value);
	pTarget->SetData(INDEX_FLAMEBLOOD, pChangeTo->GetData(INDEX_FLAMEBLOOD).Value);
		
	CreationPoints = 10;

	pChild = GetChild(IDC_STATPOINTS);

	pChild->SetText(CreationPoints);

	pChild = GetChild(IDC_SELECTBACKGROUND);
	pChild->Hide();
}

void CreateCharacterWin::GetValidProfessions()
{
	int NumItems;

	ZSOldList *pList;

	pList = (ZSOldList *)GetChild(IDC_BACKGROUNDS);

	NumItems = pList->GetNumItems();
	ScriptArg *RetVal;

	for(int n = 0; n < NumItems; n++)
	{
		Push(pTarget);
		
		ScriptContextBlock = &Backgrounds[n].Requirements;
		Backgrounds[n].Requirements.Process();
		ScriptContextBlock = NULL;
	

		RetVal = Pop();
		
		if((int)RetVal->GetValue())
		{
			pList->EnableItem(n);
		}
		else
		{
			pList->DisableItem(n);
		}
	
		delete RetVal;
	}
	
	pList->SetSelection(-1);
}

void CreateCharacterWin::SetPortrait()
{
	ZSWindow *pChild;

	
	char FileName[64];

	pChild = GetChild(IDC_SEX);
	
	if(pTarget->GetData(INDEX_SEX).Value)
	{
		//woman
		strcpy(FileName,"woman");
		pChild->SetText("Female");
	}
	else
	{
		//man
		pChild->SetText("Male");
		strcpy(FileName,"man");
	}
	
	pChild = GetChild(IDC_PORTRAIT);
	char temp[8];
	sprintf(temp,"%i",PortraitNum);
	strcat(FileName,temp);

	pChild->SetBorderWidth(8);
	
	pChild->CreatePortrait(FileName);

	pChild->AddBorder(8);

	pChild = GetChild(IDC_SEX);
	

}

int CreateCharacterWin::Command(int IDFrom, int Command, int Param)
{
	ZSWindow *pChild;
	char *ChildText;
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_QUIT)
		{
			State = WINDOW_STATE_DONE;
			ReturnCode = FALSE;
		}
		else
		if(IDFrom >= IDC_PLUS1 && IDFrom <= IDC_PLUS7)
		{
			RaiseStat(IDFrom - IDC_PLUS1);
			GetValidProfessions();
		}
		else
		if(IDFrom >= IDC_MINUS1 && IDFrom <= IDC_MINUS7)
		{
			LowerStat(IDFrom - IDC_MINUS1);
			GetValidProfessions();
		}
		else
		if(IDFrom == IDC_SEX)
		{
			pChild = GetChild(IDC_SEX);
			ChildText = pChild->GetText();
			if(!strcmp(ChildText,"Male"))
			{
				pChild->SetText("Female");	
				pTarget->SetData(INDEX_SEX,1);
			}
			else
			{
				pChild->SetText("Male");
				pTarget->SetData(INDEX_SEX,0);
			}
			SetPortrait();
		}
		else
		if(IDFrom == IDC_RACE)
		{
			//check to see if we're editting stats
			ZSOldList *pWin;
			pWin = (ZSOldList *)GetChild(IDC_BACKGROUNDS);
			if(pWin && pWin->IsVisible())
			{
				ChangeRace();
				GetValidProfessions();
			}
			else
			{
				ZSWindow *pChild;
				pChild = GetChild(IDC_DESCRIPTIONS);
				pChild->SetText("You can not change your race after you have chosen a background.");
			}
		}
		else
		if(IDFrom == IDC_PORTRAIT_RIGHT)
		{
			PortraitNum++;
			if(PortraitNum > 9)
			{
				PortraitNum = 1;
			}
			SetPortrait();
		}
		else
		if(IDFrom == IDC_PORTRAIT_LEFT)
		{
			PortraitNum--;
			if(PortraitNum < 1)
			{
				PortraitNum = 9;
			}
			SetPortrait();
		}
		else
		if(IDFrom == IDC_SELECTBACKGROUND)
		{

			ZSOldList *pWin;
			pWin = (ZSOldList *)GetChild(IDC_BACKGROUNDS);
			if(pWin && pWin->IsVisible())
			{
				pChild = GetChild(IDC_DESCRIPTIONS);
				pChild->SetText("You may select a background from the list.");
				
				int BNum;
				BNum = pWin->GetSelection();
				if(BNum != -1)
				{
					StoreTarget();
					Push(pTarget);
					Backgrounds[BNum].SelectResult.Process();
					//only show begin after all skill points are spent
					//pBegin = GetChild(IDC_BEGIN);
					//pBegin->Show();
					ScriptArg *SA;

					SA = Pop();
					MaxCombat = CurCombat = (int)SA->GetValue();
					delete SA;

					SA = Pop();
					MaxNon = CurNon = (int)SA->GetValue();
					delete SA;

					SA = Pop();
					MaxFree = CurFree = (int)SA->GetValue();
					delete SA;

					for(int n = 0; n < NumSkills; n++)
					{
						StoredSkills[n] = pTarget->GetData(n + StartSkill).Value;		
					}
					ShowSkills();
					ZSWindow *pChild;
					int StatStart;
					StatStart = pTarget->GetIndex("STRENGTH");
					for(n = 0; n < 7; n ++)
					{
						pChild = GetChild(IDC_STAT1 + n);
						pChild->SetText(pTarget->GetData(n + StatStart).Value);
					}
					ShowSkills();
					pChild = GetChild(IDC_DESCRIPTIONS);
					pChild->SetText("Choose your starting skills.(Magics cost two points from the any skill pool)");
				
					pChild = GetChild(IDC_SELECTBACKGROUND);
					pChild->SetText("Unset Background");
				}
			}
			else
			{
				pWin->Show();
				HideSkills();
				ResetTarget();
				ZSWindow *pBegin;
				pBegin = GetChild(IDC_BEGIN);
				pBegin->Hide();
				
				pChild = GetChild(IDC_SELECTBACKGROUND);
				pChild->SetText("Select Background");
					
			}
		}
		else
		if(IDFrom == IDC_BEGIN)
		{
			char *Name;
			ZSWindow *pName;
			pName = GetChild(IDC_NAME);
			Name = pName->GetText();

			if(Name && strlen(Name) > 2)
			{
				char *FinalName;
				FinalName = new char[strlen(Name) + 1];
				strcpy(FinalName, Name);
				pTarget->SetData(INDEX_NAME, FinalName);
				//PreludeParty.AddMember(pTarget);
				//PreludeParty.SetLeader(pTarget);
				ZSOldList *pWin;
				pWin = (ZSOldList *)GetChild(IDC_BACKGROUNDS);
				int BNum;
				BNum = pWin->GetSelection();
				if(BNum != -1)
				{
					Push(pTarget);
					Push(PortraitNum);
					Backgrounds[BNum].Finished.Process();
				}
				State = WINDOW_STATE_DONE;
				ReturnCode = TRUE;
				char FileName[64];
				if(pTarget->GetData(INDEX_SEX).Value)
				{
				//woman
				strcpy(FileName,"woman");
				}
				else
				{
				//man
				strcpy(FileName,"man");
				}
				char temp[8];
				sprintf(temp,"%i",PortraitNum);
				strcat(FileName,temp);

				char *PName = new char[strlen(FileName) + 1];
				strcpy(PName,FileName);
				pTarget->SetData(INDEX_PORTRAIT,PName);
			}
			else
			{
				pName = GetChild(IDC_DESCRIPTIONS);
				pName->SetText("You must enter a name.");
			}
		}
		else
		if(IDFrom >= IDC_SKILLNAME1 && IDFrom < IDC_SKILLNAME1 + NumSkills)
		{
			int SkillIndex;
			SkillIndex = (IDFrom - IDC_SKILLNAME1) + StartSkill;
			if(pTarget->GetData(SkillIndex).Value)
			{
				RemoveSkill(SkillIndex);
			}
			else
			{
				AddSkill(SkillIndex);
			}
		}
		if(IDFrom == IDC_HAIR)
		{
			int CurHair;
			CurHair = pTarget->GetData(INDEX_HAIR).Value;
			CurHair++;
			if(CurHair == 3 ||
				CurHair == 5 ||
				CurHair == 7 ||
				CurHair == 9 ||
				CurHair == 11)
			{
				CurHair++;
			}
			if(CurHair == 12)
			{
				CurHair++;
			}

			if(CurHair > 13)
			{
				CurHair = 0;
			}

			pTarget->SetData(INDEX_HAIR,CurHair);
			ZSWindow *pWin;
			pWin = GetChild(IDC_HAIR);
			switch(CurHair)
			{
			case 0:
				pWin->SetText("Blond");
				break;
			case 1:
				pWin->SetText("Lt Brown");
				break;
			case 2:
				pWin->SetText("Brown");
				break;
			case 4:
				pWin->SetText("White");
				break;
			case 6:
				pWin->SetText("Red");
				break;
			case 8:
				pWin->SetText("Black");
				break;
			case 10:
				pWin->SetText("Grey");
				break;
			case 13:
				pWin->SetText("Bald");
				break;
			}
		

		}
	}
	else
	if(Command == COMMAND_LIST_RIGHTCLICKED  ||
		Command == COMMAND_LIST_SELECTIONCHANGED)
	{
		if(IDFrom == IDC_BACKGROUNDS)
		{
			pChild = GetChild(IDC_DESCRIPTIONS);
			pChild->SetText(Backgrounds[Param].Description);
		}
	}
	else
	if(Command == COMMAND_LIST_SELECTED)
	{
		if(GetChild(IDC_SELECTBACKGROUND)->IsVisible())
		{
			ZSOldList *pWin;
			pWin = (ZSOldList *)GetChild(IDC_BACKGROUNDS);
			if(pWin && pWin->IsVisible())
			{
				int BNum;
				BNum = pWin->GetSelection();
				if(BNum != -1)
				{
					StoreTarget();
					Push(pTarget);
					Backgrounds[BNum].SelectResult.Process();
					//only show begin after all skill points are spent
					//pBegin = GetChild(IDC_BEGIN);
					//pBegin->Show();
					ScriptArg *SA;

					SA = Pop();
					MaxCombat = CurCombat = (int)SA->GetValue();
					delete SA;

					SA = Pop();
					MaxNon = CurNon = (int)SA->GetValue();
					delete SA;

					SA = Pop();
					MaxFree = CurFree = (int)SA->GetValue();
					delete SA;

					for(int n = 0; n < NumSkills; n++)
					{
						StoredSkills[n] = pTarget->GetData(n + StartSkill).Value;		
					}
					ShowSkills();
					ZSWindow *pChild;
					int StatStart;
					StatStart = pTarget->GetIndex("STRENGTH");
					for(n = 0; n < 7; n ++)
					{
						pChild = GetChild(IDC_STAT1 + n);
						pChild->SetText(pTarget->GetData(n + StatStart).Value);
					}
					ShowSkills();
					pChild = GetChild(IDC_DESCRIPTIONS);
					pChild->SetText("Choose your starting skills.(Magics cost two points from the any skill pool)");
				
					pChild = GetChild(IDC_SELECTBACKGROUND);
					pChild->SetText("Unset Background");
								
				}
			}
		}
	}
	else
	if(Command == COMMAND_BUTTON_RIGHTCLICKED)
	{
		
		pChild = GetChild(IDC_DESCRIPTIONS);
		if(IDFrom == IDC_RACE)
		{
			char *Name;
			char *HelpText;
			ZSButton *pButton;
			pButton = (ZSButton *)GetChild(IDFrom);
			Name = pButton->GetText();
			HelpText = GetHelp(Name);	
			pChild->SetText(HelpText);
			if(HelpText)
				delete[] HelpText;
		}
		
		if(IDFrom >= IDC_SNAME1 && IDFrom <= IDC_SNAME7)
		{
			ZSWindow *pStatName;
			char *Name;
		
			char *HelpText;
			pStatName = GetChild(IDFrom);
			Name = pStatName->GetText();
			HelpText = GetHelp(Name);	
			pChild->SetText(HelpText);
			if(HelpText)
				delete[] HelpText;		
		}
		else
		if(IDFrom >= IDC_SKILLNAME1 && IDFrom < IDC_SKILLNAME1 + NumSkills)
		{
			ZSWindow *pSkillName;
			char *Name;
			char *HelpText;
			pSkillName = GetChild(IDFrom);
			Name = pSkillName->GetText();
			HelpText = GetHelp(Name);	
			pChild->SetText(HelpText);
			if(HelpText)
				delete[] HelpText;		
		}
	}

	return  TRUE;
}

int CreatePartyWin::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ShowHelp("Party Creation");
		return TRUE;
	}
	
	if(CurrentKeys[DIK_ESCAPE] & 0x80)
	{
		State = WINDOW_STATE_DONE;
		ReturnCode = FALSE;
	}

	return TRUE;
}

int CreateCharacterWin::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ShowHelp("Character Creation");
		return TRUE;
	}
	
	if(CurrentKeys[DIK_ESCAPE] & 0x80)
	{
		State = WINDOW_STATE_DONE;
		ReturnCode = FALSE;
	}
	return TRUE;
}

	
CreateCharacterWin::CreateCharacterWin(Creature *pToCreate)
{
	pTarget = pToCreate;
	CreationPoints = 10;
	ID = CREATE_CHARACTER_ID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Bounds.left = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.top = 0;
	Bounds.bottom = Engine->Graphics()->GetHeight();
	Moveable = FALSE;

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("charcreation.bmp",800,600,NULL,NULL);

	RECT rPLeft;
	RECT rPRight;
	RECT rPortrait;
	RECT rName;
	RECT rContinue;
	RECT rStat[7];
	RECT rStatName[7];
	RECT rStatPlus[7];
	RECT rStatMinus[7];
	RECT rBackgrounds;
	RECT rDescribe;
	RECT rSex;
	RECT rRace;
	RECT rHair;
	RECT rHeight;
	RECT rWeight;

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	Border = 0;

	//get our based stats
	pFlameBase = (Creature *)Thing::Find(Creature::GetFirst(), "FlameBase");
	pRiverBase = (Creature *)Thing::Find(Creature::GetFirst(), "RiverBase");
	pMixedBase = (Creature *)Thing::Find(Creature::GetFirst(), "MixedBase");

	StartSkill = pFlameBase->GetIndex("SWORD");
	EndSkill = pFlameBase->GetIndex("THAUMATURGY");
	NumSkills = (EndSkill - StartSkill) + 1;
	int LastCombat;
	LastCombat = pFlameBase->GetIndex("ARMOR");
	NumCombat = LastCombat - StartSkill;
	NumNonMagic = pFlameBase->GetIndex("NATURE") - StartSkill;

	FILE *fp;

	//set up the backgrounds
	fp = SafeFileOpen("backgrounds.txt","rt");

	//count them
	NumBackgrounds = 0;

	while(SeekTo(fp,"Name:"))
	{
		NumBackgrounds++;
	}

	fclose(fp);

	Backgrounds = new CharacterBackground[NumBackgrounds];

	fp = SafeFileOpen("backgrounds.txt","rt");

	for(int n = 0; n < NumBackgrounds; n++)
	{
		SeekTo(fp,"Name:");
		Backgrounds[n].Name = GetStringNoWhite(fp);
		SeekTo(fp,"Requirements:");
		SeekTo(fp,"(");
		Backgrounds[n].Requirements.Import(fp);
		SeekTo(fp,"Description:");
		SeekTo(fp,"[");
		Backgrounds[n].Description = GetString(fp,']');
		SeekTo(fp,"SelectResult:");
		SeekTo(fp,"(");
		Backgrounds[n].SelectResult.Import(fp);
		SeekTo(fp,"Finished:");
		SeekTo(fp,"(");
		Backgrounds[n].Finished.Import(fp);
	}

	fclose(fp);	
	
	fclose(fp);

	fp = SafeFileOpen("gui.ini","rt");


	RECT rBase;
	rBase.left = 0;
	rBase.top = 0;
	rBase.bottom = 600;
	rBase.right = 800;

	SeekTo(fp,"PLEFT");
	LoadRect(&rPLeft, fp);
	ScaleRect(&rPLeft, &rBase, &Bounds);

	SeekTo(fp,"PRIGHT");
	LoadRect(&rPRight, fp);
	ScaleRect(&rPRight, &rBase, &Bounds);

	SeekTo(fp,"NAME");
	LoadRect(&rName, fp);
	ScaleRect(&rName, &rBase, &Bounds);

	SeekTo(fp,"CONTINUE");
	LoadRect(&rContinue, fp);
	ScaleRect(&rContinue, &rBase, &Bounds);

	SeekTo(fp,"STAT");
	LoadRect(&rStat[0], fp);
	ScaleRect(&rStat[0], &rBase, &Bounds);

	SeekTo(fp,"PLUS");
	LoadRect(&rStatPlus[0], fp);
	ScaleRect(&rStatPlus[0], &rBase, &Bounds);

	SeekTo(fp,"MINUS");		
	LoadRect(&rStatMinus[0], fp);
	ScaleRect(&rStatMinus[0], &rBase, &Bounds);

	SeekTo(fp,"STATHEIGHT");
	int StatHeight;
	StatHeight = GetInt(fp);

	SeekTo(fp,"BACKGROUNDS");
	LoadRect(&rBackgrounds, fp);
	ScaleRect(&rBackgrounds, &rBase, &Bounds);

	SeekTo(fp, "DESCRIBE");
	LoadRect(&rDescribe, fp);
	ScaleRect(&rDescribe, &rBase, &Bounds);

	SeekTo(fp, "PORTRAIT");
	LoadRect(&rPortrait, fp);
	ScaleRect(&rPortrait, &rBase, &Bounds);

	SeekTo(fp, "SEX");
	LoadRect(&rSex, fp);
	ScaleRect(&rSex, &rBase, &Bounds);

	SeekTo(fp, "RACE");
	LoadRect(&rRace, fp);
	ScaleRect(&rRace, &rBase, &Bounds);

	SeekTo(fp, "HAIR");
	LoadRect(&rHair, fp);
	ScaleRect(&rHair, &rBase, &Bounds);

	SeekTo(fp, "HEIGHT");
	LoadRect(&rHeight, fp);
	ScaleRect(&rHeight, &rBase, &Bounds);

	SeekTo(fp, "WEIGHT");
	LoadRect(&rWeight, fp);
	ScaleRect(&rWeight, &rBase, &Bounds);

	int HeightThousands;

	SeekTo(fp, "HEIGHTVARIANCE");
	HeightThousands = GetInt(fp);

	HeightVariance = (float)HeightThousands / 1000.f;
		
	fclose(fp);

	StatHeight = (int)((float)StatHeight * (float)Bounds.bottom / 600.0f);

	rStatName[0] = rStat[0];
	rStatName[0].top += 8;
	rStatName[0].left -= 128;
	rStatName[0].right -= 40;

	for(n = 1; n < 7; n++)
	{
		rStat[n] = rStat[n - 1];
		rStat[n].top += StatHeight;
		rStat[n].bottom += StatHeight;

		rStatMinus[n] = rStatMinus[n - 1];
		rStatMinus[n].top += StatHeight;
		rStatMinus[n].bottom += StatHeight;
		
		rStatPlus[n] = rStatPlus[n - 1]; 
		rStatPlus[n].top += StatHeight;
		rStatPlus[n].bottom += StatHeight;
		
		rStatName[n] = rStatName[n - 1];
		rStatName[n].top += StatHeight;
		rStatName[n].bottom += StatHeight;
	}
	
	//pTarget = new Creature(pRiverBase);
	
	//create all the buttons for stats
	ZSButton *pButton;
	ZSText *pText;

	int StatStart;

	StatStart = pFlameBase->GetIndex("STRENGTH");
	
	char temp[16];
	char StatName[64];
	
	for(n = 0; n < 7; n++)
	{
		//name
		strcpy(StatName,pFlameBase->GetName(StatStart + n));
		ConvertToLowerCase(&StatName[1]);
		pButton = new ZSButton(BUTTON_NONE, IDC_SNAME1 + n, rStat[n].left - 8 - Engine->Graphics()->GetFontEngine()->GetTextWidth(StatName), 
									rStat[n].top + 8, Engine->Graphics()->GetFontEngine()->GetTextWidth(pFlameBase->GetName(StatStart + n)),Engine->Graphics()->GetFontEngine()->GetTextHeight());
		pButton->Show();
		pButton->SetText(StatName);
		pButton->SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
		AddChild(pButton);

		sprintf(temp,"%i",pFlameBase->GetData(StatStart + n).Value);
		pText = new ZSText(IDC_STAT1 + n, XYWH(rStat[n]), temp, 0, TRUE);
		pText->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
		pText->Show();
		AddChild(pText);

		pButton = new ZSButton(BUTTON_PLUS, IDC_PLUS1 + n, XYWH(rStatPlus[n]));
		pButton->Show();
		AddChild(pButton);

		pButton = new ZSButton(BUTTON_MINUS, IDC_MINUS1 + n, XYWH(rStatMinus[n]));
		pButton->Show();
		AddChild(pButton);
	}

	pText = new ZSText(IDC_STATPOINTS, rStat[6].left, rStat[6].top + StatHeight,"    ");
	pText->Show();
	AddChild(pText);
	pText->SetText(10);

	int StatIDWidth;
	StatIDWidth = Engine->Graphics()->GetFontEngine()->GetTextWidth("Points:  ");
	pText = new ZSText(IDC_STATPOINT_ID, rStat[6].left - StatIDWidth, rStat[6].top + StatHeight,"Points:");
	pText->Show();
	AddChild(pText);
	
	//continue button
	pButton = new ZSButton(BUTTON_NORMAL, IDC_BEGIN, XYWH(rContinue));
	pButton->Hide();
	pButton->SetText("Begin");
	AddChild(pButton);

	ZSOldList *pList;

	pList = new ZSOldList(IDC_BACKGROUNDS, XYWH(rBackgrounds), TRUE);
	pList->Show();
	AddChild(pList);
	
	for(n = 0; n < NumBackgrounds; n++)
	{
		pList->AddItem(Backgrounds[n].Name);
	}

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SELECTBACKGROUND, rBackgrounds.left, rBackgrounds.bottom + 8, rBackgrounds.right - rBackgrounds.left, 20);
	pButton->SetText("Select Background");
	pButton->Hide();
	AddChild(pButton);
	
	//GetValidProfessions();
	pList->SetSelection(-1);

	pText = new ZSText(IDC_SEX_ID, rSex.left, rSex.top, "Sex:");
	pText->Show();
	AddChild(pText);

	pText = new ZSText(IDC_RACE_ID, rRace.left, rRace.top, "Bloodline:");
	pText->Show();
	AddChild(pText);

	pText = new ZSText(IDC_HAIR_ID, rHair.left, rHair.top, "Hair:");
	pText->Show();
	AddChild(pText);

/*
	pText = new ZSText(IDC_HEIGHT, XYWH(rHeight),"Height: ",0,FALSE);
	pText->Show();
	AddChild(pText);

	pText = new ZSText(IDC_WEIGHT, XYWH(rWeight),"Weight: ",0,FALSE);
	pText->Show();
	AddChild(pText);
*/
	
	int Width;
//	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Femaler");
	rSex.top += 20;
	rSex.bottom += 20;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_SEX, XYWH(rSex));
	pButton->SetText("Male");
	pButton->Show();
	AddChild(pButton);

	int NewWidth;
	rRace.top += 20;
	rRace.bottom += 20;
	NewWidth = Engine->Graphics()->GetFontEngine()->GetTextWidth("Riverrr");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_RACE, XYWH(rRace));
	pButton->SetText("River");
	pButton->Show();
	AddChild(pButton);

	rHair.top += 20;
	rHair.bottom += 20;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_HAIR, XYWH(rHair));
	pButton->SetText("Blond");
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_EAST, IDC_PORTRAIT_RIGHT, XYWH(rPRight));
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_WEST, IDC_PORTRAIT_LEFT, XYWH(rPLeft));
	pButton->Show();
	AddChild(pButton);

	ZSWindow *pWin;
	pWin = new ZSWindow(WINDOW_TEXT, IDC_DESCRIPTIONS, XYWH(rDescribe));
	pWin->SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
	pWin->SetText("Right click for information about an attribute, skill, or background.");
	pWin->Show();
	pWin->SetMoveable(FALSE);
	AddChild(pWin);


	PortraitNum = 1;

	pWin = new ZSWindow(WINDOW_NONE, IDC_PORTRAIT, XYWH(rPortrait));
	pWin->Show();
	pWin->SetMoveable(FALSE);
	AddChild(pWin);

	//SetPortrait();
	
	ZSEdit *pName;

	pName = new ZSEdit(WINDOW_EDIT, IDC_NAME, XYWH(rName));
	pName->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	pName->CreateParchmentBorderedBackground(4);
	pName->Show();
	AddChild(pName);

	RECT rSkillName;
	RECT rSkillValue;
	rSkillName.left = rBackgrounds.left + 38;
	rSkillName.right = rBackgrounds.right;
	rSkillValue.right = rBackgrounds.left + 36;
	rSkillValue.left = rBackgrounds.left;
	
	pText = new ZSText(IDC_SKILL_POINTS, rSkillValue.left, rBackgrounds.top - 36, 20, 18, "0",0,FALSE);
	AddChild(pText);
	
	pButton = new ZSButton(BUTTON_NONE, IDC_SKILL, rSkillValue.left+22, rBackgrounds.top - 36, (rBackgrounds.right - rBackgrounds.left) - 38,18);
	pButton->SetText("Any Skill");
	AddChild(pButton);

	pText = new ZSText(IDC_COMBAT_POINTS, rSkillValue.left, rBackgrounds.top - 18, 20, 20, "0",0,FALSE);
	AddChild(pText);
	
	pButton = new ZSButton(BUTTON_NONE, IDC_COMBAT, rSkillValue.left + 22, rBackgrounds.top - 18, (rBackgrounds.right - rBackgrounds.left) - 38,18);
	pButton->SetText("Combat");
	AddChild(pButton);

	for(n = 0; n <= NumCombat; n++)
	{
		//create each skill name
		rSkillValue.top  = rSkillName.top = 20 * n +rBackgrounds.top;
		rSkillValue.bottom = rSkillName.bottom = rSkillName.top + 24;
		
		strcpy(StatName, pFlameBase->GetName(StartSkill + n));
		ConvertToLowerCase(&StatName[1]);

		pButton = new ZSButton(BUTTON_NONE, IDC_SKILLNAME1 + n, XYWH(rSkillName));
		
		pButton->SetText(StatName);
		
		AddChild(pButton);

		pText = new ZSText(IDC_SKILL1 + n, XYWH(rSkillValue), "",0,TRUE);
		AddChild(pText);
		pText->SetText(pFlameBase->GetData(StartSkill+n).Value);
	}

	rSkillValue.top  = rSkillName.top = 20 * n +rBackgrounds.top;
	rSkillValue.bottom = rSkillName.bottom = rSkillName.top + 24;

	pText = new ZSText(IDC_NONCOMBAT_POINTS, rSkillValue.left, rSkillName.top + 2, 20, 20, "0",0,FALSE);
	AddChild(pText);

	pButton = new ZSButton(BUTTON_NONE, IDC_NONCOMBAT, rSkillValue.left + 22, rSkillName.top + 2, (rBackgrounds.right - rBackgrounds.left) - 38,18);
	pButton->SetText("NonCombat");
	pButton->Show();
	AddChild(pButton);
	
	for(; n < NumSkills; n++)
	{
		//create each skill name
		rSkillValue.top  = rSkillName.top = 20 * (n + 1) +rBackgrounds.top;
		rSkillValue.bottom = rSkillName.bottom = rSkillName.top + 24;
		pButton = new ZSButton(BUTTON_NONE, IDC_SKILLNAME1 + n, XYWH(rSkillName));

		strcpy(StatName, pFlameBase->GetName(StartSkill + n));
		ConvertToLowerCase(&StatName[1]);
		
		pButton->SetText(StatName);
		AddChild(pButton);

		pText = new ZSText(IDC_SKILL1 + n, XYWH(rSkillValue), "",0,TRUE);
		AddChild(pText);
		pText->SetText(pFlameBase->GetData(StartSkill+n).Value);
	}

	HideSkills();

	fp = SafeFileOpen("getskills.txt","rt");

	SeekTo(fp,"#GetSkillValue#");
		
	GetSkillValueBlock.Import(fp);

	fclose(fp);

	//quit

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Return");

	pButton = new ZSButton(BUTTON_NORMAL, IDC_QUIT, 0,0,Width, 20);
	pButton->Show();
	pButton->SetText("Return");
	AddChild(pButton);
}

int CreateCharacterWin::RightButtonDown(int x, int y)
{
	ZSWindow *pWin;
	
	//pass the message along to any child beneath the cursor
	pWin = GetChild(x,y);
	if(pWin)
	{
		if(pWin->GetType() == WINDOW_TEXT)
	{
		if(pWin->GetID() >= IDC_STAT1 && pWin->GetID() <= IDC_STAT7)
		{
			int i = 0;
			i = atoi(pWin->GetText());
			ZSWindow *pChild;
			pChild = GetChild(IDC_DESCRIPTIONS);
			switch(i)
			{
			case 1: case 2: case 3: case 4: case 5:
				pChild->SetText("Couldn't be much worse and still alive");
				break;
			case 6: case 7:
				pChild->SetText("Awful");
				break;
			case 8: case 9:
				pChild->SetText("Bad");
				break;
			case 10: case 11:
				pChild->SetText("Below average");
				break;
			case 12:	case 13: case 14:
				pChild->SetText("Average");
				break;
			case 15: case 16:
				pChild->SetText("Above average");
				break;
			case 17: case 18:
				pChild->SetText("Good");
				break;
			case 19: case 20:
				pChild->SetText("Very Good");
				break;
			case 21: case 22:
				pChild->SetText("Exceptional");
				break;
			case 23: case 24: case 25:
				pChild->SetText("Super Human");
				break;
			default:
				pChild->SetText("God Like");
				break;
			}
		}
	}
		return pWin->RightButtonDown(x,y);
	}
	else
	{
		SetFocus(this);
	}
	return TRUE;
}

int CreateCharacterWin::RightButtonUp(int x, int y)
{
	
	ReleaseFocus();
	return TRUE;
}
	
void CreateCharacterWin::ShowSkills()
{
	ZSWindow *pText;

	pText = GetChild(IDC_BACKGROUNDS);
	pText->Hide();

	for(int n = 0; n < NumSkills; n++)
	{
		pText = GetChild(IDC_SKILLNAME1 + n);
		pText->Show();
		if(pTarget->GetData(StartSkill + n).Value)
		{
			pText->SetTextColor(TEXT_WHITE);
		}
		else
		{
			pText->SetTextColor(TEXT_DARK_GREY);
		}
		
		pText = GetChild(IDC_SKILL1 + n);
		pText->Show();
		pText->SetText(pTarget->GetData(StartSkill + n).Value);
		if(pTarget->GetData(StartSkill +n).Value)
		{
			pText->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
		}
		else
		{
			pText->SetTextColor(TEXT_LIGHT_GREY_PARCHMENT);
		}
	}

	for(n = 0; n < 7; n++)
	{
		pText = GetChild(IDC_PLUS1 + n);
		pText->Hide();
		pText = GetChild(IDC_MINUS1 + n);
		pText->Hide();
	}

	pText = GetChild(IDC_SKILL_POINTS);
	pText->Show();
	pText->SetText(CurFree);
	
	pText = GetChild(IDC_COMBAT_POINTS);
	pText->Show();
	pText->SetText(CurCombat);
	
	pText = GetChild(IDC_NONCOMBAT_POINTS);
	pText->Show();
	pText->SetText(CurNon);

	pText = GetChild(IDC_SKILL);
	pText->Show();
	
	pText = GetChild(IDC_COMBAT);
	pText->Show();
	
	pText = GetChild(IDC_NONCOMBAT);
	pText->Show();

}

void CreateCharacterWin::HideSkills()
{
	ZSWindow *pText;

	pText = GetChild(IDC_BACKGROUNDS);
	pText->Show();

	for(int n = 0; n < NumSkills; n++)
	{
		pText = GetChild(IDC_SKILLNAME1 + n);
		pText->Hide();
		
		pText = GetChild(IDC_SKILL1 + n);
		pText->Hide();
	}

	for(n = 0; n < 7; n++)
	{
		pText = GetChild(IDC_PLUS1 + n);
		pText->Show();
		pText = GetChild(IDC_MINUS1 + n);
		pText->Show();
	}

	pText = GetChild(IDC_SKILL_POINTS);
	pText->Hide();
	
	pText = GetChild(IDC_COMBAT_POINTS);
	pText->Hide();
	
	pText = GetChild(IDC_NONCOMBAT_POINTS);
	pText->Hide();

	pText = GetChild(IDC_SKILL);
	pText->Hide();
	
	pText = GetChild(IDC_COMBAT);
	pText->Hide();
	
	pText = GetChild(IDC_NONCOMBAT);
	pText->Hide();

}

void CreateCharacterWin::StoreTarget()
{
	int StatStart;
	StatStart = pTarget->GetIndex("STRENGTH");
	for(int n = 0; n < 7; n ++)
	{
		StoredStats[n] = pTarget->GetData(StatStart + n).Value;
	}
}

void CreateCharacterWin::ResetTarget()
{
	ZSWindow *pWin;
	int StatStart;
	StatStart = pTarget->GetIndex("STRENGTH");
	for(int n = 0; n < 7; n ++)
	{
		pTarget->SetData(StatStart + n, StoredStats[n]);
		pWin = GetChild(IDC_STAT1 + n);
		pWin->SetText(StoredStats[n]);
		pWin->SetTextColor(TEXT_DARK_GREY_PARCHMENT);
	}

	for(n = StartSkill; n <= EndSkill; n++)
	{
		pTarget->SetData(n,0);
	}
}

void CreateCharacterWin::AddSkill(int Num)
{
	//where in the range does is fall
	ZSWindow *pWin;
	if(Num - StartSkill <= NumCombat)
	{
		if(CurCombat)
		{
			CurCombat--;
			Push(pTarget);
			Push(Num);
			GetSkillValueBlock.Process();
			ScriptArg *SA;
			SA = Pop();
			pTarget->SetData(Num, (int)SA->GetValue());
		}
		else
		{
			if(CurFree)
			{
				CurFree--;
				Push(pTarget);
				Push(Num);
				GetSkillValueBlock.Process();
				ScriptArg *SA;
				SA = Pop();
				pTarget->SetData(Num, (int)SA->GetValue());
			}
			else
			{
				pWin = GetChild(IDC_DESCRIPTIONS);
				pWin->SetText("You do not have enough combat or free picks for another skill");
				return;
			}
		}
	}
	else
	if(Num - StartSkill <= NumNonMagic)
	{
		if(CurNon)
		{
			CurNon--;
			Push(Num);
			Push(pTarget);
			GetSkillValueBlock.Process();
			ScriptArg *SA;
			SA = Pop();
			pTarget->SetData(Num, (int)SA->GetValue());
		}
		else
		{
			if(CurFree)
			{	
				CurFree--;
				Push(Num);
				Push(pTarget);
				GetSkillValueBlock.Process();
				ScriptArg *SA;
				SA = Pop();
				pTarget->SetData(Num, (int)SA->GetValue());
			}
			else
			{
				pWin = GetChild(IDC_DESCRIPTIONS);
				pWin->SetText("You do not have enough noncombat or free picks for another skill");
				return;
			}
		}
	}
	else
	{
		if(CurFree >= 2)
		{
			Push(Num);
			Push(pTarget);
			GetSkillValueBlock.Process();
			ScriptArg *SA;
			SA = Pop();
			pTarget->SetData(Num, (int)SA->GetValue());
			if((int)SA->GetValue())
			{
				CurFree -= 2;
			}
			else
			{
				pWin = GetChild(IDC_DESCRIPTIONS);
				pWin->SetText("Your bloodline is not capable of learning this magic.");
				return;
			}
		}
		else
		{
			pWin = GetChild(IDC_DESCRIPTIONS);
			pWin->SetText("You do not have enough free picks to chooose a magic");
			return;
		}
	}

	pWin = GetChild(IDC_SKILL1 + (Num - StartSkill));
	pWin->SetText(pTarget->GetData(Num).Value);
	ShowSkills();
	pWin = GetChild(IDC_BEGIN);
	if(!CurFree && !CurCombat && !CurNon)
	{
		pWin->Show();
	}
	else
	{
		pWin->Hide();
	}

}

void CreateCharacterWin::Show()
{
	State = WINDOW_STATE_NORMAL;
	CreationPoints = 10;
	StoreTarget();
	ResetTarget();
	SetPortrait();
	HideSkills();
	
	ZSWindow *pWin;

	pWin = GetChild(IDC_STATPOINTS);
	pWin->SetText(CreationPoints);
	
	pWin = GetChild(IDC_RACE);
	pWin->SetText("River");
		
	pWin = GetChild(IDC_SELECTBACKGROUND);
	pWin->SetText("Select Background");
	pWin->Hide();

	pWin = GetChild(IDC_BEGIN);
	pWin->Hide();

	pWin = GetChild(IDC_NAME);
	pWin->SetText("");
	
	pWin = GetChild(IDC_DESCRIPTIONS);
	pWin->SetText("Right click for information about an attribute, skill, or background.  Press F1 for additional help.");
	
	Visible = TRUE;

	if(!PreludeWorld->GetCreationHelp())
	{
		PreludeWorld->SetCreationHelp(TRUE);
		ShowHelp("Character Creation");
	}

	int CurHair;
	CurHair = pTarget->GetData(INDEX_HAIR).Value;
	CurHair++;
	if(CurHair == 3 ||
		CurHair == 5 ||
		CurHair == 7 ||
		CurHair == 9 ||
		CurHair == 11)
	{
		CurHair++;
	}
	if(CurHair == 12)
	{
		CurHair++;
	}

	if(CurHair > 13)
	{
		CurHair = 0;
	}

	pTarget->SetData(INDEX_HAIR,CurHair);
	pWin = GetChild(IDC_HAIR);
	switch(CurHair)
	{
	case 0:
		pWin->SetText("Blond");
		break;
	case 1:
		pWin->SetText("Lt Brown");
		break;
	case 2:
		pWin->SetText("Brown");
		break;
	case 4:
		pWin->SetText("White");
		break;
	case 6:
		pWin->SetText("Red");
		break;
	case 8:
		pWin->SetText("Black");
		break;
	case 10:
		pWin->SetText("Grey");
		break;
	case 13:
		pWin->SetText("Bald");
		break;
	}
}

void CreateCharacterWin::RemoveSkill(int Num)
{
	ZSWindow *pWin;
	if(StoredSkills[Num - StartSkill])
	{
		pWin = GetChild(IDC_DESCRIPTIONS);
		pWin->SetText("You cannot change a background specific skill.");
		return;
	}

	//where in the range does is fall
	if((Num - StartSkill) <= NumCombat)
	{
		if(CurCombat < MaxCombat)
		{
			CurCombat++;
			pTarget->SetData(Num, 0);
		}
		else
		if(CurFree < MaxFree)
		{
			CurFree++;
			pTarget->SetData(Num, 0);
		}
	}
	else
	if((Num - StartSkill) <= NumNonMagic)
	{
		if(CurNon < MaxNon)
		{
			CurNon++;
			pTarget->SetData(Num, 0);
		}
		else
		if(CurFree < MaxFree)
		{
			CurFree++;
			pTarget->SetData(Num, 0);
		}
	}
	else
	{
		if(CurFree + 1 < MaxFree)
		{
			CurFree += 2;
			pTarget->SetData(Num, 0);
		}
	}
	pWin = GetChild(IDC_SKILL1 + (Num - StartSkill));
	pWin->SetText(pTarget->GetData(Num).Value);
	ShowSkills();
	
	pWin = GetChild(IDC_BEGIN);
	if(!CurFree && !CurCombat && !CurNon)
	{
		pWin->Show();
	}
	else
	{
		pWin->Hide();
	}
}
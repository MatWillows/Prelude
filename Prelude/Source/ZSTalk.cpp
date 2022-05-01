#include "zstalk.h"
#include "zsEngine.h"
#include "zsListbox.h"
#include "zsdescribe.h"
#include "zsutilities.h"
#include "translucentwindow.h"
#include "world.h"
#include "party.h"
#include "things.h"
#include "zshelpwin.h"

LPDIRECTDRAWSURFACE7 ZSTalkWin::TalkSurface = NULL;

int ZSTalkWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_LIST_SELECTED)
	{
		if(IDFrom == IDC_REPLY)
		{
			char GotoWord[256];
			ZSList *pWin;
			ZSDescribe *pSay;
			pSay = (ZSDescribe *)GetChild(IDC_SAY);
			pSay->Clear();
			pWin = (ZSList *)GetChild(IDC_REPLY);
			pWin->GetText(Param, GotoWord);
			pWin->RemoveItem(Param);
			ScriptContextBlock->FindLabel(GotoWord)->Process();
			DEBUG_INFO("going to word: ");
			DEBUG_INFO(GotoWord);
			DEBUG_INFO("\n");
		}
	}
	return TRUE;
}

ZSTalkWin::ZSTalkWin(int NewID, int x, int y, int Width, int Height, ScriptBlock *ContextBlock)
{
	RECT rBase;
	RECT rCurrent;
	
	rBase.left = 0;
	rBase.right = 800;
	rBase.top = 0;
	rBase.bottom = 600;
	
	rCurrent.left = 0;
	rCurrent.top = 0;
	rCurrent.right = Engine->Graphics()->GetWidth();
	rCurrent.bottom = Engine->Graphics()->GetHeight();

	
	Type = WINDOW_TALK;
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	Bounds.left = x - 16;
	Bounds.right = x + Width + 16;
	Bounds.top = y - 16;
	Bounds.bottom = y + Height + 16;

	ScaleRect(&Bounds, &rBase, &rCurrent);

	Cursor = CURSOR_POINT;

	if(!TalkSurface)
	{
		CreateWoodBorderedBackground(12,2);
		TalkSurface = BackGroundSurface;
	}

	BackGroundSurface = TalkSurface;
	BackGroundSurface->AddRef();

/*	BackGroundSurface = Engine->Graphics()->CreateSurface(Bounds.right - Bounds.left, Bounds.bottom - Bounds.top, NULL, RGB(255,0,255));

	RECT ToFill;
	ToFill.left = 8;
	ToFill.right = Bounds.right - Bounds.left - 8;
	ToFill.top = 8;
	ToFill.bottom = Bounds.bottom - Bounds.top - 8; 
	Engine->Graphics()->FillSurface(BackGroundSurface, RGB(255,0,255), NULL);
	Engine->Graphics()->FillSurface(BackGroundSurface, 0, &ToFill);
	AddBorder(12,3);
*/
	
	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	Border = 0;
	ZSWindow *pWin;

	FILE *fp;

	fp = SafeFileOpen ("gui.ini","rt");

	SeekTo(fp, "[TALK]");

	RECT rSay;
	RECT rReply;
	RECT rPlayerPortrait;
	RECT rNPCPortrait;

	//create the four windows
	//npc portrait	
	SeekTo(fp, "NPCPORTRAIT");

	LoadRect(&rNPCPortrait, fp);
	ScaleRect(&rNPCPortrait, &rBase, &rCurrent);

	
	pWin = new ZSWindow(WINDOW_PORTRAIT, IDC_NPC_PORTRAIT, XYWH(rNPCPortrait));
	pWin->CreateEmptyBorderedBackground(8);
	pWin->Show();
	//pWin->SetText((char *)ContextBlock->GetArg(0)->GetValue());
	AddChild(pWin);


	
	//say box
	SeekTo(fp, "SAY");

	LoadRect(&rSay,fp);
	ScaleRect(&rSay, &rBase, &rCurrent);
	
	pWin = new ZSDescribe(IDC_SAY, XYWH(rSay),TRUE);
	pWin->Show();
	AddChild(pWin);
	
	//reply
	SeekTo(fp, "REPLY");

	LoadRect(&rReply,fp);
	ScaleRect(&rReply, &rBase, &rCurrent);

	
	pWin = new ZSList(IDC_REPLY, XYWH(rReply));
	pWin->Show();
	pWin->SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
	AddChild(pWin);

//	pTrans = new TransWin(pWin, D3DRGBA(0.0f, 0.0f, 0.0f, 0.25f), 0);
//	PreludeWorld->AddMainObject((Object *)pTrans);

	//player
	SeekTo(fp, "PLAYERPORTRAIT");

	LoadRect(&rPlayerPortrait,fp);
	ScaleRect(&rPlayerPortrait, &rBase, &rCurrent);

	pWin = new ZSWindow(WINDOW_PORTRAIT, IDC_PLAYER_PORTRAIT, XYWH(rPlayerPortrait));
	pWin->SetBorderWidth(8);
	if(PreludeParty.GetLeader())
	{
		pWin->CreatePortrait(PreludeParty.GetLeader()->GetData(INDEX_PORTRAIT).String);
		pWin->SetText(PreludeParty.GetLeader()->GetData(INDEX_NAME).String);
	}
	pWin->AddBorder(8);
	pWin->Show();
	AddChild(pWin);
	
	fclose(fp);

	PrevContextBlock = ScriptContextBlock;
	PrevContextWindow = ScriptContextWindow;

	ScriptContextBlock = ContextBlock;
	ScriptContextWindow = this;

	return; 

}


ZSTalkWin::~ZSTalkWin()
{
	if(ScriptContextWindow == this)
	{
		ScriptContextBlock = PrevContextBlock;
		ScriptContextWindow = PrevContextWindow;
	}
	//PreludeWorld->RemoveMainObject(pTrans);
	//delete pTrans;
}

int ZSTalkWin::GoModal()
{
	State = WINDOW_STATE_NORMAL;

	ScriptBlock *BeginBlock;

	AddWord("Goodbye");

	BeginBlock = ScriptContextBlock->FindLabel("begin");

	BeginBlock->Process();

	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		pMainWindow->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
#ifdef AUTOTEST
		ZSList *pList;
		pList = (ZSList *)this->GetChild(IDC_REPLY);
		int randitem;
				char GotoWord[256];
		do
		{
			randitem = rand() %  pList->GetNumItems();
	
			pList->GetText(randitem,GotoWord);
		}	while(GotoWord[0] == '\0');
		this->Command(IDC_REPLY, COMMAND_LIST_SELECTED, randitem);
#endif
	
	}
	if(ScriptContextWindow == this)
	{
		ScriptContextBlock = PrevContextBlock;
		ScriptContextWindow = PrevContextWindow;
	}

	return ReturnCode;
}

void ZSTalkWin::AddWord(char *ToAdd)
{
	ZSList *pWin;
	pWin = (ZSList *)GetChild(IDC_REPLY);
	if(pWin->FindItem(ToAdd) == -1)
		pWin->AddItem(ToAdd);
}

void ZSTalkWin::RemoveWord(char *ToRemove)
{
	ZSList *pWin;
	pWin = (ZSList *)GetChild(IDC_REPLY);
	int ItemNum;
	ItemNum = pWin->FindItem(ToRemove);
	if(ItemNum != -1)
		pWin->RemoveItem(ItemNum);
}

void ZSTalkWin::Say(char *ToSay)
{
	ZSDescribe *pWin;
	pWin = (ZSDescribe *)GetChild(IDC_SAY);
//	pWin->Clear();
	pWin->AddItem(ToSay);
	pWin->ResetToTop();
}

void ZSTalkWin::SayDesc(char *ToSay)
{
	ZSDescribe *pWin;
	pWin = (ZSDescribe *)GetChild(IDC_SAY);
	pWin->AddItem(ToSay, TEXT_LIGHT_GREY_PARCHMENT);
	pWin->ResetToTop();
}

void ZSTalkWin::SayClear()
{
	ZSDescribe *pWin;
	pWin = (ZSDescribe *)GetChild(IDC_SAY);
	pWin->Clear();
	pWin->ResetToTop();
}

void ZSTalkWin::SayAdd(char *ToAdd)
{
	ZSDescribe *pWin;
	pWin = (ZSDescribe *)GetChild(IDC_SAY);
	pWin->AddItem(ToAdd);
	pWin->ResetToTop();
}

int ZSTalkWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_Q] & 0x80 && !(LastKeys[DIK_Q] & 0x80))
	{
		State = WINDOW_STATE_DONE;	
	}

	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ShowHelp("Talk Screen");
		return TRUE;
	}

	return TRUE;
}

int ZSTalkWin::RemoveAll()
{
	int NumItems;
	ZSList *pWin;
	pWin = (ZSList *)GetChild(IDC_REPLY);
	NumItems = pWin->GetNumItems();
	
	pWin->ClearExcept("Goodbye");
	return NumItems;
}

int ZSTalkWin::LeftButtonDown(int x, int y)
{
	if(x < Bounds.left || x > Bounds.right ||
		y < Bounds.top || y > Bounds.bottom)
	{
		//	if(pParent)
		//	{
		//		return pParent->LeftButtonDown(x,y);
		//	}
	}

	//check to see if there's child beneath the cursor who should receive the message
	ZSWindow *pWin;
	pWin = GetChild(x,y);
	if(pWin)
	{
		return pWin->LeftButtonDown(x,y);
	}

	//if our cursor is the pointing hand, make the finger depress
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(1);
	}

	//grabe the window and start moving
	SetFocus(this);
	return TRUE; 

}

void ZSTalkWin::SetPortrait(const char *PortraitName)
{
	ZSWindow *pWin;
	if(!PortraitName)
	{
		pWin = GetChild(IDC_NPC_PORTRAIT);
		if(pWin->IsVisible())
		{
			pWin->Hide();
			pWin = GetChild(IDC_SAY);
			RECT rBounds;

			pWin->GetBounds(&rBounds);
			rBounds.left -= 100;
			pWin->SetBounds(&rBounds);
		}
	}
	else
	{
		pWin = GetChild(IDC_NPC_PORTRAIT);
		pWin->CreatePortrait(PortraitName);
		pWin->AddBorder(8);
	}
	//set the portrait to be the specified portrait
}

void Talk(Thing *pWho)
{
	char FileName[32];
	char CharacterID[32];

	static Thing *pLast = NULL;
	static Thing *pLast2 = NULL;
	static Thing *pLast3 = NULL;

	strcpy(FileName,"people.txt");

	sprintf(CharacterID,"#%s#",pWho->GetData(INDEX_NAME).String);

	FILE *fp;
	fp = SafeFileOpen(FileName,"rt");
	
	if(SeekTo(fp,CharacterID))
	{
		fseek(fp, -(strlen(CharacterID) + 1), SEEK_CUR);
	
		//improve leader's speech skill
		if(pWho != pLast && pWho != pLast2 && pWho != pLast3)
		{
			PreludeParty.GetLeader()->ImproveSkill(INDEX_SPEECH);
			pLast3 = pLast2;
			pLast2 = pLast;
			pLast = pWho;
		}
	
		ScriptBlock SB;
		SB.Import(fp);	
		fclose(fp);		
	
		ZSTalkWin *pWin;
		pWin = new ZSTalkWin(-20,125, 125, 550,350, &SB);
		pWin->SetPortrait(pWho->GetData(INDEX_PORTRAIT).String);
		pWin->SetText(pWho->GetData(INDEX_NAME).String);

		pWin->Show();
	
		ZSWindow::GetMain()->AddChild(pWin);
	
		pWin->SetFocus(pWin);

		pWin->GoModal();

		pWin->ReleaseFocus();
		pWin->Hide();
		
		ZSWindow::GetMain()->RemoveChild(pWin);
	//	NextTalkWin--;
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
	
	return; 
}
	

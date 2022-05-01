#include "zsrest.h"
#include "party.h"
#include "zsbutton.h"
#include "zstext.h"
#include "zsutilities.h"
#include "scriptfuncs.h"
#include "world.h"
#include "zshelpwin.h"
#include "events.h"
#include "flags.h"


typedef enum
{
	IDC_REST_NONE,
	IDC_REST_EXIT,
	IDC_REST_REST,
	IDC_REST_NAME1,
	IDC_REST_NAME2,
	IDC_REST_NAME3,
	IDC_REST_NAME4,
	IDC_REST_NAME5,
	IDC_REST_NAME6,
	IDC_REST_ACTION1,
	IDC_REST_ACTION2,
	IDC_REST_ACTION3,
	IDC_REST_ACTION4,
	IDC_REST_ACTION5,
	IDC_REST_ACTION6,
	IDC_REST_RESULT1,
	IDC_REST_RESULT2,
	IDC_REST_RESULT3,
	IDC_REST_RESULT4,
	IDC_REST_RESULT5,
	IDC_REST_RESULT6,
	IDC_REST_PASS1,
	IDC_REST_PASS2,
	IDC_REST_PASS4,
	IDC_REST_PASS8,
} REST_CONTROLS;

int PartyHP[MAX_PARTY_MEMBERS];
int PartyRP[MAX_PARTY_MEMBERS];
int PartyWP[MAX_PARTY_MEMBERS];

LPDIRECTDRAWSURFACE7 ZSRest::RestSurface = NULL;

int ZSRest::Command(int IDFrom, int Command, int Param)
{
	int n;
	char DescribeText[64];
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
		case IDC_REST_EXIT:
			State = WINDOW_STATE_DONE;
			return TRUE;
		case IDC_REST_REST:
			if(!TownName)
			{
				//check for sleep event
				int SleepEvent;

				
				PreludeFlags.Get("GuardAwake")->Value = 0;

				//call any rest overrides here.
				for(n = 0; n < PreludeParty.GetNumMembers(); n++)
				{
					if(PreludeParty.GetRestAction(n) == 1)
					{
						PreludeFlags.Get("GuardAwake")->Value =
						(void *)((int)PreludeFlags.Get("GuardAwake")->Value + 1);
					}	
					if(PreludeParty.GetMember(n))
						PreludeParty.GetMember(n)->SetData(INDEX_BATTLESTATUS, CREATURE_STATE_NONE);
				}

				SleepEvent = PreludeEvents.RandomSleep();
				State = WINDOW_STATE_DONE;
				Hide();
								
				if(!SleepEvent)
				{
					//call any rest overrides here.
					for(n = 0; n < PreludeParty.GetNumMembers(); n++)
					{
						Push(PreludeParty.GetMember(n));
						RestOptions[PreludeParty.GetRestAction(n)].Process();
					}
					RestEvent.Process();
					PreludeEvents.SetLastRandomTime(PreludeEvents.GetLastRandomTime() + 480);
					
					for(n = 0; n < PreludeParty.GetNumMembers(); n++)
					{
						if(PreludeParty.GetMember(n)->GetData(INDEX_MEDICAL).Value)
						{
							PreludeParty.GetMember(n)->ImproveSkill(INDEX_MEDICAL);
						}
					}
				
				}
				else
				{
					int pCurHP;
					int pCurWP;
					int pCurRP;
					int pNewHP;
					int pNewWP;
					int pNewRP;

					//when interrupted, party regains only half of rest values
					for(n = 0; n < PreludeParty.GetNumMembers(); n++)
					{
						pCurHP = PreludeParty.GetMember(n)->GetData(INDEX_HITPOINTS).Value;
						pCurWP = PreludeParty.GetMember(n)->GetData(INDEX_WILLPOINTS).Value;
						pCurRP = PreludeParty.GetMember(n)->GetData(INDEX_RESTPOINTS).Value;
						Push(PreludeParty.GetMember(n));
						RestOptions[PreludeParty.GetRestAction(n)].Process();
						pNewHP = pCurHP + PreludeParty.GetMember(n)->GetData(INDEX_HITPOINTS).Value;
						pNewHP /= 2;
						PreludeParty.GetMember(n)->SetData(INDEX_HITPOINTS,pNewHP);
						pNewWP = pCurWP + PreludeParty.GetMember(n)->GetData(INDEX_WILLPOINTS).Value;
						pNewWP /= 2;
						PreludeParty.GetMember(n)->SetData(INDEX_WILLPOINTS,pNewWP);
						pNewRP = pCurRP + PreludeParty.GetMember(n)->GetData(INDEX_RESTPOINTS).Value;
						pNewRP /= 2;
						PreludeParty.GetMember(n)->SetData(INDEX_RESTPOINTS,pNewRP);
					}
				
					SetFocus(ZSWindow::GetMain());

					PreludeWorld->AdvanceTime(240);
					PreludeEvents.SetLastRandomTime(PreludeEvents.GetLastRandomTime() + 240);

					PreludeEvents.RunEvent(SleepEvent);
					
					ReleaseFocus(); //undo the mainwindow focus
				
				}
				
			}
			return TRUE;
		case IDC_REST_PASS1:
			Hide();
			PreludeWorld->AdvanceTime(60);

			Show();
			sprintf(DescribeText,"It is now %i o'clock.", PreludeWorld->GetHour());
			Describe(DescribeText);
			return TRUE;
		case IDC_REST_PASS2:
			Hide();
			PreludeWorld->AdvanceTime(120);
			Show();
			sprintf(DescribeText,"It is now %i o'clock.", PreludeWorld->GetHour());
			Describe(DescribeText);
			return TRUE;
		case IDC_REST_PASS4:
			Hide();
			PreludeWorld->AdvanceTime(240);
			Show();
			sprintf(DescribeText,"It is now %i o'clock.", PreludeWorld->GetHour());
			Describe(DescribeText);
			return TRUE;
		case IDC_REST_PASS8:
			Hide();
			PreludeWorld->AdvanceTime(480);
			Show();
			sprintf(DescribeText,"It is now %i o'clock.", PreludeWorld->GetHour());
			Describe(DescribeText);
			return TRUE;
		default:
			if(!TownName)
			{
				int Member;
				int NewAction;
				Member = IDFrom - IDC_REST_ACTION1;	
				NewAction = PreludeParty.GetRestAction(Member);
				NewAction++;
				if(NewAction >= NumOptions)
				{
					NewAction = 0;
				}
				PreludeParty.SetRestAction(Member,NewAction);

				ZSWindow *pWin;
				pWin = GetChild(IDFrom);
				pWin->SetText(&OptionNames[NewAction][0]);
				
				pWin = GetChild(IDC_REST_RESULT1 + Member);
				Push(PreludeParty.GetMember(Member));
				Push(NewAction);
				ScriptArg *SA;
				RestResults.Process();
				SA = Pop();
				pWin->SetText((char *)SA->GetValue());
				delete SA;
			}
			return TRUE;
		}
	}
	return TRUE;
}


ZSRest::ZSRest(int NewID, int x, int y, int width, int height)
{
	LoadRestOptions();
	
	ID = NewID;
	Moveable = FALSE;
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;

	Bounds.left = x;
	Bounds.top = y;
	Bounds.bottom = y + height;
	Bounds.right = x + width;

	if(!RestSurface)
	{
		BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("restscreen.bmp",600,400,NULL, -1);
		RestSurface = BackGroundSurface;
	}

	BackGroundSurface = RestSurface;
	BackGroundSurface->AddRef();
		
	int n, NumMembers;

	NumMembers = PreludeParty.GetNumMembers();
	
	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[REST]");
	
	ZSText *pText;
	ZSButton *pButton;

	RECT rName;
	RECT rAction;
	RECT rResult;
	RECT rRest;
	RECT rWait;
	RECT rClose;

	int BHeight;

	SeekTo(fp,"REST");
	LoadRect(&rRest,fp);

	SeekTo(fp,"NAME1");
	LoadRect(&rName,fp);

	SeekTo(fp,"ACTION1");
	LoadRect(&rAction,fp);

	SeekTo(fp,"RESULT1");
	LoadRect(&rResult,fp);

	SeekTo(fp,"WAIT");
	LoadRect(&rWait,fp);

	SeekTo(fp,"CLOSE");
	LoadRect(&rClose,fp);

	SeekTo(fp,"BUTTONHEIGHT");
	BHeight = GetInt(fp);

	if(!PreludeWorld->GetCurAreaNum())
	{
		TownName = PreludeWorld->GetTown((int)PreludeParty.GetLeader()->GetPosition()->x,(int)PreludeParty.GetLeader()->GetPosition()->y);
	}
	else
	{
		TownName = NULL;
	}
	for(n = 0; n < NumMembers; n++)
	{
		//name
		pText = new ZSText(IDC_REST_NAME1 + n, XYWH(rName), PreludeParty.GetMember(n)->GetData(INDEX_NAME).String,0,FALSE);
		pText->Show();
		AddChild(pText);

		//action
		pButton = new ZSButton(BUTTON_NORMAL, IDC_REST_ACTION1 + n, XYWH(rAction));
		pButton->Show();
		if(!TownName)
		{
			pButton->SetText(&OptionNames[PreludeParty.GetRestAction(n)][0]);
		}
		else
		{
			pButton->SetText("Wait.");
		}

		AddChild(pButton);
		
		//result
		pText = new ZSText(IDC_REST_RESULT1 + n, XYWH(rResult),"_", 0, FALSE);
		pText->Show();
		
		if(!TownName)
		{
			ScriptArg *SA;
			Push(PreludeParty.GetMember(n));
			Push(PreludeParty.GetRestAction(n));
			RestResults.Process();
			SA = Pop();
			pText->SetText((char *)SA->GetValue());
			delete SA;
		}
		else
		{
			char Blarg[64];
			sprintf(Blarg,"Can't rest within %s.",TownName);
			pText->SetText(Blarg);
		}

		AddChild(pText);
		rName.top += BHeight;
		rName.bottom += BHeight;
		rAction.top += BHeight;
		rAction.bottom += BHeight;
		rResult.top += BHeight;
		rResult.bottom += BHeight;
	}

	pButton = new ZSButton("Closebutton", IDC_REST_EXIT, XYWH(rClose), 57,34, 1);
	//pButton->SetText("X");
	pButton->Show();
	AddChild(pButton);

	if(!TownName)
	{
		pButton = new ZSButton("RestButtonBig", IDC_REST_REST, XYWH(rRest), 78, 26, 1);
		//pButton->SetText("Rest");
		pButton->Show();
		AddChild(pButton);
	}

	pButton = new ZSButton("waitbutton", IDC_REST_PASS1, XYWH(rWait), 78, 26, 1);
	//pButton->SetText("Wait 1 Hours");
	pButton->Show();
	AddChild(pButton);

	
	
/*
	pButton = new ZSButton(BUTTON_NORMAL, IDC_REST_PASS2, Bounds.left + 96, Bounds.bottom - 32, 90, 24);
	pButton->SetText("Wait 2 Hours");
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_REST_PASS4, Bounds.left + 192, Bounds.bottom - 32, 90, 24);
	pButton->SetText("Wait 4 Hours");
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_REST_PASS8, Bounds.left + 288, Bounds.bottom - 32, 90, 24);
	pButton->SetText("Wait 8 Hours");
	pButton->Show();
	AddChild(pButton);
*/
	fclose(fp);

}

void ZSRest::LoadRestOptions()
{
	FILE *fp;
	fp = fopen("rest.bin","rb");
	if(!fp)
	{
		LoadRestOptionsText();
		return;
	}
	
	fread(&NumOptions,sizeof(int),1,fp);
	
	for(int n = 0; n < NumOptions; n++)
	{
		fread(&OptionNames[n][0],sizeof(char),32,fp);
		RestOptions[n].Load(fp);
	}

	RestResults.Load(fp);

	RestEvent.Load(fp);

	fclose(fp);
}


void ZSRest::LoadRestOptionsText()
{
	int n;

	FILE *fp;
	fp = SafeFileOpen("rest.txt","rt");

	SeekTo(fp,"NumOptions:");
	NumOptions = GetInt(fp);

	for(n = 0; n < NumOptions; n++)
	{
		SeekTo(fp, "OptionName:");
		GetString(fp,&OptionNames[n][0]);
		
		SeekTo(fp, "(");
		
		RestOptions[n].Import(fp);
	}

	SeekTo(fp, "(");
	
	RestResults.Import(fp);
	
	SeekTo(fp, "(");

	RestEvent.Import(fp);

	fclose(fp);

	fp = fopen("rest.bin","wb");
	
	fwrite(&NumOptions,sizeof(int),1,fp);
	
	for(n = 0; n < NumOptions; n++)
	{
		fwrite(&OptionNames[n][0],sizeof(char),32,fp);
		RestOptions[n].Save(fp);
	}

	RestResults.Save(fp);

	RestEvent.Save(fp);

	fclose(fp);
}

void ZSRest::Show()
{
	Visible = TRUE;
	if(!PreludeWorld->GetRestHelp())
	{
		PreludeWorld->SetRestHelp(TRUE);
		ShowHelp("Rest Screen");
	}

}

int ZSRest::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		ShowHelp("Rest Screen");
		return TRUE;
	}
	if(CurrentKeys[DIK_ESCAPE] & 0x80)
	{
		State = WINDOW_STATE_DONE;
	}
	return TRUE;
}



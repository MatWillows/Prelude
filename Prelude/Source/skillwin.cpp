#include "skillwin.h"
#include "creatures.h"
#include "zsbutton.h"
#include "zstext.h"
#include "zsconfirm.h"
#include "characterwin.h"
#include "script.h"
#include "scriptfuncs.h"
#include "ZSMEssage.h"
#include "world.h"
#include <assert.h>

#define IDC_SKILL_XP 9999

void SkillWin::Reset()
{
	
	ZSWindow *pWin;

	pWin = GetChild();
	
	while(pWin)
	{
		if(pWin->GetID() != IDC_SKILL_XP)
		{
			pWin->Hide();
		}
		pWin = pWin->GetSibling();
	}
	
	int SkillStart;
	int SkillEnd;
	int SkillNum;

	int n;

	SkillStart = pTarget->GetIndex("SWORD");
	SkillEnd = pTarget->GetIndex("SOUL OF WIND");

	SkillNum = 0;

	RECT rSkill;
	int Left;
	int Top;
	int width;
	width = Bounds.right - Bounds.left;

	ZSText *pText;
	ZSButton *pButton;
	RECT rButton;
	char SkillName[64];

	StartXP = pTarget->GetData(INDEX_XP).Value;

	for(n = SkillStart; n <= SkillEnd; n++)
	{
		if(pTarget->GetData(n).Value)
		{
			Left = (SkillNum % 2) * (width/2) + Bounds.left;
			Top = (SkillNum / 2) * 28 + Bounds.top;

			//base
			rSkill.left = Left;
			rSkill.right = Left + 24;
			rSkill.top = Top;
			rSkill.bottom = Top + 24;
			
			//value
			pText = (ZSText *)GetChild(n*10 + 2);
			if(!pText)
			{
				pText = new ZSText(n*10 + 2, XYWH(rSkill), "000" , 0, TRUE);
				AddChild(pText);
			}
			pText->Show();
			pText->SetText(pTarget->GetData(n).Value);
			
			//plus
			rSkill.left += 25;
			rSkill.right = rSkill.left + 24;

			rButton = rSkill;
			rButton.top += 4;
			rButton.bottom = rButton.top + 16;
			rButton.right = rButton.left + 16;

			if(StartXP)
			{
				pButton = (ZSButton *)GetChild(n*10 + 3);
				if(!pButton)
				{
					pButton = new ZSButton(BUTTON_PLUS, n*10 + 3, XYWH(rButton));
					AddChild(pButton);
				}
				pButton->Show();
			}
			
			rSkill.left += 17;
			rSkill.right = rSkill.left + 128;

			strcpy(SkillName,pTarget->GetName(n));
			ConvertToLowerCase(&SkillName[1]);
			
			pText = (ZSText *)GetChild(n*10);
			if(!pText)
			{
				pText = new ZSText(n*10, XYWH(rSkill), SkillName,0);
				AddChild(pText);
			}
			pText->Show();
				

			StartValues[n - SkillStart] = pTarget->GetData(n).Value;
			
			SkillNum++;
		}
	}

	pWin = GetChild(IDC_SKILL_XP);
	char tempXP[16];
	sprintf(tempXP,"XP: %i",pTarget->GetData(INDEX_XP).Value);
	pWin->SetText(tempXP);
}


int SkillWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		int ButtonType;
		ButtonType = IDFrom % 3;
		int SkillNum;
		int ImproveCost;
		char ImproveString[64];
		SkillNum = IDFrom / 10;
	
		ImproveCost = (pTarget->GetData(SkillNum).Value / 20) + 1;
		if(pTarget->GetData(INDEX_XP).Value >= ImproveCost)
		{
			if(PreludeWorld->XPConfirm())
			{
				sprintf(ImproveString,"Spend %i XP to improve %s?",ImproveCost, GetChild(IDFrom - 3)->GetText());
				if(Confirm(this->pParent,ImproveString,"Yes","No"))
				{
					pTarget->SetData(INDEX_XP,pTarget->GetData(INDEX_XP).Value - ImproveCost);
					Push(1);
					Push(SkillNum);
					Push((Creature *)pTarget);
					CallScript("BumpSkill","getskills.txt");
					this->GetChild(IDFrom - 1)->SetText(pTarget->GetData(SkillNum).Value);
					Reset();
				}
			}
			else
			{
				pTarget->SetData(INDEX_XP,pTarget->GetData(INDEX_XP).Value - ImproveCost);
				Push(1);
				Push(SkillNum);
				Push((Creature *)pTarget);
				CallScript("BumpSkill","getskills.txt");
				this->GetChild(IDFrom - 1)->SetText(pTarget->GetData(SkillNum).Value);
				Reset();
			}
		}
		else
		{
			sprintf(ImproveString,"You need %i XP to improve %s.",ImproveCost, GetChild(IDFrom - 3)->GetText());
			Message(ImproveString,"OK");
		}
	}

	return TRUE;
}

SkillWin::SkillWin(int NewID, int x, int y, int width, int height, Creature *pNewTarget)
{
	assert(pNewTarget);
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Moveable = FALSE;
	Type = WINDOW_NONE;

	pChild = NULL;
	pParent = NULL;

	//set up the bounding rect
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	BackGroundSurface = NULL;

	pTarget = pNewTarget;

	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");
	SeekTo(fp,"[CHARACTER]");
	RECT rBounds;

	//create the XP window
	SeekTo(fp,"XP");
	LoadRect(&rBounds,fp);

	ZSText *pText;
	pText = new ZSText(IDC_SKILL_XP, rBounds.left, rBounds.top, "XP: 000");
	pText->Show();
	AddChild(pText);

	fclose(fp);

	Reset();

}

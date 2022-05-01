#include "zsspellwindow.h"
#include "zslistbox.h"
#include "zsdescribe.h"
#include "zsbutton.h"
#include "creatures.h"
#include "spells.h"
#include "zstext.h"

typedef enum
{
	IDC_SPELL_NONE,
	IDC_SPELL_SET_READY_SPELL,
	IDC_SPELL_SPELL_LIST,
	IDC_SPELL_SPELL_DESCRIPTION,
	IDC_SPELL_READY_SPELL,
}	SPELL_WIN_CONTROLS;


void SpellWin::SetCaster(Creature *pNewCaster)
{
	pCaster = pNewCaster;
	ZSList *pList;
	pList = (ZSList *)this->GetChild(IDC_SPELL_SPELL_LIST);
	pList->Clear();
	int n, ln;
	int MapIndex;
	MapIndex = 0;
	int Skill;
	int KnownSpells = 0;
	for(n = 0; n < MAX_SPELLS; n++)
	{
		if(pCaster->KnowsSpell(n))
		{
			KnownSpells++;
			switch(PreludeSpells.GetSpell(n)->GetType())
			{
			case SPELL_FLAME:
				Skill = pCaster->GetData(INDEX_POWER_OF_FLAME).Value;
				break;		
			case SPELL_RIVER:
				Skill = pCaster->GetData(INDEX_GIFTS_OF_GODDESS).Value;
				break;
			case SPELL_THAUMATURGY:
				Skill = pCaster->GetData(INDEX_THAUMATURGY).Value;
				break;
			}

			for(ln = PreludeSpells.GetSpell(n)->GetNumLevels()-1; ln >= 0; ln--)
			{
				if(Skill && Skill >= PreludeSpells.GetSpell(n)->GetLevel(ln)->SkillReq)
				{
					SpellMap[MapIndex] = n * 100 + ln;
					pList->AddItem(PreludeSpells.GetSpell(n)->GetLevel(ln)->Name);
					MapIndex ++;
				}
			}
		}
	}
	
/*	ZSWindow *pWin;
	pWin = GetChild(IDC_SPELL_READY_SPELL);
	if(pWin)
	{
		if(pCaster->GetReadySpell() != -1)
			pWin->SetText(PreludeSpells.GetSpell(pCaster->GetReadySpell())->GetLevel(0)->Name);
		else
			pWin->SetText("No Spell Ready");
	}
*/
}

int SpellWin::Command(int IDFrom, int Command, int Param)
{
	int SpellNum;
	int nspell, nlevel;
				
	if(Command == COMMAND_LIST_SELECTED)
	{
	//set the spell description;
		//get the spell number
		SpellNum = SpellMap[Param];
		ZSDescribe *pDesc;
		pDesc = (ZSDescribe *)this->GetChild(IDC_SPELL_SPELL_DESCRIPTION);
		pDesc->Clear();
		nspell = SpellNum / 100;
		nlevel = SpellNum % 100;
		pDesc->AddItem(PreludeSpells.GetSpell(nspell)->GetLevel(nlevel)->Description);
		return TRUE;
	}

	/*
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		ZSList *pList;
		pList = (ZSList *)GetChild(IDC_SPELL_SPELL_LIST);
		if(pList->GetSelection() != -1)
		{
			SpellNum = SpellMap[pList->GetSelection()];
			nspell = SpellNum / 100;
			nlevel = SpellNum % 100;

			pCaster->SetReadySpell((char)nspell);

			ZSWindow *pWin;
			pWin = GetChild(IDC_SPELL_READY_SPELL);
			if(pWin)
			{
				pWin->SetText(PreludeSpells.GetSpell(pCaster->GetReadySpell())->GetLevel(0)->Name);
			}
		}

	}
*/
	return TRUE;
}

int SpellWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{

	return TRUE;
}


SpellWin::SpellWin(int NewID, int x, int y, int width, int height, Creature *pNewCaster)
{
	ZeroMemory(SpellMap,sizeof(int) * MAX_SPELLS);
	ID = NewID;
	Visible = FALSE;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	ZSList *pList;
	pList = new ZSList(IDC_SPELL_SPELL_LIST, x, y, 128, height, FALSE);
	pList->SetSelection(0);
	AddChild(pList);
	pList->Show();

	ZSDescribe *pDesc;
	pDesc = new ZSDescribe(IDC_SPELL_SPELL_DESCRIPTION, x + 130, y, width - 130, height - 64);
	AddChild(pDesc);
	pDesc->Show();

/*	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_NORMAL, IDC_SPELL_SET_READY_SPELL, x + 130, y + height - 32, width - 130, 24);
	AddChild(pButton);
	pButton->Show();
	pButton->SetText("Set Ready Spell");

	ZSText *pText;
	pText = new ZSText(IDC_SPELL_READY_SPELL, x + 130, y + height - 64, width - 130, 24, "", 0);
	pText->Show();
	AddChild(pText);
*/	
	SetCaster(pNewCaster);
}


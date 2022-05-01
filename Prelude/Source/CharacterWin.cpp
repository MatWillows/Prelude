#include "characterWin.h"
#include "creatures.h"
#include "items.h"
#include "inventorywin.h"
#include "equipwin.h"
#include "skillwin.h"
#include "zsutilities.h"
#include "zsengine.h"
#include "zstext.h"
#include "zsbutton.h"
#include "zshelpwin.h"
#include "zsspellwindow.h"
#include "mainwindow.h"
#include "world.h"
#include "zsportrait.h"
#include <assert.h>

void CharacterWin::Show()
{
	Visible = TRUE;

	if(!PreludeWorld->GetCharacterHelp())
	{
		PreludeWorld->SetCharacterHelp(TRUE);
		ShowHelp("Character Screen");
	}

}

void CharacterWin::ResetStats()
{
	if(!pTarget) return;

	char TempString[64];

	RECT rBase;
	rBase.left = 0;
	rBase.top = 0;
	rBase.bottom = 600;
	rBase.right = 800;
	RECT rCur;
	rCur.left = 0;
	rCur.right = Engine->Graphics()->GetWidth();
	rCur.top = 0;
	rCur.bottom = Engine->Graphics()->GetHeight();

	FILE *fp;
	ZSText *pText;

	RECT rBounds;
	RECT rBaseBounds;
	int BackWidth;
	int BackHeight;
	char *FileName;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[CHARACTER]");

	SeekTo(fp,"BACKGROUND");
	FileName = GetStringNoWhite(fp);

	BackWidth = GetInt(fp);
	BackHeight = GetInt(fp);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,BackWidth,BackHeight,NULL,NULL);

	delete[] FileName;

	ZSWindow *pWin;

	SeekTo(fp,"POSITION");		
	
	LoadRect(&rBounds, fp);
	rBaseBounds = rBounds;

	ScaleRect(&rBounds, &rBase, &rCur);

	Bounds = rBounds;

	SeekTo(fp,"PORTRAIT");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	pWin = new ZSWindow(WINDOW_NONE, CS_PORTRAIT, XYWH(rBounds));
	pWin->SetBorderWidth(8);
	pWin->CreatePortrait(pTarget->GetData(INDEX_PORTRAIT).String);
	pWin->AddBorder(8,2);
	pWin->Show();
	AddChild(pWin);

	SeekTo(fp,"NAME");
	LoadRect(&rBounds,fp);

	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;
	
	Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, pTarget->GetData(INDEX_NAME).String, TEXT_LIGHT_PAINTED_WOOD);
	
	SeekTo(fp,"SEX");
	LoadRect(&rBounds,fp);

	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;
	
	if(pTarget->GetData(INDEX_SEX).Value)
	{
		Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Female", TEXT_LIGHT_PAINTED_WOOD);
	}
	else
	{
		Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Male", TEXT_LIGHT_PAINTED_WOOD);
	}

	SeekTo(fp,"RACE");
	LoadRect(&rBounds,fp);

	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;

	if(pTarget->GetData(INDEX_RIVERBLOOD).Value)
	{
		if(pTarget->GetData(INDEX_FLAMEBLOOD).Value)
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Mixed", TEXT_LIGHT_PAINTED_WOOD);
		}
		else
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "River",  TEXT_LIGHT_PAINTED_WOOD);
		}
	}
	else
	{
		if(pTarget->GetData(INDEX_FLAMEBLOOD).Value)
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Flame", TEXT_LIGHT_PAINTED_WOOD);
		}
		else
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "VanGesh", TEXT_LIGHT_PAINTED_WOOD);
		}
	}

	SeekTo(fp,"ARMOR");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	SeekTo(fp,"DAMAGE");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	SeekTo(fp,"HP");
	LoadRect(&rBounds,fp);
//	ScaleRect(&rBounds, &rBase, &rCur);
	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;
	sprintf(TempString,"HP:  %i / %i",pTarget->GetData(INDEX_HITPOINTS).Value, pTarget->GetData(INDEX_MAXHITPOINTS).Value);
	Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, TempString, TEXT_LIGHT_PAINTED_WOOD);

	
	SeekTo(fp,"STAT");
	LoadRect(&rBounds,fp);
	RECT rStatBase;
	rStatBase = rBounds;
	
	ScaleRect(&rBounds, &rBase, &rCur);

	int NumStats;
	int StatStart;
	StatStart = pTarget->GetIndex("STRENGTH");
	NumStats = pTarget->GetIndex("SPEED") - StatStart;
	
	RECT rStatBound;
	RECT rStatNameBound;

	rStatBound = rBounds;
	char temp[16];

	for(int n = 0; n <= NumStats; n++)
	{
		rStatBound = rStatBase;
		ScaleRect(&rStatBound,&rBase, &rCur);

		rStatBound.left = rBounds.left;
		rStatBound.right = rBounds.right;
		sprintf(temp,"%i",pTarget->GetData(StatStart + n).Value);
		
		pWin = GetChild(CS_STAT1 + n);
		if(pWin)
			RemoveChild(pWin);

		pText = new ZSText(CS_STAT1 + n, XYWH(rStatBound), temp, 0, TRUE);
		pText->Show();
		AddChild(pText);

		rStatNameBound = rStatBase;
		rStatNameBound.left = rStatNameBound.right - rBaseBounds.left;
		rStatNameBound.right = rStatNameBound.left + 256;
		rStatNameBound.top -= rBaseBounds.top - 4;
	
		rStatNameBound.bottom -= Bounds.top - 4;

		char StatName[32];
		strcpy(StatName,pTarget->GetName(StatStart + n));
		ConvertToLowerCase(&StatName[1]);

		Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rStatNameBound, StatName,  TEXT_LIGHT_PAINTED_WOOD);
		rStatBase.top += 24;
		rStatBase.bottom += 24;
	}

	fclose(fp);

	SkillWin *pSkill;
	pSkill = (SkillWin *)GetChild(CS_SKILLS);

	pSkill->Reset();

	//reset the skill window
}

BOOL CharacterWin::RightButtonUp(int x, int y)
{
	//if we're moving we can stop
	if(State == WINDOW_STATE_MOVING)
	{
		//validate our current location
		if(!ValidateMove(&Bounds))
		{
			//if we're intersecting a sibling, rebound to our start position
			Move(OldBounds);	
		}
		//return to normal operations
		State = WINDOW_STATE_NORMAL;
	}
	
	//if our cursor is the pointing hand, raise the finger
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}

	ReleaseFocus();
	
	ZSWindow *pWin;
	pWin = GetMain()->GetBottomChild(x,y);
	if(pWin)
	{
		if(pWin->GetID() == 66666)
		{
			if(((ZSPortrait *)pWin)->GetTarget() != this->GetTarget())
			{
				this->ID = -11;
				this->SetState(WINDOW_STATE_DONE);
				this->Hide();
				pWin->RightButtonDown(x,y);
				pWin->RightButtonUp(x,y);
			}
		}
	}

	//done
	return TRUE; 
} // LeftButtonUp



int CharacterWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == CS_CLOSE)
		{
			State = WINDOW_STATE_DONE;
		}
		
		if(((ZSMainWindow *)ZSWindow::GetMain())->InventoryShown())
		{
			if(IDFrom == CS_GOSKILLS)
			{
				ZSWindow *pWin;
				pWin = GetChild(CS_SKILLS);
				pWin->Show();

				pWin = GetChild(CS_INVENTORY);
				pWin->Hide();

				pWin = GetChild(CS_EQUIP);
				pWin->Hide();
				
				pWin = GetChild(CS_SPELLBOOK);
				pWin->Hide();

			}
			if(IDFrom == CS_GOEQUIP)
			{
				ZSWindow *pWin;
				pWin = GetChild(CS_SKILLS);
				pWin->Hide();

				pWin = GetChild(CS_INVENTORY);
				pWin->Show();

				pWin = GetChild(CS_EQUIP);
				pWin->Show();

				pWin = GetChild(CS_SPELLBOOK);
				pWin->Hide();
			}
			if(IDFrom == CS_GOSPELLS)
			{
				ZSWindow *pWin;
				pWin = GetChild(CS_SKILLS);
				pWin->Hide();

				pWin = GetChild(CS_INVENTORY);
				pWin->Hide();

				pWin = GetChild(CS_EQUIP);
				pWin->Hide();

				pWin = GetChild(CS_SPELLBOOK);
				pWin->Show();
			}
		}

		
	}
	return TRUE;
}

int CharacterWin::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ShowHelp("Character Screen");
		return TRUE;
	}
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}
	return TRUE;
}

CharacterWin::CharacterWin(int NewID, int x, int y, int width, int height, Creature *pNewTarget)
{
	assert(pNewTarget);
	char TempString[64];

	RECT rBase;
	rBase.left = 0;
	rBase.top = 0;
	rBase.bottom = 600;
	rBase.right = 800;
	RECT rCur;
	rCur.left = 0;
	rCur.right = Engine->Graphics()->GetWidth();
	rCur.top = 0;
	rCur.bottom = Engine->Graphics()->GetHeight();

	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Type = WINDOW_CHARACTERINFO;
	Moveable = FALSE;
	Visible = FALSE;
	pTarget = pNewTarget;

	FILE *fp;
	ZSText *pText;

	RECT rBounds;
	RECT rBaseBounds;
	int BackWidth;
	int BackHeight;
	char *FileName;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[CHARACTER]");

	SeekTo(fp,"BACKGROUND");
	FileName = GetStringNoWhite(fp);

	BackWidth = GetInt(fp);
	BackHeight = GetInt(fp);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,BackWidth,BackHeight,NULL,NULL);

	delete[] FileName;

	ZSWindow *pWin;

	SeekTo(fp,"POSITION");		
	
	LoadRect(&rBounds, fp);
	rBaseBounds = rBounds;

	ScaleRect(&rBounds, &rBase, &rCur);

	Bounds = rBounds;

	SeekTo(fp,"PORTRAIT");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	pWin = new ZSWindow(WINDOW_NONE, CS_PORTRAIT, XYWH(rBounds));
	pWin->SetBorderWidth(8);
	pWin->CreatePortrait(pTarget->GetData(INDEX_PORTRAIT).String);
	pWin->AddBorder(8,2);
	pWin->Show();
	AddChild(pWin);

	SeekTo(fp,"NAME");
	LoadRect(&rBounds,fp);

	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;
	
	Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, pTarget->GetData(INDEX_NAME).String, TEXT_LIGHT_PAINTED_WOOD);
	
	SeekTo(fp,"SEX");
	LoadRect(&rBounds,fp);

	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;
	
	if(pTarget->GetData(INDEX_SEX).Value)
	{
		Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Female", TEXT_LIGHT_PAINTED_WOOD);
	}
	else
	{
		Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Male", TEXT_LIGHT_PAINTED_WOOD);
	}

	SeekTo(fp,"RACE");
	LoadRect(&rBounds,fp);

	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;

	if(pTarget->GetData(INDEX_RIVERBLOOD).Value)
	{
		if(pTarget->GetData(INDEX_FLAMEBLOOD).Value)
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Mixed", TEXT_LIGHT_PAINTED_WOOD);
		}
		else
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "River",  TEXT_LIGHT_PAINTED_WOOD);
		}
	}
	else
	{
		if(pTarget->GetData(INDEX_FLAMEBLOOD).Value)
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "Flame", TEXT_LIGHT_PAINTED_WOOD);
		}
		else
		{
			Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, "VanGesh", TEXT_LIGHT_PAINTED_WOOD);
		}
	}

	SeekTo(fp,"ARMOR");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	SeekTo(fp,"DAMAGE");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	SeekTo(fp,"HP");
	LoadRect(&rBounds,fp);
//	ScaleRect(&rBounds, &rBase, &rCur);
	rBounds.left   -= rBaseBounds.left;
	rBounds.right  -= rBaseBounds.left;
	rBounds.top    -= rBaseBounds.top;
	rBounds.bottom -= rBaseBounds.top;
	sprintf(TempString,"HP:  %i / %i",pTarget->GetData(INDEX_HITPOINTS).Value, pTarget->GetData(INDEX_MAXHITPOINTS).Value);
	Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rBounds, TempString, TEXT_LIGHT_PAINTED_WOOD);


	SeekTo(fp,"STAT");
	LoadRect(&rBounds,fp);
	RECT rStatBase;
	rStatBase = rBounds;
	
	ScaleRect(&rBounds, &rBase, &rCur);

	int NumStats;
	int StatStart;
	StatStart = pTarget->GetIndex("STRENGTH");
	NumStats = pTarget->GetIndex("SPEED") - StatStart;
	
	RECT rStatBound;
	RECT rStatNameBound;

	rStatBound = rBounds;
	char temp[16];

	for(int n = 0; n <= NumStats; n++)
	{
		rStatBound = rStatBase;
		ScaleRect(&rStatBound,&rBase, &rCur);

		rStatBound.left = rBounds.left;
		rStatBound.right = rBounds.right;
		sprintf(temp,"%i",pTarget->GetData(StatStart + n).Value);
		pText = new ZSText(CS_STAT1 + n, XYWH(rStatBound), temp, 0, TRUE);
		pText->Show();
		AddChild(pText);

		rStatNameBound = rStatBase;
		rStatNameBound.left = rStatNameBound.right - rBaseBounds.left;
		rStatNameBound.right = rStatNameBound.left + 256;
		rStatNameBound.top -= rBaseBounds.top - 4;
	
		rStatNameBound.bottom -= Bounds.top - 4;

		char StatName[32];
		strcpy(StatName,pTarget->GetName(StatStart + n));
		ConvertToLowerCase(&StatName[1]);

		Engine->Graphics()->GetFontEngine()->DrawText(BackGroundSurface, &rStatNameBound, StatName,  TEXT_LIGHT_PAINTED_WOOD);
		rStatBase.top += 24;
		rStatBase.bottom += 24;
	}

	
	ZSButton *pButton;

	SeekTo(fp,"CLOSE");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);
	pButton = new ZSButton("closebutton", CS_CLOSE, XYWH(rBounds),57,34,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"GOEQUIP");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);
	pButton = new ZSButton("equipbutton", CS_GOEQUIP, XYWH(rBounds),79,26,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"GOSKILLS");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);
	pButton = new ZSButton("skillsbutton", CS_GOSKILLS, XYWH(rBounds),79,26,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"GOSPELLS");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);
	pButton = new ZSButton("spellsbutton", CS_GOSPELLS, XYWH(rBounds),79,26,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"SKILLS");		
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	SkillWin *pSWin;

	pSWin = new SkillWin(CS_SKILLS, XYWH(rBounds), pTarget);
	pSWin->Hide();
	AddChild(pSWin);


	SeekTo(fp,"SPELLS");		
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	SpellWin *pSpells;
	pSpells = new SpellWin(CS_SPELLBOOK, XYWH(rBounds), pTarget);
	pSpells->Hide();
	AddChild(pSpells);

	SeekTo(fp,"EQUIPMENT");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);

	EquipWin *pEWin;
	pEWin = new EquipWin(CS_EQUIP, XYWH(rBounds), pTarget);
	AddChild(pEWin);
	if(((ZSMainWindow *)ZSWindow::GetMain())->InventoryShown())
		pEWin->Show();
	
	SeekTo(fp,"INVENTORY");
	LoadRect(&rBounds,fp);
	ScaleRect(&rBounds, &rBase, &rCur);
	InventoryWin *pIWin;
	
	pIWin = new InventoryWin(CS_INVENTORY, XYWH(rBounds), pTarget);
	AddChild(pIWin);
	if(((ZSMainWindow *)ZSWindow::GetMain())->InventoryShown())
		pIWin->Show();
	
	fclose(fp);

	//Portrait
	//name
	//sex
	//race
	//height
	//weight
	//hair
	//stats
	//xp
	//kills?
	//hp
	//damage
	//ac
	//defense
	//stats
	//equipment
	//inventory
	//buttons to switch between stats and inventory
}


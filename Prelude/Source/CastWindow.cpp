#include "castwindow.h"

#include "zsbutton.h"
#include "creatures.h"
#include "ZSEngine.h"
#include "things.h"

int CastWindow::Command(int IDFrom, int Command, int Param)
{
	switch(Command)
	{
		case COMMAND_BUTTON_CLICKED:
			((Creature *)pCaster)->InsertAction(ACTION_CAST,(void *)pTarget,(void *)IDFrom);
			pParent->RemoveChild(this);
			break;
		default:
			break;
	}
	return TRUE;
}

int CastWindow::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	pParent->RemoveChild(this);
	return TRUE;
}


CastWindow::CastWindow(int x, int y, Object *NewCaster, Object *NewTarget)
{
	ID = CAST_MENU_ID;
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;
	Cursor = CURSOR_POINT;
	
	pCaster = NewCaster;
	pTarget = NewTarget;
	
	//calculate width
	int MaxWidth = 0;
	int Width = 0;

	int n, sn;
	int NumItems = 0;
	char *SpellName;

	Thing *pSpellbook;
	Thing *pSpell;

	pSpellbook = Thing::Find(Spellbook::GetFirst(),pCaster->GetData(INDEX_SPELLBOOK).Value);

	for(n = pSpellbook->GetIndex("READYSPELL") + 1; n < pSpellbook->GetNumFields(); n++)
	{
		if(pSpellbook->GetData(n).Value)
		{
			pSpell = Thing::Find(Spellbook::GetFirst(), pSpellbook->GetName(n));

			for(sn = INDEX_DESCRIPTOR1; sn <= INDEX_DESCRIPTOR4; sn++)
			{
				SpellName = pSpell->GetData(sn).String;
				Width = Engine->Graphics()->GetFontEngine()->GetTextWidth(SpellName);
				if(Width > MaxWidth)
				{
					MaxWidth = Width;
				}
				if(strcmp("N",SpellName))
				{
					NumItems++;
				}
			}
		}
	}

	MaxWidth += 8;
	int FHeight;  
	FHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();

	Bounds.top = y;
	Bounds.left = x;
	Bounds.right = x + MaxWidth;
	Bounds.bottom = y + FHeight * NumItems;

	int HOff;
	int WOff;

	RECT rParentBounds;

	ZSWindow::GetMain()->GetBounds(&rParentBounds);

	if(Bounds.bottom > rParentBounds.bottom)
	{
		HOff = Bounds.bottom - rParentBounds.bottom;
		Bounds.top -= HOff;
		Bounds.bottom -= HOff;
	}
	
	if(Bounds.left < rParentBounds.left)
	{
		WOff = Bounds.left - rParentBounds.left;
		Bounds.left += WOff;
		Bounds.right += WOff;
	}

	if(Bounds.right > rParentBounds.right)
	{
		WOff = Bounds.right - rParentBounds.right;
		Bounds.left -= WOff;
		Bounds.right -= WOff;
	}

	ZSButton *pWin;
	int CurItem = 0;
	
	for(n = pSpellbook->GetIndex("READYSPELL") + 1; n < pSpellbook->GetNumFields(); n++)
	{
		if(pSpellbook->GetData(n).Value)
		{
			pSpell = Thing::Find(Spellbook::GetFirst(), pSpellbook->GetName(n));

			for(sn = INDEX_DESCRIPTOR1; sn <= INDEX_DESCRIPTOR4; sn++)
			{
				SpellName = pSpell->GetData(sn).String;
				if(strcmp("N",SpellName))
				{
					//encode the spells ID and Power level
					pWin = new ZSButton(BUTTON_NORMAL, 10 * pSpell->GetData(INDEX_ID).Value + sn, Bounds.left, Bounds.top+CurItem*FHeight, MaxWidth, FHeight);
					pWin->SetText(SpellName);
					AddChild(pWin);
					CurItem++;
				}
			}
		}
	}
	
	Engine->Graphics()->SetCursor(CURSOR_POINT);
}

CastWindow::~CastWindow()
{
	ReleaseFocus();

	ZSWindow *pWin;
	pWin = pChild;
	while(pChild)
	{
		pWin = pChild;
		pChild = pChild->GetSibling();
		delete pWin;
	}
}

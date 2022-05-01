#include "ZSactionwindow.h"
#include "ZSbutton.h"
#include "creatures.h"
#include "world.h"
#include "ZSEngine.h"
#include "MainWindow.h"
#include "party.h"
#include "Characterwin.h"

int ZSActionWin::Command(int IDFrom, int Command, int Param)
{
	switch(Command)
	{
		case COMMAND_BUTTON_CLICKED:
			ReleaseFocus();
			
			Creature *pActive;
			if(pActor)
				pActive = pActor;
			else
				pActive = (Creature *)PreludeWorld->GetActive();
			ACTION_T NewType;
			Action *pAction;
			void *NewTarget;
			void *NewData;

			NewType = pMenu->GetAction(IDFrom)->GetType();
			NewTarget = pMenu->GetAction(IDFrom)->GetTarget();
			NewData = pMenu->GetAction(IDFrom)->GetData();
			
			switch(NewType)
			{
			case ACTION_NONE:
				break;
			case ACTION_LOOKAT:
				Hide();
				pAction = Action::GetNewAction(NewType, NewTarget, NewData, FALSE);
				pAction->SetNext(pActive->GetAction());
				pActive->SetAction(pAction);
				pActive->LookAt();
				if(pActive->GetAction() == pAction && pAction->GetNext())
				{
					pActive->SetAction(pAction->GetNext());
					pAction->Release();
				}
				break;
			case ACTION_USEITEM:
				Hide();
				pAction = Action::GetNewAction(NewType, NewTarget, NewData, FALSE);
				pAction->SetNext(pActive->GetAction());
				pActive->SetAction(pAction);
				pActive->UseItem();
				if(pActive->GetAction() == pAction && pAction->GetNext())
				{
					pActive->SetAction(pAction->GetNext());
					pAction->Release();
				}
				break;
			case ACTION_DROP:
				Hide();
				pAction = Action::GetNewAction(NewType, NewTarget, NewData, FALSE);
				pAction->SetNext(pActive->GetAction());
				pActive->SetAction(pAction);
				pActive->Drop();
				if(pActive->GetAction() == pAction && pAction->GetNext())
				{
					pActive->SetAction(pAction->GetNext());
					pAction->Release();
				}
				break;
			case ACTION_CAST:
				//check to see if the inventory window is open
				ZSWindow *pWin;
				pWin = ZSWindow::GetMain()->GetChild(CHARACTER_WINDOW_ID); //character window ID
				if(pWin)
				{
					pWin->Hide();
					pWin->SetState(WINDOW_STATE_DONE);
				}
					
				if(!NewData && !Casting)
				{
					Hide();
					CastMenu((Object *)pActive, (Object *)NewTarget);
					break;
				}//else fall through
			default:
				pActive->InsertAction(NewType,NewTarget,NewData);			
				break;
			}

			PreludeWorld->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
			
			State = WINDOW_STATE_DONE;
			break;
		default:
			break;
	}
	return TRUE;
}

int ZSActionWin::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	State = WINDOW_STATE_DONE;
	return TRUE;
}

int ZSActionWin::RightButtonDown(int x, int y)
{
	if(ZSWindow::GetFocus() != this)
		SetFocus(this);
	return TRUE;
}

int ZSActionWin::MoveMouse(long *x, long *y, long *z)
{
	//clamp the mouse position to between our boundaries
	if(*x < Bounds.left)
	{
		*x = Bounds.left;
	}
	else
	if(*x > Bounds.right)
	{
		*x = Bounds.right;
	}

	if(*y < Bounds.top)
	{
		*y = Bounds.top;
	}
	else
	if(*y > Bounds.bottom)
	{
		*y = Bounds.bottom;
	}
	ZSWindow *pWin;
	pWin = GetChild(*x,*y);
	if(pWin)
	{
		Engine->Graphics()->SetCursor(pWin->GetCursor());
	}
	//done
	return TRUE; 

}



ZSActionWin::ZSActionWin(ActionMenu *NewMenu, int x, int y, ZSWindow *NewParent)
{
	Casting = FALSE;
	ID = ACTION_MENU_ID;
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;
	Cursor = CURSOR_POINT;
	BackGroundSurface = NULL;
	NewParent->AddTopChild(this);
	pMenu = NewMenu;
	pActor = NULL;

	//calculate width
	int MaxWidth = 0;
	int Width = 0;

	int n;
	int NumItems;

	NumItems = NewMenu->GetNumActions();

	if(!NumItems)
	{
		State = WINDOW_STATE_DONE;
		return;
	}

	for(n = 0; n < NumItems; n++)
	{
		Width = Engine->Graphics()->GetFontEngine()->GetTextWidth(NewMenu->GetString(n));
		if(Width > MaxWidth)
		{
			MaxWidth = Width;
		}
	}
	
	MaxWidth += 8;
	int FHeight;  
	FHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight() + 4;

	Bounds.top = y;
	Bounds.left = x;
	Bounds.right = x + MaxWidth + 16;
	Bounds.bottom = y + FHeight * NumItems + 16;

	int HOff;
	int WOff;

	RECT rParentBounds;

	NewParent->GetBounds(&rParentBounds);

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
	for(n = 0; n < NumItems; n++)
	{
		if(NewMenu->GetAction(n)->GetType() == ACTION_NONE)
		{
			pWin = new ZSButton(BUTTON_NONE, n, Bounds.left + 8, 8 + Bounds.top+n*FHeight, MaxWidth, FHeight);
			pWin->SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
		}
		else
		{
			pWin = new ZSButton(BUTTON_NORMAL, n, Bounds.left + 8, 8 + Bounds.top+n*FHeight, MaxWidth, FHeight);
		}
		pWin->SetText(NewMenu->GetString(n));
		if(NewMenu->GetAction(n)->GetType() == ACTION_CAST)
		{
		   if(!NewMenu->GetAction(n)->GetData())
		   {
			   if(PreludeWorld->GetActive())
			   {
				   Creature *pCreature;
				   pCreature = (Creature *)PreludeWorld->GetActive();
					if(pCreature->GetData(INDEX_POWER_OF_FLAME).Value)
						pWin->SetCursor(CURSOR_FLAME);
					if(pCreature->GetData(INDEX_GIFTS_OF_GODDESS).Value)
						pWin->SetCursor(CURSOR_RIVER);
					if(pCreature->GetData(INDEX_THAUMATURGY).Value)
						pWin->SetCursor(CURSOR_THAUMATURGY);
			   }
		   }
           else
		   if((int)(NewMenu->GetAction(n)->GetData()) / 10 < 10)
		   {
               pWin->SetCursor(CURSOR_FLAME);
		   }
		   else
		   if((int)(NewMenu->GetAction(n)->GetData()) / 10 < 20)
		   {
               pWin->SetCursor(CURSOR_RIVER);
		   }
		   else
		       pWin->SetCursor(CURSOR_THAUMATURGY);




		}
		AddChild(pWin);
		pWin->Show();
	}

	this->CreateWoodBorderedBackground(8);

	Engine->Graphics()->SetCursor(CURSOR_POINT);
	SetFocus(this);

}

ZSActionWin::~ZSActionWin()
{
	delete pMenu;
	ZSWindow *pWin;
	pWin = pChild;
	while(pChild)
	{
		pWin = pChild;
		pChild = pChild->GetSibling();
		delete pWin;
	}
}

void ZSActionWin::CastMenu(Object *pCaster, Object *pTarget)
{
	//create a new action window
	Creature *pCreature;
	pCreature = (Creature *)pCaster;
	ActionMenu pMenu;

	pCreature->GetSpellMenu(pTarget, &pMenu);

	pMenu.AddAction(ACTION_NONE,NULL,NULL,"Abort Casting");

	ZSActionWin *pAWin;
	pAWin = new ZSActionWin(&pMenu,Bounds.left, Bounds.top, GetParent());
	pAWin->Casting = TRUE;
	pAWin->Show();
	pAWin->SetActor(pActor);
	pAWin->GoModal();
	
	GetParent()->RemoveChild(pAWin);
	

}

Creature *ZSActionWin::GetActor()
{
	return pActor;
}

void ZSActionWin::SetActor(Creature *NewCreature)
{
	pActor = NewCreature;
}



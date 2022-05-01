#include "zslistbox.h"
#include "zsverticalscroll.h"
#include "zsbutton.h"
#include "zsengine.h"
#include "zsutilities.h"

#define IDC_LIST_SCROLL 1024


ListItem::ListItem(char *NewText, int NewID)
{
	Text = new char[strlen(NewText) + 1];
	strcpy(Text,NewText);
	
	pNext = NULL;
	pPrev = NULL;

	ID = NewID;

	Selected = FALSE;
	Enabled = TRUE;
}

ListItem::~ListItem()
{
	delete Text;
	if(pPrev)
	{
		pPrev->pNext = pNext;
	}
	if(pNext)
	{
		pNext->pPrev = pPrev;
	}

}


ZSList::~ZSList()
{
	Clear();
}

void ZSList::DisableItem(int DisableID)
{
	//first find the top of the list of item
	ListItem *pLI;

	pLI = pCurTop;
	if(!pLI)
	{
		return;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}
	
	while(pLI)
	{
		if(pLI->ID == DisableID)
		{
			pLI->Enabled = FALSE;
		}
		pLI = pLI->pNext;
	}

}

void ZSList::SetSelection(int NewSelection)
{
	//first find the top of the list of item
	if(NewSelection < 0)
	{
		SelectID = NewSelection;
		return;
	}

	ListItem *pLI;
	int n = 0;
	ZSWindow *pWin;

	pLI = pCurTop;
	if(!pLI)
	{
		for(n = 0; n < NumVisible; n++)
		{
			pWin = GetChild(n);
			pWin->SetBackGround(NULL);
		}
		return;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}
	
	while(pLI)
	{
		if(pLI->ID == NewSelection)
		{
			pLI->Selected = TRUE;
		}
		else
		{
			pLI->Selected = FALSE;
		}
		pLI = pLI->pNext;
	}

	SelectID = NewSelection;

	pLI = pCurTop;
	n = 0;
	while(pLI && n <NumVisible )
	{
		pWin = GetChild(n);
		n++;
		pLI = pLI->pNext;
	}

}

void ZSList::EnableItem(int EnableID)
{
	//first find the top of the list of item
	ListItem *pLI;

	pLI = pCurTop;
	if(!pLI)
	{
		return;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}
	
	while(pLI)
	{
		if(pLI->ID == EnableID)
		{
			pLI->Enabled = TRUE;
		}
		pLI = pLI->pNext;
	}

}

void ZSList::AddItem(char *Text)
{
	//first find the top of the list of item
	ListItem *pLI, *pNewLI;

	pLI = pCurTop;

	if(pLI)
	{
		while(pLI->pPrev)
		{
			pLI = pLI->pPrev;
		}
	}

	int NumLines;
	char **NewLines;

	NumLines = Engine->Graphics()->GetFontEngine()->BreakText(Text, &NewLines, (Bounds.right - Bounds.left) - (Border * 3));

	char Blarg[128];
	
	for(int n = NumLines-1; n >= 0; n--)
	{
		if(n > 0)
			sprintf(Blarg,"  %s",NewLines[n]);	
		else
			sprintf(Blarg,"%s",NewLines[n]);	
		pNewLI = new ListItem(Blarg, NumItems);
	
		pNewLI->pNext = pLI;
		if(pLI)
		{
			pLI->pPrev = pNewLI;
		}

		pLI = pNewLI;

		NumSelectors++;
		delete[] NewLines[n];
	}
	
	delete[] NewLines;

	pCurTop = pLI;

	NumItems++;

	ZSVScroll	*pScroll;

	pScroll = (ZSVScroll *)GetChild(IDC_LIST_SCROLL);

	pScroll->SetUpper(NumSelectors - NumVisible);

	if(NumSelectors - NumVisible < 1)
	{
		pScroll->Hide();
	}
	else
	{
		pScroll->Show();
	}

}

char *ZSList::GetText(int Number, char *Dest)
{
	Dest[0] = '\0';

	ListItem *pLI;

	pLI = pCurTop;

	if(!pLI)
	{
		return NULL;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}

	char Blarg[128];

	while(pLI)
	{
		if(pLI->ID == Number)
		{
			if(strlen(Dest) > 0)
			{
				sprintf(Blarg, pLI->Text);
				strcat(Dest,&Blarg[1]);
			}
			else
			{
				strcat(Dest,pLI->Text);
			}
		}
		pLI = pLI->pNext;
	}

	return Dest;
}

void ZSList::RemoveItem(int Number)
{
	ListItem *pLI;

	pLI = pCurTop;

	if(!pLI)
	{
		return;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}

	while(pLI)
	{
		if(pLI->ID == Number)
		{
			if(pLI == pCurTop)
			{
				pCurTop = pLI->pNext;
			}

			delete pLI;

			pLI = pCurTop;

			if(pLI)
			{
				while(pLI->pPrev)
				{
					pLI = pLI->pPrev;
				}
			}
			NumSelectors--;
		}
		else
		if(pLI)
		{
			pLI = pLI->pNext;
		}
	}

	if(SelectID == Number)
	{
		SetSelection(-1);
	}

	ZSVScroll	*pScroll;

	pScroll = (ZSVScroll *)GetChild(IDC_LIST_SCROLL);

	pScroll->SetUpper(NumSelectors - NumVisible);


	if(NumSelectors - NumVisible < 1)
	{
		pScroll->Hide();
	}
	else
	{
		pScroll->Show();
	}


}

int ZSList::Command(int IDFrom, int Command, int Param)
{
	ListItem *pLI;
	int n;

	if(Command == COMMAND_BUTTON_CLICKED)
	{
		pLI = pCurTop;
		n = 0;
		while(pLI && n < IDFrom)
		{
			pLI = pLI->pNext;
			n++;
		}
		if(pLI)
		{
			if(pLI->Enabled)
			{
				if(SelectID == pLI->ID)
				{
					if(pParent)
					{
						pParent->Command(ID,COMMAND_LIST_SELECTED, pLI->ID);
					}
				}
				else
				{
					SetSelection(pLI->ID);
					if(pParent)
					{
						pParent->Command(ID,COMMAND_LIST_SELECTIONCHANGED, pLI->ID);
					}
				}
			}
		}
		
	}

	if(Command == COMMAND_BUTTON_RIGHTCLICKED)
	{
		pLI = pCurTop;
		n = 0;
		while(pLI && n < IDFrom)
		{
			pLI = pLI->pNext;
			n++;
		}
		if(pLI)
		{
			if(pParent)
			{
				pParent->Command(ID,COMMAND_LIST_RIGHTCLICKED, pLI->ID);
			}
		}
	}

	if(Command == COMMAND_SCROLL)
	{
		pLI = pCurTop;
		if(pLI)
		{
			while(pLI->pPrev)
			{
				pLI = pLI->pPrev;
			}

			n = 0;
			while(pLI && n < Param)
			{
				pLI = pLI->pNext;
				n++;
			}
			if(pLI)
				pCurTop = pLI;	
		SetSelection(SelectID);
		}
	}

	return TRUE;
}

int ZSList::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{

	return FALSE;
}

void ZSList::Clear()
{
	ListItem *pLI;

	pLI = pCurTop;

	if(!pLI)
	{
		return;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}

	while(pLI)
	{
		pCurTop = pLI->pNext;
		delete pLI;
		pLI = pCurTop;
	}
	NumItems = 0;
	SelectID = 0;
}

void ZSList::ClearExcept(char *ExceptText)
{
	ListItem *pLI;

	pLI = pCurTop;

	if(!pLI)
	{
		return;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}

	while(pLI)
	{
		pCurTop = pLI->pNext;
		delete pLI;
		pLI = pCurTop;
	}
	AddItem(ExceptText);
}


ZSList::ZSList(int NewID, int x, int y, int Width, int Height, BOOL IsParchment)
{
	Type = WINDOW_LIST;
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Bounds.left = x;
	Bounds.right = x + Width;
	Bounds.top = y;
	Bounds.bottom = y + Height;

	SelectID = -1;

	Cursor = CURSOR_POINT;
	
	Border = 8;

	Parchment = IsParchment;

	if(!Parchment)
		CreateEmptyBorderedBackground(8);
	else
		CreateParchmentBorderedBackground(8);

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	NumItems = 0;
	pCurTop = NULL;
	NumSelectors = 0;
	NumVisible = ((Bounds.bottom - Bounds.top) - Border * 2) / (Engine->Graphics()->GetFontEngine()->GetTextHeight() + 1);

	ZSWindow *pWin;
	pWin = new ZSVScroll(IDC_LIST_SCROLL, Bounds.right - (Border *2), Bounds.top, Border * 2, Bounds.bottom - Bounds.top);
	pWin->Hide();
	AddChild(pWin);

	((ZSVScroll *)pWin)->SetPage(NumVisible);
	
    int n;
	int TextHeight;
	TextHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();
	
	for(n = 0; n < NumVisible; n++)
	{
		pWin = new ZSButton(BUTTON_NONE, n, Bounds.left + Border, Bounds.top + Border + n * TextHeight, (Bounds.right- Bounds.left) - 24, TextHeight + 1);
		pWin->Show();
		pWin->SetBackGround(NULL);
		AddChild(pWin);
	}

	SetSelection(-1);
	return; 
}

int ZSList::Draw()
{
	//first standard draw
	if(Visible)
	{
		//draw our background if any
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
		}

		if(pChild)
		{
			pChild->Draw();
		}
		int mx, my;
		mx = Engine->Input()->GetMouseRect()->left; 
		my = Engine->Input()->GetMouseRect()->top; 
		ZSWindow *pWin;
		pWin = GetChild(mx, my);
		int n;
		ListItem *pLI;
		if(pWin && pWin->GetID() != IDC_LIST_SCROLL)
		{
			pLI = pCurTop;
			n = 0;
			while(pLI && n < pWin->GetID())
			{
				pLI = pLI->pNext;
				n++;
			}
			if(pLI && pLI->ID != SelectID)
			{
				if(pLI->Enabled)
				{
					SetSelection(pLI->ID);
					if(pParent)
					{
						pParent->Command(ID,COMMAND_LIST_SELECTIONCHANGED, pLI->ID);
					}
				}
			}
		}


		//now draw text
		pLI = pCurTop;
		int x,y, FHeight;
		x = Bounds.left + Border + 4;
		y = Bounds.top + Border;
			
		FHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();

		while(pLI && y + FHeight <= (Bounds.bottom - Border))
		{
			if(Parchment)
			{
				if(!pLI->Enabled)
					Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pLI->Text, TEXT_LIGHT_GREY_PARCHMENT);
				else
				if(pLI->Selected)
					Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pLI->Text, TEXT_GREEN_PARCHMENT);
				else
					Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pLI->Text, TEXT_DARK_GREY_PARCHMENT);
			}
			else
			{
				if(!pLI->Enabled)
					Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pLI->Text, TEXT_DARK_GREY);
				else
				if(pLI->Selected)
					Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pLI->Text, TextColor);
				else
					Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), x, y, pLI->Text, TextColor + 1);
			}	

			
			pLI = pLI->pNext;
			y += FHeight;
		}

		if(Text)
		{
			Engine->Graphics()->DrawText(&Bounds,Text);
		}
	}
	
	if(pSibling)
	{
		pSibling->Draw();
	}

	return TRUE;
}

int ZSList::FindItem(char *FindText)
{
	ListItem *pLI;

	pLI = pCurTop;
	if(!pLI)
	{
		return -1;
	}

	while(pLI->pPrev)
	{
		pLI = pLI->pPrev;
	}

	while(pLI)
	{
		if(!strcmp(pLI->Text, FindText))
		{
			break;
		}
		pLI = pLI->pNext;
	}

	if(pLI)
	{
		return pLI->ID;	
	}
	DEBUG_INFO("Couldn't find list item: ");
	DEBUG_INFO(FindText);
	DEBUG_INFO("\n");
	return -1;
}

void ZSList::SetBorderWidth(int NewWidth)
{
	int n;
	for(n = 0; n < NumVisible; n++)
	{
		RemoveChild(GetChild(n));
	}
	
	NumVisible = ((Bounds.bottom - Bounds.top) - Border * 2) / (Engine->Graphics()->GetFontEngine()->GetTextHeight() + 1);
	Border = NewWidth;

	int TextHeight;
	TextHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();
	ZSWindow *pWin;
	
	for(n = 0; n < NumVisible; n++)
	{
		pWin = new ZSButton(BUTTON_NONE, n, Bounds.left + Border, Bounds.top + Border + n * TextHeight, (Bounds.right- Bounds.left) - 24, TextHeight + 1);
		pWin->Show();
		pWin->SetBackGround(NULL);
		AddChild(pWin);
	}

	Border = NewWidth;

}

/*

int ZSList::MoveMouse(long *x, long *y, long *z)
{
	RECT rParBounds;
	if(pParent)
		pParent->GetBounds(&rParBounds);
	else
		ZSWindow::GetMain()->GetBounds(&rParBounds);

	//if we're moving, move
	if(State == WINDOW_STATE_MOVING)
	{
		int Width;
		int Height;
		RECT NewBounds;

		Width = Bounds.right - Bounds.left;
		Height = Bounds.bottom - Bounds.top;

		NewBounds.left = *x - XMoveOffset;
		NewBounds.right = NewBounds.left + Width;
		NewBounds.top = *y - YMoveOffset;
		NewBounds.bottom = NewBounds.top + Height;
		
		//make sure we haven't left our parent's boundaries
		if(NewBounds.left < rParBounds.left)
		{
			NewBounds.left = rParBounds.left;
			NewBounds.right = NewBounds.left + Width;
		}
		else
		if(NewBounds.right > rParBounds.right)
		{
			NewBounds.right = rParBounds.right;
			NewBounds.left = NewBounds.right - Width;
		}
		
		if(NewBounds.top < rParBounds.top)
		{
			NewBounds.top = rParBounds.top;
			NewBounds.bottom = NewBounds.top + Height;
		}
		else
		if(NewBounds.bottom > rParBounds.bottom)
		{
			NewBounds.bottom = rParBounds.bottom;
			NewBounds.top = NewBounds.bottom - Height;
		}
		Move(NewBounds);
	}

	if(pParent)
	{
		//clamp the mouse position to between our boundaries
		if(*x < rParBounds.left)
		{
			*x = rParBounds.left;
		}
		else
		if(*x > rParBounds.right)
		{
			*x = rParBounds.right;
		}

		if(*y < rParBounds.top)
		{
			*y = rParBounds.top;
		}
		else
		if(*y > rParBounds.bottom)
		{
			*y = rParBounds.bottom;
		}
	}

	ZSWindow *pWin;
	pWin = GetChild(*x, *y);
	int n;
	ListItem *pLI;
	if(pWin->GetID() != IDC_LIST_SCROLL)
	{
		pLI = pCurTop;
		n = 0;
		while(pLI && n < pWin->GetID())
		{
			pLI = pLI->pNext;
			n++;
		}
		if(pLI && pLI->ID != SelectID)
		{
			if(pLI->Enabled)
			{
				SetSelection(pLI->ID);
				if(pParent)
				{
					pParent->Command(ID,COMMAND_LIST_SELECTIONCHANGED, pLI->ID);
				}
			}
		}
	}

	return TRUE;
}

*/	
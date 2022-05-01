#include "zsaskwin.h"
#include "zslistbox.h"
#include "zsengine.h"

#define IDC_ASK_LIST	567
LPDIRECTDRAWSURFACE7 ZSAskWin::AskSurface = NULL;


int ZSAskWin::Command(int IDFrom, int Command, int Param)
{

	if(IDFrom == IDC_ASK_LIST && Command == COMMAND_LIST_SELECTED)
	{
		State = WINDOW_STATE_DONE;

		ReturnCode = Param;
	}

	return TRUE;
}

ZSAskWin::ZSAskWin(int NewID, int x, int y, int Width, int Height, ScriptArg *pOptions)
{
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.right = x + Width;
	Bounds.top = y;
	Bounds.bottom = y + Height;

	Cursor = CURSOR_POINT;

	if(!AskSurface)
	{
		CreateWoodBorderedBackground(0,1);
		AskSurface = BackGroundSurface;
	}
	BackGroundSurface = AskSurface;
	BackGroundSurface->AddRef();
	
	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	Border = 0;

	ZSList *pWin;

	pWin = new ZSList(IDC_ASK_LIST,x,y,Width,Height);
	pWin->SetTextColor(TEXT_LIGHT_PAINTED_WOOD);
	
	int n = 0;

	while(pOptions[n].GetType() != ARG_TERMINATOR)
	{
		pWin->AddItem((char *)pOptions[n].Evaluate()->GetValue());
		n++;
	}
	
	pWin->Show();

	AddChild(pWin);

#ifdef AUTOTEST
	State = WINDOW_STATE_DONE;

	ReturnCode = rand() % pWin->GetNumItems();
#endif

	return; 
}

int ZSAskWin::LeftButtonDown(int x, int y)
{
	ZSWindow *pWin;
	
	//check to see if there's child beneath the cursor who should receive the message
	pWin = GetChild(x,y);
	if(pWin)
	{
		return pWin->LeftButtonDown(x,y);
	}
	else
	{
		pWin = this->GetMain()->GetChild(x,y);
		if(pWin)
		{
			if(this->pParent == pWin)
				return pWin->LeftButtonDown(x,y);
		}
	}

	//if our cursor is the pointing hand, make the finger depress
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(1);
	}

	SetFocus(this);
	return TRUE; 
}